/* copyright <happyteam@thinkware.co.kr> */
#ifndef  STI_CTRL_S_H_
#define  STI_CTRL_S_H_

#include "thholiday.h"
//#include "thtime.h"
#include "sti_cache_s.h"

/**
 *  @brief  통계교통 정보 전체를 관리하는 컨트롤러
 */
class sTrfCtrl
{
 public :
  enum __DEFINE {
    SEC_IN_DAY      = 86400, // 1day sec
    TIMEZONE_OFFSET = 32400, // +9시간의 sec
    PTN_LIST_CNT    = 2,
    TODAY           = 0,     // Pattern index
    NEXTDAY         = 1,
    INTERVAL        = 15,     // Stats time interval
    INTERVAL_SEC    = 900,    // 15 * 60sec
    CNT_IN_24H      = 96      // 24hour / interval(15)
  };

  thlib::thHolidayManager* _holiday_mng;
  sTrfCache*  _cache;
  int         _type;
  int         _ptn_list[PTN_LIST_CNT];
  int         _ptn_over_timer;
  int         _start_timer;  ///< 시작시간(UTC)
  int         _search_dir;
  /////////////////////////////////////////////////////////
  // for performance
  int         _interval;
  int         _tblk_real_cnt;

/////////////////////////////////////////////////////////////////////////
 public :
  sTrfCtrl() : _holiday_mng(NULL), _cache(NULL), _type(sTrfCache::STRF_TYPE_KS),
               _ptn_over_timer(0), _start_timer(0), _search_dir(0),
               _interval(0), _tblk_real_cnt(0) {}
  ~sTrfCtrl() {}
  int init(const char* map_path, int cache_size, 
           int cache_target, int type, time_t cur_timer = 0);
  int speed(uint8_t* spd, int trf_blk_idx, int itm_idx, time_t timer, 
            float link_length);
  int speed_block(std::vector<uint8_t>& vtblock, time_t timer);

  //int speed(uint8_t* spd_blk, 
  //          int trf_blk_idx, int blk_cnt, int ptn_id, int time_idx);
  //int speed(uint8_t* spd, int ptn_id, int tidx, uint16_t bidx, uint16_t iidx);

  /**
   *  @brief  헤더정보 중 Traffic 블럭의 실제 개수를 Return.
   */
  //int traffic_block_count() {
  //  return _tblk_real_cnt;
  //}

  /**
   *  @brief  탐색 방향(forward / backward)
   */
  //void search_direction(int dir) {
  //  _search_dir = dir;
  //}
  /**
   *  @brief  cache release
   */
  void release() {
    if (_cache != NULL) {
      _cache->release();
    }
  }

  /**
   *  @note DP용
   */
  //int time_index(time_t timer) {
  //  return ((timer + TIMEZONE_OFFSET) % SEC_IN_DAY) / (_interval * 60);
  //}
  //int pattern_id(time_t timer) {
  //  int ptn_id = _holiday_mng->pattern(timer);
  //  if (ptn_id < 0 || ptn_id >= PTN_COUNT) return -0xfe;
  //  return ptn_id;
  //}

  size_t cache_size() {
    if (_cache != NULL) {
      return _cache->size();
    } else {
      return 0;
    }
  }
};

#endif // STI_CTRL_S_H_

