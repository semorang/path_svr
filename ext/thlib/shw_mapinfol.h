/* copyright <happyteam@thinkware.co.kr> */
#ifndef _SHW_MAPINFOL_H_
#define _SHW_MAPINFOL_H_

#if defined(_WIN32_WCE)
//  #include "thlib/thusewindows.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
//  #include "thlib/thusewindows.h"
  #pragma warning(disable : 4786)
#else
#endif

//#include "shw_define.h"
#include "shw_maprecord.h"
//#include "shw_datast.h"
//#include "thlib/thmapbase.h"
//#include "thlib/thstring.h"
//#include "thlib/thfile.h"
//#include "thlib/threct.h"
//#include "thlib/typelist.h"
//#include "thlib/thlog.h"
//#include "thlib/thlock.h"


/// Type List의 정의
//typedef TYPELIST_4(thNodeRecL, thLinkRecL, thAdjNodeRec, thCrossNodeRecL) CacheListL;
//typedef TYPELIST_9(thNodeRecL, thLinkRecL, thAdjNodeRec, thCrossNodeRecL,
//                   thVertexRec, thLinkGuideL, thLinkExtraL, 
//                   thLinkShapeL, thLinkPositionL) LoDSList;


static const int obj2pst[9][3] = {  
  { 0, 1, 2 },    /// node position
  { 3, 4, 5 },    /// linkrecl position
  { 6, 6, 6 },    /// adjcent node position
  { 7, 8, 9 },    /// cross node position
  { 10, 10, 10 }, /// vertex position
  { 11, 12, 13 }, /// linkguide position
  { 14, 15, 16 }, /// linkextra position
  { 17, 18, 19 }, /// linkshape position
  { 20, 21, 22 }  /// linkposition position
};


//class MapIndexL
//{
//public :
//  uint32_t  _ds_order[23];
//  uint16_t  _cnt_order[3];
//  uint16_t  _umapid;
//  uint16_t  _xratio;  // width of the x-axis in meter
//  ///
//  int    ds_ct;   // 교차로명 테이블까지의 DS
//  int    ds_ext;  // 부가정보
//  thlib::thRect<int> rect;
//
//public :
//  void operator()(thMapRecL& map) {  // NOLINT
//    _ds_order[0] = map._dsnr;
//    _ds_order[1] = map._dsnr + sizeof(thNodeRecL) * map._cntnr[0];
//    _ds_order[2] = map._dsnr + sizeof(thNodeRecL) * map._cntnr[1];
//    /// Add For GNF Format
//    _ds_order[3] = map._dslrec;
//    _ds_order[4] = map._dslrec + sizeof(thLinkRecL) * map._cntlr[0];
//    _ds_order[5] = map._dslrec + sizeof(thLinkRecL) * map._cntlr[1];
//
//    _ds_order[6] = map._dsanr;
//    _ds_order[7] = map._dscnr[0];
//    _ds_order[8] = map._dscnr[1];
//    _ds_order[9] = map._dscnr[2];
//    _ds_order[10] = map._dsvtx;
//
//    _ds_order[11] = map._dslguide;
//    _ds_order[12] = map._dslguide + sizeof(thLinkGuideL) * map._cntlr[0];
//    _ds_order[13] = map._dslguide + sizeof(thLinkGuideL) * map._cntlr[1];
//    _ds_order[14] = map._dslextra;
//    _ds_order[15] = map._dslextra + sizeof(thLinkExtraL) * map._cntlr[0];
//    _ds_order[16] = map._dslextra + sizeof(thLinkExtraL) * map._cntlr[1];
//    _ds_order[17] = map._dslshape;
//    _ds_order[18] = map._dslshape + sizeof(thLinkShapeL) * map._cntlr[0];
//    _ds_order[19] = map._dslshape + sizeof(thLinkShapeL) * map._cntlr[1];
//    _ds_order[20] = map._dslposition;
//    _ds_order[21] = map._dslposition + sizeof(thLinkPositionL) * map._cntlr[0];
//    _ds_order[22] = map._dslposition + sizeof(thLinkPositionL) * map._cntlr[1];
//
//    ///////////////////////////////////////////////////////////////////////////
//    _cnt_order[0] = map._cntnr[2];
//    _cnt_order[1] = map._cntlr[2];
//    _cnt_order[2] = map._cntanr[2];
//    _xratio = map._xratio;
//    _umapid = map._umapid;
//    ds_ct = map._dscname;
//    ds_ext = map._dsexinfo;
//    rect(map._llon, map._tlat, map._rlon, map._blat);
//  }
//  /**
//  * 필히 index, rank의 경계 검사를 꼭 해야 한다. 
//  */
//  size_t operator()(int index, int rank) {
//    return (size_t)_ds_order[obj2pst[index][rank]];
//  }
//};


class MapCntlL
{
public :
  enum { MRRANK_FIRST, MRRANK_SECOND, MRRANK_LOWEST, MRRANK_COUNT };
  enum {
    FNUM_RANK2, FNUM_VERTEX, FNUM_CROSSNAME,  FNUM_REST,
    FNUM_TOLL,  FNUM_EXINFO, FNUM_STREETNAME, FNUM_ROTARY,
    FNUM_COUNT
  };

//public :
//  thlib::thLock     _lock;
//  thlib::thString   _map_path;
//  static MapCntlL*  _instance;
//  MapIndexL*        _map_indexL;
//
public :
  static const char* F_MAPMNG;
//  static const char* F_RANK2;
//  static const char* F_VERTEX;
//  static const char* F_REST;
//  static const char* F_TOLL;
//  static const char* F_METROTOLL;
//  static const char* F_DALINK;
//  static const char* F_RB_TIME;
//  static const char* F_RB_VEHICLE;
//  static const char* F_S_RATIO;
//  static const char* F_PREFERENCE_ROAD;
//  static const char* F_ROAD_SECTION;
//  // 나중에 정리해야 할 것들.
//  static const char* F_EXINFO;
//  static const char* F_CROSSNAME;
//  static const char* F_STREETNAME;
//
//  /**
//   *  @note 기본정보.
//   */
//  uint32_t              _file_size[FNUM_COUNT];
//  uint32_t              _map_version;
//  int                   _memsize;
//  int16_t               _map_cnt[MRRANK_COUNT];
//  int16_t               _last_mapid; // 연산을 줄이기 위해 사용.
//  int8_t                _rank_cnt;
//  int8_t                _is_init;
//
//  /**
//   *  @note 성능을위해 메모리에 올려둔 것들.
//   */
//  thTollHeader          _tollhead;
//  thTollRec*            _toll_open; // open tollgate record
//  thTollEx1Rec*         _tollex1blk;
//  thTollEx2Rec*         _tollex2blk;
//  char*                 _toll_name;
//  thTollTimeDomain*     _toll_td_info;
//  RoadCharge*           _road_charge;     // 도로요금 : 민자 도로별 요금.
//  HiPassException*      _toll_exhipass;
//  char*                 _toll_td;
//
//  TimeRoadBlockingIdx** _rbt_idx[MRRANK_COUNT];
//  RoadBlockingTime*     _rbt_time;
//  char*                 _rbt_td;
//  TruckRoadBlockingIdx** _trb_idx_tbl[MRRANK_COUNT];
//  uint16_t*              _trb_weight_tbl;
//  thRestAreaRec*        _rest_area;
//  uint8_t**             _s_ratio[MRRANK_COUNT];
//  char*                 _preference_road_list; // 선호도로 Link List Block
//  PreferenceRoadMap     _preference_index; // 선호도로 명칭 MultiMap
//  RoadSectionVec        _road_section;
//  hwLinkVec             _road_section_link;
//  hwKeyIdxMap           _road_section_key2idx;
//  int                   _ex_opentoll[3];  // Metro Open Toll : 남산1,3호, 우면산 toll index
//  float                 _pub_highway_base[4]; // 도로요금 : 국영(편도 1, 2, 3↑에 대한 기본요금)
//
//  // 위에 것들 중에 갯수가 필요한 것들.
//  uint16_t              _road_charge_cnt; // 도로요금 : 민자도로 개수.
//  uint16_t              _toll_exhipass_cnt;
//  int                   _rest_area_cnt;
//  
//  thlib::thRect<int>    _total_map_rect; // 전체 지도의 범위 ( 경위도 )
//  
//private :
//  static void destroy_bridge();
//  int load_tollgate();
//  int load_metroopentoll();
//  int load_restarea();
//  int load_dalink();
//  int load_roadblocking_time();
//  int load_roadblocking_vehicle();
//  int load_special_ratio();
//  int load_preference_road();
//  int load_road_section();
//
//public :
//  MapCntlL();
//  ~MapCntlL();
//  static MapCntlL* instance();
//  void destroy();
//  int  init(const char* map_path);
//  void version(char* map_version);
//
//  /**
//  * 객체 복사가 일어 나지 않도록 포인터를 리턴하도록 디자인 함. 
//  * 복잡성이 발생해도 복사되는것 보다는 좋을 것으로 판단. 
//  */
//  MapIndexL* operator()(int mapid) {
//    return &_map_indexL[mapid];
//  }
//
//  // 각종 형의 rank 별 시작 ds를 return 한다. 
//  template<class MRType>
//  size_t ds(MRType& , int mapid, int rank = 0) {
//    int pos = Loki::TL::IndexOf<LoDSList, MRType>::value;
//    return _map_indexL[mapid](pos, rank);
//  }
//  // 객체의 size와 지도 size가 틀릴 경우 사용하지 말것. 
//  template<class MRType>
//  size_t ds(MRType&, int mapid, int id, int rank) {
//    int pos = Loki::TL::IndexOf<LoDSList, MRType>::value;
//    return _map_indexL[mapid](pos, rank) + sizeof(MRType) * id;
//  }
//  template<class MRType>
//  size_t size(MRType& obj, int mapid, int rank) {
//    size_t pos = Loki::TL::IndexOf<LoDSList, MRType>::value;
//    size_t ds1 = ds(obj, mapid, 0);
//    size_t ret_size = 0;
//    if (rank == MRRANK_LOWEST) {
//      if (mapid == _last_mapid) {
//        if (pos == Loki::TL::IndexOf<LoDSList, thCrossNodeRecL>::value)
//          ret_size = _file_size[FNUM_RANK2] - ds1;
//        else
//          ret_size = _map_indexL[mapid]._cnt_order[pos] * sizeof(MRType);
//      } else {
//        ret_size = ds(obj, mapid+1, 0) - ds1;
//      }
//    } else {
//      ret_size = ds(obj, mapid, rank+1) - ds1;
//    }
//    return ret_size;
//  }
//  size_t size(thAdjNodeRec& obj, int mapid, int) {
//    int pos = Loki::TL::IndexOf<LoDSList, thAdjNodeRec>::value;
//    size_t ret_size = 0;
//    //  AdjNode는 랭크별로 따로있지않다. 따라서 최하랭크로 가정
//    if (mapid == _last_mapid) {
//      ret_size = _map_indexL[mapid]._cnt_order[pos] * thAdjNodeRec::size();
//    } else {
//      size_t ds1 = ds(obj, mapid, 0);
//      ret_size = ds(obj, mapid+1, 0) - ds1;
//    }
//    return ret_size;
//  }
//  size_t size(thVertexRec& obj, int mapid, int) {
//    size_t ds1 = ds(obj, mapid, 0);
//    size_t ret_size = 0;
//    //  Vertex는 랭크별로 따로있지않다. 따라서 최하랭크로 가정
//    if (mapid == _last_mapid)
//      ret_size = _file_size[FNUM_VERTEX] - ds1;
//    else
//      ret_size = ds(obj, mapid+1, 0) - ds1;
//    return ret_size;
//  }
//
//  int seekmapid(thlib::thPoint<int>& pt) {
//    int cur, lower = 0, upper = _last_mapid;
//    /* Seek by Bineary Search*/
//    while (true) {
//      if (lower > upper)
//        break;
//      cur = (lower + upper) / 2;
//      if (_map_indexL[cur].rect.isInside(pt)) {
//        return cur;
//      }
//      if (pt._x < _map_indexL[cur].rect._left || 
//        (pt._y < _map_indexL[cur].rect._bottom && 
//        pt._x < _map_indexL[cur].rect._right)) {
//        upper = cur - 1;
//        continue;
//      }
//      if (pt._x >= _map_indexL[cur].rect._right || 
//        (pt._y >= _map_indexL[cur].rect._top && 
//        pt._x >= _map_indexL[cur].rect._left)) {
//        lower = cur + 1;
//        continue;
//      }
//    }
//    /* if search fail, it checks by linear search */
//    for (int i = 0 ; i < _map_cnt[MRRANK_LOWEST]; ++i) {
//      if (_map_indexL[cur].rect.isInside(pt)) {
//        return i;
//      }
//    }
//    return -1;
//  }
//
//  int memsize() {
//    return _memsize;
//  }
//  int mapcnt() {
//    return _map_cnt[MRRANK_LOWEST];
//  }
//  int nodecnt(int mapid) {
//    return _map_indexL[mapid]._cnt_order[0];
//  }
//  int linkcnt(int mapid) {
//    return _map_indexL[mapid]._cnt_order[1];
//  }
//  char* map_path() {
//    return _map_path();
//  }
//  bool is_available_time_roadblocking() {
//    return (_rbt_idx[MRRANK_FIRST] == NULL) ? false : true;
//  }
//  bool is_available_vehicle_roadblocking() {
//    return (_trb_weight_tbl == NULL) ? false : true;
//  }
};

#endif

