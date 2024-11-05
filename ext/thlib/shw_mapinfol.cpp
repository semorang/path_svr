///* copyright <happyteam@thinkware.co.kr> */
#include "shw_mapinfol.h"
//
const char* MapCntlL::F_MAPMNG          = "mrdata/mrmanage.bin";
//const char* MapCntlL::F_RANK2           = "mrdata/mrrank2.bin";
//const char* MapCntlL::F_VERTEX          = "mrdata/mrvertex.bin";
//const char* MapCntlL::F_REST            = "mrdata/linkidvscode.db";
//const char* MapCntlL::F_TOLL            = "mrdata/tolltable.bin";
//const char* MapCntlL::F_METROTOLL       = "mrdata/metroopentoll.bin";
//const char* MapCntlL::F_DALINK          = "mrdata/dalink.bin";
//const char* MapCntlL::F_RB_TIME         = "mrdata/roadblocking_time.bin";
//const char* MapCntlL::F_RB_VEHICLE      = "mrdata/roadblocking_vehicle.bin";
//const char* MapCntlL::F_S_RATIO         = "mrdata/s_ratio.bin";
//const char* MapCntlL::F_PREFERENCE_ROAD = "mrdata/preference_road.bin";
//const char* MapCntlL::F_ROAD_SECTION    = "mrdata/roadsection.bin";
//const char* MapCntlL::F_EXINFO          = "mrdata/mrexinfo.bin";
//const char* MapCntlL::F_CROSSNAME       = "mrdata/mrcrossname.bin";
//const char* MapCntlL::F_STREETNAME      = "mrdata/streetname.bin";
//
//
//MapCntlL* MapCntlL::_instance = NULL;
//
//MapCntlL::MapCntlL() : _map_indexL(NULL), 
//  _map_version(0), _memsize(0), _last_mapid(0), _rank_cnt(0), _is_init(0),
//  _toll_open(NULL), _tollex1blk(NULL), _tollex2blk(NULL), _toll_name(NULL),
//  _toll_td_info(NULL), _road_charge(NULL), _toll_exhipass(NULL), _toll_td(NULL),
//  _rbt_time(NULL), _rbt_td(NULL), _trb_weight_tbl(NULL), _rest_area(NULL), _preference_road_list(NULL),
//  _road_charge_cnt(0), _rest_area_cnt(0), _toll_exhipass_cnt(0)
//{
//  memset(_file_size,        0x00, sizeof(_file_size));
//  memset(_map_cnt,          0x00, sizeof(_map_cnt));
//  memset(&_tollhead,        0x00, sizeof(_tollhead));
//  memset(_rbt_idx,          0x00, sizeof(_rbt_idx));
//  memset(_trb_idx_tbl,      0x00, sizeof(_trb_idx_tbl));
//  memset(_s_ratio,          0x00, sizeof(_s_ratio));
//  memset(_ex_opentoll,      0x00, sizeof(_ex_opentoll));
//  memset(_pub_highway_base, 0x00, sizeof(_pub_highway_base));
//}
//
//MapCntlL::~MapCntlL()
//{
//  destroy();
//}
//
//MapCntlL* MapCntlL::instance()
//{
//  if (_instance == NULL) {
//    _instance = thlib::MemPolicyNew<MapCntlL>::alloc(1);
//    atexit(destroy_bridge);
//    return _instance;
//  }
//  return _instance;
//}
//
//void MapCntlL::destroy_bridge()
//{
//  if (MapCntlL::_instance != NULL) {
//    MapCntlL::instance()->destroy();
//    thlib::MemPolicyNew<MapCntlL>::memfree(MapCntlL::_instance);
//    MapCntlL::_instance = NULL;
//  }
//}
//
//void MapCntlL::destroy()
//{
//  int i = 0;
//  if (_map_indexL != NULL) {
//    thlib::MemPolicy<MapIndexL>::memfree(_map_indexL);
//    _map_indexL = NULL;
//  }
//  if (_toll_open != NULL) {
//    thlib::MemPolicy<thTollRec>::memfree(_toll_open);
//    _toll_open = NULL;
//  }
//  if (_tollex1blk != NULL) {
//    thlib::MemPolicy<thTollEx1Rec>::memfree(_tollex1blk);
//    _tollex1blk = NULL;
//  }
//  if (_tollex2blk != NULL) {
//    thlib::MemPolicy<thTollEx2Rec>::memfree(_tollex2blk);
//    _tollex2blk = NULL;
//  }
//  if (_toll_name != NULL) {
//    thlib::MemPolicy<char>::memfree(_toll_name);
//    _toll_name = NULL;
//  }
//  if (_toll_td_info != NULL) {
//    thlib::MemPolicy<thTollTimeDomain>::memfree(_toll_td_info);
//    _toll_td_info = NULL;
//  }
//  if (_road_charge != NULL) {
//    thlib::MemPolicy<RoadCharge>::memfree(_road_charge);
//    _road_charge = NULL;
//  }
//  if (_toll_exhipass != NULL) {
//    thlib::MemPolicy<HiPassException>::memfree(_toll_exhipass);
//    _toll_exhipass = NULL;
//  }
//  if (_toll_td != NULL) {
//    thlib::MemPolicy<char>::memfree(_toll_td);
//    _toll_td = NULL;
//  }
//  for (i = 0; i < MRRANK_COUNT; ++i) {
//    if (_rbt_idx[i] != NULL) {
//      for (int idx = 0; idx < (int)(_map_cnt[i]); ++idx) {
//        if (_rbt_idx[i][idx] != NULL) {
//          thlib::MemPolicy<TimeRoadBlockingIdx>::memfree(_rbt_idx[i][idx]);
//          _rbt_idx[i][idx] = NULL;
//        }
//      }
//      thlib::MemPolicy<TimeRoadBlockingIdx*>::memfree(_rbt_idx[i]);
//      _rbt_idx[i] = NULL;
//    }
//  }
//  if (_rbt_time != NULL) {
//    thlib::MemPolicy<RoadBlockingTime>::memfree(_rbt_time);
//    _rbt_time = NULL;
//  }
//  if (_rbt_td != NULL) {
//    thlib::MemPolicy<char>::memfree(_rbt_td);
//    _rbt_td = NULL;
//  }
//  for (i = 0; i < MRRANK_COUNT; ++i) {
//    if (_trb_idx_tbl[i] != NULL) {
//      for (int idx = 0; idx < (int)(_map_cnt[i]); ++idx) {
//        if (_trb_idx_tbl[i][idx] != NULL) {
//          thlib::MemPolicy<TruckRoadBlockingIdx>::memfree(_trb_idx_tbl[i][idx]);
//          _trb_idx_tbl[i][idx] = NULL;
//        }
//      }
//      thlib::MemPolicy<TruckRoadBlockingIdx*>::memfree(_trb_idx_tbl[i]);
//      _trb_idx_tbl[i] = NULL;
//    }
//  }
//  if (_trb_weight_tbl != NULL) {
//    thlib::MemPolicy<uint16_t>::memfree(_trb_weight_tbl);
//    _trb_weight_tbl = NULL;
//  }
//  if (_rest_area != NULL) {
//    thlib::MemPolicy<thRestAreaRec>::memfree(_rest_area);
//    _rest_area = NULL;
//  }
//  for (i = 0; i < MRRANK_COUNT; ++i) {
//    if (_s_ratio[i] != NULL) {
//      for (int idx = 0; idx < (int)(_map_cnt[i]); ++idx) {
//        if (_s_ratio[i][idx] != NULL) {
//          thlib::MemPolicy<uint8_t>::memfree(_s_ratio[i][idx]);
//          _s_ratio[i][idx] = NULL;
//        }
//      }
//      thlib::MemPolicy<uint8_t*>::memfree(_s_ratio[i]);
//      _s_ratio[i] = NULL;
//    }
//  }
//  if (_preference_road_list != NULL) {
//    thlib::MemPolicy<char>::memfree(_preference_road_list);
//    _preference_road_list = NULL;
//    _preference_index.clear();
//    PreferenceRoadMap().swap(_preference_index);
//  }
//
//  _is_init = 0;
//}
//
//int MapCntlL::init(const char* map_path)
//{
//  if (_is_init) return 0;
//
//  _lock.entercs();
//
//  size_t obj_size = 0, ds = 0;
//  int ret = 0, map_year = 0, map_month = 0, map_day = 0, i = 0;
//  thMapHeader mapheader;
//  thRankManager rankmng[MRRANK_COUNT];
//  thMapRecL* map_buff = NULL;
//  thlib::thFile<> file;
//  thlib::thString path;
//  _memsize = 0;
//
//  ////////// Map Header //////////
//  _map_path.printf("%s", map_path);
//  path.printf("%s/%s", map_path, F_MAPMNG);
//  if (file.open(path, "rb") < 0) {
//    _lock.leavecs();
//    return -MSG_FAIL_FILE_OPEN;
//  }
//  obj_size = sizeof(thMapHeader);
//  if (file.read((char*)&mapheader, ds, obj_size) != obj_size) {
//    _lock.leavecs();
//    return -MSG_FAIL_FILE_READ;
//  }
//#if defined(WIN32) || defined(_WIN32_WCE)
//  sscanf_s(mapheader.VolumUpadateDate, "%d/%d/%d", &map_year, &map_month, &map_day);
//#else
//  sscanf(mapheader.VolumUpadateDate, "%d/%d/%d", &map_year, &map_month, &map_day);
//#endif
//  _map_version = (map_year * 10000) + (map_month * 100) + map_day;
//  _rank_cnt    = static_cast<int8_t>(mapheader.rankMngCnt);
//
//  ////////// Rank Manager //////////
//  ds = sizeof(thMapHeader);
//  obj_size = sizeof(thRankManager) * _rank_cnt;
//  if (file.read((char*)rankmng, ds, obj_size) != obj_size) {
//    _lock.leavecs();
//    return -MSG_FAIL_FILE_READ;
//  }
//  for (i = 0; i < MRRANK_COUNT; ++i) {
//    _map_cnt[i] = static_cast<int16_t>(rankmng[i]._map_cnt);
//  }
//  _last_mapid = _map_cnt[MRRANK_LOWEST] - 1;
//
//  ////////// Map Index //////////
//  if ((_map_indexL = thlib::MemPolicy<MapIndexL>::alloc(_map_cnt[MRRANK_LOWEST])) == NULL) {
//    _lock.leavecs();
//    return -MSG_FAIL_MEMORY_ALLOC;
//  }
//  if ((map_buff = thlib::MemPolicy<thMapRecL>::alloc(_map_cnt[MRRANK_LOWEST])) == NULL) {
//    _lock.leavecs();
//    return -MSG_FAIL_MEMORY_ALLOC;
//  }
//
//  obj_size = _map_cnt[MRRANK_LOWEST] * sizeof(thMapRecL);
//  if (file.read((char*)map_buff, rankmng[MRRANK_LOWEST]._dsmr, obj_size) != obj_size) {
//    thlib::MemPolicy<thMapRecL>::memfree(map_buff);
//    _lock.leavecs();
//    return -MSG_FAIL_FILE_READ;
//  }
//
//  _total_map_rect.clear();// 초기화
//  for (i = 0; i < _map_cnt[MRRANK_LOWEST]; ++i) {
//    _map_indexL[i](map_buff[i]);
//    // 전체 지도 범위를 구하기 위해 모든 mesh를 순회하면서 rect를 확장.
//    _total_map_rect.expand(_map_indexL[i].rect);
//  }
//  thlib::MemPolicy<thMapRecL>::memfree(map_buff);
//  _memsize += static_cast<int>(sizeof(MapIndexL) * _map_cnt[MRRANK_LOWEST]);
//  file.close();
//
//  if ((ret = load_tollgate()) != 0) return ret;
//  if ((ret = load_metroopentoll()) != 0) return ret;
//  if ((ret = load_restarea()) != 0) return ret;
//  if ((ret = load_dalink()) != 0) return ret;
//  if ((ret = load_roadblocking_time()) != 0) return ret;
//  if ((ret = load_roadblocking_vehicle()) != 0) return ret;
//  if ((ret = load_special_ratio()) != 0) return ret;
//  if ((ret = load_preference_road()) != 0) return ret;
//  if ((ret = load_road_section()) != 0) return ret;
//
//  // block size계산을 위해 파일의 사이즈를 정의한다.
//  path.printf("%s/%s", map_path, F_RANK2);
//  if (file.open(path, "rb") < 0) return -MSG_FAIL_FILE_OPEN;
//  _file_size[FNUM_RANK2] = file.size();
//  file.close();
//
//  path.printf("%s/%s", map_path, F_VERTEX);
//  if (file.open(path, "rb") < 0) return -MSG_FAIL_FILE_OPEN;
//  _file_size[FNUM_VERTEX] = file.size();
//  file.close();
//
//  path.printf("%s/%s", map_path, F_CROSSNAME);
//  if (file.open(path, "rb") < 0) return -MSG_FAIL_FILE_OPEN;
//  _file_size[FNUM_CROSSNAME] = file.size();
//  file.close();
//
//  path.printf("%s/%s", map_path, F_EXINFO);
//  if (file.open(path, "rb") < 0) return -MSG_FAIL_FILE_OPEN;
//  _file_size[FNUM_EXINFO] = file.size();
//  file.close();
//
//  path.printf("%s/%s", map_path, F_STREETNAME);
//  if (file.open(path, "rb") < 0) return -MSG_FAIL_FILE_OPEN;
//  _file_size[FNUM_STREETNAME] = file.size();
//  file.close();
//
//  _is_init = 1;
//  _lock.leavecs();
//  return 0;
//}
//
//int MapCntlL::load_tollgate()
//{
//  size_t obj_size = 0, obj_cnt = 0, ds = 0;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_TOLL);
//  if (file.open(path, "rb") < 0) {
//    return -MSG_FAIL_FILE_OPEN;
//  }
//  _file_size[FNUM_TOLL] = file.size();
//
//  // Read Toll Header
//  if (file.read((char*)&_tollhead, 0, sizeof(thTollHeader)) != sizeof(thTollHeader)) {
//    return -MSG_FAIL_FILE_READ;
//  }
//  // 개방형 톨게이트.
//  if (_toll_open == NULL) {
//    ds = sizeof(thTollHeader) + (_tollhead._closecnt * _tollhead._closecnt * sizeof(thTollRec));
//    obj_size = sizeof(thTollRec) * _tollhead._opencnt;
//    if ((_toll_open = thlib::MemPolicy<thTollRec>::alloc(_tollhead._opencnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_toll_open, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // 예외 1형.
//  if (_tollex1blk == NULL) {
//    ds = sizeof(thTollHeader) + 
//         (_tollhead._closecnt * _tollhead._closecnt * sizeof(thTollRec)) + // 폐쇄형.
//         (_tollhead._opencnt * sizeof(thTollRec)); // 개방형.
//    obj_size = sizeof(thTollEx1Rec) * _tollhead._ex1cnt;
//    if ((_tollex1blk = thlib::MemPolicy<thTollEx1Rec>::alloc(_tollhead._ex1cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_tollex1blk, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // 예외 2형.
//  if (_tollex2blk == NULL) {
//    ds += obj_size; // ※ caution !!
//    obj_size = sizeof(thTollEx2Rec) * _tollhead._ex2cnt;
//    if ((_tollex2blk = thlib::MemPolicy<thTollEx2Rec>::alloc(_tollhead._ex2cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_tollex2blk, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // 톨게이트 명칭.
//  if (_toll_name == NULL) {
//    ds = _tollhead._toll_name_ds;
//    obj_size = _tollhead._td_charge_ratio_ds - ds;
//    if ((_toll_name = thlib::MemPolicy<char>::alloc(obj_size)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read(_toll_name, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // Toll TimeDomain Info.
//  if (_toll_td_info == NULL) {
//    ds = _tollhead._td_charge_ratio_ds;
//    obj_size = _tollhead._unitrate_ds - ds;
//    obj_cnt  = obj_size / sizeof(thTollTimeDomain);
//    if ((_toll_td_info = thlib::MemPolicy<thTollTimeDomain>::alloc(obj_cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_toll_td_info, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // 고속도로 기본요금 (앞쪽3개는 국영, 나머지는 민자)
//  if (_road_charge == NULL) {
//    // 국영
//    RoadCharge tmp_road_charge[3];
//    ds = _tollhead._unitrate_ds;
//    obj_size = sizeof(tmp_road_charge);
//    memset(tmp_road_charge, 0x00, sizeof(tmp_road_charge));
//    if (file.read((char*)tmp_road_charge, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    // 국영은 기본요금만 있으면 된다.
//    _pub_highway_base[1] = tmp_road_charge[0]._charge_per_m;
//    _pub_highway_base[2] = tmp_road_charge[1]._charge_per_m;
//    _pub_highway_base[3] = tmp_road_charge[2]._charge_per_m;
//    // 민자
//    ds = _tollhead._unitrate_ds + sizeof(tmp_road_charge);
//    obj_size = _tollhead._ex_hipass_ds - ds;
//    _road_charge_cnt = static_cast<uint16_t>(obj_size / sizeof(RoadCharge));
//    if ((_road_charge = thlib::MemPolicy<RoadCharge>::alloc_zero(_road_charge_cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_road_charge, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // Hipass 예외형 요금표.
//  if (_toll_exhipass == NULL) {
//    ds = _tollhead._ex_hipass_ds;
//    obj_size = _tollhead._timedomain_block_ds - ds;
//    _toll_exhipass_cnt = static_cast<uint16_t>(obj_size / sizeof(HiPassException));
//    if ((_toll_exhipass = thlib::MemPolicy<HiPassException>::alloc(_toll_exhipass_cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_toll_exhipass, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//  // TimeDomain.
//  if (_toll_td == NULL) {
//    ds = _tollhead._timedomain_block_ds;
//    obj_size = _file_size[FNUM_TOLL] - ds;
//    if ((_toll_td = thlib::MemPolicy<char>::alloc(obj_size)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_toll_td, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//
//  file.close();
//  return 0;
//}
//
//int MapCntlL::load_metroopentoll()
//{
//  thlib::thFile<> file;
//  thlib::thString path;
//  path.printf("%s/%s", _map_path(), F_METROTOLL);
//  if (file.open(path, "rb") == 0) {
//    file.read((char*)_ex_opentoll, sizeof(_ex_opentoll));
//    file.close();
//  }
//  return 0;
//}
//
//int MapCntlL::load_restarea()
//{
//  size_t obj_size = 0, ds = 0;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_REST);
//  if (file.open(path, "rb") < 0) {
//    return -MSG_FAIL_FILE_OPEN;
//  }
//  _file_size[FNUM_REST] = file.size();
//
//  if (_rest_area == NULL) {
//    if (file.read((char*)&_rest_area_cnt, sizeof(int)) != sizeof(int)) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    if ((_rest_area = thlib::MemPolicy<thRestAreaRec>::alloc(_rest_area_cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    ds = sizeof(int);
//    obj_size = sizeof(thRestAreaRec) * _rest_area_cnt;
//    if (file.read((char*)_rest_area, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _memsize += static_cast<int>(obj_size);
//  }
//
//  file.close();
//  return 0;
//}
//
//int MapCntlL::load_dalink()
//{
//  thlib::thFile<> file;
//  thlib::thString path;
//  path.printf("%s/%s", _map_path(), F_DALINK);
//  if (file.open(path, "rb") == 0) {
//    file.close();
//  }
//  return 0;
//}
//
//int MapCntlL::load_roadblocking_time()
//{
//  size_t obj_size = 0, obj_cnt = 0, ds = 0;
//  int rank, idx, next_idx;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_RB_TIME);
//  if (file.open(path, "rb") == 0) {
//    // header
//    TimeRoadBlockingHeader header;
//    file.read((char*)&header, sizeof(TimeRoadBlockingHeader));
//
//    // DS Table alloc & read
//    uint32_t* ds_tbl[MRRANK_COUNT] = { NULL };
//    for (rank = 0; rank < MRRANK_COUNT; ++rank) {
//      // Object 개수 계산을 위해 실제 Map 보다 1개 더 만든다.
//      ds_tbl[rank] = thlib::MemPolicy<uint32_t>::alloc_zero(_map_cnt[rank] + 1);
//      obj_size = _map_cnt[rank] * sizeof(uint32_t);
//      if (file.read((char*)ds_tbl[rank], obj_size) != obj_size) {
//        return -MSG_FAIL_FILE_READ;
//      }
//    }
//
//    // 마지막 도엽에 DS 정보 설정.
//    for (rank = 1; rank < MRRANK_COUNT; ++rank) {
//      for (idx = 0; idx < (int)(_map_cnt[rank]); ++idx) {
//        if (ds_tbl[rank][idx] != UINT_MAX) {
//          ds_tbl[rank - 1][_map_cnt[rank - 1]] = ds_tbl[rank][idx];
//          break;
//        }
//      }
//    }
//    ds_tbl[MRRANK_LOWEST][_map_cnt[MRRANK_LOWEST]] = header._time_table_ds;
//
//    // Index Table alloc & read
//    for (rank = 0; rank < MRRANK_COUNT; ++rank) {
//      _rbt_idx[rank] = thlib::MemPolicy<TimeRoadBlockingIdx*>::alloc_zero(_map_cnt[rank]); // Map Header
//      _memsize += sizeof(TimeRoadBlockingIdx*) * _map_cnt[rank];
//      for (idx = 0; idx < (int)(_map_cnt[rank]); ++idx) {
//        if (ds_tbl[rank][idx] != UINT_MAX) {
//          next_idx = idx + 1;
//          while (ds_tbl[rank][next_idx] == UINT_MAX) next_idx++;
//          ds       = ds_tbl[rank][idx];
//          obj_size = ds_tbl[rank][next_idx] - ds_tbl[rank][idx];
//          obj_cnt  = obj_size / sizeof(TimeRoadBlockingIdx);
//          if ((_rbt_idx[rank][idx] = thlib::MemPolicy<TimeRoadBlockingIdx>::alloc_zero(obj_cnt)) == NULL) {
//            for (rank = 0; rank < MRRANK_COUNT; ++rank)
//              thlib::MemPolicy<uint32_t>::memfree(ds_tbl[rank]);
//            return -MSG_FAIL_MEMORY_ALLOC;
//          }
//          if (file.read((char*)_rbt_idx[rank][idx], ds, obj_size) != obj_size) {
//            return -MSG_FAIL_FILE_READ;
//          }
//          _memsize += static_cast<int>(obj_size);
//        }
//      }
//      thlib::MemPolicy<uint32_t>::memfree(ds_tbl[rank]);
//      ds_tbl[rank] = NULL;
//    }
//
//    // RoadBlocking Time info
//    if (_rbt_time == NULL) {
//      ds = header._time_table_ds;
//      obj_size = header._timedomain_ds - header._time_table_ds;
//      obj_cnt  = obj_size / sizeof(RoadBlockingTime);
//      if ((_rbt_time = thlib::MemPolicy<RoadBlockingTime>::alloc(obj_cnt)) == NULL) {
//        return -MSG_FAIL_MEMORY_ALLOC;
//      }
//      if (file.read((char*)_rbt_time, ds, obj_size) != obj_size) {
//        return -MSG_FAIL_FILE_READ;
//      }
//      _memsize += static_cast<int>(obj_size);
//    }
//    // TimeDomain
//    if (_rbt_td == NULL) {
//      ds = header._timedomain_ds;
//      obj_size = file.size() - ds;
//      if ((_rbt_td = thlib::MemPolicy<char>::alloc(obj_size)) == NULL) {
//        return -MSG_FAIL_MEMORY_ALLOC;
//      }
//      if (file.read(_rbt_td, ds, obj_size) != obj_size) {
//        return -MSG_FAIL_FILE_READ;
//      }
//      _memsize += static_cast<int>(obj_size);
//    }
//
//    file.close();
//  }
//  return 0;
//}
//
//int MapCntlL::load_roadblocking_vehicle()
//{
//  size_t obj_size = 0, obj_cnt = 0, ds = 0;
//  int rank, idx, next_idx;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_RB_VEHICLE);
//  if (file.open(path, "rb") == 0) {
//    // header
//    TruckRoadBlockingHeader trb_header;
//    obj_size = sizeof(TruckRoadBlockingHeader);
//    if (file.read((char*)&trb_header, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//
//    // DS Table alloc & read
//    uint32_t* ds_tbl[MRRANK_COUNT] = { NULL };
//    for (rank = 0; rank < MRRANK_COUNT; ++rank) {
//      // Object 개수 계산을 위해 실제 Map 보다 1개 더 만든다.
//      ds_tbl[rank] = thlib::MemPolicy<uint32_t>::alloc_zero(_map_cnt[rank] + 1);
//      obj_size = _map_cnt[rank] * sizeof(uint32_t);
//      if (file.read((char*)ds_tbl[rank], obj_size) != obj_size) {
//        _lock.leavecs();
//        return -MSG_FAIL_FILE_READ;
//      }
//    }
//    ds_tbl[MRRANK_FIRST][_map_cnt[MRRANK_FIRST]]   = ds_tbl[MRRANK_SECOND][0];
//    ds_tbl[MRRANK_SECOND][_map_cnt[MRRANK_SECOND]] = ds_tbl[MRRANK_LOWEST][0];
//    ds_tbl[MRRANK_LOWEST][_map_cnt[MRRANK_LOWEST]] = trb_header._weight_table_ds;
//
//    // Index Table alloc & read
//    int linkcnt;
//    for (rank = 0; rank < MRRANK_COUNT; ++rank) {
//      _trb_idx_tbl[rank] = thlib::MemPolicy<TruckRoadBlockingIdx*>::alloc_zero(_map_cnt[rank]); // Map Header
//      _memsize += sizeof(TruckRoadBlockingIdx*) * _map_cnt[rank];
//      for (idx = 0; idx < (int)(_map_cnt[rank]); ++idx) {
//        linkcnt  = (ds_tbl[rank][idx + 1] - ds_tbl[rank][idx]) / sizeof(TruckRoadBlockingIdx);
//        obj_size = sizeof(TruckRoadBlockingIdx) * linkcnt;
//        if (linkcnt != 0) {
//          _trb_idx_tbl[rank][idx] = thlib::MemPolicy<TruckRoadBlockingIdx>::alloc(linkcnt);
//          if (file.read((char*)_trb_idx_tbl[rank][idx], obj_size) != obj_size) {
//            _lock.leavecs();
//            return -MSG_FAIL_FILE_READ;
//          }
//          _memsize += obj_size;
//        }
//      }
//      thlib::MemPolicy<uint32_t>::memfree(ds_tbl[rank]);
//    }
//
//    // weight table
//    if (_trb_weight_tbl == NULL) {
//      ds = trb_header._weight_table_ds;
//      obj_size = trb_header._timedomain_ds - trb_header._weight_table_ds;
//      obj_cnt  = obj_size / sizeof(uint16_t);
//      if ((_trb_weight_tbl = thlib::MemPolicy<uint16_t>::alloc(obj_cnt)) == NULL) {
//        return -MSG_FAIL_MEMORY_ALLOC;
//      }
//      if (file.read((char*)_trb_weight_tbl, ds, obj_size) != obj_size) {
//        return -MSG_FAIL_FILE_READ;
//      }
//      _memsize += static_cast<int>(obj_size);
//    }
//
//    file.close();
//  }
//  return 0;
//}
//
//int MapCntlL::load_special_ratio()
//{
//  uint32_t  obj_size = 0;
//  int i;
//  SpecialRatioHeader header;
//  std::vector<SpecialRatio> ratio;
//  std::vector<SpecialRatioMapInfo> map_info;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_S_RATIO);
//  if (file.open(path, "rb") == 0) {
//    if (file.read((char*)&header, sizeof(header)) != sizeof(header)) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    for (int rank = 0; rank < MRRANK_COUNT; ++rank) {
//      // read map info
//      obj_size = sizeof(SpecialRatioMapInfo) * header._map_cnt[rank];
//	  if (header._map_cnt[rank] > 0) {
//		map_info.resize(header._map_cnt[rank]);
//		if (file.read((char*)&map_info[0], header._map_info_ds[rank], obj_size) != obj_size) {
//		  return -MSG_FAIL_FILE_READ;
//		}
//	  }
//      
//      // read link
//      obj_size = sizeof(SpecialRatio) * header._list_cnt[rank];
//	  if (header._list_cnt[rank] > 0) {
//		ratio.resize(header._list_cnt[rank]);
//		if (file.read((char*)&ratio[0], header._list_ds[rank], obj_size) != obj_size) {
//		  return -MSG_FAIL_FILE_READ;
//		}
//		///////////////////////////////////////////
//		if ((_s_ratio[rank] = thlib::MemPolicy<uint8_t*>::alloc_zero(_map_cnt[rank])) == NULL) {
//		  return -MSG_FAIL_MEMORY_ALLOC;
//		}
//		for (i = 0; i < (int)map_info.size(); ++i) {
//		  if ((_s_ratio[rank][map_info[i]._mapid] = thlib::MemPolicy<uint8_t>::alloc_zero(map_info[i]._link_cnt)) == NULL) {
//			  return -MSG_FAIL_MEMORY_ALLOC;
//		  }
//		}
//		for (i = 0; i < (int)ratio.size(); ++i) {
//		  _s_ratio[rank][ratio[i]._mapid][ratio[i]._linkid] = ratio[i]._ratio;
//		}
//	  }
//    }
//    file.close();
//    ratio.clear();
//    map_info.clear();
//    std::vector<SpecialRatio>().swap(ratio);
//    std::vector<SpecialRatioMapInfo>().swap(map_info);
//  }
//  return 0;
//}
//
//int MapCntlL::load_preference_road()
//{
//  uint32_t obj_size = 0, ds = 0, roadname_cnt = 0;
//  PreferenceRoadRec* road_rec = NULL;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_PREFERENCE_ROAD);
//  if (file.open(path, "rb") == 0) {
//    if (file.read((char*)&roadname_cnt, sizeof(roadname_cnt)) != sizeof(roadname_cnt)) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    if ((road_rec = thlib::MemPolicy<PreferenceRoadRec>::alloc(roadname_cnt)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    ds = sizeof(roadname_cnt);
//    obj_size = sizeof(PreferenceRoadRec) * roadname_cnt;
//    if (file.read((char*)road_rec, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    for (int i = 0; i < (int)roadname_cnt; ++i) {
//      _preference_index.insert(PreferenceRoadMap::value_type(thlib::thString::hashCode(road_rec[i]._road_name), road_rec[i]));
//    }
//    thlib::MemPolicy<PreferenceRoadRec>::memfree(road_rec);
//    road_rec = NULL;
//    // Map/Link table
//    ds = sizeof(roadname_cnt) + obj_size;
//    obj_size = file.size() - ds;
//    if ((_preference_road_list = thlib::MemPolicy<char>::alloc(obj_size)) == NULL) {
//      return -MSG_FAIL_MEMORY_ALLOC;
//    }
//    if (file.read((char*)_preference_road_list, ds, obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    file.close();
//  }
//
//  return 0;
//}
//
//int MapCntlL::load_road_section()
//{
//  uint32_t obj_size = 0;
//  int idx = 0;
//  RoadSectionHeader header;
//  thlib::thFile<> file;
//  thlib::thString path;
//
//  path.printf("%s/%s", _map_path(), F_ROAD_SECTION);
//  if (file.open(path, "rb") == 0) {
//    if (file.read((char*)&header, sizeof(header)) != sizeof(header)) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    _road_section.resize(header._section_cnt);
//    _road_section_link.resize(header._link_cnt);
//    // Section info
//    obj_size = sizeof(RoadSection) * header._section_cnt;
//    if (file.read((char*)&_road_section[0], obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    // Link
//    obj_size = sizeof(hwLink) * header._link_cnt;
//    if (file.read((char*)&_road_section_link[0], obj_size) != obj_size) {
//      return -MSG_FAIL_FILE_READ;
//    }
//    // Make key-idx map
//    for (int i = 0; i < (int)header._section_cnt; ++i) {
//      for (int j = 0; j < _road_section[i]._link_cnt; ++j) {
//        _road_section_key2idx.insert(std::make_pair(_road_section_link[idx].makekey(), i));
//        ++idx;
//      }
//    }
//    file.close();
//  }
//
//  return 0;
//}
//
//void MapCntlL::version(char* map_version)
//{
//  uint32_t year = 0, month = 0, day = 0;
//  year  = _map_version / 10000;
//  month = (_map_version - (year * 10000)) / 100;
//  day   = _map_version - (year * 10000) - (month * 100);
//
//#if defined(WIN32)
//  sprintf_s(map_version, 16, "%d/%s%d/%s%d", year, (month < 10) ? "0" : "", month, (day < 10) ? "0" : "", day);
//#else
//  snprintf(map_version, 16, "%d/%s%d/%s%d", year, (month < 10) ? "0" : "", month, (day < 10) ? "0" : "", day);
//#endif
//}
//
