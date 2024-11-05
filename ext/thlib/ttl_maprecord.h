/* copyright <happyteam@thinkware.co.kr> */
#ifndef TTL_MAPRECORD_H_
#define TTL_MAPRECORD_H_

#include <vector>
//#include "thlog.h"
#include "thallocator.h"

/**
 *  @brief  TTL 통계교통정보 헤더 (Statistic Traffic Header)
 */
typedef struct __sTrfTTLHdr {
  uint8_t   _rank;
  uint8_t   _pattern_cnt;
  uint8_t   _interval;          ///< 단위:분 
  uint8_t   _eblk_size;         ///< 단위:KiB 
  uint16_t  _eblk_cnt_in_ptn;   ///< 패턴당 External Block 개수 
  uint16_t  _eblk_cnt_in_time;  ///< 시간당 External Block 개수 
  uint16_t  _tblk_real_cnt;     ///< 시간당 실제 Traffic Block 개수 
  uint16_t  _tblk_max_cnt;      ///< 시간당 최대 Traffic Block 개수
  uint32_t  _item_cnt_in_tblk;  ///< Traffic 블럭 당 Item(속도정보) 개수 
  uint32_t  _eblk_total_cnt;    ///< External Block 전체 개수
  uint32_t  _eblk_tbl_ds;       ///< External Block Table의 DS
  uint32_t  _speed_blk_ds;      ///< 속도 데이터 DS
  /// 패턴 정보 Table DS는 sizeof(sTrfTTLHdr)
} sTrfTTLHdr;

/**
 * TTL Index Item(Rank 0, 1, 2)
 */
class thTTLIndexItem {
public:
  /// block index가 없다면 1023값.
  enum INVALID_VALUE { BLOCK_MAX = 1023 };
public:
  thTTLIndexItem() {
    _data = 0;
    _length = 0;
  }
  void init() {
    _data = 0;
    _length = 0;
  }
  uint32_t get_block_index()      { return (0x3ff80000 & _data) >> 19; }
  uint32_t get_block_item_index() { return (0x0007ff00 & _data) >> 8; }
  uint32_t get_speed()            { return (0x000000ff & _data); }
  uint32_t get_length()           { return _length; }

public:
  ///< 2(reserved) / 11(block idx) / 11(in idx) / 8(default speed 0: 추천
  uint32_t  _data;
  uint32_t  _length;
};

/**
 * TTL Index List (Rank2)
 */
class thTTLIndexL {
 public: 
  int   get_fw_block_idx() { return _fw_idx.get_block_index(); }
  int   get_bw_block_idx() { return _bw_idx.get_block_index(); }

  int   get_fw_item_idx() { return _fw_idx.get_block_item_index(); }
  int   get_bw_item_idx() { return _bw_idx.get_block_item_index(); }

  uint8_t get_speed(int dir, int opt = 0) {
    if (dir == FORWARD_DIR) {
      return _fw_idx.get_speed();
    } else {
      return _bw_idx.get_speed();
    }
  }
 public:
  thTTLIndexItem    _fw_idx;
  thTTLIndexItem    _bw_idx;
};

/**
 * TTL Index List (Rank 0, 1)
 */
class thTTLIndexU {
 public:
  thTTLIndexU(): _lindex(0) {
    _lindex = 0;
    _fw_cnt = _bw_cnt = 0;
    memset(_reserved, 0x00, sizeof(_reserved));
  }
  
 public:
  unsigned int  _lindex;      ///< 시작부터 링크위치 까지 인덱스

  uint8_t _fw_cnt;
  uint8_t _bw_cnt;
                              ///< 10bit bw_block_idx / 10bit bw_item_idx
  char          _reserved[2]; 
};

/**
 * link match record (rank 0, 1)
 */
class thTTLLinkMatchRec {
 public:
  uint32_t get_block_index()  { return _idx.get_block_index(); }
  uint32_t get_item_index()   { return _idx.get_block_item_index(); }
  int get_length()            { return _idx.get_length(); }
  
  uint8_t get_speed(int opt = 0) { return _idx.get_speed(); }

 public:
  thTTLIndexItem  _idx;
};


/**
 * 하판 교통정보 ID Block
 */
class thTTLIDBlock {
 public:
  enum SIZE { BLOCK_ITEM_CNT = 2048 };
 public:
  uint32_t size() { return BLOCK_ITEM_CNT; }
 public:
  unsigned long long  _id[BLOCK_ITEM_CNT];      ///< TTL ID
};

typedef std::vector<thTTLIDBlock, thallocator<thTTLIDBlock> >   thTTLIDBlVec;
typedef std::vector<thTTLIndexItem, thallocator<thTTLIndexItem> > thTTLIdxItemVec;

#endif
