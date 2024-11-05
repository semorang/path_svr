/* copyright <happyteam@thinkware.co.kr> */
#include "ttl_mapinfo_r.h"

const char* ttlMapFileDefs::F_TR_ID_TTL0 = "mrdata/trf_ttl0.bin";
const char* ttlMapFileDefs::F_TR_ID_TTL1 = "mrdata/trf_ttl1.bin";
const char* ttlMapFileDefs::F_TR_INDEX_TTL0 = "mrdata/trfidx_ttl0.bin";
const char* ttlMapFileDefs::F_TR_INDEX_TTL1 = "mrdata/trfidx_ttl1.bin";
const char* ttlMapFileDefs::F_TR_INDEX_TTL2 = "mrdata/trfidx_ttl2.bin";

const char* ttlMapFileDefs::F_TR_ID_TTL2 = "mrdata/trf_ttl2.bin";

ttlIndexMapCntl* ttlIndexMapCntl::_instance = NULL;
ttlIDMapCntl* ttlIDMapCntl::_instance = NULL;
/**
 * create singleton instance
 */
ttlIndexMapCntl* ttlIndexMapCntl::instance() {
  if (_instance == NULL) {
    _instance = thlib::MemPolicyNew<ttlIndexMapCntl>::alloc(1);
    atexit(destruct);
  }
  return _instance;
}

/**
 * destruct 
 */
void ttlIndexMapCntl::destruct() {
  if (_instance != NULL) {
    ttlIndexMapCntl::_instance->destroy();
    thlib::MemPolicyNew<ttlIndexMapCntl>::memfree(ttlIndexMapCntl::_instance);
    ttlIndexMapCntl::_instance = NULL;
  }
}

ttlIndexMapCntl::ttlIndexMapCntl() : _map_index(NULL) {
  int i = 0;
  for (i = 0; i < MapCntlL::MRRANK_COUNT-1; ++i) {
    _map_cnt[i] = 0;
  }
  for (i = 0; i < F_TR_NUM; ++i) {
    _file_size[i] = 0;
    _file_name[i] = NULL;
  }
  _init_state = 0;
}

ttlIndexMapCntl::~ttlIndexMapCntl() {
  destroy();
}

void ttlIndexMapCntl::destroy() {
  int i = 0;
  if (_map_index != NULL) {
    for (i = 0; i < _rank_cnt; ++i) {
      _map_index[i].destroy();
    }
    thlib::MemPolicy<ttlMapIndex>::memfree(_map_index);
    _map_index = NULL;
  }
  for (i = 0; i < MapCntlL::MRRANK_COUNT-1; ++i) {
    _map_cnt[i] = 0;
  }
  for (i = 0; i < F_TR_NUM; ++i) {
    _file_size[i] = 0;
    _file_name[i] = NULL;
  }
  _rank_cnt = 0;
  _init_state = 0;
}

/**
 * rank mng파일 오픈. Rank별 도엽개수 Read
 */
int ttlIndexMapCntl::init_mapinfo() {
  thlib::thFile<> mapfile;
  thlib::thString path;
  path.printf("%s/%s", _mapdir(), MapCntlL::F_MAPMNG);

  if (mapfile.open(path, "rb") < 0) {
    return -EFILE_OPEN;
  }

  thMapHeader mapheader;
  if (mapfile.read(&mapheader, sizeof(thMapHeader)) 
      != sizeof(thMapHeader)) {
     mapfile.close();
     return -EFILE_READ;
  }
  thassert(mapheader.rankMngCnt != 0);

  _rank_cnt = mapheader.rankMngCnt;
  thRankManager* rankmng = thlib::MemPolicy<thRankManager>::alloc(_rank_cnt);
  if (mapfile.read(rankmng, sizeof(thRankManager) * _rank_cnt) 
      != size_t(sizeof(thRankManager) * _rank_cnt)) {
    thlib::MemPolicy<thRankManager>::memfree(rankmng);
    mapfile.close();
    return -EFILE_READ;
  }
  
  for (int i = 0; i < _rank_cnt; ++i) {
    _map_cnt[i] = rankmng[i]._map_cnt;
  }

  thlib::MemPolicy<thRankManager>::memfree(rankmng);
  mapfile.close();
  /// map info 처리.
  
  _map_index = thlib::MemPolicy<ttlMapIndex>::alloc(_rank_cnt);
  memset(&_map_index[0], 0x00, sizeof(_map_index[0]) * _rank_cnt);
  int ri = 0; 
  /*int error = 0;*/
  for ( ; ri < _rank_cnt; ++ri) {
    if (_map_index[ri].alloc_data(ttlMapIndex::DS_TYPE_INDEX, _map_cnt[ri]) 
        != 0) {
      return -EMEM_ALLOC;
    }
  }
  
  /// rank 0, 1 link match index alloc
  for (ri = 0; ri < _rank_cnt - 1; ++ri) {
    if (_map_index[ri].alloc_data(ttlMapIndex::DS_TYPE_MATCH_INDEX, 
         _map_cnt[ri]) != 0) {
      return -EMEM_ALLOC;
    }
  }

  return 0;
}

/**
 * 파일 사이즈 리드 및 도엽별 정보 미리 로딩. 
 */
int ttlIndexMapCntl::init_traffic_file() {
  uint32_t read_size = 0;
  
  _file_name[F_TR_IDX_0] = ttlMapFileDefs::F_TR_INDEX_TTL0;
  _file_name[F_TR_IDX_1] = ttlMapFileDefs::F_TR_INDEX_TTL1;
  _file_name[F_TR_IDX_2] = ttlMapFileDefs::F_TR_INDEX_TTL2;
  _file_name[F_TR_ID_MATCH_0] = ttlMapFileDefs::F_TR_ID_TTL0;
  _file_name[F_TR_ID_MATCH_1] = ttlMapFileDefs::F_TR_ID_TTL1;

  /// block size 계산을 위한 파일 사이즈 정의.
  thlib::thFile<> mapfile;
  thlib::thString path;
  int error = 0;
  int ri = 0; 
  int file_idx = F_TR_IDX_0;
  
  /// index파일 로딩.
  for ( ; ri < _rank_cnt; ++ri) {
    read_size = sizeof(ttlMapTrIndex) * _map_cnt[ri];
    path.printf("%s/%s", _mapdir(), _file_name[file_idx]);
    error = mapfile.open(path);
    if (error) {
      return -EFILE_OPEN;
    }
    _file_size[file_idx] = mapfile.size(); 
    if (mapfile.read((char*)&_map_index[ri]._map_trindex[ttlMapIndex::
                                            DS_TYPE_INDEX][0], read_size)
                                                           != read_size) {
      mapfile.close();
      return -EFILE_READ;  
    } 
    mapfile.close();
    ++file_idx;
  }
  
  /// link match 로딩.
  for (ri = 0 ; ri < _rank_cnt - 1; ++ri) {
    read_size = sizeof(ttlMapTrIndex) * _map_cnt[ri];
    path.printf("%s/%s", _mapdir(), _file_name[file_idx]);
    error = mapfile.open(path);
    if (error) {
      return -EFILE_OPEN;
    }
    _file_size[file_idx] = mapfile.size(); 
    if (mapfile.read((char*)&_map_index[ri]._map_trindex[ttlMapIndex::
                                              DS_TYPE_MATCH_INDEX][0], 
                                               read_size) != read_size) {
      mapfile.close();
      return -EFILE_READ;    
    }
    mapfile.close();
    ++file_idx;
  }
  
  return 0;
}
