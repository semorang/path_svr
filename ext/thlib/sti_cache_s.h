/* copyright <happyteam@thinkware.co.kr> */
#ifndef  STI_CACHE_S_H_
#define  STI_CACHE_S_H_

#include "sti_maprecord.h"
#include "ttl_maprecord.h"
#include "sti_reader_s.h"

//#include "thmemalloc.h"
#include "thstring.h"
#include "thfile.h"

struct TTLSPD
{
	uint64_t ttlid = 0;
	uint8_t spd = 0xFF;
};


class sTrfCache {
public :
  enum __STRF_TYPE {
    STRF_TYPE_KS  = 0,  // KS 기반
    STRF_TYPE_TTL = 1   // TTL 기반
  };
  enum __CACHE_TARGET {
    CACHE_RP  = 0, // 탐색
    CACHE_DP  = 1, // Display
    CACHE_TTL = 2, // TTL
    CACHE_CNT
  };
private :
  static sTrfCache* _instance[CACHE_CNT];
  thlib::thString   _mappath;
  CQ          _cq;
  sTrfReader  _reader;
  sTrfHdr     _header;
  sTrfTTLHdr  _ttl_header;
  ExtBlockDS* _eblk_ds_tbl; // External Block의 DS Table
  uint8_t*    _tmp_buf;     // 압축해제를 위한 임시 버퍼
  uint8_t**   _cache_tbl;   // speed 블럭의 주소 Table
  uint8_t**   _partial_tbl; // 통계부분업데이트용 임시 Table
  int         _type;
  int         _pattern_cnt;
  int         _interval;
  int         _tblk_real_cnt;
  int         _eblk_ksize;    // 단위:KiB
  int         _eblk_bsize;    // 단위:byte
  int         _eblk_total_cnt;
  int         _eblk_cnt_in_ptn;
  int         _eblk_cnt_in_time;
  int         _eblk_tbl_ds;
  int         _item_cnt_in_tblk;
  int         _last_blk_idx;  // 마지막 블럭의 index
  int         _last_blk_size; // 마지막 블럭의 크기
  int         _is_init;
  size_t  compressed_block_size(int idx);
  int     load_block(int idx);
  void    unload_block(int idx);
  int     external_block_idx(int ptn_id, int time_idx, int trf_blk_idx);
  int     traffic_item_idx(int eblk_idx, int trf_blk_idx, int item_idx);
  static void destroy_bridge();

public :
  sTrfCache();
  ~sTrfCache();
  static sTrfCache* instance(int target);
  int     init(const char* map_path, size_t cache_size, int type);
  int     speed(uint8_t* spd, int ptn_id, int time_idx, 
                int trf_blk_idx, int item_idx);
  int speed_block(std::vector<uint8_t>& vtblock, int ptn_id, int time_idx);
  //int     speed_block(uint8_t* spd_blk, int ptn_id, 
  //                    int time_idx, int trf_blk_idx, int blk_cnt);
  int     interval();
  int     real_cnt();
  //int     pattern_cnt();
  void    release();
  void    destroy();
  //void    destroy_partial();
  size_t  size();
  size_t  strf_filesize();

  //int     load_partial_pattern(int ptn);
  //int     replace_pattern(int ptn);
};

inline int sTrfCache::interval() {
  return _interval;
}

inline int sTrfCache::real_cnt() {
  return _tblk_real_cnt;
}

//inline int sTrfCache::pattern_cnt() {
//  return _pattern_cnt;
//}

/**
 *   @brief  External 블럭의 index를 구한다.
 */
inline int sTrfCache::external_block_idx(int ptn_id, int time_idx, 
                                         int trf_blk_idx) {
  return (ptn_id   * _eblk_cnt_in_ptn)  + ///< 패턴 단위
         (time_idx * _eblk_cnt_in_time) + ///< 시간 단위
         ((int)(trf_blk_idx / _eblk_ksize));
}

/**
 *   @brief  item의 index를 구한다.
 */
inline int sTrfCache::traffic_item_idx(int eblk_idx, int trf_blk_idx, 
                                       int item_idx) {
  (void) eblk_idx; /* unused value */
  return (_item_cnt_in_tblk * ///< 1024
           ///< 외부 index
          (trf_blk_idx - (((int)(trf_blk_idx/_eblk_ksize))*_eblk_ksize))) +
         item_idx; ///< 내부 index
}

/**
 *   @brief  item의 index를 구한다.
 */
inline size_t sTrfCache::size() {
  return 
  /// ds table size
  (sizeof(ExtBlockDS) * _eblk_total_cnt) + 
  /// tmp block size
  _eblk_bsize + 
  /// cache table size
  (sizeof(uint8_t*) * _eblk_total_cnt) + 
  /// real used cache size
  (_cq.size() * _eblk_bsize);
}

#endif // STI_CACHE_S_H_

