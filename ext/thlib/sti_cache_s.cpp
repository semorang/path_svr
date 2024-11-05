/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "sti_cache_s.h"

#if defined(_WIN32_WCE) || defined(WIN32)
//#include "zlib.h"
#include "ZLIB.H"
#else
//#include <zlib.h>
#include "ZLIB.H"
#endif

sTrfCache* sTrfCache::_instance[CACHE_CNT] = { NULL, NULL, NULL };

sTrfCache::sTrfCache() : _eblk_ds_tbl(NULL), _tmp_buf(NULL), _cache_tbl(NULL),// _partial_tbl(NULL),
                         _eblk_ksize(0), _eblk_bsize(0), 
                         _last_blk_idx(0), _last_blk_size(0), _is_init(0) {}

sTrfCache::~sTrfCache() {}

sTrfCache* sTrfCache::instance(int target) {
  if (_instance[target] == NULL) {
    _instance[target] = thlib::MemPolicyNew<sTrfCache>::alloc(1);
    atexit(destroy_bridge);
  }
  return _instance[target];
}

void sTrfCache::destroy_bridge() {
  for (int i = 0; i < CACHE_CNT; ++i) {
    if (sTrfCache::_instance[i] != NULL) {
      sTrfCache::_instance[i]->destroy();
      thlib::MemPolicyNew<sTrfCache>::memfree(sTrfCache::_instance[i]);
      sTrfCache::_instance[i] = NULL;
    }
  }
}

/**
 *  @brief  release
 */
void sTrfCache::release() {
#if defined(WIN32) || defined(_WIN32_WCE) || defined(ANDROID) || defined(_LINUX_PND_)
  for (int i = 0; i < (int)_eblk_total_cnt; ++i) {
    if (_cache_tbl[i] != NULL) {
      thlib::MemPolicy<uint8_t>::memfree(_cache_tbl[i]);
      _cache_tbl[i] = NULL;
    }
  }
  /// Q도 클리어...
  _cq.clear();
#endif
}

/**
 *  @brief  release partial table
 */
//void sTrfCache::destroy_partial() {
//  for (int i = 0; i < (int)_eblk_cnt_in_ptn; ++i) {
//    if (_partial_tbl[i] != NULL) {
//      thlib::MemPolicy<uint8_t>::memfree(_partial_tbl[i]);
//      _partial_tbl[i] = NULL;
//    }
//  }
//  thlib::MemPolicy<uint8_t*>::memfree(_partial_tbl);
//  _partial_tbl = NULL;
//}

/**
 *  @brief  destroy
 */
void sTrfCache::destroy() {
  if (_cache_tbl != NULL) {
    for (int i = 0; i < (int)_eblk_total_cnt; ++i) {
      if (_cache_tbl[i] != NULL) {
        thlib::MemPolicy<uint8_t>::memfree(_cache_tbl[i]);
        _cache_tbl[i] = NULL;
      }
    }
    thlib::MemPolicy<uint8_t*>::memfree(_cache_tbl);
    _cache_tbl = NULL;
  }
  //if (_partial_tbl != NULL) {
  //  destroy_partial();
  //}
  /// 임시 버퍼제거
  if (_tmp_buf != NULL) {
    thlib::MemPolicy<uint8_t>::memfree(_tmp_buf);
    _tmp_buf = NULL;
  }
  /// External 블럭 DS Table 제거
  if (_eblk_ds_tbl != NULL) {
    thlib::MemPolicy<ExtBlockDS>::memfree(_eblk_ds_tbl);
    _eblk_ds_tbl = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// interface

/**
 *  @brief  init
 */
int sTrfCache::init(const char* map_path, size_t cache_size, int type) {
  if (_is_init) return 0;

  // Reader Init
  int error = _reader.init(map_path, type);
  if (error) return error;

  _mappath = map_path;
  _type = type;

  // Read Header
  if (type == sTrfReader::STRF_TYPE_KS) {
  error = _reader.read(&_header, 0, sizeof(_header));
  if (error) return error;
    _pattern_cnt      = _header._pattern_cnt;
    _interval         = _header._interval;
    _tblk_real_cnt    = _header._tblk_real_cnt;
    _eblk_ksize       = _header._eblk_size;
    _eblk_bsize       = _eblk_ksize * _header._item_cnt_in_tblk;
    _eblk_total_cnt   = _header._eblk_total_cnt;
    _eblk_cnt_in_ptn  = _header._eblk_cnt_in_ptn;
    _eblk_cnt_in_time = _header._eblk_cnt_in_time;
    _eblk_tbl_ds      = _header._eblk_tbl_ds;
    _item_cnt_in_tblk = _header._item_cnt_in_tblk;
    _last_blk_idx     = _header._eblk_total_cnt -1;
  } else {
    error = _reader.read(&_ttl_header, 0, sizeof(_ttl_header));
    if (error) return error;
    _pattern_cnt      = _ttl_header._pattern_cnt;
    _interval         = _ttl_header._interval;
    _tblk_real_cnt    = _ttl_header._tblk_real_cnt;
    _eblk_ksize       = _ttl_header._eblk_size;
    _eblk_bsize       = _eblk_ksize * _ttl_header._item_cnt_in_tblk;
    _eblk_total_cnt   = _ttl_header._eblk_total_cnt;
    _eblk_cnt_in_ptn  = _ttl_header._eblk_cnt_in_ptn;
    _eblk_cnt_in_time = _ttl_header._eblk_cnt_in_time;
    _eblk_tbl_ds      = _ttl_header._eblk_tbl_ds;
    _item_cnt_in_tblk = _ttl_header._item_cnt_in_tblk;
    _last_blk_idx     = _ttl_header._eblk_total_cnt -1;
  }

  /// 임시버퍼
  _tmp_buf = thlib::MemPolicy<uint8_t>::alloc_zero(_eblk_bsize);
  if (_tmp_buf == NULL) return -EMEM_ALLOC;

  /// external 블럭 ds table
  _eblk_ds_tbl = thlib::MemPolicy<ExtBlockDS>::alloc(_eblk_total_cnt);
  if (_eblk_ds_tbl == NULL) return -EMEM_ALLOC;

  /// cache table
  _cache_tbl = thlib::MemPolicy<uint8_t*>::alloc_zero(_eblk_total_cnt);
  if (_cache_tbl == NULL) return -EMEM_ALLOC;

  /**
   *  DS 데이터 Load
   */
  size_t size = sizeof(ExtBlockDS) * _eblk_total_cnt;
  error = _reader.read(_eblk_ds_tbl, _eblk_tbl_ds, size);
  if (error) return error;

  _last_blk_size = _reader.size() - _eblk_ds_tbl[_last_blk_idx];

#if defined(WIN32) || defined(_WIN32_WCE) || defined(ANDROID) || defined(_LINUX_PND_)
  error = _cq.init(cache_size, _eblk_bsize);
  if (error) return error;
#else  
  (void)cache_size;
  for (int i = 0; i < _eblk_total_cnt; ++i) {
    error = load_block(i);
    if (error) return error;
  }
#endif

  _is_init = 1;
  return 0;
}

/**
 *  @brief  speed
 */
int sTrfCache::speed(uint8_t* spd, int ptn_id, int time_idx, 
                     int trf_blk_idx, int item_idx) {
  int eblk_idx = external_block_idx(ptn_id, time_idx, trf_blk_idx);

#if defined(WIN32) || defined(_WIN32_WCE) || defined(ANDROID) || defined(_LINUX_PND_)
  /// 로드 되어있지 않다면 로드하믄 되고...
  if (_cache_tbl[eblk_idx] == NULL) {
    int ret = _cq.push(eblk_idx);
    if (ret >= 0) {
      unload_block(ret);
    }
    ret = load_block(eblk_idx);
    if (ret) return ret;
  }
#endif

  int tblk_itm_idx = traffic_item_idx(eblk_idx, trf_blk_idx, item_idx);
  int tmp_spd = _cache_tbl[eblk_idx][tblk_itm_idx];

  if (tmp_spd == 0xff || tmp_spd == 0) {
    return -0xfe;
  }
  *spd = (uint8_t)tmp_spd;
  return 0;
}

/**
 *  @brief  speed_block
 */
//int sTrfCache::speed_block(uint8_t* spd_blk, int ptn_id, int time_idx, 
//                           int trf_blk_idx, int blk_cnt) {
//  int eblk_idx = external_block_idx(ptn_id, time_idx, trf_blk_idx);
//
//  if (_cache_tbl[eblk_idx] == NULL) {
//    int ret = _cq.push(eblk_idx);
//    if (ret >= 0) {
//      unload_block(ret);
//    }
//    ret = load_block(eblk_idx);
//    if (ret) return ret;
//  }
//  int tblk_itm_idx = traffic_item_idx(eblk_idx, trf_blk_idx, 0);
//  memcpy(spd_blk, _cache_tbl[eblk_idx] + tblk_itm_idx, 
//         _item_cnt_in_tblk * blk_cnt);
//  return 0;
//}

/**
 *  @brief  24만 통계파일 구분을 위한 traffic_pattern.bin 크기 리턴
 */
size_t sTrfCache::strf_filesize() {
  return _reader.size();
}

/**
 * @brief   [통계부붙업데이트] 패턴 로딩
 */
//int sTrfCache::load_partial_pattern(int ptn) {
//  int err = 0;
//  if (_partial_tbl != NULL) {
//    destroy_partial();
//  }
//  _partial_tbl = thlib::MemPolicy<uint8_t*>::alloc(_eblk_cnt_in_ptn);
//  if (_partial_tbl == NULL) return -EMEM_ALLOC;
//  for (int i = 0; i < _eblk_cnt_in_ptn; ++i) {
//    _partial_tbl[i] = thlib::MemPolicy<uint8_t>::alloc(_eblk_bsize);
//    if (_partial_tbl[i] == NULL) return -EMEM_ALLOC;
//  }
//
//  thlib::thFile<> f;
//  thlib::thString path;
//  path.printf("%s/%s_%d.bin", _mappath.c_str(), PARTIAL_STRF_FILENAME_TTL ,ptn);
//  if ((err = f.open(path, "rb")) != 0) {
//    return -EFILE_OPEN;
//  }
//  int size = f.size();
//  if (size != _eblk_bsize * _eblk_cnt_in_ptn) {
//    f.close();
//    destroy_partial();
//    return -1;
//  }
//
//  for (int i = 0; i < _eblk_cnt_in_ptn; ++i) {
//    f.read(_partial_tbl[i], _eblk_bsize);
//  }
//
//  f.close();
//  return 0;
//}

/**
 * @brief   [통계부분업데이트] 패턴 교체
 */
//int sTrfCache::replace_pattern(int ptn) {
//  uint8_t *tmp = NULL;
//  int sidx = ptn * _eblk_cnt_in_ptn;
//  for (int i = 0; i < _eblk_cnt_in_ptn; ++i) {
//    tmp = _cache_tbl[sidx + i];
//    _cache_tbl[sidx + i] = _partial_tbl[i];
//    _partial_tbl[i] = tmp;
//  }
//  return 0;
//}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
/// 내부 처리용 Method

/**
 *  @brief  압축된 하나의 External Block 크기를 구한다.
 */
size_t sTrfCache::compressed_block_size(int idx) {
  if (idx != _last_blk_idx) {
    return _eblk_ds_tbl[idx+1] - _eblk_ds_tbl[idx];
  } else { /// 마지막 블럭
    return _last_blk_size;
  }
}

/**
 *  @brief  External Block 하나 Load
 */
int sTrfCache::load_block(int idx) {
  _cache_tbl[idx] = 
      thlib::MemPolicy<uint8_t>::alloc(_eblk_bsize);
  if (_cache_tbl[idx] == NULL) {
    return -EMEM_ALLOC;
  }

  size_t compressed_size = compressed_block_size(idx);
//  memset(_tmp_buf, 0x00, _eblk_bsize);  ///< 안해도 되지?? size를 아니까..!!
  _reader.read(_tmp_buf, _eblk_ds_tbl[idx], compressed_size);

  /// 압축 해제하고...
  uLongf uncompressed_size = static_cast<uLongf>(_eblk_bsize);
  int error = uncompress(_cache_tbl[idx], &uncompressed_size, _tmp_buf, compressed_size);
  if (error != Z_OK) {
    return error;
  }

  return 0;
}

/**
 *  @brief  External Block Unload
 */
void sTrfCache::unload_block(int idx) {
  if (_cache_tbl[idx] != NULL) {
    thlib::MemPolicy<uint8_t>::memfree(_cache_tbl[idx]);
    _cache_tbl[idx] = NULL;
  }
}


