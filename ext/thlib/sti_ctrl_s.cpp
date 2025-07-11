/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "sti_ctrl_s.h"

// exception_jump
// 2024 07 02
#include <setjmp.h>
jmp_buf exception_jump;

/**
 *  @brief  init
 */
int sTrfCtrl::init(const char* map_path, int cache_size, 
                   int cache_target, int type, time_t cur_timer) {
  int error;
  _type = type;
  if ((_cache = sTrfCache::instance(cache_target)) == NULL) return -EMEM_ALLOC;
  if ((error = _cache->init(map_path, cache_size, type))) return error;

  _holiday_mng = thlib::thHolidayManager::instance();
  if ((error = _holiday_mng->init(map_path))) return error;

  if (_cache->strf_filesize() < 30 * 1024 * 1024) {
    _holiday_mng->use_old_pattern(1);
  }

  _interval = _cache->interval();
  _tblk_real_cnt = _cache->real_cnt();

  _start_timer = cur_timer;
  _ptn_over_timer = SEC_IN_DAY - ((_start_timer + TIMEZONE_OFFSET) % SEC_IN_DAY);
  _ptn_list[TODAY]   = _holiday_mng->pattern(cur_timer);
  _ptn_list[NEXTDAY] = _holiday_mng->pattern(cur_timer + SEC_IN_DAY);

  return 0;
}

/*
 *  @brief  하나의 링크에 대해 각 시간대를 고려하여 평균 speed를 구한다.
 *  @note   속도값에 0xff는 없다고 가정한다.
 *          이 값들은 컨버터에서 전처리되어서 나오믄 안되는 값들임..
 *          그러나 나오면..-_-?
 */
int sTrfCtrl::speed(uint8_t* spd, int trf_blk_idx, int item_idx,
	time_t timer, float link_length)
{
	/// 속도 자체가 없는 링크는 error로 처리한다.
	if (trf_blk_idx >= _tblk_real_cnt) {
		return -0xfe;
	}

  /**
   *  ※ 진입 시점의 속도를 구하는 방법
   *   1. 그 시간대의 속도값 그대로 사용
   *   2. 현재 시간대의 속도와 다음 시간대의 속도를 평균
   *   3. 초단위의 비례식을 이용
   *  일단 1번 방법을 사용하자...
   */
  // UTC기준이므로 필히 +9시간 해야한다.
  const int local_timer = (timer + TIMEZONE_OFFSET) % SEC_IN_DAY;
  const int tidx = local_timer / (_interval * 60);
  int pidx = (timer - _start_timer) >= _ptn_over_timer ? NEXTDAY : TODAY;
  int tmp_spd = 0;
  int error = _cache->speed((uint8_t*)&tmp_spd, _ptn_list[pidx], 
                            tidx, trf_blk_idx, item_idx);
  if (error) {
    return -0xfe;
  }

#if 1 ///< 각 시간별 속도에 대한 조화 평균
  const float kmh2ms           = 3.6f; ///< Km/h -> m/s
  const int   kIntervalSec     = _interval * 60;
  const int   k1stTimeValue[2] = {1, 0};  // 첫번째 속도를 구할때 사용할 테이블

  /// 항상 현재 구하는 값들이 마지막이라고 생각한다.
  /// 검사 후 마지막이 아니면 sum에 추가해주는 거고..
  float last_length = 0.0f;
  int   last_time = 0;

  int next_tidx = tidx + k1stTimeValue[_search_dir];
  int next_time = next_tidx * kIntervalSec;

  if (_search_dir == FORWARD_DIR) last_time = next_time   - local_timer;
  else                            last_time = local_timer - next_time;

  /// 첫구간에 대해 일단 함 해주고(?)...
  last_length = ((tmp_spd * last_time) / kmh2ms);

  if (last_length < link_length) {
    const int   kMaxTimeIdx     = 1440 / _interval; ///< 1440 = 24hour * 60min
    /// forward/backward 분기를 안타기 위해 아래 값들을 테이블화 한다.
    /// 두번째 속도를 구할때 사용할 테이블

    const int   k2ndTimeValue[2] = {1, -1};

    float length_sum  = 0.0f; ///< 마지막 구간을 뺀 길이
    float time_sum = 0.0f;    ///< 마지막 구간을 뺀 시간

    /// BackWard일때는 -1해야 Time Index가 맞다.
    if (_search_dir == BACKWARD_DIR)
      --next_tidx;

    do {
      length_sum += last_length;
      time_sum   += last_time;

      if (next_tidx == kMaxTimeIdx) {
        next_tidx = 0;
        ++pidx;
        /// 방어코드...
        if (pidx == PTN_LIST_CNT) pidx = PTN_LIST_CNT - 1;
      }
      if (next_tidx == -1) {
        next_tidx = kMaxTimeIdx-1;
        --pidx;
        /// 방어코드...
        if (pidx < 0) pidx = 0;
      }

      error = _cache->speed((uint8_t*)&tmp_spd, _ptn_list[pidx], 
                            next_tidx, trf_blk_idx, item_idx);
      if (error) {
        return -0xfe;
      }

      last_length = ((tmp_spd * kIntervalSec) / kmh2ms);
      last_time   = kIntervalSec;
      next_tidx += k2ndTimeValue[_search_dir];
    } while ((length_sum + last_length) < link_length);

    /// 속도가 2개 이상이니까 조화평균을 구해야겠지...
    last_length = link_length - length_sum; ///<  지대로된 마지막 구간 길이
    time_sum += (float)((last_length * kmh2ms) / tmp_spd);
    tmp_spd = (int)((link_length / time_sum) * kmh2ms + 0.5f);
    /// 방어코드...
    if (tmp_spd == 0) {
      tmp_spd = 60; ///< 60 Km 강제 설정.
    }
    *spd = tmp_spd;
  } else {
    *spd = tmp_spd;
  }
#else ///< 진입시점의 속도만 사용
  *spd = tmp_spd;
#endif

  return 0;
}

int sTrfCtrl::speed_block(std::vector<uint8_t>& vtblock, time_t timer)
{
	// UTC기준이므로 필히 +9시간 해야한다.
	const int local_timer = (timer + TIMEZONE_OFFSET) % SEC_IN_DAY;
	const int tidx = local_timer / (_interval * 60);
	int pidx = (timer - _start_timer) >= _ptn_over_timer ? NEXTDAY : TODAY;

	_cache->speed_block(vtblock, _ptn_list[pidx], tidx);

	return 0;
}

/*
 *  @brief  Traffic Block을 통으로 Load
 *  @note   display 공유용
 */
//int sTrfCtrl::speed(uint8_t* spd_blk, 
//                    int trf_blk_idx, int blk_cnt, int ptn_id, int time_idx) {
//  return _cache->speed_block(spd_blk, ptn_id, time_idx, trf_blk_idx, blk_cnt);
//}
//
//int sTrfCtrl::speed(uint8_t* spd, int ptn_id, int tidx, uint16_t bidx, uint16_t iidx) {
//  return _cache->speed(spd, ptn_id, tidx, bidx, iidx);
//}
