/* copyright <happyteam@thinkware.co.kr> */
#ifndef TTL_MAPINFO_H_
#define TTL_MAPINFO_H_

#include "thstring.h"
#include "thfile.h"
//#include "threct.h"
#include "typelist.h"
#include "thmemalloc.h"
#include "thlock.h"
//#include "thlog.h"

#include "sti_define.h"
#include "sti_datast.h"
#include "ttl_maprecord.h"

#include "shw_mapinfol.h"

/// ds: Rank0,1 ������ ID DS, Rank2 ������ �ε��� ds, Rank0,1 ���� �� �ε��� ds
/// record: ���� TTL INDEX, ����/���� TTL INDEX, 
///         ���� TTL ID, ����/���ǿ� ��Ī�� ���� TTL ID����Ʈ

typedef TYPELIST_2(thTTLIndexU, thTTLLinkMatchRec) ttlCacheListU;


/**
 * ttl traffic data file
  */
class ttlMapFileDefs {
 public:
  // index file
  static const char* F_TR_ID_TTL0;
  static const char* F_TR_ID_TTL1;
  static const char* F_TR_INDEX_TTL0;
  static const char* F_TR_INDEX_TTL1;
  static const char* F_TR_INDEX_TTL2;
    
  /// id file
  static const char* F_TR_ID_TTL2;
};


/**
 * ttl Index ������ ���� ����.
 * ttl Link Match Index ����. 
 */
class ttlMapTrIndex {
 public:
  ttlMapTrIndex(): _ds(0) {}
 public:
  int    _ds;      ///< rank 0, 1, 2 => ds
};

/**
 * ttl Index ����
 */
class ttlMapIndex {
 public:
  enum { DS_TYPE_INDEX = 0, DS_TYPE_MATCH_INDEX, DS_TYPE_CNT };
 public:
  ttlMapIndex() {
    _map_trindex[0] = NULL;
    _map_trindex[1] = NULL;
  }
  ~ttlMapIndex() {
    destroy();
  }

  size_t operator()(int pos, int mapid) {
    thassert(pos < DS_TYPE_CNT);
    return (size_t)_map_trindex[pos][mapid]._ds;
  }
  void destroy() {
    for (int i = 0; i < DS_TYPE_CNT; ++i) {
      if (_map_trindex[i] != NULL) {
        thlib::MemPolicy<ttlMapTrIndex>::memfree(_map_trindex[i]);
        _map_trindex[i] = NULL;
      }
    }
  }
  
  int alloc_data(int type, int size) {
    _map_trindex[type] = thlib::MemPolicy<ttlMapTrIndex>::alloc(size);
    if (_map_trindex[type] == NULL) {
      return -EMEM_ALLOC;
    }
    return 0;
  }
 public:
  ttlMapTrIndex*    _map_trindex[DS_TYPE_CNT];   /// index, link match index
};


/**
 * TTL ���� MR��������. 
 * Rank 0, 1, 2 �������� Index���� ���� ��Ʈ��.
 * 
 * @note �ʱ�ȭ�� ���� ��������, ���� ��������, ���� path�� �޴´�.
 */
class ttlIndexMapCntl {
 public:
  enum { F_TR_IDX_0 = 0, F_TR_IDX_1, F_TR_IDX_2, 
         F_TR_ID_MATCH_0, F_TR_ID_MATCH_1, F_TR_NUM };
 public: 
  static ttlIndexMapCntl* instance();
  static void destruct();

  ttlIndexMapCntl();
  ~ttlIndexMapCntl();

  /**
   * @param mapdir map path
   * @param type traffic type
   */
  int init(const char* mapdir) {
    _lock.entercs();
    if (_init_state) {
      _lock.leavecs();
      return 0;
    }

    _mapdir.printf("%s", mapdir);
    /// rank mng���� ����. Rank�� �������� Read
    int error = 0;
    if ((error = init_mapinfo()) != 0) {
      _lock.leavecs();
      return error;
    }
    /// �������� Ÿ�Կ� ���� ���� size loading.
    if ((error = init_traffic_file()) != 0) {
      _lock.leavecs();
      return error;
    }    
    _init_state = 1;
    _lock.leavecs();
    return 0;
  }
  
  void destroy();

  /**
   * ���� ds
   */
  template<class TRType>
  size_t ds(TRType&, int mapid, int rank) {
    thassert(rank < _rank_cnt);
    return _map_index[rank](ttlMapIndex::DS_TYPE_INDEX, mapid);
  }

  size_t ds(thTTLLinkMatchRec&, int mapid, int rank) {
    thassert(rank < _rank_cnt - 1);
    return _map_index[rank](ttlMapIndex::DS_TYPE_MATCH_INDEX, mapid);
  }

  template<class TRType>
  size_t ds(TRType&, int mapid, int id, int rank) {
    thassert(rank < _rank_cnt);
    return _map_index[rank](ttlMapIndex::DS_TYPE_INDEX, mapid) + 
                                                   sizeof(TRType) * id;
  }
 
  /// ���� ������� ������� ����!
  template<class TRType>
  size_t size(TRType& obj, int mapid, int rank) {
    int file_idx = 0 + rank;
    size_t ret_size = 0;
    size_t ds1 = ds(obj, mapid, rank);
    if (mapid == _map_cnt[rank] - 1) {
      ret_size = _file_size[file_idx] - ds1;
    } else {
      ret_size = ds(obj, mapid+1, rank) - ds1;
    }
    return ret_size;
  }

  /// thLinkMatchRec: FILE�� �ٸ��� ������ ������ ���.
  size_t size(thTTLLinkMatchRec& obj, int mapid, int rank) {
    int file_idx = F_TR_ID_MATCH_0 + rank;
    size_t ret_size = 0;
    size_t ds1 = ds(obj, mapid, rank);
    if (mapid == _map_cnt[rank] - 1) {
      ret_size = _file_size[file_idx] - ds1;
    } else {
      ret_size = ds(obj, mapid+1, rank) - ds1;
    }
    return ret_size;
  }

 private:
  int init_mapinfo();
  int init_traffic_file();

 public:
  static ttlIndexMapCntl*    _instance;     ///< TTL ���� ��Ʈ��.
  const char* _file_name[F_TR_NUM];         ///< TTL index file
  int     _map_cnt[MapCntlL::MRRANK_COUNT]; ///< ��ũ�� ��������.
 private:
  thlib::thString _mapdir;
  int             _init_state;  
  int             _rank_cnt;
  ttlMapIndex*    _map_index;
  int             _file_size[F_TR_NUM];
  thlib::thLock   _lock;
};


///////////////////////////////////////////////////////////////////////////////
// # TTL ID ���� Map info ���� 
///////////////////////////////////////////////////////////////////////////////
/**
 * Rank2 TTL ID ���� ��Ʈ��.
 */
class ttlIDMapCntl {
public:
	enum SVC_SHM_INDEX {
		SVC_SHM_CTT,
		SVC_SHM_MAX
	};
	enum {
    SH_RTM_MAX_CNT = 128,
	  SH_NWS_MAX_CNT = 30,
	};
public:
  static ttlIDMapCntl* instance() {
    if (_instance == NULL) {
      _instance = thlib::MemPolicyNew<ttlIDMapCntl>::alloc(1);
      atexit(destruct);
    }
    return _instance;
  }

  static void destruct() {
    if (_instance != NULL) {
      ttlIDMapCntl::_instance->destroy();
      thlib::MemPolicyNew<ttlIDMapCntl>::memfree(ttlIDMapCntl::_instance);
      ttlIDMapCntl::_instance = NULL;
    }
  }

  ttlIDMapCntl() {
    _file_name = NULL;
    _init_state = 0;
    _map_cnt = 0; 
	  for (int i = 0; i < SVC_SHM_MAX; ++i) {
	    _svc_shm_size[i] = 0;
	  }
  }

  ~ttlIDMapCntl() {
    destroy();
  }

  void destroy() {
    _file_name = NULL;
    _file_size = 0;
    _init_state = 0;
	  for (int i = 0; i < SVC_SHM_MAX; ++i) {
	    _svc_shm_size[i] = 0;
	  }
  }

  int init(const char* mapdir) {
    _lock.entercs();
    if (_init_state) {
      _lock.leavecs();
      return 0;
    }
    /// file name
    _file_name = ttlMapFileDefs::F_TR_ID_TTL2;

    /// file read & init
    thlib::thFile<> mapfile;
    thlib::thString path;

    path.printf("%s/%s", mapdir, _file_name);
    if (mapfile.open(path(), "rb") != 0) {
      _lock.leavecs();
      return -EFILE_OPEN;
    }

    if (mapfile.read(&_map_cnt, sizeof(_map_cnt)) != sizeof(_map_cnt)) {
      mapfile.close();
      _lock.leavecs();
      return -EFILE_READ;
    }
      
    mapfile.close();
    /// open�� �����޸� size ����.
    size_t max_share_ctt_blk_cnt = _map_cnt;
    _svc_shm_size[SVC_SHM_CTT] = sizeof(stiSharedTTLCTTInfo) +   /// info header
      (sizeof(stiSharedTTLCTTBlock) * max_share_ctt_blk_cnt) + /// speed block
      sizeof(int);

    _init_state = 1;
    _lock.leavecs();
    return 0;
  }

  /**
   * happyway_win ~ iNavi���� service �����޸� ������ ����.
   * @param con_type �����޸� ����. (������ ���ǵ� ���� �״�� ���.)
   * @return 0 �ʱ�ȭ ���� ����. �׿� ������ �����޸� ������. 
   */
  int svc_shmem_size(int con_type) {
	  if (con_type < 0 || con_type >= SVC_SHM_MAX) {
		  return 0;
	  }
	  return _svc_shm_size[con_type];
  }

  /// ds (file������ ds)
  size_t ds(size_t block_idx) {
    thassert(block_idx < _map_index.block_cnt());
    size_t ds = sizeof(int);
    ds += sizeof(thTTLIDBlock) * block_idx;
    return ds;
  }

  /// size
  size_t size(size_t block_idx) {
    (void) block_idx;
    return sizeof(thTTLIDBlock);
  }
  
 public:
  static ttlIDMapCntl*  _instance;
  const char*           _file_name; ///< TTL id file  
  uint32_t              _map_cnt;   ///< ��ü block cnt

 private:
  int                   _init_state;
  thlib::thLock         _lock;
  int                   _file_size;
  size_t _svc_shm_size[SVC_SHM_MAX]; ///< traffic ~ inavi ���� �����޸� ������.
};

#endif
