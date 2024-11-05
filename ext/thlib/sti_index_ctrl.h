/* copyright 2017 <happyteam@thinkware.co.kr> */
#ifndef _STI_INDEX_CTRL_H__
#define _STI_INDEX_CTRL_H__

//#include "sti_mapinfo_r.h"
//#include "ttl_mapinfo_r.h"

#include "sti_maprecord.h"
#include "ttl_maprecord.h"
#include "shw_mapinfol.h"
#include "thlock.h"
#include "thfile.h"
#include "mapInfo.h"

#if !defined(WIN32)
  #include <linux/limits.h>
#endif

/****************************************************
 * TTL index data
 ****************************************************/
// trfidx_ttl2.bin
class trfTTLLowerIndex {
public:
  uint32_t get_block_index()      { return (0x3ff80000 & _data) >> 19; }
  uint32_t get_block_item_index() { return (0x0007ff00 & _data) >> 8; }
  uint32_t get_speed()            { return (0x000000ff & _data); }

  uint32_t _data;
};

class trfTTLUpperIndexItem {
public:
  uint32_t get_block_index()      { return (0x3ff80000 & _data) >> 19; }
  uint32_t get_block_item_index() { return (0x0007ff00 & _data) >> 8; }
  uint32_t get_speed()            { return (0x000000ff & _data); }
  uint32_t get_length()           { return _length; }

  uint32_t _data;
  uint32_t _length;
};

// trfidx_ttl0.bin, trf_ttl0.bin, trfidx_ttl1.bin, trf_ttl1.bin
class trfTTLUpperIndex {
public:
  //trfTTLUpperIndex() : _fw_cnt(0), _bw_cnt(0), _fw_item(NULL), _bw_item(NULL) {}
  ~trfTTLUpperIndex() {
    //destroy();
  }
  //void destroy() {
  //  if (_fw_item != NULL) {
  //    thlib::MemPolicyNew<trfTTLUpperIndexItem>::memfree(_fw_item);
  //    _fw_cnt = 0;
  //  }
  //  if (_bw_item != NULL) {
  //    thlib::MemPolicyNew<trfTTLUpperIndexItem>::memfree(_bw_item);
  //    _bw_cnt = 0;
  //  }
  //}
public:
  //uint16_t             _fw_cnt;
  //uint16_t             _bw_cnt;
  //trfTTLUpperIndexItem *_fw_item;
  //trfTTLUpperIndexItem *_bw_item;
};

/****************************************************
 * class stiIndexCtrl
 ****************************************************/
class stiIndexCtrl
{
private:
  stiIndexCtrl(void)
  : _is_init(0), _is_ttl_init(0), _idx_buf_rank2(NULL), //_idx_buf_rank1(NULL), _tbl_buf_rank1(NULL),
    //_idx_buf_rank0(NULL), _tbl_buf_rank0(NULL), 
	  _ttl_idx_buf_rank2(NULL)//,
  //  _ttl_idx_buf_rank1(NULL), _ttl_tbl_buf_rank1(NULL), _ttl_idx_buf_rank0(NULL), _ttl_tbl_buf_rank0(NULL) 
  {
    for (int i = 0; i < MapCntlL::MRRANK_COUNT; ++i) {
      _link_cnt[i] = NULL;
      _ttl_link_cnt[i] = NULL;
    }
  }
  ~stiIndexCtrl(void) {
    destroy();
  }

public:
  static stiIndexCtrl* instance() {
    if (_instance == NULL) {
      _instance = thlib::MemPolicy<stiIndexCtrl>::alloc_zero(1);
      atexit(destroy_bridge);
    }
    return _instance;
  }
  static void destroy_bridge() {
    if (_instance != NULL) {
      _instance->destroy();
      thlib::MemPolicy<stiIndexCtrl>::memfree(_instance);
      _instance = NULL;
    }
  }

  void destroy();

  int init(const char* mappath);

  // KS
  thTrIndexItem* get_item(int mapid, int linkid, int dir);
  void get_index(uint16_t &bidx, uint16_t &iidx, int mapid, int linkid, int dir);
  int get_block_index(int mapid, int linkid, int dir);
  int get_block_item_index(int mapid, int linkid, int dir);

  //int get_match_count(int mapid, int linkid, int dir, int rank);
  //thTrIndexItem* get_match_index(int mapid, int linkid, int dir, int rank);

  // TTL
  thTTLIndexItem* get_ttl_item(int mapid, int linkid, int dir);
  void get_ttl_index(uint16_t &bidx, uint16_t &iidx, int mapid, int linkid, int dir);
  int get_ttl_block_index(int mapid, int linkid, int dir);
  int get_ttl_block_item_index(int mapid, int linkid, int dir);

  //int get_ttl_match_count(int mapid, int linkid, int dir, int rank);
  //thTTLIndexItem* get_ttl_match_index(int mapid, int linkid, int dir, int rank);

  //int is_ttl_init() {
  //  return _is_ttl_init;
  //}

private:
  int init_index_rank2();
//  int init_index_rank1();
//  int init_index_rank0();
  int init_ttl_index_rank2();
//  int init_ttl_index_rank1();
//  int init_ttl_index_rank0();

private:
  static stiIndexCtrl *_instance;

  thlib::thLock     _init_lock;
  int               _is_init;
  int               _is_ttl_init;

#if !defined(WIN32)
  char              _map_path[PATH_MAX];
#else
  char              _map_path[MAX_PATH];
#endif
  
  int               _map_cnt[MapCntlL::MRRANK_COUNT];
  int               *_link_cnt[MapCntlL::MRRANK_COUNT];
  int               *_ttl_link_cnt[MapCntlL::MRRANK_COUNT];

  char              *_idx_buf_rank2;
  //char              *_idx_buf_rank1;
  //char              *_tbl_buf_rank1;
  //char              *_idx_buf_rank0;
  //char              *_tbl_buf_rank0;

  char              *_ttl_idx_buf_rank2;
  //char              *_ttl_idx_buf_rank1;
  //char              *_ttl_tbl_buf_rank1;
  //char              *_ttl_idx_buf_rank0;
  //char              *_ttl_tbl_buf_rank0;
};

#endif
