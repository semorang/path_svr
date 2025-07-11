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
 *  @brief  �ϳ��� ��ũ�� ���� �� �ð��븦 ����Ͽ� ��� speed�� ���Ѵ�.
 *  @note   �ӵ����� 0xff�� ���ٰ� �����Ѵ�.
 *          �� ������ �����Ϳ��� ��ó���Ǿ ������ �ȵǴ� ������..
 *          �׷��� ������..-_-?
 */
int sTrfCtrl::speed(uint8_t* spd, int trf_blk_idx, int item_idx,
	time_t timer, float link_length)
{
	/// �ӵ� ��ü�� ���� ��ũ�� error�� ó���Ѵ�.
	if (trf_blk_idx >= _tblk_real_cnt) {
		return -0xfe;
	}

  /**
   *  �� ���� ������ �ӵ��� ���ϴ� ���
   *   1. �� �ð����� �ӵ��� �״�� ���
   *   2. ���� �ð����� �ӵ��� ���� �ð����� �ӵ��� ���
   *   3. �ʴ����� ��ʽ��� �̿�
   *  �ϴ� 1�� ����� �������...
   */
  // UTC�����̹Ƿ� ���� +9�ð� �ؾ��Ѵ�.
  const int local_timer = (timer + TIMEZONE_OFFSET) % SEC_IN_DAY;
  const int tidx = local_timer / (_interval * 60);
  int pidx = (timer - _start_timer) >= _ptn_over_timer ? NEXTDAY : TODAY;
  int tmp_spd = 0;
  int error = _cache->speed((uint8_t*)&tmp_spd, _ptn_list[pidx], 
                            tidx, trf_blk_idx, item_idx);
  if (error) {
    return -0xfe;
  }

#if 1 ///< �� �ð��� �ӵ��� ���� ��ȭ ���
  const float kmh2ms           = 3.6f; ///< Km/h -> m/s
  const int   kIntervalSec     = _interval * 60;
  const int   k1stTimeValue[2] = {1, 0};  // ù��° �ӵ��� ���Ҷ� ����� ���̺�

  /// �׻� ���� ���ϴ� ������ �������̶�� �����Ѵ�.
  /// �˻� �� �������� �ƴϸ� sum�� �߰����ִ� �Ű�..
  float last_length = 0.0f;
  int   last_time = 0;

  int next_tidx = tidx + k1stTimeValue[_search_dir];
  int next_time = next_tidx * kIntervalSec;

  if (_search_dir == FORWARD_DIR) last_time = next_time   - local_timer;
  else                            last_time = local_timer - next_time;

  /// ù������ ���� �ϴ� �� ���ְ�(?)...
  last_length = ((tmp_spd * last_time) / kmh2ms);

  if (last_length < link_length) {
    const int   kMaxTimeIdx     = 1440 / _interval; ///< 1440 = 24hour * 60min
    /// forward/backward �б⸦ ��Ÿ�� ���� �Ʒ� ������ ���̺�ȭ �Ѵ�.
    /// �ι�° �ӵ��� ���Ҷ� ����� ���̺�

    const int   k2ndTimeValue[2] = {1, -1};

    float length_sum  = 0.0f; ///< ������ ������ �� ����
    float time_sum = 0.0f;    ///< ������ ������ �� �ð�

    /// BackWard�϶��� -1�ؾ� Time Index�� �´�.
    if (_search_dir == BACKWARD_DIR)
      --next_tidx;

    do {
      length_sum += last_length;
      time_sum   += last_time;

      if (next_tidx == kMaxTimeIdx) {
        next_tidx = 0;
        ++pidx;
        /// ����ڵ�...
        if (pidx == PTN_LIST_CNT) pidx = PTN_LIST_CNT - 1;
      }
      if (next_tidx == -1) {
        next_tidx = kMaxTimeIdx-1;
        --pidx;
        /// ����ڵ�...
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

    /// �ӵ��� 2�� �̻��̴ϱ� ��ȭ����� ���ؾ߰���...
    last_length = link_length - length_sum; ///<  ����ε� ������ ���� ����
    time_sum += (float)((last_length * kmh2ms) / tmp_spd);
    tmp_spd = (int)((link_length / time_sum) * kmh2ms + 0.5f);
    /// ����ڵ�...
    if (tmp_spd == 0) {
      tmp_spd = 60; ///< 60 Km ���� ����.
    }
    *spd = tmp_spd;
  } else {
    *spd = tmp_spd;
  }
#else ///< ���Խ����� �ӵ��� ���
  *spd = tmp_spd;
#endif

  return 0;
}

int sTrfCtrl::speed_block(std::vector<uint8_t>& vtblock, time_t timer)
{
	// UTC�����̹Ƿ� ���� +9�ð� �ؾ��Ѵ�.
	const int local_timer = (timer + TIMEZONE_OFFSET) % SEC_IN_DAY;
	const int tidx = local_timer / (_interval * 60);
	int pidx = (timer - _start_timer) >= _ptn_over_timer ? NEXTDAY : TODAY;

	_cache->speed_block(vtblock, _ptn_list[pidx], tidx);

	return 0;
}

/*
 *  @brief  Traffic Block�� ������ Load
 *  @note   display ������
 */
//int sTrfCtrl::speed(uint8_t* spd_blk, 
//                    int trf_blk_idx, int blk_cnt, int ptn_id, int time_idx) {
//  return _cache->speed_block(spd_blk, ptn_id, time_idx, trf_blk_idx, blk_cnt);
//}
//
//int sTrfCtrl::speed(uint8_t* spd, int ptn_id, int tidx, uint16_t bidx, uint16_t iidx) {
//  return _cache->speed(spd, ptn_id, tidx, bidx, iidx);
//}
