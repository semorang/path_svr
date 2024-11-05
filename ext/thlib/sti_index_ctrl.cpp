#include "sti_index_ctrl.h"

stiIndexCtrl* stiIndexCtrl::_instance = NULL;

void stiIndexCtrl::destroy() {
  // index rank2
  if (_idx_buf_rank2 != NULL) {
    thlib::MemPolicy<char>::memfree(_idx_buf_rank2);
    _idx_buf_rank2 = NULL;
  }

  // index rank1
  //if (_idx_buf_rank1 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_idx_buf_rank1);
  //  _idx_buf_rank1 = NULL;
  //}

  //if (_tbl_buf_rank1 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_tbl_buf_rank1);
  //  _tbl_buf_rank1 = NULL;
  //}

  // index rank0
  //if (_idx_buf_rank0 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_idx_buf_rank0);
  //  _idx_buf_rank0 = NULL;
  //}

  //if (_tbl_buf_rank0 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_tbl_buf_rank0);
  //  _tbl_buf_rank0 = NULL;
  //}

  // TTL index rank2
  if (_ttl_idx_buf_rank2 != NULL) {
    thlib::MemPolicy<char>::memfree(_ttl_idx_buf_rank2);
    _ttl_idx_buf_rank2 = NULL;
  }

  // TTL index rank1
  //if (_ttl_idx_buf_rank1 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_ttl_idx_buf_rank1);
  //  _ttl_idx_buf_rank1 = NULL;
  //}

  //if (_ttl_tbl_buf_rank1 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_ttl_tbl_buf_rank1);
  //  _ttl_tbl_buf_rank1 = NULL;
  //}

  // TTL index rank0
  //if (_ttl_idx_buf_rank0 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_ttl_idx_buf_rank0);
  //  _ttl_idx_buf_rank0 = NULL;
  //}

  //if (_ttl_tbl_buf_rank0 != NULL) {
  //  thlib::MemPolicy<char>::memfree(_ttl_tbl_buf_rank0);
  //  _ttl_tbl_buf_rank0 = NULL;
  //}

  // link count
  for (int i = 0; i < MapCntlL::MRRANK_COUNT; ++i) {
    if (_link_cnt[i] != NULL) {
      thlib::MemPolicy<int>::memfree(_link_cnt[i]);
      _link_cnt[i] = NULL;
    }
    if (_ttl_link_cnt[i] != NULL) {
      thlib::MemPolicy<int>::memfree(_ttl_link_cnt[i]);
      _ttl_link_cnt[i] = NULL;
    }
  }

  _is_init = 0;
}

int stiIndexCtrl::init(const char* mappath) {
  if (_is_init) { return 0; }

  strcpy(_map_path, mappath);

  // init map count & link count
  //thlib::thFile<> mrmng_file;
  //thlib::thString path;
  //path.printf("%s/%s", _map_path, MapCntlL::F_MAPMNG);

  //if (mrmng_file.open(path, "rb") < 0) {
  //  return -EFILE_OPEN;
  //}

  //thMapHeader maphdr;
  //if (mrmng_file.read(&maphdr, sizeof(thMapHeader)) != sizeof(thMapHeader)) {
  //  mrmng_file.close();
  //  return -EFILE_READ;
  //}

  //int rank_cnt = maphdr.rankMngCnt;

  //thRankManager *rankmng = thlib::MemPolicy<thRankManager>::alloc(rank_cnt);
  //if (mrmng_file.read(rankmng, sizeof(thRankManager) * rank_cnt) 
  //    != (sizeof(thRankManager) * rank_cnt)) {
  //  thlib::MemPolicy<thRankManager>::memfree(rankmng);
  //  mrmng_file.close();
  //  return -EFILE_READ;
  //}

  //for (int i = 0; i < rank_cnt; ++i) {
  //  _map_cnt[i] = rankmng[i]._map_cnt;
  //  _link_cnt[i] = thlib::MemPolicy<int>::alloc_zero(_map_cnt[i]);
  //  _ttl_link_cnt[i] = thlib::MemPolicy<int>::alloc_zero(_map_cnt[i]);
  //}

  //thlib::MemPolicy<thRankManager>::memfree(rankmng);
  //mrmng_file.close();

  // init KS Traffic Index
  int err = 0;
  if ((err = init_index_rank2()) != 0) {
    destroy();
    return err;
  }
  
  //if ((err = init_index_rank1()) != 0) {
  //  destroy();
  //  return err;
  //}

  //if ((err = init_index_rank0()) != 0) {
  //  destroy();
  //  return err;
  //}
  _is_init = 1;

  // init TTL Traffic Index
  if ((err = init_ttl_index_rank2()) != 0) {
    return err;
  }
  
  //if ((err = init_ttl_index_rank1()) != 0) {
  //  return err;
  //}

  //if ((err = init_ttl_index_rank0()) != 0) {
  //  return err;
  //}
  _is_ttl_init = 1;
  return 0;
}

int stiIndexCtrl::init_index_rank2() {
  thlib::thFile<> f;
  thlib::thString path;
  path.printf("%s/%s", _map_path, stiMapFileDef::F_TR_INDEX_TPEG2);
  if (f.open(path, "rb") < 0) {
    return -EFILE_OPEN;
  }

  uint32_t size = f.size();
  _idx_buf_rank2 = thlib::MemPolicy<char>::alloc(size);
  f.read(_idx_buf_rank2, size);
  f.close();

  int linkcnt = 0;
  int ds = 0, nextds = 0;
  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_LOWEST]; ++i) {
    ds = *(int*)(_idx_buf_rank2 + (i * sizeof(int)));
    if (i + 1 == _map_cnt[MapCntlL::MRRANK_LOWEST]) {
      nextds = size;
    } else {
      nextds = *(int*)(_idx_buf_rank2 + ((i + 1) * sizeof(int)));
    }
    linkcnt = (nextds - ds) / sizeof(thTrIndexL);
    _link_cnt[MapCntlL::MRRANK_LOWEST][i] = linkcnt;
  }
  return 0;
}

//int stiIndexCtrl::init_index_rank1() {
//  thlib::thFile<> idx_f, tbl_f;
//  thlib::thString idx_path, tbl_path;
//
//  // index file
//  idx_path.printf("%s/%s", _map_path, stiMapFileDef::F_TR_INDEX_TPEG1);
//  if (idx_f.open(idx_path, "rb") < 0) {
//    return -EFILE_OPEN;
//  }
//
//  uint32_t idx_size = idx_f.size();
//  _idx_buf_rank1 = thlib::MemPolicy<char>::alloc(idx_size);
//  idx_f.read(_idx_buf_rank1, idx_size);
//  idx_f.close();
//
//  // table file
//  tbl_path.printf("%s/%s", _map_path, stiMapFileDef::F_TR_ID_TPEG1);
//  if (tbl_f.open(tbl_path, "rb") < 0) {
//    thlib::MemPolicy<char>::memfree(_idx_buf_rank1);
//    return -EFILE_OPEN;
//  }
//
//  uint32_t tbl_size = tbl_f.size();
//  _tbl_buf_rank1 = thlib::MemPolicy<char>::alloc(tbl_size);
//  tbl_f.read(_tbl_buf_rank1, tbl_size);
//  tbl_f.close();
//
//  thTrIndexU *tridxu = NULL;
//  thTrIndexItem *tritem = NULL;
//  int linkcnt = 0;
//  int ds = 0, nextds = 0;
//  int tbl_ds = 0;
//  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_SECOND]; ++i) {
//    ds = *(int*)(_idx_buf_rank1 + (i * sizeof(int)));
//    if (i + 1 == _map_cnt[MapCntlL::MRRANK_SECOND]) {
//      nextds = idx_size;
//    } else {
//      nextds = *(int*)(_idx_buf_rank1 + ((i + 1) * sizeof(int)));
//    }
//    linkcnt = (nextds - ds) / sizeof(thTrIndexU);
//    _link_cnt[MapCntlL::MRRANK_SECOND][i] = linkcnt;
//  }
//  return 0;
//}
//
//int stiIndexCtrl::init_index_rank0() {
//  thlib::thFile<> idx_f, tbl_f;
//  thlib::thString idx_path, tbl_path;
//
//  // index file
//  idx_path.printf("%s/%s", _map_path, stiMapFileDef::F_TR_INDEX_TPEG0);
//  if (idx_f.open(idx_path, "rb") < 0) {
//    return -EFILE_OPEN;
//  }
//
//  uint32_t idx_size = idx_f.size();
//  _idx_buf_rank0 = thlib::MemPolicy<char>::alloc(idx_size);
//  idx_f.read(_idx_buf_rank0, idx_size);
//  idx_f.close();
//
//  // table file
//  tbl_path.printf("%s/%s", _map_path, stiMapFileDef::F_TR_ID_TPEG0);
//  if (tbl_f.open(tbl_path, "rb") < 0) {
//    thlib::MemPolicy<char>::memfree(_idx_buf_rank0);
//    return -EFILE_OPEN;
//  }
//
//  uint32_t tbl_size = tbl_f.size();
//  _tbl_buf_rank0 = thlib::MemPolicy<char>::alloc(tbl_size);
//  tbl_f.read(_tbl_buf_rank0, tbl_size);
//  tbl_f.close();
//
//  thTrIndexU *tridxu = NULL;
//  thTrIndexItem *tritem = NULL;
//  int linkcnt = 0;
//  int ds = 0, nextds = 0;
//  int tbl_ds = 0;
//  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_FIRST]; ++i) {
//    ds = *(int*)(_idx_buf_rank0 + (i * sizeof(int)));
//    if (i + 1 == _map_cnt[MapCntlL::MRRANK_FIRST]) {
//      nextds = idx_size;
//    } else {
//      nextds = *(int*)(_idx_buf_rank0 + ((i + 1) * sizeof(int)));
//    }
//    linkcnt = (nextds - ds) / sizeof(thTrIndexU);
//    _link_cnt[MapCntlL::MRRANK_FIRST][i] = linkcnt;
//  }
//  return 0;
//}

int stiIndexCtrl::init_ttl_index_rank2() {
  thlib::thFile<> f;
  thlib::thString path;
  path.printf("%s/%s", _map_path, ttlMapFileDef::F_TR_INDEX_TTL2);
  if (f.open(path, "rb") < 0) {
    return 0;
  }

  uint32_t size = f.size();
  _ttl_idx_buf_rank2 = thlib::MemPolicy<char>::alloc(size);
  f.read(_ttl_idx_buf_rank2, size);
  f.close();

  int linkcnt = 0;
  int ds = 0, nextds = 0;
  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_LOWEST]; ++i) {
    ds = *(int*)(_ttl_idx_buf_rank2 + (i * sizeof(int)));
    if (i + 1 == _map_cnt[MapCntlL::MRRANK_LOWEST]) {
      nextds = size;
    } else {
      nextds = *(int*)(_ttl_idx_buf_rank2 + ((i + 1) * sizeof(int)));
    }
    linkcnt = (nextds - ds) / sizeof(thTTLIndexL);
    _ttl_link_cnt[MapCntlL::MRRANK_LOWEST][i] = linkcnt;
  }
  return 0;
}

//int stiIndexCtrl::init_ttl_index_rank1() {
//  thlib::thFile<> idx_f, tbl_f;
//  thlib::thString idx_path, tbl_path;
//
//  // index file
//  idx_path.printf("%s/%s", _map_path, ttlMapFileDef::F_TR_INDEX_TTL1);
//  if (idx_f.open(idx_path, "rb") < 0) {
//    return 0;
//  }
//
//  uint32_t idx_size = idx_f.size();
//  _ttl_idx_buf_rank1 = thlib::MemPolicy<char>::alloc(idx_size);
//  idx_f.read(_ttl_idx_buf_rank1, idx_size);
//  idx_f.close();
//
//  // table file
//  tbl_path.printf("%s/%s", _map_path, ttlMapFileDef::F_TR_ID_TTL1);
//  if (tbl_f.open(tbl_path, "rb") < 0) {
//    thlib::MemPolicy<char>::memfree(_ttl_idx_buf_rank1);
//    return 0;
//  }
//
//  uint32_t tbl_size = tbl_f.size();
//  _ttl_tbl_buf_rank1 = thlib::MemPolicy<char>::alloc(tbl_size);
//  tbl_f.read(_ttl_tbl_buf_rank1, tbl_size);
//  tbl_f.close();
//
//  thTTLIndexU *ttlidxu = NULL;
//  thTTLIndexItem *ttlitem = NULL;
//  int linkcnt = 0;
//  int ds = 0, nextds = 0;
//  int tbl_ds = 0;
//  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_SECOND]; ++i) {
//    ds = *(int*)(_ttl_idx_buf_rank1 + (i * sizeof(int)));
//    if (i + 1 == _map_cnt[MapCntlL::MRRANK_SECOND]) {
//      nextds = idx_size;
//    } else {
//      nextds = *(int*)(_ttl_idx_buf_rank1 + ((i + 1) * sizeof(int)));
//    }
//    linkcnt = (nextds - ds) / sizeof(thTTLIndexU);
//    _ttl_link_cnt[MapCntlL::MRRANK_SECOND][i] = linkcnt;
//  }
//  return 0;
//}
//
//int stiIndexCtrl::init_ttl_index_rank0() {
//  thlib::thFile<> idx_f, tbl_f;
//  thlib::thString idx_path, tbl_path;
//
//  // index file
//  idx_path.printf("%s/%s", _map_path, ttlMapFileDef::F_TR_INDEX_TTL0);
//  if (idx_f.open(idx_path, "rb") < 0) {
//    return 0;
//  }
//
//  uint32_t idx_size = idx_f.size();
//  _ttl_idx_buf_rank0 = thlib::MemPolicy<char>::alloc(idx_size);
//  idx_f.read(_ttl_idx_buf_rank0, idx_size);
//  idx_f.close();
//
//  // table file
//  tbl_path.printf("%s/%s", _map_path, ttlMapFileDef::F_TR_ID_TTL0);
//  if (tbl_f.open(tbl_path, "rb") < 0) {
//    thlib::MemPolicy<char>::memfree(_ttl_idx_buf_rank0);
//    return 0;
//  }
//
//  uint32_t tbl_size = tbl_f.size();
//  _ttl_tbl_buf_rank0 = thlib::MemPolicy<char>::alloc(tbl_size);
//  tbl_f.read(_ttl_tbl_buf_rank0, tbl_size);
//  tbl_f.close();
//
//  thTTLIndexU *ttlidxu = NULL;
//  thTTLIndexItem *ttlitem = NULL;
//  int linkcnt = 0;
//  int ds = 0, nextds = 0;
//  int tbl_ds = 0;
//  for (int i = 0; i < _map_cnt[MapCntlL::MRRANK_FIRST]; ++i) {
//    ds = *(int*)(_ttl_idx_buf_rank0 + (i * sizeof(int)));
//    if (i + 1 == _map_cnt[MapCntlL::MRRANK_FIRST]) {
//      nextds = idx_size;
//    } else {
//      nextds = *(int*)(_ttl_idx_buf_rank0 + ((i + 1) * sizeof(int)));
//    }
//    linkcnt = (nextds - ds) / sizeof(thTTLIndexU);
//    _ttl_link_cnt[MapCntlL::MRRANK_FIRST][i] = linkcnt;
//
//    if (linkcnt == 0) {
//      continue;
//    }
//  }
//  return 0;
//}

thTrIndexItem* stiIndexCtrl::get_item(int mapid, int linkid, int dir) {
  int ds = *(int*)(_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTrIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return &(((thTrIndexL*)(_idx_buf_rank2 + ds))->_fw_idx);
  } else {
    return &(((thTrIndexL*)(_idx_buf_rank2 + ds))->_bw_idx);
  }
}

void stiIndexCtrl::get_index(uint16_t &bidx, uint16_t &iidx, int mapid, int linkid, int dir) {
  int ds = *(int*)(_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTrIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    bidx = ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_fw_block_idx();
    iidx = ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_fw_item_idx();
  } else {
    bidx = ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_bw_block_idx();
    iidx = ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_bw_item_idx();
  }
}

int stiIndexCtrl::get_block_index(int mapid, int linkid, int dir) {
  int ds = *(int*)(_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTrIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_fw_block_idx();
  } else {
    return ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_bw_block_idx();
  }
}

int stiIndexCtrl::get_block_item_index(int mapid, int linkid, int dir) {
  int ds = *(int*)(_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTrIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_fw_item_idx();
  } else {
    return ((thTrIndexL*)(_idx_buf_rank2 + ds))->get_bw_item_idx();
  }
}

//int stiIndexCtrl::get_match_count(int mapid, int linkid, int dir, int rank) {
//  char *idx_buf = NULL, *tbl_buf = NULL;
//  if (rank == MapCntlL::MRRANK_FIRST) {
//    idx_buf = _idx_buf_rank0;
//  } else {
//    idx_buf = _idx_buf_rank1;
//  }
//
//  int ds = *(int*)(idx_buf + (mapid * sizeof(int)));
//  ds    += sizeof(thTrIndexU) * linkid;
//  
//  if (dir == FORWARD_DIR) {
//    return ((thTrIndexU*)(idx_buf + ds))->_fw_cnt;
//  } else {
//    return ((thTrIndexU*)(idx_buf + ds))->_bw_cnt;
//  }
//}

//thTrIndexItem* stiIndexCtrl::get_match_index(int mapid, int linkid, int dir, int rank) {
//  char *idx_buf = NULL, *tbl_buf = NULL;
//  if (rank == MapCntlL::MRRANK_FIRST) {
//    idx_buf = _idx_buf_rank0;
//    tbl_buf = _tbl_buf_rank0;
//  } else {
//    idx_buf = _idx_buf_rank1;
//    tbl_buf = _tbl_buf_rank1;
//  }
//
//  int ds = *(int*)(idx_buf + (mapid * sizeof(int)));
//  ds    += sizeof(thTrIndexU) * linkid;
//  thTrIndexU *tridxu = (thTrIndexU*)(idx_buf + ds);
//
//  int tbl_ds = *(int*)(tbl_buf + (mapid * sizeof(int)));
//  if (dir == FORWARD_DIR) {
//    return (thTrIndexItem*)(tbl_buf + tbl_ds + (tridxu->_lindex * sizeof(thTrIndexItem)));
//  } else {
//    return (thTrIndexItem*)(tbl_buf + tbl_ds + ((tridxu->_lindex + tridxu->_fw_cnt) * sizeof(thTrIndexItem)));
//  }
//}

// TTL
thTTLIndexItem* stiIndexCtrl::get_ttl_item(int mapid, int linkid, int dir) {
  int ds = *(int*)(_ttl_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTTLIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return &(((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->_fw_idx);
  } else {
    return &(((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->_bw_idx);
  }
}

void stiIndexCtrl::get_ttl_index(uint16_t &bidx, uint16_t &iidx, int mapid, int linkid, int dir) {
  int ds = *(int*)(_ttl_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTTLIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    bidx = ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_fw_block_idx();
    iidx = ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_fw_item_idx();
  } else {
    bidx = ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_bw_block_idx();
    iidx = ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_bw_item_idx();
  }
}

int stiIndexCtrl::get_ttl_block_index(int mapid, int linkid, int dir) {
  int ds = *(int*)(_ttl_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTTLIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_fw_block_idx();
  } else {
    return ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_bw_block_idx();
  }
}

int stiIndexCtrl::get_ttl_block_item_index(int mapid, int linkid, int dir) {
  int ds = *(int*)(_ttl_idx_buf_rank2 + (mapid * sizeof(int)));
  ds    += sizeof(thTTLIndexL) * linkid;
  if (dir == FORWARD_DIR) {
    return ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_fw_item_idx();
  } else {
    return ((thTTLIndexL*)(_ttl_idx_buf_rank2 + ds))->get_bw_item_idx();
  }
}

//int stiIndexCtrl::get_ttl_match_count(int mapid, int linkid, int dir, int rank) {
//  char *idx_buf = NULL, *tbl_buf = NULL;
//  if (rank == MapCntlL::MRRANK_FIRST) {
//    idx_buf = _ttl_idx_buf_rank0;
//  } else {
//    idx_buf = _ttl_idx_buf_rank1;
//  }
//
//  int ds = *(int*)(idx_buf + (mapid * sizeof(int)));
//  ds    += sizeof(thTTLIndexU) * linkid;
//  
//  if (dir == FORWARD_DIR) {
//    return ((thTTLIndexU*)(idx_buf + ds))->_fw_cnt;
//  } else {
//    return ((thTTLIndexU*)(idx_buf + ds))->_bw_cnt;
//  }
//}

//thTTLIndexItem* stiIndexCtrl::get_ttl_match_index(int mapid, int linkid, int dir, int rank) {
//  char *idx_buf = NULL, *tbl_buf = NULL;
//  if (rank == MapCntlL::MRRANK_FIRST) {
//    idx_buf = _ttl_idx_buf_rank0;
//    tbl_buf = _ttl_tbl_buf_rank0;
//  } else {
//    idx_buf = _ttl_idx_buf_rank1;
//    tbl_buf = _ttl_tbl_buf_rank1;
//  }
//
//  int ds = *(int*)(idx_buf + (mapid * sizeof(int)));
//  ds    += sizeof(thTTLIndexU) * linkid;
//  thTTLIndexU *tridxu = (thTTLIndexU*)(idx_buf + ds);
//
//  int tbl_ds = *(int*)(tbl_buf + (mapid * sizeof(int)));
//  if (dir == FORWARD_DIR) {
//    return (thTTLIndexItem*)(tbl_buf + tbl_ds + (tridxu->_lindex * sizeof(thTTLIndexItem)));
//  } else {
//    return (thTTLIndexItem*)(tbl_buf + tbl_ds + ((tridxu->_lindex + tridxu->_fw_cnt) * sizeof(thTTLIndexItem)));
//  }
//}