/* copyright <happyteam@thinkware.co.kr> */
#ifndef STI_MAPRECORD_H_
#define STI_MAPRECORD_H_

#include <vector>
//#include "thlog.h"
#include "thallocator.h"

/* link direction */
#define FORWARD_DIR            (0)
#define BACKWARD_DIR          (1)
///////////////////////////////////////////////////////////////////////////////
// ��� �������� (Statistic Traffic)
///////////////////////////////////////////////////////////////////////////////
/**
 *  @brief  Kind of Pattern
 */
enum __PATTERN_TYPE {
  PTN_NORMAL = 0,         ///< ����
  PTN_FRIDAY_EFFECT = 1,  ///< ���� ����
  PTN_HOLIDAY = 2,        ///< ����
  PTN_COUNT               ///< ���� ����
};

class CQ {
  int  _qsize;
  int  _size;
  int  _front;
  int  _rear;
  int* _buf;
 public :
  CQ() : _qsize(0), _size(0), _front(0), _rear(0), _buf(NULL) {}
  ~CQ() {
    if (_buf != NULL) {
      thlib::MemPolicy<int>::memfree(_buf);
      _size = 0;
    }
  }
  void clear() {
    _size = _front = _rear = 0;
  }
  int init(size_t cache_size, size_t block_size) {
    _qsize = (int)(cache_size / block_size);
    if (_qsize <= 0)
      return -0xfe;
    _buf = thlib::MemPolicy<int>::alloc_zero(_qsize);
    if (_buf == NULL) return -EMEM_ALLOC;
    clear();
    return 0;
  }
  int push(int index) {
    int ret = -1;
    if (_qsize == _size) {
      ret = _buf[_front];
      _front = (_front + 1) % _qsize;
      _buf[_rear] = index;
      _rear = (_rear + 1) % _qsize;
    } else { ///< ������ �ִٸ�
      _buf[_rear] = index;
      _rear = (_rear + 1) % _qsize;
      ++_size;
    }
    return ret;
  }
  int size() {
    return _size;
  }
};

/**
 *  @brief  ��豳������ ��� (Statistic Traffic Header)
 */
typedef struct __sTrfHdr {
  uint8_t   _rank;
  uint8_t   _pattern_cnt;
  uint8_t   _interval;            ///< ����:�� 
  uint8_t   _eblk_size;           ///< ����:KiB 

  uint16_t  _eblk_total_cnt;      ///< External Block ��ü ���� 
  uint16_t  _eblk_cnt_in_ptn;     ///< ���ϴ� External Block ���� 
  uint16_t  _eblk_cnt_in_time;    ///< �ð��� External Block ���� 

  uint16_t  _tblk_real_cnt;       ///< �ð��� ���� Traffic Block ���� 
  uint16_t  _tblk_max_cnt;        ///< �ð��� �ִ� Traffic Block ���� 

  uint16_t  _item_cnt_in_tblk;    ///< Traffic �� �� Item(�ӵ�����) ���� 

  uint32_t  _eblk_tbl_ds;   ///< External Block Table�� DS
  uint32_t  _speed_blk_ds;  ///< �ӵ� ������ DS 
  /// ���� ���� Table DS�� sizeof(sTrfHdr)
} sTrfHdr;

/**
 *  @brief  External ��
 */
typedef uint32_t  ExtBlockDS;  ///< ����� External Block�� DS

/**
 *  @brief  �ӵ�
 */
//typedef uint8_t  Speed;  ///< ����� External Block�� DS



///////////////////////////////////////////////////////////////////////////////
// traffic map record (�ǽð�)
///////////////////////////////////////////////////////////////////////////////
/**
 * �������� Index Item(Rank 0, 1, 2)
 */ 
class thTrIndexItem {
public:
  /// block index�� ���ٸ� 1023��.
  enum INVALID_VALUE { BLOCK_MAX = 1023 };
public:
  thTrIndexItem() {
    _data = 0;
    _length = 0;
  }
  void init() {
    _data = 0;
    _length = 0;
  }
  uint32_t get_block_index()      { return (0x0ffc0000 & _data) >> 18; }
  uint32_t get_block_item_index() { return (0x0003ff00 & _data) >> 8; }
  uint32_t get_speed()            { return (0x000000ff & _data); }
  uint32_t get_length()           { return _length; }

public:
  ///< 4(reserved) / 10(block idx) / 10(in idx) / 8(default speed 0: ��õ
  uint32_t  _data;
  uint32_t  _length;
};

/**
 * Traffic Index List (Rank2)
 */
class thTrIndexL {
 public: 
  int get_fw_block_idx() { return _fw_idx.get_block_index(); }
  int get_bw_block_idx() { return _bw_idx.get_block_index(); }
  int get_fw_item_idx() { return _fw_idx.get_block_item_index(); }
  int get_bw_item_idx() { return _bw_idx.get_block_item_index(); }  

  uint8_t get_speed(int dir, int opt = 0) {
    if (dir == FORWARD_DIR) {
      return _fw_idx.get_speed();
    } else {
      return _bw_idx.get_speed();
    }
  }
 public:
  thTrIndexItem    _fw_idx;
  thTrIndexItem    _bw_idx;
};

/**
 * Traffic Index List (Rank 0, 1)
 */
class thTrIndexU {
 public:
  thTrIndexU(): _lindex(0) {
    _lindex = 0;
    //_fw_cnt = _bw_cnt = 0;
    memset(_reserved, 0x00, sizeof(_reserved));
  }
 public:
  unsigned int  _lindex;      ///< ���ۺ��� ��ũ��ġ ���� �ε���

  //uint8_t _fw_cnt;
  //uint8_t _bw_cnt;

  char          _reserved[2]; 
}; 

/**
 * link match record (rank 0, 1)
 */
class thTrLinkMatchRec {
 public:
  uint32_t get_block_index()  { return _idx.get_block_index(); }
  uint32_t get_item_index()   { return _idx.get_block_item_index(); }
  int get_length()            { return _idx.get_length(); }
  
  uint8_t get_speed(int opt = 0) { return _idx.get_speed(); }

 public:
  thTrIndexItem  _idx;
};

/**
 * ���� ���� �� ���� �ڵ� �ε��� (�뷫 200�� �̻�)
 * Rank2 ������
 */
class thTrRegionCodeInfo {
 public:
  thTrRegionCodeInfo() {
    clear();
  }
  void clear() {
    _is_broad = 0;
    _in_highway = _in_national_highway = _in_normalway = 0;
    _rcode_min = _rcode_max = -1;
  }
 public:
  char        _is_broad;                ///< ���⿩��.
  char        _in_highway;              ///< ��ӵ��� ���Կ���.
  char        _in_national_highway;     ///< ���� ���Կ���.
  char        _in_normalway;            ///< �Ϲݵ��� ���Կ���.
  int16_t       _rcode_min;               ///< �����ڵ� �ּҰ�.
  int16_t       _rcode_max;               ///< �����ڵ� �ִ밪.
};

/**
 * ���� �������� ID Block
 */
class thTrIDBlock {
 public:
  enum SIZE { BLOCK_ITEM_CNT = 1024 };
 public:
  uint32_t size() { return BLOCK_ITEM_CNT; }
 public:
  unsigned int        _id[BLOCK_ITEM_CNT];      ///< �������� Real ID
};

typedef std::vector<thTrRegionCodeInfo, thallocator<thTrRegionCodeInfo> >     
                                                            thTrRegionInfoVec;
typedef std::vector<thTrIDBlock, thallocator<thTrIDBlock> >     thTrIDBlVec;
typedef std::vector<thTrIndexItem, thallocator<thTrIndexItem> >          
                                                               thTrIdxItemVec;

/**
 * CTT SUM Info
 */
class thCTTSUMInfoRec {
 public:
  thCTTSUMInfoRec() {
    clear();
  }
  void clear() {
    _sumid = 0;
    _match_cnt = 0;
    _match_tbl_idx = 0xffff; 
  }

 public:
  unsigned int   _sumid;
  uint16_t _match_cnt;
  uint16_t _match_tbl_idx;
};

/**
 * CTT SUM Match Record
 */
class thCSMatchRec {
 public:
  thCSMatchRec() {
    clear();
  }
  void clear() {
    _block_idx = thTrIndexItem::BLOCK_MAX;
    _item_idx = thTrIndexItem::BLOCK_MAX;
  }
 public:
  uint16_t _block_idx;
  uint16_t _item_idx;
};

//typedef std::vector<thCTTSUMInfoRec>   CTTSUMInfoVec;  
//typedef std::vector<thCSMatchRec>      CSMatchVec;

class thYtnMr2KsHeader {
public:
  thYtnMr2KsHeader() : _ks_count(0), _mr_count(0) {}
public:
  uint32_t  _ks_count;
  uint32_t  _mr_count;
  uint32_t  _reserved[8];
};

class thYtnKsInfo {
public:
  thYtnKsInfo() : _ksid(0), _len(0), _level(0xff) {}
public:
  uint32_t  _ksid;
  uint32_t  _len;
  uint8_t   _level;
  uint8_t   _reserved[3];
};

class thYtnMrInfo {
public:
  thYtnMrInfo() : _mapid(0xffff), _linkid(0xffff), _dir(0xffff), _ksid(0) {}
public:
  uint16_t  _mapid;
  uint16_t  _linkid;
  uint16_t  _dir;
  uint8_t   _reserved[2];
  uint32_t  _ksid;
};

#endif
