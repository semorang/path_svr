/*****************************************************************************
 *      
 *      
 *      CopyRight (c) 2003, Thinkwaresys Designed by cjchoi@thinkwaresys.com
 *****************************************************************************/

#ifndef _MAPRECORDSET_H__
#define _MAPRECORDSET_H__

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #pragma warning(disable : 4786)
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif
///
/// fixed size와 부모 자료 구조는 template 인자가 없고 
/// non-fixed size는 template인자가 있다 
///

#include "shw_define.h"
#include "thtype.h"
#include "thdef.h"
#include "thallocator.h"
#include <stdlib.h>
#include <string.h>
#include <vector>

/**
 *  @brief the header of rank map management record
 */
class thMapHeader {
 public:
  uint16_t rankMngCnt;
  char  VolumStructType[5];
  char  VolumnSerial;
  char  SystemType[32];
  char  VolumType[32];
  char  Author[128];
  char  DataPreParity[128];
  char  AppType[128];
  char  VolumDate[16];
  char  VolumUpadateDate[16];
};
/**
 *  @brief  the information of each rank management
 */
class thRankManager {
 public:
  uint16_t  _rank_num;   ///  rank number
  uint16_t  _map_cnt;    ///  count of map
  int    _dsmr;        ///  ds of map record
};
/**
 *  @brief  the record of rank0,1 map information
 */
class thMapRecU {
 public:
  uint16_t  _umapid;      ///  mapid of upper rank
  uint16_t  _node_cnt;    ///  count of node record
  uint16_t  _link_cnt;    ///  count of link record
  uint16_t  _adj_cnt;     ///  count of adjacent node record
  int    _llon;         ///  longitude of left side
  int    _blat;         ///  latitude of bottom side
  int    _dsnr;         ///  ds of start node record
  int    _dslr;         /// ds of start link base record
  int    _dsanr;        ///  ds of start adjacent node record
  int    _dscnr;        ///  ds of start cross node record
  int    _dscor;        ///  ds of start rank2 correspond record
  uint16_t  _xratio;      ///  ratio of x-axis in meter(not width)
  uint16_t  _reserved;    ///  reserve for future using(for 4 byte align)
};
/**
 *  @brief  the record of rank2 map information
 */
class thMapRecL {
 public:
  int    _llon;        /// left edge longitude of the map
  int    _blat;        /// bottom edge latitude of the map
  int    _rlon;        /// right edge longitude of the map
  int    _tlat;        /// top edge latitude of the map
  int    _dsnr;        /// ds of start node record
  int    _dslrec;      /// ds of start link base record
  int    _dslguide;    /// ds of start link route record
  int    _dslextra;    /// ds of start link extra record
  int    _dslshape;    /// ds of start link shape record
  int    _dslposition; /// ds of start link position record
  int    _dsanr;       /// ds of start adjacent node record
  int    _dscnr[3];    /// ds of start cross node record of each logical rank
  uint16_t  _umapid;     /// mapid of upper rank
  uint16_t  _cntnr[3];   /// node record count of each logical rank
  uint16_t  _cntlr[3];   /// link record count of each logical rank
  uint16_t  _cntanr[3];  /// adjacent node record count of each logical rank
  int    _dsvtx;       /// ds of start vertex record
  int    _dscname;     /// ds of start cross name record
  int    _dsexinfo;    /// ds of start extra information record
  uint16_t  _xratio;     /// width of the x-axis in meter
  uint16_t  _reserved;   /// the space reserved future using
};
/**
 *  @brief  the node record of rank0,1
 */
class thNodeRecU {
 public:
  uint32_t  _x;            /// longitude of the node
  uint32_t  _y;            /// latitude of the node
  uint16_t  _unodeid;    /// upper node id
  uint16_t  _reserved;   /// reserved
  int    _en_ds;       /// the number of adjacent links + displacement
                       /// to extended node record of the node (1/3 bytes)
 public:
  bool  isadjnode() const { return (_en_ds < 0); }
  bool  existup() const { return (_en_ds & 0x40000000) == 0x40000000; }
  int    jclink() const { return (_en_ds & 0x3F000000) >> 24; }
  int    ds() const { return _en_ds & 0x00FFFFFF; }
  int    unodeid() const { return _unodeid; }
};
/**
 *  @brief  the node record of rank2
 */
class thNodeRecL {
 public:
  uint16_t _x; // offset of longitude of the node from the left edge of the map
  uint16_t _y; // offset of latitude of the node from the bottom edge of the map
  uint16_t _unodeid;     /// upper node id
  uint16_t _reserved;    /// reserved
  int _en_ds;          /// the number of adjacent links + displacement
                       /// to extended node record of the node (1/3 bytes)
 public:
  bool isadjnode() const { return (_en_ds < 0); }
  bool existup() const { return (_en_ds & 0x40000000) >> 30 == 1; }
  int  jclink() const { return (_en_ds & 0x3F000000) >> 24; }
  int  ds() const { return _en_ds & 0x00FFFFFF; }
  int  unodeid() const { return _unodeid; }
};
/**
 *  @brief  the adjacent node record of rank0,1,2
 */
class thAdjNodeRec {
 public:
  uint16_t _adj_mapid;      /// adjacent mapid 
  uint16_t _adj_nodeid;     /// adjacent nodeid
  uint16_t _adj_linkid;     /// adjacent linkid

  bool operator == (const thAdjNodeRec &rhs) const {
    if (_adj_mapid == rhs._adj_mapid &&
        _adj_linkid == rhs._adj_linkid &&
        _adj_nodeid == rhs._adj_nodeid) {
      return true;
    }
    return false;
  }

  static size_t size() { return 6; }
};
/**
 *  @brief  the cross node record of rank0,1
 */
#define MR_MAXCROSS    8
#define MR_MAXCRGUIDE  64
class thCrossNodeRecU {
 public:
  uint16_t _conlink[MR_MAXCROSS];     /// link id connected to cross node
  uint8_t _conpasscode[MR_MAXCROSS];  /// pass code connected to cross node
  uint8_t _guidecode[MR_MAXCRGUIDE];  /// guide code connected to cross node
  /// channelization code for each guide code
  uint8_t _channelization[MR_MAXCRGUIDE]; 
 public:
  void clear() {
    memset((char*)_conlink, 0xff, sizeof(uint16_t) * MR_MAXCROSS);
    memset((char*)_conpasscode, 0, sizeof(uint8_t) * MR_MAXCROSS);
    memset((char*)_guidecode, 0, sizeof(uint8_t) * MR_MAXCRGUIDE);
    memset((char*)_channelization, 0, sizeof(uint8_t) * MR_MAXCRGUIDE);
  }
  size_t size(size_t jclink) {
    if (jclink > 1)
      return ((jclink * 2) + jclink + 2 * (jclink * jclink));
    else
      return ((jclink * 2) + jclink);
  }
  size_t guidesize(size_t jclink) { return (jclink > 1)?jclink*jclink:0; }
  int linkpos(int nextlinkid) {
    for (int i = 0; i < MR_MAXCROSS; ++i) {
      if (_conlink[i] == nextlinkid) return i;
    }
    return -1;
  }
  void copyitem(char* buf, size_t jclink) {
    clear();
    memcpy(&_conlink, &(buf[0]), 2 * jclink);
    memcpy(&_conpasscode, &(buf[2 * jclink]), jclink);
    if (jclink > 1) {
      /// 2 * jclink + jclink
      memcpy(&_guidecode, &(buf[3 * jclink]), jclink * jclink);
      memcpy(&_channelization, &(buf[3 * jclink + jclink * jclink]), 
             jclink * jclink);
    }
  }
  void tostream(char* buf, size_t jclink) {
    memcpy(&(buf[0]), &_conlink, 2 * jclink);
    memcpy(&(buf[2 * jclink]), &_conpasscode, jclink);
    if (jclink > 1) {
      /// 2 * jclink + jclink
      memcpy(&(buf[3 * jclink]), &_guidecode, jclink * jclink);
      memcpy(&(buf[3 * jclink + jclink * jclink]), &_channelization, 
             jclink * jclink);
    }
  }
  /// Guide Code
  int guide_code(int pos) {
    return _guidecode[pos] & 0x3f;
  }
  void guide_code(int pos, int g_code) {
    _guidecode[pos] = (_guidecode[pos] & 0xc0) | (g_code & 0x3f);
  }
  /// 0 : 일반교차 1 : 비신호교차 2 : 신호교차
  int traffic_light_type(int pos) {
    return _guidecode[pos] >> 6;
  }
  /// 0 : 도로화 되지 않은 도로 1 : 도류화된 도로 
  int is_channelization(int pos) const {
    return _channelization[pos] >> 7;
  }
  /// 0 : 공용차로 1 : 전용차로
  int is_exclusive_lane(int pos) const {
    return (_channelization[pos] & 0x4f) >> 6;
  }
};

/// 교차로 신호등 Type
enum __TrafficLightType {
  TTYPE_NORMAL = 0,  ///< 일반 교차로
  TTYPE_NO_SIGNAL,  ///< 비신호 교차로
  TTYPE_SIGNAL,    ///< 비신호 교차로
  TTYPE_CNT
};

/// 도류화 Type
enum __ChannelizationType {
  CTYPE_NO_CHANNELIZATION = 0,  ///< 도류화 되지 않은 도로
  CTYPE_CHANNELIZATION = 1,    ///< 도류화 된 도로
  CTYPE_CNT
};

/// 차선 Type
enum __LaneType {
  LTYPE_PUBLIC_LANE  = 0,      ///< 공용차로
  LTYPE_PRIVATE_LANE = 1,      ///< 전용차로
  LTYPE_CNT
};

/// road spec
enum __RoadSpec {
  HP_UNKNOWN      = 0,
  HP_UNDIVIDED    = 1,
  HP_DIVIDED      = 2,
  HP_JUNCTION     = 3,
  HP_INNER_LINK   = 4,
  HP_RAMP         = 5,
  HP_SERVICE_AREA = 6,
  HP_ROADSPEC_CNT = 7
};

/// road type
enum __RoadType {
  HP_FERRY        = 0,
  HP_HIGHWAY      = 1,
  HP_URBANWAY     = 2,
  HP_NATIONALWAY  = 3,
  HP_LOCALWAY     = 4,
  HP_STREETWAY    = 5,
  HP_CARONEYWAY   = 6,
  HP_ROUTE_UNABLE = 7,
  HP_ROADTYPE_CNT = 8
};

/// road pass
enum __RoadPass {
  HP_NORMAL_PASS      = 0,
  HP_ELEVATED_PASS    = 1,
  HP_UNDER_PASS       = 2,
  HP_TUNNAL_PASS      = 3,
  HP_BRIGE_PASS       = 4,
  HP_PRIVATE_PASS     = 5,
  HP_METRO_BRIGE_PASS = 6,
  HP_ROADPASS_CNT     = 7
};

/// one way
enum __OneWayType {
  HP_NO_ONEWAY      = 0,
  HP_F_ONEWAY       = 1,
  HP_B_ONEWAY       = 2,
  HP_NOPASSING      = 3,
  HP_ONEWAYTYPE_CNT = 4
};

// Vehicle Restriction Type
enum __VehicleRestrictionType {
  HP_NORESTRICTION = 0,  // 무제한
  HP_CAR_ONLY      = 1,  // 자동차 전용
  HP_SEDAN_ONLY    = 2,  // 승용차 전용
  HP_MOTORCYCLE_RESTRICTION = 4
};

/**
 *  @brief  the cross node record of rank2
 *
 */
class thCrossNodeRecL {
 public:
  typedef thCrossNodeRecL RecType;

  int _ds_ct;                      /// size and ds of cross name record
  uint16_t _conlink[MR_MAXCROSS];    /// link id connected to cross node
  uint8_t _conpasscode[MR_MAXCROSS]; /// pass code connected to cross node
  uint8_t _guidecode[MR_MAXCRGUIDE]; /// guide code connected to cross node
  /// channelization code for each guide code
  uint8_t _channelization[MR_MAXCRGUIDE];
 public:
  void clear() {
    memset((char*)&_ds_ct, 0, sizeof(_ds_ct));
    memset((char*)_conlink, 0xff, sizeof(uint16_t) * MR_MAXCROSS);
    memset((char*)_conpasscode, 0, sizeof(uint8_t) * MR_MAXCROSS);
    memset((char*)_guidecode, 0, sizeof(uint8_t) * MR_MAXCRGUIDE);
    memset((char*)_channelization, 0, sizeof(uint8_t) * MR_MAXCRGUIDE);
  }
  int name_size() const { return (_ds_ct >> 24) ; }
  int name_ds() const { return _ds_ct & 0x00FFFFFF; }
  int size(int jclink) {
    if (jclink > 1)
      return (4 + (jclink * 2) + jclink + 2 * (jclink * jclink));
    else
      return (4 + (jclink * 2) + jclink);
  }
  int guidesize(int jclink) { return (jclink > 1)?jclink*jclink:0; }
  int linkpos(int nextlinkid) {
    for (int i = 0; i < MR_MAXCROSS; ++i) {
      if (_conlink[i] == nextlinkid) return i;
    }
    return -1;
  }
  void copyitem(char* buf, int jclink) {
    clear();
    memcpy(&_ds_ct, buf, sizeof(int));
    memcpy(&_conlink, &(buf[4]), 2 * jclink);
    memcpy(&_conpasscode, &(buf[4 + 2 * jclink]), jclink);
    if (jclink > 1) {
      memcpy(&_guidecode, &(buf[4 + 3 * jclink]), jclink * jclink);
      memcpy(&_channelization, &(buf[4 + 3 * jclink + jclink * jclink]), 
          jclink * jclink);
    }
  }
  void tostream(char* buf, int jclink) {
    memcpy(buf, &_ds_ct, sizeof(int));
    memcpy(&(buf[4]), &_conlink, 2 * jclink);
    memcpy(&(buf[4 + 2 * jclink]), &_conpasscode, jclink);
    if (jclink > 1) {
      memcpy(&(buf[4 + 3 * jclink]), &_guidecode, jclink * jclink);
      memcpy(&(buf[4 + 3 * jclink + jclink * jclink]), &_channelization, 
          jclink * jclink);
    }
  }
  /// Guide Code
  int guide_code(int pos) {
    return _guidecode[pos] & 0x3f;
  }
  void guide_code(int pos, int g_code) {
    _guidecode[pos] = (_guidecode[pos] & 0xc0) | (g_code & 0x3f);
  }
  /// 0 : 일반교차 1 : 비신호교차 2 : 신호교차
  int traffic_light_type(int pos) {
    return _guidecode[pos] >> 6;
  }
  /// 0 : 도류화 되지 않은 도로 1 : 도류화된 도로 
  int is_channelization(int pos) const {
    return _channelization[pos] >> 7;
  }
  /// 0 : 공용차로 1 : 전용차로
  int is_exclusive_lane(int pos) const {
    return (_channelization[pos] & 0x4f) >> 6;
  }
};
/**
 * static information about how to divide the level into rank
 * level 0, 1    -> rank 0
 * level 2, 3, 4 -> rank 0
 * level 5, 6    -> rank 1
 * level 7, 8, 9 -> rank 2
 * rest..........-> rank 2
 */
const int level2rank[16] = {
  /* 0 */ 0, /* 1 */ 0, /* 2 */ 0, /* 3 */ 0,
  /* 4 */ 0, /* 5 */ 1, /* 6 */ 1, /* 7 */ 2,
  /* 8 */ 2, /* 9 */ 2, /* 0 */ 2, /* 1 */ 2,
  /* 2 */ 2, /* 3 */ 2, /* 4 */ 2, /* 5 */ 2
};
/**
 *  @brief  the link record of rank2
 *  G1, M31 version
 */
///  _high_dir information
#define  RS_COSTINFO    1  //  유/무료 정보
///  _exinfo_ds information
#define  RS_TOLLCLOSE  0  //  폐쇄형 요금소
#define  RS_TOLLOPEN    1  //  개방형 요금소

/**
 *  @brief  the link record of rank0,1
 */
/// Link Base For GNF Format
class thLinkRecU {
 public:
  uint16_t _startnodeid;  /// start node id
  uint16_t _endnodeid;    /// end node id
  uint16_t _length;       /// length of link
  uint16_t _attribute;    /// attribute of link
  uint16_t _upperlinkid;  /// link id of matching upper ranl link
  uint16_t _roadnum;      /// private highway(1) / road number(15)
  uint16_t _trf_light;    /// char -> int16_t (8) forward / (8) backward)
  uint16_t _speedhump;    /// OpenType Tollgate Index(10) / count of speed hump(6)
  uint8_t  _level;        /// rank level(4) / cost level(4)
  uint8_t  _sf;           /// 차량제한(1)/오토바이제한(1)/자동차전용(2)/scale factor(4)
  uint8_t  _crosswalk;    /// count of cross walk
  uint8_t  _selinkpos;    /// 시간대별통제(1)/다링크회전제한(1)/시작노드연결Position(3)/종료노드연결Position(3)
  uint32_t _inexinfo;     /// extra information (= dsexinfo)
  uint32_t _lminfo;       /// link match information (= offcor)
  uint8_t  _road_function;/// 진출입가능(1)/Curve등급(2)/도로급수(5)
  uint8_t  _road_info;    /// 어린이보호구역(2)
  uint16_t _trf_nolight;  /// forward(8 bits) count, backward(8 bits) count

 public:
  int lane() const { return ( _attribute & 0xF000) >> 12; }
  int is_payroad() const { return ( _attribute & 0x0800) >> 11; }
  int road_pass() const { return ( _attribute & 0x0700) >> 8; }
  int road_type() const { return ( _attribute & 0x00E0) >> 5; }
  int road_spec() const { return ( _attribute & 0x001C) >> 2; }
  int one_way() const { return ( _attribute & 0x0003); }
  int level() const { return _level >> 4; }

  int vehicle_restriction()    { return  _sf >> 7; }
  int motorcycle_restriction() { return (_sf >> 6) & 0x01; }
  int road_restriction()       { return (_sf >> 4) & 0x07; }
  int car_only()               { return (_sf >> 4) & 0x03; } // @caution !!!
  int sf()                     { return  _sf & 0x0F; }

  int time_road_blocking() { return  _selinkpos >> 7; }
  int dalink()             { return (_selinkpos >> 6) & 0x01; }
  int slink_pos()          { return (_selinkpos >> 3) & 0x07; }
  int elink_pos()          { return  _selinkpos & 0x07; }

  int rank() const { return level2rank[level()]; }
  uint32_t map_count() const  { return (_lminfo >> 28); }
  uint32_t link_count() const { return (_lminfo & 0xFF00000) >> 20; }
  uint32_t match_ds() const   { return (_lminfo & 0xFFFFF); }
  uint32_t match_cnt(int rank) const { 
    if (rank == 0) 
      return (_lminfo >> 28);
    else 
      return (_lminfo & 0xFF00000) >> 20;  
  }
  void set_map_count(int count, int /*rank*/) {
    _lminfo = _lminfo & 0x0FFFFFFF;
    _lminfo = _lminfo | (count << 28);
  }
  void set_link_count(int count, int /*rank*/) {
    _lminfo = _lminfo & 0xF00FFFFF;
    _lminfo = _lminfo | (count << 20);
  }
  void set_match_ds(int ds, int /*rank*/) {
    _lminfo = _lminfo & 0xFFF00000;
    _lminfo = _lminfo | ds;
  }
  int tollgate_type() const { return (_inexinfo >> 30); }
  int tollgate_idx()  const { return (_inexinfo & 0x3FFFFFFF) >> 20; }
  int is_tollbooth()  const { return (_inexinfo != 0xffffffff); }
  int is_open_tollgate() const {
    return (_inexinfo >> 30) == RS_TOLLOPEN ? 1/*true*/ : 0/*false*/;
  }
  int road_num() { return _roadnum & 0x7FFF; }
  int is_private_highway() { return _roadnum >> 15; }
  int crosswalk() const { return _crosswalk; }
  int speed_hump() const { return _speedhump & 0x003F; }
  int traffic_light(int dir) const {
    return dir == 0 ? (_trf_light >> 8) : (_trf_light & 0x00ff);
  }
  int opentollindex() const {
    return (_speedhump >> 6) & 0x03FF;
  }
  int enter_with_restriction() const { return _road_function >> 7; }
  int curve_level()            const { return (_road_function >> 5) & 0x03; }
  int is_schoolzone()          const { return _road_info & 0x03; }
  int traffic_nolight(int dir) const {
    return dir == 0 ? (_trf_nolight >> 8) : (_trf_nolight & 0x00ff); 
  }
};
/**
 * @brief the Link Record of Rank2 
 */
class thLinkRecL {
 public:
  enum dir { MRLR_DIR_BOTH, MRLR_DIR_FOR, MRLR_DIR_REV };
  //22.08.26 INTERSECTION_DISTANCE_UNIT = 10, Onda_PM(정재우 수석)과 협의하여 결정된 값
  enum intersection{LANE_WIDTH = 3};
 public:
  uint16_t _startnodeid;  ///< Start Node ID 
  uint16_t _endnodeid;    ///< End Node ID
  uint16_t _length;       ///< Length of Link
  uint16_t _attribute;    ///< Attribute of Link
  uint16_t _upperlinkid;  ///< Upper Matching Link ID
  uint16_t _roadnum;      ///< Private Highway Road(1)/Road Number(15)
  uint16_t _opentollindex;///< Open Type Tollgate Index 
  uint8_t  _level;        ///< Level Of Link(Rank(4)/Cost(4)) 
  uint8_t  _sf;           ///< 차량제한(1)/오토바이제한(1)/자동차전용(2)/scale factor(4)
  uint8_t  _selinkpos;    ///< 시간대별통제(1)/다링크회전제한(1)/시작노드연결Position(3)/종료노드연결Position(3)
  uint8_t  _crosswalk;    ///< Crosswalk Count/Speed Humo Count (4/4)
  uint8_t  _road_function;///< 진출입가능(1)/Curve등급(2)/도로급수(5)
  uint8_t  _highwaydata;  ///< Express Data 
 public:
#if 0
  /// Don't make a constructor function !! - ccj
  thLinkRecL(): _startnodeid(0), _endnodeid(0), _length(0), 
        _attribute(0), _upperlinkid(0xFFFF), _roadnum(0),
        _opentollindex(0), _level(0), _sf(0), 
        _selinkpos(0), _crosswalk(0), _road_function(0),
        _highwaydata(0)
        {}
#endif
  thLinkRecL() {}
  thLinkRecL(const thLinkRecL &src) {
    if (this != &src) {
      _startnodeid = src._startnodeid;
      _endnodeid = src._endnodeid;
      _length = src._length;
      _attribute = src._attribute;
      _upperlinkid = src._upperlinkid;
      _roadnum = src._roadnum;
      _opentollindex = src._opentollindex;
      _level = src._level;
      _sf = src._sf;
      _selinkpos = src._selinkpos;
      _crosswalk = src._crosswalk;
      _road_function = src._road_function;
      _highwaydata = src._highwaydata;
    }
  }
  thLinkRecL& operator = (const thLinkRecL &src) {
    if (this != &src) {
      _startnodeid = src._startnodeid;
      _endnodeid = src._endnodeid;
      _length = src._length;
      _attribute = src._attribute;
      _upperlinkid = src._upperlinkid;
      _roadnum = src._roadnum;
      _opentollindex = src._opentollindex;
      _level = src._level;
      _sf = src._sf;
      _selinkpos = src._selinkpos;
      _crosswalk = src._crosswalk;
      _road_function = src._road_function;
      _highwaydata = src._highwaydata;
    }
    return *this;
  }
  void start_nodeid(uint16_t value) { _startnodeid = value; }
  int start_nodeid() { return _startnodeid; }
  void end_nodeid(uint16_t value) { _endnodeid = value; }
  int end_nodeid() { return _endnodeid; }
  void length(uint16_t value) { _length = value; }
  int length() { return _length; }
  void attribute(uint16_t value) { _attribute = value; }
  int attribute() { return _attribute; }
  int lane() const { return (_attribute & 0xF000) >> 12; }
  int is_payroad() { return (_attribute & 0x0800) >> 11; }
  int road_pass() const { return (_attribute & 0x0700) >> 8; }
  int road_type() const { return (_attribute & 0x00E0) >> 5; }
  int road_spec() const { return (_attribute & 0x001C) >> 2; }
  int one_way() const { return (_attribute & 0x0003); }
  int is_rotary() const { return (_highwaydata & 0x08) >> 3; }
  void upper_linkid(uint16_t value) { _upperlinkid = value; }
  int upper_linkid() { return _upperlinkid; }
  int  rank() const { return level2rank[level()]; }
  void level(uint8_t value) { _level = value; }
  /// @todo : If use the cost Level, Must Change
  /// return _level;
  int level() const { return _level >> 4; }
  void rank_level(uint8_t value) { _level |= (value << 4) & 0xF0; }
  int rank_level() { return _level >> 4; }
  void cost_level(uint8_t value) { _level |= value & 0x0F; }
  int cost_level() { return _level & 0x0F; }
  void highway_data(uint16_t value) { _highwaydata = (uint8_t)value; }
  int highway_data() { return _highwaydata; }
  int highway_type() { return (_highwaydata >> 5); }
  int is_tollbooth() { return ((_highwaydata & 0x10) >> 4); }
  int slatype() { return (_highwaydata & 0x07); }

  int  vehicle_restriction()           { return  _sf >> 7; }
  int  motorcycle_restriction()        { return (_sf >> 6) & 0x01; }
  int  road_restriction()              { return (_sf >> 4) & 0x07; }
  int  car_only()                      { return (_sf >> 4) & 0x03; } // @caution !!!
  int  sf()                            { return  _sf & 0x0F; }
  void road_restriction(uint8_t value) { _sf = (_sf & 0x8F) | ((value & 0x07) << 4); }
  void sf(uint8_t value)               { _sf = (_sf & 0xF0) | (value & 0x0F); }

  int  time_road_blocking()     { return  _selinkpos >> 7; }
  int  dalink()                 { return (_selinkpos >> 6) & 0x01; }
  int  slink_pos()              { return (_selinkpos >> 3) & 0x07; }
  int  elink_pos()              { return  _selinkpos & 0x07; }
  void slink_pos(uint8_t value) { _selinkpos |= value << 3; }
  void elink_pos(uint8_t value) { _selinkpos |= value; }

/*
  int flexible_turn(int dir) {
    const int tmp[] = {0x80, 0x40};
    return (_selinkpos & tmp[dir]) == 0 ? 0 : 1;
  }
*/
  void crosswalk(uint8_t value) {
    _crosswalk &= 0xF0;
    _crosswalk |= (value << 4) & 0xF0;
  }
  uint8_t crosswalk() { return (_crosswalk >> 4) & 0x0F; }
  void speed_hump(uint8_t value) {
    _crosswalk &= 0xF0;
    _crosswalk |= value & 0x0F;
  }
  int speed_hump() { return _crosswalk & 0x0F; }
/* 이거.. 고정된 값이나까 입력받을 필요 없잔수?
  void road_num(uint16_t value) {
    _roadnum &= 0x8000;
    _roadnum |= value & 0x7FFF; 
  }
*/
  int road_num() { return _roadnum & 0x7FFF; }
/* 요거도 마찬가지...
  void private_highway(uint8_t value) {
    _roadnum &= 0x7FFF;
    _roadnum |= (value << 15) & 0x8000; 
  }
*/
  int is_private_highway() { return (int)(_roadnum >> 15); }
  int speed_limit_area() { return ((_highwaydata >> 1) & 0x03); }
  int tollgate_type() { 
    return ((_opentollindex & 0x03ff) != 0x03ff) ? RS_TOLLOPEN : RS_TOLLCLOSE;
  }
  int is_open_tollgate() { 
    return ((_opentollindex & 0x03ff) != 0x03ff) ? 1/*true*/ : 0/*false*/;
  }
  int protectedArea() { return ((_opentollindex >> 12) & 0x03); }
  void open_tollgate_idx(uint16_t value) { _opentollindex = value; }
  int open_tollgate_idx() const { return _opentollindex & 0x03FF; }
  int enter_restriction_info() const { return (_road_function >> 3) & 0x03; }
  int enter_with_restriction() const { return _road_function >> 7; }
  int curve_level()            const { return (_road_function >> 5) & 0x03; }

 public:
  enum EXINFO_TYPE { LANE = 1, SIGN, STREET, TOLLGATE, STREETNAMEFORSEARCH, 
    THREE_D_ZOOMVIEW, EXIT_ORDER_INFO, 
    F_ALTITUDE = 9, B_ALTITUDE = 10};
};

///< Link Guide of Rank 2
class thLinkGuideL {
  uint16_t  _extrainfo; // Extra Information Bit Field
  uint16_t  _group_id;

 public:
  thLinkGuideL(): _extrainfo(0), _group_id(0xffff) {}
  thLinkGuideL(const thLinkGuideL &src) {
    if (this != &src) {
      _extrainfo  = src._extrainfo;
      _group_id   = src._group_id;
    }
  }
  thLinkGuideL& operator = (const thLinkGuideL &src) {
    if (this != &src) {
      _extrainfo  = src._extrainfo;
      _group_id   = src._group_id;
    }
    return *this;
  }
  void extrainfo(uint16_t value) {
    _extrainfo = value;
  }
  int extrainfo() {
    return _extrainfo;
  }
  ///< if exinfo data exists then isexinfo_exst() more than 1..
  int isexinfo() { /// = isexinfo_exst();
    return ((_extrainfo & 0xfffc) >> 2); 
  }
  bool isextratype(int type) {  
    // 해당 타입의 부가정보가 있는가?
    if (type > 10 && type < 1) {
      //("thLinkRecL::isThisType()에서 부가정보 없는 타입!!");
      return false;
    }
    int seq = 16 - type; // 16bits에서 자릿수
    int ret = _extrainfo >> seq;
    return ((ret & 1) == 1);
  }
  int isbuslane() {
    return (_extrainfo >> 5) & 0x0001;
  }
  uint16_t groupid() {
    return _group_id;
  }
  void groupid(uint16_t gid) {
    _group_id = gid;
  }
};
/**
 * Link Extra of Rank 2
 */
class thLinkExtraL {
  uint32_t  _dsexinfo;    ///< DS of Extra Information
 public:
#if 0
  thLinkExtraL(): _dsexinfo(0) {}
#endif
  thLinkExtraL() {}
  thLinkExtraL(const thLinkExtraL &src) {
    if (this != &src) {
      _dsexinfo  = src._dsexinfo;
    }
  }
  thLinkExtraL& operator = (const thLinkExtraL &src) {
    if (this != &src) {
      _dsexinfo  = src._dsexinfo;
    }
    return *this;
  }
  void dsexinfo(uint32_t value) {
    _dsexinfo = value;
  }
  int dsexinfo() {
    return _dsexinfo;
  }
  void exInfods(uint32_t value) {
    _dsexinfo |= value;
  }
  void exInfosize(uint32_t value) {
    _dsexinfo = value << 22;
  }
  int exinfosize() { 
    return (_dsexinfo >> 22);
  }
  int exinfods() { 
    return (_dsexinfo & 0x3FFFFF); 
  }
  int tollgate_type() {
    return _dsexinfo >> 30;
  }
  int tollgate_idx() {
    return (_dsexinfo & 0x3FFFFFFF) >> 20; 
  }
  void tollgate_type(uint32_t value) {
    _dsexinfo = (_dsexinfo & 0x3FFFFFFF) | (value << 30);
  }
  void tollgate_idx(uint32_t value) {
    _dsexinfo = (_dsexinfo & 0xC0000000) | (value << 20); 
  }
  int tollgate_name_ds() {
    return _dsexinfo & 0xFFFFF; 
  }
};
/**
 * Link Shape of Rank 2
 */
class thLinkShapeL {
 public:
  uint16_t  _fwvtxcnt;    ///< Forward Vertex Count
  uint16_t  _fwvtxidx;    ///< Forward Vertex Index of Vertex List
  uint16_t  _bwvtxcnt;    ///< Backward Vertex Count
  uint16_t  _bwvtxidx;    ///< Backward Vertex Index of Vertex List
 public:
#if 0
  thLinkShapeL(): _fwvtxcnt(0), _fwvtxidx(0),
          _bwvtxcnt(0), _bwvtxidx(0)
          {}
#endif
  thLinkShapeL() {}
  thLinkShapeL(const thLinkShapeL &src) {
    if (this != &src) {
      _fwvtxcnt  = src._fwvtxcnt;
      _fwvtxidx  = src._fwvtxidx;
      _bwvtxcnt  = src._bwvtxcnt;
      _bwvtxidx  = src._bwvtxidx;
    }
  }
  thLinkShapeL& operator = (const thLinkShapeL &src) {
    if (this != &src) {
      _fwvtxcnt  = src._fwvtxcnt;
      _fwvtxidx  = src._fwvtxidx;
      _bwvtxcnt  = src._bwvtxcnt;
      _bwvtxidx  = src._bwvtxidx;
    }
    return *this;
  }
  void init() {
    _fwvtxcnt = 0;
    _fwvtxidx = 0;
    _bwvtxcnt = 0;
    _bwvtxidx = 0;
  }
  void fwvtxcnt(uint16_t value) { _fwvtxcnt = value; }
  int fwvtxcnt() { return _fwvtxcnt; }
  void fwvtxidx(uint16_t value) { _fwvtxidx = value; }
  int fwvtxidx() { return _fwvtxidx; }
  void bwvtxcnt(uint16_t value) { _bwvtxcnt = value; }
  int bwvtxcnt() { return _bwvtxcnt; }
  void bwvtxidx(uint16_t value) { _bwvtxidx = value; }
  int bwvtxidx() { return _bwvtxidx; }
};
/**
 * Link Position of Rank 2
 */
class thLinkPositionL {
  uint8_t  _qtleft;      ///< QuadTree left
  uint8_t  _qttop;       ///< QuadTree top
  uint8_t  _qtright;     ///< QuadTree right
  uint8_t  _qtbottom;    ///< QuadTree bottom
  uint16_t _qtid;        ///< QuadTree Node ID
  uint16_t _reserved;  
 public:
#if 0
  thLinkPositionL() : _qtleft(0), _qttop(0), _qtright(0), 
            _qtbottom(0), _qtid(0), _reserved(0) 
  {}
#endif
  thLinkPositionL() {}
  thLinkPositionL(const thLinkPositionL &src) {
    if (this != &src) {
      _qtleft = src._qtleft;
      _qttop = src._qttop;
      _qtright = src._qtright;
      _qtbottom = src._qtbottom;
      _qtid = src._qtid;
      _reserved = src._reserved;
    }
  }
  thLinkPositionL& operator = (const thLinkPositionL &src) {
    if (this != &src) {
      _qtleft = src._qtleft;
      _qttop = src._qttop;
      _qtright = src._qtright;
      _qtbottom = src._qtbottom;
      _qtid = src._qtid;
      _reserved = src._reserved;
    }
    return *this;
  }
  void qtleft(uint8_t value) { _qtleft = value; }
  int qtleft() { return _qtleft; }
  void qttop(uint8_t value) { _qttop = value; }
  int qttop() { return _qttop; }
  void qtright(uint8_t value) { _qtright = value; }
  int qtright() { return _qtright; }
  void qtbottom(uint8_t value) { _qtbottom = value; }
  int qtbottom() { return _qtbottom; }
  void qtid(uint16_t value) { _qtid = value; }
  int qtid() { return _qtid; }
};

/**
 *  @brief  the vertex record
 */
class thVertexRec {
 public:
  uint16_t  _x;
  uint16_t  _y;
};

typedef std::vector<thVertexRec, thallocator<thVertexRec> >  thVertexVec;

/**
 *  @brief  the class of extent of link (it is calculated dynamically)
 *          this extent is used only low rank link
 */
class thLinkExtent {
 public:
#if 0
  thLinkExtent() : _lextent(-1), _textent(-1), _rextent(-1), _bextent(-1)  {}
#endif
  thLinkExtent() {}
  ~thLinkExtent() {}
  void init(const thLinkShapeL& ls, thNodeRecL& snr,
            thNodeRecL& enr, thVertexRec* pvl) {
    // start node를 이용하여 extent 초기화
    _lextent = _rextent = snr._x;
    _textent = _bextent = snr._y;

    // end node 점검
    uint16_t lon, lat;
    lon = enr._x;
    lat = enr._y;
    if (lon < _lextent)
      _lextent = lon;
    if (lon > _rextent)
      _rextent = lon;
    if (lat < _bextent)
      _bextent = lat;
    if (lat > _textent)
      _textent = lat;

    // vertex list 점검
    if (pvl != NULL) {
      for (int i = 0; i < ls._fwvtxcnt; ++i) {
        lon = pvl[i]._x;
        lat = pvl[i]._y;
        if (lon < _lextent)
          _lextent = lon;
        if (lon > _rextent)
          _rextent = lon;
        if (lat < _bextent)
          _bextent = lat;
        if (lat > _textent)
          _textent = lat;
      }
    }
  }
// jihyun을 위한 함수. 
#if defined(WIN32)
  // 추가, 상판뷰어용 노드는 이미 보간점리스트에 포함되어 있다.
  void init2(const int _cnt_vtx, POINT* pvl) {
    // start vertex를 이용하여 extent 초기화
    _lextent = _rextent = pvl[0].x;
    _textent = _bextent = pvl[0].y;
    
    int lon, lat;
    // vertex list만 점검
    if (pvl != NULL) {
      // _cnt_vtx는 ReadVertexList()호출시 구함
      for (int i = 1; i < _cnt_vtx; ++i) {
        lon = pvl[i].x;
        lat = pvl[i].y;
        if (lon < _lextent)
          _lextent = lon;
        if (lon > _rextent)
          _rextent = lon;
        if (lat < _bextent)
          _bextent = lat;
        if (lat > _textent)
          _textent = lat;
      }
    }
  }
#endif

  int lextent()  { return _lextent; }
  int textent()  { return _textent; }
  int rextent()  { return _rextent; }
  int bextent()  { return _bextent; }

 private:
  int  _lextent;
  int  _textent;
  int  _rextent;
  int  _bextent;
};
/**
 *  class Mt2MrMatch
 *  @brief  manage matched data between MT and MR link
 */
class Mt2MrMatch {
public:
  class Rec {
  public:
    uint32_t _mt_mapid;
    uint16_t _mt_snodeid;
    uint16_t _mt_enodeid;
    uint16_t _mr_mapid;
    uint16_t _mr_linkid;
  };

  class MTRec {
  public:
    uint32_t _mapid;
    uint16_t _snodeid;
    uint16_t _enodeid;
  };

  class MRRec {
  public:
    uint16_t _mapid;
    uint16_t _linkid;
  };
};
/**
 *  class PhLinkMatch
 *  @brief  manage correspondent data between upper link and lower link
 *  @brief  this struct is previous structure
 */
class PhLinkMatch {
 public:
  class LMRec {
   public:
    uint16_t _mapid;
    uint16_t _linkid;
  };

 public:
  PhLinkMatch() {}
  ~PhLinkMatch()  { destroy(); }
 public:
  bool copyitem(char* data, size_t count) {
    if (data == NULL) {
      return false;
    }
    destroy();
    _lmrec.reserve(count);
    _lmrec.resize(count);
    memcpy(&_lmrec[0], data, sizeof(LMRec) * count);
    return true;
  }
  void destroy() {
    _lmrec.clear();
  }
  int g_count() {
    return (int)_lmrec.size();
  }
  bool savebin(char* buff) {
    if (_lmrec.size() == 0) {
      return false;
    }
    memcpy(buff, reinterpret_cast<char*>(&_lmrec[0])
        , binsize(0xffffffff));
    return true;
  }
  size_t binsize(size_t cnt) {
    ///  if size_t is not uint32_t, how do process this?
    if (cnt == 0xffffffff)
      return (size_t)(sizeof(LMRec) * _lmrec.size());
    else
      return (size_t)(sizeof(LMRec) * cnt);
  }
 public:
  std::vector<LMRec, thallocator<LMRec> > _lmrec;
};
/**
 *  class PhUpLinkMatch
 *  @brief  manage correspondent data between upper link and lower link
 *  @brief  this data doesn't match 4byte alignment, so it's items have to be read each other.
 */
class PhUpLinkMatch {
 public:
  class LVMRec {
   public:
#if 1 
    LVMRec() : _linkcnt(0), _vertexcnt(0), _linkds(0),
        _vertexds(0), _mapid(0xffff), _slinkid(0xffff)  {}
#endif
#if 0
    LVMRec() {}
#endif
   public:
    void init() {
      _linkcnt = 0;
      _vertexcnt = 0;
      _linkds = 0;
      _vertexds = 0;
      _mapid = 0xffff;
      _slinkid = 0xffff;
    }
    int linkcount()    { return _linkcnt; }
    int linkds()    { return _linkds; }
    int vertexcount()  { return _vertexcnt; }
    int vertexds()    { return _vertexds; }
    int mapid()  { return _mapid; }
    int slinkid()  { return _slinkid; }
   private:
    uint16_t  _linkcnt;
    uint16_t  _vertexcnt;
    uint32_t  _linkds;
    uint32_t  _vertexds;
    uint16_t  _mapid;
    uint16_t  _slinkid;
  };

 public:
  PhUpLinkMatch() : _lastmap(-1)  {}
  ~PhUpLinkMatch()  { destroy(); }
 public:
  bool copyitem(char* data, size_t mapcnt) {
    if (data == NULL) {
      return false;
    }
    destroy();
    _lvmrec.reserve(mapcnt);
    _lvmrec.resize(mapcnt);
    memcpy(&_lvmrec[0], data, sizeof(LVMRec) * mapcnt);
    return true;
  }
  void destroy() {
    _lvmrec.clear();
    _lastmap = -1;
  }
  size_t binsize(size_t cnt) {
    ///  if size_t is not uint32_t, how do process this?
    if (cnt == 0xffffffff)
      return (size_t)(sizeof(LVMRec) * _lvmrec.size());
    else
      return (size_t)(sizeof(LVMRec) * cnt);
  }
  bool savebin(char* buff) {
    if (_lvmrec.size() == 0) {
      return false;
    }
    memcpy(buff, reinterpret_cast<char*>(&_lvmrec[0])
        , binsize(0xffffffff));
    return true;
  }

 public:
  std::vector<LVMRec, thallocator<LVMRec> >  _lvmrec;
  int  _lastmap;
};

class thGuideRec {
 public:
  enum code { GDE_DEST = 60 };
 public:
  char _code; 
};

class thAltitude {
 public:
  // 고저정보 (접속링크순서/고저코드/거리1/거리2 = 3/5/12/12 bits)
  uint32_t _data;

 public:
  int getOrder() const { return (_data & 0xE0000000) >> 29; }
  int getAltitudeCode() const { return (_data & 0x1F000000) >> 24; }
  int getLen1() const { return (_data & 0xFFF000) >> 12; }
  int getLen2() const { return _data & 0xFFF; }
};

// by soooa, 부가정보 레코드 추가
// @TODO 나중엔 모든 타입을 읽기..
class thExInfoRec {
 public: // LANE 차선정보, SIGN 방면정보, ALTITUDE 고저정보...
  enum EXINFO_TYPE { 
    F_LANE, B_LANE, 
    F_SIGN, B_SIGN, 
    STREET, 
    TOLLGATE, 
    F_STREETNAMEFORSEARCH, 
    B_STREETNAMEFORSEARCH, 
    F_THREE_D_ZOOMVIEW, B_THREE_D_ZOOMVIEW, 
    EXIT_ORDER_INFO, 
    F_ALTITUDE, 
    B_ALTITUDE
  };

 public:
  char _id;
  char _cnt;
  thAltitude _data[10]; // 고저정보 배열
};

class thTollHeader {
 public:
  uint16_t _closecnt;             // 폐쇄형 요금소 개수
  uint16_t _opencnt;              // 개방형 요금소 개수
  uint16_t _ex1cnt;               // 예외1형 레코드 개수
  uint16_t _ex2cnt;               // 예외2형 레코드 개수
  uint32_t _toll_name_ds;         // tollgate name start ds
  uint32_t _td_charge_ratio_ds;   // 시간대별 요금정보 DS
  uint32_t _unitrate_ds;          // Unit Rate Table Start DS
  uint32_t _ex_hipass_ds;         // 예외형 Hi-Pass 요금소 테이블 DS
  uint32_t _timedomain_block_ds;  // Time Domain Block Start DS
};

/**
 *  use this class when read normal tollgate cost record
 */
#define TOLL_MAXCOSTCNT       6
#define TOLL_MAXCOSTCNT_EX    7
#define TD_BLOCK_SIZE         16
#define TD_MAX_CNT            3
#define TD_MAX_LENGTH         128
#define TD_CHARGE_RATIO_BASE  1000000.0f    // toll table에 들어있는 % 값
#define TD_CHARGE_RATIO       100000000.0f  // 탐색시 사용하기 위한 비율 값

class thTollRec
{
public :
  uint8_t   _exinfo;
  uint8_t   _resv1;
  uint8_t   _resv2;
  uint8_t   _timedomain_idx[TD_MAX_CNT];
  uint16_t  _tollcost[TOLL_MAXCOSTCNT_EX]; // 차종요금 + 1종 주말요금

  void clear() {
    _exinfo = _resv1 = _resv2 = 0;
    _timedomain_idx[0] = 0xff;
    _timedomain_idx[1] = 0xff;
    _timedomain_idx[2] = 0xff;
    memset(_tollcost, 0x00, sizeof(_tollcost));
  }
  // 부가정보.
  int is_exception_toll()   { return  _exinfo & 0x01; }
  int is_lower_20km()       { return (_exinfo >> 1) & 0x01; }
  int is_hipass_discount()  { return (_exinfo >> 2) & 0x01; }
  int is_hipass_only_in()   { return (_exinfo >> 3) & 0x01; }
  int is_hipass_only_out()  { return (_exinfo >> 4) & 0x01; }
  int is_virtual_toll()     { return (_exinfo >> 5) & 0x01; }
  int is_one_tolling()      { return (_exinfo >> 6) & 0x01; }
  int is_undecided_charge() { return (_exinfo >> 7) & 0x01; }

  int timedomain_index(int pos) {
    return (int)_timedomain_idx[pos];
  }
  int timedomain_count() {
    int cnt = 0;
    if (_timedomain_idx[0] != 0xff) ++cnt;
    else return cnt;
    if (_timedomain_idx[1] != 0xff) ++cnt;
    else return cnt;
    if (_timedomain_idx[2] != 0xff) ++cnt;
    return cnt;
  }
};

class thTollEx1Rec {
 public:
  thTollEx1Rec() {
    init();
  }
  ~thTollEx1Rec() {}
  void init() {
    _tollcnt = 0;
    int i;
    for (i = 0; i < 5; ++i)
      _tollindex[i] = 0xffff;
    for (i = 0; i < TOLL_MAXCOSTCNT; ++i)
      _tollexcost[i] = 0;
  }
 public:
  uint16_t _tollcnt;       ///  연속 예외 조건 톨게이트 개수
  uint16_t _tollindex[5];  ///  예외 조건 톨게이트 인덱스(최종부터 차례대로...)
  int16_t _tollexcost[TOLL_MAXCOSTCNT];  ///  각 차종별 요금(환불일 수도 있음
};

class thTollEx2Rec {
 public:
  thTollEx2Rec() {}
  ~thTollEx2Rec() {}
  void init() {
    memset(_ex2name, 0, 16);
    _upnextidx = 0xffff;
    _upinidx = 0xffff;
    _dnnextidx = 0xffff;
    _dninidx = 0xffff;
  }
 public:
  char  _ex2name[16];      ///  Unique한 예외2 명칭
  uint16_t  _upnextidx;      ///  다음에 만나는 상행 요금소 인덱스
  uint16_t  _upinidx;        ///  진입으로 적용할 예외 요금소 인덱스
  uint16_t  _dnnextidx;      ///  다음에 만나는 하행 요금소 인덱스
  uint16_t  _dninidx;        ///  진입으로 적용할 에외 요금소 인덱스
};
/**
 *  @brief  시간대별 요금 정보
 */
typedef struct __thTollTimeDomain {
  uint32_t  _charge_ratio[TOLL_MAXCOSTCNT]; // 차종별 요금 할인율
  uint16_t  _timedomain_block_idx;
  uint8_t   _timedomain_len;
  uint8_t   _resv;
} thTollTimeDomain;
/**
 *  @brief  도로 번호별 요금 정보
 */
typedef struct __RoadCharge {
  uint16_t  _road_num;      // 도로 번호
  uint16_t  _base_charge;   // 기본 요금
  float     _charge_per_m;  // m당 요금
} RoadCharge;
/**
 *  @brief  하이패스 예외형 요금 레코드
 */
typedef struct __HiPassException {
  uint8_t   _timedomain_idx[2];
  uint16_t  _index[5];
  uint16_t  _discount_cost[TOLL_MAXCOSTCNT];
} HiPassException;
/**
 *  @brief  휴게소/분기점 정보 레코드
 */
class thRestAreaRec {
 public:
  uint16_t  mapid;      //휴게소/분기점가 있는 map id
  uint16_t  linkid;      //휴게소/분기점으로 들어 가는 링크 ID
  int16_t  RA_IDX;      //휴게소/분기점 DB의 인덱스 
  char  drct;      //0x00(일방), 0x01(정방향), 0x02(역방향)
  char  HighWayDir;    // 0 없음, 1 : 고속, 2: 방면 정보 
};

/**
 *  @brief  Time RoadBlocking Header
 */
typedef struct __TimeRoadBlockingHeader {
  uint32_t  _ds_table_ds[RANK_COUNT];
  uint32_t  _time_table_ds;
  uint32_t  _timedomain_ds;
} TimeRoadBlockingHeader;

typedef uint16_t TimeRoadBlockingIdx;
/**
 *  @brief  RoadBlocking infomation per time
 */
typedef struct __RoadBlockingTime {
  uint16_t  _timedomain_idx;
  uint8_t   _timedomain_len;
  uint8_t   _p_link_cnt;
  uint16_t  _p_link[MR_MAXCROSS];
} RoadBlockingTime;

/**
 *  @brief  Truck RoadBlocking Header
 */
typedef struct __TruckRoadBlockingHeader {
  uint32_t  _ds_table_ds[RANK_COUNT];
  uint32_t  _weight_table_ds;
  uint32_t  _timedomain_ds;
} TruckRoadBlockingHeader;

/**
 *  @brief  Truck RoadBlocking Index
 */
typedef struct __TruckRoadBlockingIdx {
  uint8_t   _height_idx;
  uint8_t   _weight_idx;
  uint8_t   _exinfo; // resv(7), 상수원보호구역진입제한(1)
  uint8_t   _time_idx;

  __TruckRoadBlockingIdx() {
    _height_idx = _weight_idx = _time_idx = 0xff;
    _exinfo = 0;
  }
  int is_water_protection_restriction() {
    return _exinfo & 0x01;
  }
  int is_drive_truck_restriction() {
    return _exinfo & 0x02;
  }
  int opposite_lane() {
    return (_exinfo >> 4) & 0x0F;
  }
} TruckRoadBlockingIdx;
#if 0
/**
 *  @brief  Truck RoadBlocking infomation per time
 *  @note   차후 사용을 위해 미리 정의 해 둠.
 */
typedef struct __TruckRoadBlockingTime {
  uint16_t  _timedomain_idx;
  uint8_t   _timedomain_len;
} TruckRoadBlockingTime;
#endif

/**
 *  @brief  SpecialRatio Header
 */
typedef struct __SpecialRatioHeader {
  uint32_t _list_cnt[RANK_COUNT]; // 컨텐츠 개수.
  uint32_t _map_cnt[RANK_COUNT]; // 유효 도엽 개수.
  uint32_t _map_info_ds[RANK_COUNT]; // 유요도엽정보 DS
  uint32_t _list_ds[RANK_COUNT];
} SpecialRatioHeader;
/**
 *  @brief  SpecialRatio Map Info
 */
typedef struct __SpecialRatioMapInfo {
  uint16_t  _mapid;
  uint16_t  _link_cnt;
} SpecialRatioMapInfo;
/**
 *  @brief  SpecialRatio
 */
typedef struct __SpecialRatio {
  uint16_t  _mapid;
  uint16_t  _linkid;
  uint8_t   _ratio;
  uint8_t   _resv[3];
} SpecialRatio;

/**
 *  @brief  Preference Road Record
 */
typedef struct __PreferenceRoadRec {
  uint32_t  _offset;
  uint16_t  _link_cnt[RANK_COUNT];
  uint16_t  _exinfo;
  char      _road_name[40];
  int road_type() { return _exinfo & 0x07; }
} PreferenceRoadRec;

/**
 *  @brief  Road Section Header
 */
typedef struct __RoadSectionHeader {
  uint32_t _section_ds;
  uint32_t _section_cnt;
  uint32_t _link_ds;
  uint32_t _link_cnt;
} RoadSectionHeader;

/**
 *  @brief  Road Section Info
 */
typedef struct __RoadSection {
  char      _road_name[40];
  char      _start_section_name[40];
  char      _end_section_name[40];
  uint32_t  _index;     // Link Table에서의 index
  uint16_t  _link_cnt;
  uint16_t  _road_num;  // 도로번호
  uint32_t  _length;    // 구간 길이
  uint32_t  _extinfo;   // 부가정보 : 종별(3bit)

  int  road_type()            { return _extinfo & 0x07; }
  void road_type(uint32_t in) { _extinfo = (_extinfo & 0xfffffff8) | (in & 0x07); }
} RoadSection;
typedef std::vector<RoadSection, thallocator<RoadSection> > RoadSectionVec;

/** *********************************************
 *  @brief  Auout Debug infomation
 *  *********************************************
 */
class ExpInfo {
 public:
  ///  SS_ : the type before matching link list
  ///  SSR_ : the type after matching link list
  enum { SS_FORRK0, SS_FORRK1, SS_FORRK2,
        SS_BACKRK0, SS_BACKRK1, SS_BACKRK2,
        SSR_FORRK0, SSR_FORRK1, SSR_FORRK2,
        SSR_BACKRK0, SSR_BACKRK1, SSR_BACKRK2 };
 public:
  ExpInfo() : _mapid(-1), _linkid(-1), _stype(-1), _reserved(-1) {}
  ExpInfo(int mapid, int linkid, int stype, int reserved) {
    _mapid = mapid;
    _linkid = linkid;
    _stype = stype;
    _reserved = reserved;
  }
  ~ExpInfo()  {}

 public:
  int16_t _mapid;
  int16_t _linkid;
  int16_t _stype;
  int16_t _reserved;
};

#endif
