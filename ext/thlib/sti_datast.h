/* copyright <happyteam@thinkware.co.kr> */
#ifndef _STI_DATAST_H_
#define _STI_DATAST_H_

#include "thusewindows.h"
#include "thtime.h"
#include "thstring.h"
#include "thfile.h"

#if defined(WIN32) || defined(_WIN32_WCE)
#include <tchar.h>
#endif
#include <vector>
#include <map>

#include "sti_define.h"
#include "sti_maprecord.h"
#include "ttl_maprecord.h"
#include "s_errormsg.h"

#if 0
// write traffic shared memory data &
// tpge mw message
#ifndef _STI_WS_
#define _STI_WS_
#endif
#endif
#if 0
#ifndef GIS_VERIFY
#define GIS_VERIFY
#endif
#endif

static thlib::thString g_pnm_traffic = "thtraffic";

#if defined(_WIN32_WCE) || defined(WIN32)
/// inavi <-> happyway_win
#define TRAFFIC_SHAREDMEM_STATUS    _T("iNavi_sharedMem_traffic_status")
#define TRAFFIC_SHAREDMEM_CTT        _T("iNavi_sharedMem_traffic_ctt")
#define TRAFFIC_SHAREDMEM_SCTT      _T("iNavi_sharedMem_traffic_static")
#define TRAFFIC_SHAREDMEM_RTM       _T("iNavi_sharedMem_traffic_rtm")
// for affect link by jmpark129
#define TRAFFIC_SHAREDMEM_AFFECT    _T("iNavi_sharedMem_traffic_affect")
/////////////////////////////////////////////////////////////////////////
#define TRAFFIC_SHAREDMEM_NWS       _T("iNavi_sharedMem_traffic_nws")
#define TRAFFIC_SHAREDMEM_TCON_CTT  _T("iNavi_sharedMem_tcon_ctt")
#define TRAFFIC_SHAREDMEM_TCON_RTM  _T("iNavi_sharedMem_tcon_rtm")
#define TRAFFIC_MUTEX_STATUS        _T("MTX_HW_INAVI_STATUS")
#define TRAFFIC_MUTEX_CTT           _T("MTX_HW_INAVI_CTT")
#define TRAFFIC_MUTEX_SCTT          _T("MTX_HW_INAVI_SCTT")
#define TRAFFIC_MUTEX_RTM           _T("MTX_HW_INAVI_RTM")
// for affect link by jmpark129
#define TRAFFIC_MUTEX_AFFECT        _T("MTX_HW_INAVI_AFFECT")
/////////////////////////////////////////////////////////////
#define TRAFFIC_MUTEX_NWS           _T("MTX_HW_INAVI_NWS")
#endif
/// tpeg mw <-> happyway_win
#define MNBT_SM_CTT_NAME            _T("TWTPEG_CTT")
#define MNBT_SM_CTTSUM_NAME         _T("TWTPEG_CTTSUM")
#define MNBT_SM_RTM_NAME            _T("TWTPEG_RTM")
#define MNBT_SM_NWS_NAME            _T("TWTPEG_NWS")
#define MNBT_SM_STATUS_NAME         _T("TWTPEG_STATUS")

#define MNBT_SM_CTT_SIZE            500000
#define MNBT_SM_CTTSUM_SIZE         200000
#define MNBT_SM_RTM_SIZE            100000
#define MNBT_SM_NWS_SIZE            100000
#define MNBT_SM_STATUS_SIZE         1000
#define TCON_SM_SIZE                1000000

/// log name.
//static thlib::thString g_pnm_sti_mgr  ="tpeg_test";

///////////////////////////////////////////////////////////////////////////////
// 서비스 데이터 정의 
///////////////////////////////////////////////////////////////////////////////
/**
 * 교통정보 TPEG 요청 정보.
 * @note 외부에서 초기화하는 경우는 어떤경우일까???
 */
class trTpegReq {
 public:
  //enum SM_NUM { SM_CTT = 0, SM_RTM, SM_NWS, SM_STATUS };
  enum AUTH_CODE { APPROVED_YTN_AUTH = 0x65, /// 101
    APPROVED_MBC_AUTH, 
    APPROVED_KBS_AUTH, 
    APPROVED_SBS_AUTH
  };
  enum BROAD_TYPE    { BR_YTN = 0x01, BR_MBC = 0x02, 
    BR_KBS = 0x03, BR_SBS = 0x04, BR_MAX = 0x05 };
  
  enum DEVICE_TYPE { 
    COMMON_DEV = 0, 
    SMART_ES_DEV, 
    STAR_DEV, 
    RESERVED1_DEV, 
    X7_DEV 
  };  
  enum SM_SIZE_ORDER { SM_CTT = 0, SM_CTTSUM, SM_RTM, SM_NWS };

  trTpegReq(): _device_type(0), _broad_type(0x02), 
               _auth_code(0x00), _reserved(0x00) {
    memset(_dll_name, 0x00, sizeof(_dll_name));
    _sm_size[0] = _sm_size[1] = _sm_size[2] = _sm_size[3] = 0;
  }
  ~trTpegReq() {}
  /**
   * 요청 데이터값 체크.
   */
  bool valid_param() {
    if (_device_type > X7_DEV) {
      return false;
    }
    if (_broad_type < BR_YTN || _broad_type >= BR_MAX) {
      return false;
    }
#if defined(_WIN32_WCE) || defined(WIN32)
    if (_sm_size[0] != 0 && _sm_size[0] < MNBT_SM_CTT_SIZE) {
      return false;
    }
    if (_sm_size[1] != 0 && _sm_size[1] < MNBT_SM_CTTSUM_SIZE) {
      return false;
    }
    if (_sm_size[2] != 0 && _sm_size[2] < MNBT_SM_RTM_SIZE) {
      return false;
    }
    if (_sm_size[3] != 0 && _sm_size[3] < MNBT_SM_NWS_SIZE) {
      return false;
    }
#endif
  }
  /**
   * 인증 값 체크.
   */
  bool is_authorization() {
    if (_broad_type == BR_YTN && 
        _auth_code == APPROVED_YTN_AUTH) {
      return true;
    } else if (_broad_type == BR_MBC && 
               _auth_code == APPROVED_MBC_AUTH) {
      return true;
    } else if (_broad_type == BR_KBS && 
      _auth_code == APPROVED_KBS_AUTH) {
      return true;
    } else if (_broad_type == BR_SBS && 
      _auth_code == APPROVED_SBS_AUTH) {
      return true;
    }    
    return false;
  }
 public:
  uint8_t    _device_type;      ///< device type
  uint8_t    _broad_type;      ///< 송출사 type
  uint8_t    _auth_code;      ///< 인증여부. 인증 (200) 그외(0).
  uint8_t    _reserved;  
  char      _dll_name[40];
  uint32_t      _sm_size[4];        ///< CTT, CTTSUM, RTM, NWS사이즈.
};

/**
 * IDIO 교통정보 실행시 필요한 정보.(Serial port에 대해서만)
 * GPS combo 수신기 관련 부분 추가 필요. 
 * (combo 수신기의 경우 shared mem으로 데이터 전달)
 *
class trIDIOReq {
 public:
  enum TYPE { TY_DARC_COM = 0, TY_DARCGPS_COM, TY_DARC_LOG, TY_DARCGPS_LOG };
 public:
  trIDIOReq() {
    memset(_comport, 0x00, sizeof(_comport));
    _budrate  = 0;
    _type    = TY_DARC_COM;
    _is_atlas_gps = 0;
    _is_vitas_comm_exception = 0;
    memset(_darc_logfile, 0x00, sizeof(_darc_logfile));
  }
  ~trIDIOReq() {}
 public:
#if defined(WIN32) || defined(_WIN32_WCE)
  TCHAR    _comport[8];        ///< com port string
#else 
  char    _comport[8];        ///< com port string
#endif
  int      _budrate;          ///< 전송속도?
  uint8_t    _type;            ///< 0: DARC COM, 1: DARC + GPS COMBO 2: DARC LOG
  // 아틀라스 단말의 DARC port와 GPS port구분을 휘해
  uint8_t    _is_atlas_gps;
  // VITAS 단말의 경우는 BaudRate, ByteSize, StopBits, Parity 셋팅을 하지않는다.
  uint8_t    _is_vitas_comm_exception;  
  /// 하게 되는 경우, 단말이 멈춰 버리는 증상이 보고되어 있음
  char    _reserved;
  char    _darc_logfile[128];      ///< darc log file name
}; */

/**
 * CDMA 교통정보 실행시 필요한 정보.
 * T-CON 서비스 방식으로 변경.
 */
class trCDMAReq {
 public:
  enum LEN_DEF {
    MAX_RAS_PATH = 260
  };

  trCDMAReq() {
    clear();
  }

  void clear() {
    _type      = 1;
    _map_ver    = 0;
    _res_ver    = 0;
    _mid      = 0;
    _is_netspot    = false;
    memset(_ip, 0x00, sizeof(_ip));
    _port      = 0;
    _compensated_time 
            = 0;
    memset(_serial, 0x00, sizeof(_serial));
    memset(_ras_entry_name, 0x00, sizeof(_ras_entry_name));
    memset(_ras_user_name, 0x00, sizeof(_ras_user_name));
    memset(_ras_passwd, 0x00, sizeof(_ras_passwd));
  }

  bool valid_param() {
    return true;
  }

 public:
  int      _type;        ///< 1: CDMA, 0: TCON
  int      _map_ver;      ///< 지도 버젼.
  int      _res_ver;      ///< cdma resource version
  int      _mid;        ///< 전화번호: 01187654321 => 1187654321
  bool    _is_netspot;    ///< 넷스팟 여부. (1: true, 0: false)
  char    _ip[128];      ///< 교통정보 server ip
  int      _port;        ///< 교통정보 server port
  int      _compensated_time;  ///< 단말별로 시간 계산 관련 하여...
  char    _serial[20];    ///< SD 인증 serial number. 필요없을지도..;;
#if defined(WIN32) || defined(_WIN32_WCE)
  TCHAR    _ras_entry_name[MAX_RAS_PATH];  ///< RAS INFO
  TCHAR    _ras_user_name[MAX_RAS_PATH];  
  TCHAR    _ras_passwd[MAX_RAS_PATH];
#else 
  char    _ras_entry_name[MAX_RAS_PATH];  ///< RAS INFO
  char    _ras_user_name[MAX_RAS_PATH];  
  char    _ras_passwd[MAX_RAS_PATH];  
#endif
}; 

/**
 * 실시간 교통정보 기본 정보. 
 */
class trBaseInfo {
 public:
  enum SERVICE_TYPE {
    TR_TYPE_IDIO = 0,  /* IDIO */
    TR_TYPE_REAL,        /* CDMA */
    TR_TYPE_TPEG,        /* TPEG */
    TR_TYPE_TCON,        /* TCON */
    TR_TYPE_NOT_USED = 255  /* NOT USED */
  };
  enum SUB_TYPE {
    ST_NOT_USE = 0, 
    ST_GPS_LOG_USE = 1
  };
  enum UPDATE_TIME {  
    BASE_UPDATE_TIME = 30    /* thread sleep time: 30sec */
  };

  trBaseInfo() {
    _tr_type    = TR_TYPE_NOT_USED;
    _sub_type    = ST_NOT_USE;
    _update_time  = BASE_UPDATE_TIME;
    _reserved    = 0;
    _ctt_idx_cache_size = 0;
  }

 public:
  uint8_t     _tr_type;        ///< 실시간 교통정보 타입. 
  uint8_t     _sub_type;        ///< 위 note내용 참조. 
  uint8_t     _update_time;    ///< 업데이트 주기. (second) 
  uint8_t     _reserved;
  uint32_t    _ctt_idx_cache_size;      ///< ctt index cache memory size 
};

/**
 * 교통정보 실행시 필요한 Request정보.
 */
class trafficReqInfo {
 public:  
  enum ERROR_DEF {
    ERR_TYPE = 1, 
    ERR_SUBTYPE, 
    ERR_UPTIME, 
    ERR_REROUTETIME, 
    ERR_DIR_PATH
  };
  enum TRFFIC_TYPE { TT_NOT_USED = 0, TT_REAL, TT_STATIC, 
                    TT_REAL_STATIC, TT_MAX };
 public:
  trafficReqInfo() {  
    _type = 0;
    _mapdir = _savedir = NULL;
  }
  ~trafficReqInfo() {}

  int get_broad_type() {
    if (_type != TT_STATIC) {
      if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_TPEG) {
        return _tpeg_info._broad_type;
      } else {
        return trTpegReq::BR_MAX;
      } 
    } else {
      return trTpegReq::BR_MAX;
    }
  }

 /**
  * 해당 교통정보에 맞는 service param을 리턴.
  */
  char* get_service_param() {
    if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_IDIO) {
     //return (char*)&_idio_info;
      return NULL;
    } else if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_REAL ||
               _baseinfo._tr_type == trBaseInfo::TR_TYPE_TCON) {
      //return (char*)&_cdma_info;
      return NULL;
    } else if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_TPEG) {
      return (char*)&_tpeg_info;
    } else {
      return NULL;
    }
  }

  /**
   * check input
   */
  int check_param() {
    if (_type <= 0 || _type > TT_REAL_STATIC) {
      return -ERR_TYPE;
    }

    if (_type == TT_REAL_STATIC) { /// 통계가 TPEG ID체계인지 체크.
      if (_baseinfo._tr_type != trBaseInfo::TR_TYPE_TPEG &&
          _baseinfo._tr_type != trBaseInfo::TR_TYPE_TCON) {
        return -ERR_TYPE;
      }
    }
   
    if (_type == TT_REAL || _type == TT_REAL_STATIC) {
      if (_baseinfo._tr_type > trBaseInfo::TR_TYPE_TCON) {
        return -ERR_TYPE;
      }
      //if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_IDIO) {
      //  return -ERR_TYPE; /// idio 선택시 error!
      //}
      /// 음수 또는 8분 이상의 값.
      if (_baseinfo._update_time > trBaseInfo::BASE_UPDATE_TIME * 8) {
        return -ERR_UPTIME;
      } 
    }
 
    if (_mapdir == NULL || _savedir == NULL) {
      return -ERR_DIR_PATH;
    }

    return 0;
  }
  
  /**
   * 요청 stream을 포맷에 채우기. 
   */
  int todata(char* buf) {
    int off = 0;

    memcpy(&_type, buf+off, sizeof(_type));
    off += sizeof(_type);

    if (_type == TT_REAL || _type == TT_REAL_STATIC) {
      memcpy(&_baseinfo, buf + off, sizeof(_baseinfo));
      off += sizeof(_baseinfo);

      if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_TPEG) {
        memcpy(&_tpeg_info , buf + off, sizeof(_tpeg_info));
        off += sizeof(_tpeg_info);
      } 
    }
    return off;
  }

  /**
   * 포맷의 내용을 stream으로 
   */
  int tostream(char* buf) {
    int off = 0;

    memcpy(buf + off, &_type, sizeof(_type));
    off += sizeof(_type);

    if (_type == TT_REAL || _type == TT_REAL_STATIC) {
      memcpy(buf + off, &_baseinfo, sizeof(_baseinfo));
      off += sizeof(_baseinfo);

      if (_baseinfo._tr_type == trBaseInfo::TR_TYPE_TPEG) {
        memcpy(buf + off , &_tpeg_info, sizeof(_tpeg_info));
        off += sizeof(_tpeg_info);
      } 
    }
    return off;
  }
 public:
  /* shared memory로 받을 데이터 */ 
  int         _type;          ///< 교통정보 사용 타입.  
  trBaseInfo  _baseinfo;       ///< 실시간 교통정보 기본 정보. 
  trTpegReq   _tpeg_info;     ///< 실시간 TPEG 정보.  
  /* 여기 아래 data는 전달받는 메시지 아님 */ 
  char*       _mapdir;       ///< mapdir  
  char*       _savedir;       ///< debug용으로 stream 저장 path.  
};

/**
 * 공통 status 값.
 * @note 
 * idio : Frequency, receiver rate, receiver type(company type으로!)
 * cdma : update time에 따른 status (원래 score로 표기함.)
 * (3: 도시 + 고속,  2: 도시, 1: 고속, 0: 데이터 없음)
 * tcon : 
 * tpeg : status(1:good, 2:poor, 3:off), percent
 * 
 * ext_info[0]: cdma metro update time, 
 * ext_info[1]: cdma high update time, 
 */
class trStatus {
 public:
  trStatus() {
    clear();
  }
  ~trStatus() { }
  void clear() {
    _status  = 0;
    _percent  = 0;
    _ext_info[0] = _ext_info[1] = 0;
  }
 public:
  int16_t    _status;  ///< status  (TPEG, darc: frequency, cdma)
  int16_t    _percent;  ///< (tpeg, idio: receive rate)
  ///< darc(0:receive version) tpeg read status(ctt, cttsum / rtm, nws)
  int    _ext_info[2];
};

/**
 * 상태 정보.
 */
class stiSharedStatus {
 public:
  enum REGION_CNT { REGION_CNT = 16};
  enum READ_ITEM { CTT_READ = 0, 
    CTTSUM_READ, RTM_READ, NWS_READ, 
    READ_ITEM_CNT };
  stiSharedStatus& operator=(const stiSharedStatus& lhs) {
    _status = lhs._status;
    _percent = lhs._percent;
    _region_cnt = lhs._region_cnt;
    _region_percent = lhs._region_percent;
    int i;
    for (i = 0; i < REGION_CNT; ++i) {
      _region_status[i] = lhs._region_status[i];
    }
    for (i = 0; i < READ_ITEM_CNT; ++i) {
      _item_status[i] = lhs._item_status[i]; 
    }
    return *this;
  }
  void clear() {
    _status = 0;
    _percent = 0;
    _region_cnt = 0;
    _region_percent = 0;
    memset(_region_status, 0x00, sizeof(_region_status));
    memset(_item_status, 0x00, sizeof(_region_status));
  }
 public:
  uint8_t _status;
  uint8_t _percent;           ///< percent
  uint8_t _region_cnt;        ///< region cnt
  uint8_t _region_percent;
  uint8_t _region_status[REGION_CNT]; ///< 권역별 수신현황.
  uint8_t _item_status[READ_ITEM_CNT];
};

/**
 * CTT 교통정보 공유메모리
 */
class stiSharedCTTInfo {
 public:
  enum { MIN_CTT_REGION_PERCENT = 30 };
 public:
  stiSharedCTTInfo() {
    clear();
  }
  void clear() {
    _update_time = 0;
    _ctt_blk_cnt = 0;
    _rt_type = 0;
    _route_start_percent = 0;
    _b_high_blk_sidx = 0;
    _b_na_blk_sidx = 0;
    _b_nor_blk_sidx = 0;
    _nb_high_blk_sidx = 0;
    _nb_na_blk_sidx = 0;
    _nb_nor_blk_sidx = 0;
  }
 public:
  unsigned int  _update_time;         ///< 최근 만료처리 시간.(expire)
  uint16_t      _ctt_blk_cnt;         ///< CTT 총 Block 시간
  uint8_t       _rt_type;             ///< 실시간 교통정보 타입.
  uint8_t       _route_start_percent; ///< 탐색 시작 Flag셋팅하는 수신율 기준.
  uint16_t      _b_high_blk_sidx;     ///< 송출 고속블록 시작 인덱스.
  uint16_t      _b_na_blk_sidx;       ///< 송출 국도블록 시작 인덱스.
  uint16_t      _b_nor_blk_sidx;      ///< 송출 일반블록 시작 인덱스.
  uint16_t      _nb_high_blk_sidx;    ///< 비송출 고속블록 시작 인덱스.
  uint16_t      _nb_na_blk_sidx;      ///< 비송출 국도블록 시작 인덱스.
  uint16_t      _nb_nor_blk_sidx;     ///< 비송출 일반블록 시작 인덱스.
  char 	        _reserved[4];         ///< reserved 
};

/**
 * CTT Speed Block Header
 */
class stiSharedCTTHdr {
 public:
  stiSharedCTTHdr() {
    clear();
  }
  void clear() {
    _is_shared = 0;
    _speed_blidx = 0xffff;
    _valid_cnt = 0;
  }
  void update_hdr(uint16_t is_shared, uint16_t index, uint16_t validcnt) {  
    _speed_blidx = index;
    _is_shared = is_shared;
    _valid_cnt = validcnt;
  }
 public:
  uint16_t  _is_shared;    ///< 공유 여부.
  uint16_t  _speed_blidx;    ///< 교통정보 Speed Block Index
  uint16_t  _valid_cnt;    ///< 유효 교통정보 개수.
  uint16_t  _reserved;
};

/**
 * CTT 교통정보 Shared memory Record
 */
class stiSharedCTTRec {
 public:
  enum { NO_PASS = 0, NOT_RECEIVE = 255 };
 public:
  stiSharedCTTRec() {
    clear();
  }
  ~stiSharedCTTRec() {}

  void clear() {
    _time = 0;
    _speed = NOT_RECEIVE;
    _rtm_idx = 0xff;
  }

  stiSharedCTTRec(const stiSharedCTTRec& lhs) {
    _time = lhs._time;
    _speed = lhs._speed;
    _rtm_idx = lhs._rtm_idx;
  }

  stiSharedCTTRec& operator=(const stiSharedCTTRec& lhs) {
    if (this != &lhs) {
      _time = lhs._time;
      _speed = lhs._speed;
      _rtm_idx = lhs._rtm_idx;
    }
    return *this;
  }

 public:
  ///< clock tic count값의 minute값. (second로 바꿔서 계산해보자.)
  uint16_t   _time;
  uint8_t    _speed;    ///< speed 값.
  ///< reserved. (유고정보 INDEX값 0~255), speed 동기화 관련 필드.
  uint8_t    _rtm_idx;
};



class stiSharedCTTBlock {
 public:
  void clear() {
    for (int i = 0; i < thTrIDBlock::BLOCK_ITEM_CNT; ++i) {
      _tbl[i].clear();
    }
  }
 public:
  stiSharedCTTRec _tbl[thTrIDBlock::BLOCK_ITEM_CNT];
};

/**
 * 통계 교통정보 Speed Block
 */
class stiSharedSCCTBlock {
 public:
  void clear() {
    for (int i = 0; i < thTrIDBlock::BLOCK_ITEM_CNT; ++i) {
      _tbl[i] = 0xff;
    }
  }

 public:
  uint8_t _tbl[thTrIDBlock::BLOCK_ITEM_CNT];
};

/**
 * 유고정보 결과 레코드
 */
class stiSharedRTMRec {
public:
  /**
   * mbc 기준 정의. 
   * 나중에 지도로 빼는게 좋지 않을까????
   */ 
  enum RTM_TBL_INFO {
  RTM_ACC_NO1 = 3,          ///< 차량 사고 유형 
  RTM_ACC_NO2 = 20,         ///< 사람 문제 종류
  RTM_ACC_NO3 = 43,         ///< 교통 신호 상황
  RTM_ACC_CODE11 = 3, 
  RTM_ACC_CODE12 = 4, 
  RTM_ACC_CODE13 = 8, 
  RTM_ACC_CODE14 = 17, 
  RTM_ACC_CODE15 = 19, 
  RTM_ACC_CODE16 = 22, 
  RTM_ACC_CODE21 = 255, 
  RTM_ACC_CODE31 = 4,

  RTM_NC_NO1 = 50,          ///< 도로 공사 종류
  RTM_NC_CODE11 = 1,
  RTM_NC_CODE12 = 2, 
  RTM_NC_CODE13 = 4, 
  RTM_NC_CODE14 = 5, 
  RTM_NC_CODE15 = 7, 
  RTM_NC_CODE16 = 14, 
  RTM_NC_CODE17 = 255, 

  RTM_AV_NO1 = 24,         ///< 활동 종류 (Activity)
  RTM_AV_CODE11 = 1, 
  RTM_AV_CODE12 = 3, 
  RTM_AV_CODE13 = 5, 
  RTM_AV_CODE14 = 255, 

  RTM_RC_NO1 = 12,         ///< 객체 문제
  RTM_RC_NO2 = 18,         ///< 노면 상태
  RTM_RC_NO3 = 39,         ///< 접착력 문제
  RTM_RC_NO4 = 17,         ///< 주행 방해 원인
  RTM_RC_CODE11 = 10, 
  RTM_RC_CODE12 = 11,
  RTM_RC_CODE13 = 255,
  RTM_RC_CODE21 = 4,
  RTM_RC_CODE22 = 5,
  RTM_RC_CODE31 = 8,
  RTM_RC_CODE32 = 14,
  RTM_RC_CODE33 = 17,
  RTM_RC_CODE34 = 20,
  RTM_RC_CODE35 = 23,
  RTM_RC_CODE36 = 255,
  RTM_RC_CODE41 = 2,
  
  RTM_NO_PASS_NO = 47,     ///< 상태 상황 
  RTM_NO_PASS_CODE = 18
  };

  enum RTM_TBL_CATEGORY {
    RTM_TBL_NO15 = 15,   ///< 차선 표시 문제 (예고된 주의)
    RTM_TBL_NO15_CODE001 = 1, 
    RTM_TBL_NO15_CODE002 = 2, 
    RTM_TBL_NO15_CODE003 = 3, 
    RTM_TBL_NO15_CODE004 = 4, 
    RTM_TBL_NO15_CODE005 = 5, 
    RTM_TBL_NO15_CODE006 = 6,
    RTM_TBL_NO15_CODE007 = 7,
    RTM_TBL_NO15_CODE255 = 255,
    RTM_TBL_NO24 = 24,   ///< 행사 (예고된 주의)
    RTM_TBL_NO24_CODE001 = 1,
    RTM_TBL_NO24_CODE002 = 2,
    RTM_TBL_NO24_CODE003 = 3, 
    RTM_TBL_NO24_CODE004 = 4, 
    RTM_TBL_NO24_CODE005 = 5,
    RTM_TBL_NO24_CODE255 = 255,
    RTM_TBL_NO50 = 50,   ///< 공사 (예고된 주의)
    RTM_TBL_NO50_CODE001 = 1, 
    RTM_TBL_NO50_CODE002 = 2, 
    RTM_TBL_NO50_CODE003 = 3, 
    RTM_TBL_NO50_CODE004 = 4, 
    RTM_TBL_NO50_CODE005 = 5,
    RTM_TBL_NO50_CODE006 = 6,
    RTM_TBL_NO50_CODE007 = 7,
    RTM_TBL_NO50_CODE011 = 11,
    RTM_TBL_NO50_CODE014 = 14,
    RTM_TBL_NO50_CODE015 = 15,
    RTM_TBL_NO50_CODE018 = 18,
    RTM_TBL_NO50_CODE019 = 19,
    RTM_TBL_NO50_CODE255 = 255,
    RTM_TBL_NO12 = 12,   ///< 장애물 장애 (돌발적 주의)
    RTM_TBL_NO12_CODE001 = 1, 
    RTM_TBL_NO12_CODE010 = 10, 
    RTM_TBL_NO12_CODE011 = 11, 
    RTM_TBL_NO12_CODE012 = 12, 
    RTM_TBL_NO12_CODE013 = 13, 
    RTM_TBL_NO12_CODE014 = 14,
    RTM_TBL_NO12_CODE255 = 255,
    RTM_TBL_NO13 = 13,   ///< 시야 장애 (돌발적 주의)
    RTM_TBL_NO13_CODE001 = 1, 
    RTM_TBL_NO13_CODE002 = 2, 
    RTM_TBL_NO13_CODE255 = 255, 
    RTM_TBL_NO17 = 17,   ///< 주행방해요인 (돌발적 주의)
    RTM_TBL_NO17_CODE001 = 1, 
    RTM_TBL_NO17_CODE002 = 2, 
    RTM_TBL_NO17_CODE255 = 255, 
    RTM_TBL_NO18 = 18,   ///< 노면상태 (돌발적 주의)
    RTM_TBL_NO18_CODE004 = 4, 
    RTM_TBL_NO18_CODE005 = 5, 
    RTM_TBL_NO18_CODE255 = 255, 
    RTM_TBL_NO34 = 34,   ///< 소통상황 (돌발적 주의)
    RTM_TBL_NO34_CODE001 = 1, 
    RTM_TBL_NO34_CODE003 = 3, 
    RTM_TBL_NO34_CODE005 = 5, 
    RTM_TBL_NO36 = 36,   ///< 보안 경보 종류 (돌발적 주의)
    RTM_TBL_NO36_CODE002 = 2, 
    RTM_TBL_NO36_CODE003 = 3, 
    RTM_TBL_NO36_CODE004 = 4, 
    RTM_TBL_NO36_CODE005 = 5, 
    RTM_TBL_NO36_CODE006 = 6, 
    RTM_TBL_NO36_CODE007 = 7, 
    RTM_TBL_NO36_CODE008 = 8, 
    RTM_TBL_NO36_CODE009 = 9, 
    RTM_TBL_NO36_CODE255 = 255, 
    RTM_TBL_NO39 = 39,   ///< 보안 경보 종류 (돌발적 주의)
    RTM_TBL_NO39_CODE007 = 7, 
    RTM_TBL_NO39_CODE008 = 8, 
    RTM_TBL_NO39_CODE014 = 14, 
    RTM_TBL_NO39_CODE017 = 17, 
    RTM_TBL_NO39_CODE020 = 20, 
    RTM_TBL_NO39_CODE023 = 23, 
    RTM_TBL_NO39_CODE255 = 255,
    RTM_TBL_NO03 = 3,   ///< 차량 사고 유형 (사고)
    RTM_TBL_NO03_CODE003 = 3, 
    RTM_TBL_NO03_CODE004 = 4, 
    RTM_TBL_NO03_CODE008 = 8, 
    RTM_TBL_NO03_CODE017 = 17, 
    RTM_TBL_NO03_CODE019 = 19, 
    RTM_TBL_NO03_CODE021 = 21, 
    RTM_TBL_NO03_CODE022 = 22, 
    RTM_TBL_NO03_CODE255 = 255, 
    RTM_TBL_NO20 = 20,   ///< 사람 문제 종류 (사고)
    RTM_TBL_NO20_CODE255 = 255, 
    RTM_TBL_NO43 = 43,   ///< 교통 제어 상태 (사고)
    RTM_TBL_NO43_CODE004 = 4, 
    RTM_TBL_NO47 = 47,   ///< 통행상태 (통제)
    RTM_TBL_NO47_CODE008 = 8, 
    RTM_TBL_NO47_CODE018 = 18, 
    RTM_TBL_NO47_CODE255 = 255 
  };

  enum RTM_TYPE_PRIORITY {
    RTM_PRO_NOT_DEFINE = 0, ///< 미정의 .
    RTM_PRO_NET_COND,       ///< 공사.
    RTM_PRO_ACTIVITY,       ///< 행사.
    RTM_PRO_ROAD_COND,      ///< 재해, 돌발상황.
    RTM_PRO_ACCIDENT,       ///< 사고.
    RTM_PRO_NO_PASS         ///< 차단 및 통제. 
  };

  enum RTM_CLASS {
    RTM_CL_NOT_DEFINE = 0, ///< 미정의.
    RTM_CL_CROWDED,        ///< 주의(예보<공사, 행사>) 
    RTM_CL_CAUTION,        ///< 주의(돌발<재해>)
    RTM_CL_UNEXPECTED_ACC, ///< 사고.
    RTM_CL_DETOUR          ///< 차단 및 통제.
  };
 public:
  stiSharedRTMRec() {
    clear();
  }

  stiSharedRTMRec& operator=(const stiSharedRTMRec& rhs) {
    _expire = rhs._expire;
    _type = rhs._type;
    _rtm_id = rhs._rtm_id;      
    _gtime = rhs._gtime;
    _stime = rhs._stime;
    _etime = rhs._etime;
    _linkid = rhs._linkid;
    _ctt_block_idx = rhs._ctt_block_idx;
    _ctt_item_idx = rhs._ctt_item_idx;
    _road_num = rhs._road_num;
    _affect_lcnt = rhs._affect_lcnt;
    _acc_cnt = rhs._acc_cnt;
    _acc_car_cnt = rhs._acc_car_cnt;
    _weather = rhs._weather;
    _lane_info = rhs._lane_info;
    _viewing_dis = rhs._viewing_dis;
    _rtm_tbl_no = rhs._rtm_tbl_no;
    _rtm_tbl_code = rhs._rtm_tbl_code;
    _region_code = rhs._region_code;
    _rtm_priority = rhs._rtm_priority;
    _rtm_class = rhs._rtm_class;
    _rtm_key = rhs._rtm_key;
    _lon = rhs._lon;
    _lat = rhs._lat;
    memcpy(_desc, rhs._desc, sizeof(_desc));    
    return *this;
  }

  void clear() {
    _expire = 0;
    _type = 0;
    _rtm_id = 0xffff;      /// 65535로 setting.
    _gtime = _stime = _etime = 0;
    _linkid = 0;
    _ctt_block_idx = thTrIndexItem::BLOCK_MAX;
    _ctt_item_idx = thTrIndexItem::BLOCK_MAX;
    _road_num = 0;  
    _affect_lcnt = 0; 
    _acc_cnt = _acc_car_cnt = 0;
    _weather = 0;
    _lane_info = _viewing_dis = 0;
    _rtm_tbl_no = _rtm_tbl_code = 0;
    _region_code = -1;
    _rtm_priority = RTM_PRO_NOT_DEFINE; 
    _rtm_class = RTM_CL_NOT_DEFINE;
    _rtm_key = 0xffff;
    _lon = _lat = 0;
    memset(_desc, 0x00, sizeof(_desc));    
  }

  //bool expire_record(unsigned int curtime, int exptime_sec) {
  //  if (_expire == 1) {
  //    int diffsec = thlib::thTime::get_diff_second(_gtime, curtime);
  //    if (diffsec < 0) {
  //      diffsec *= -1;
  //    }  
  //    if (diffsec >= exptime_sec) {
  //      return true;
  //    }
  //  }
  //  return false;
  //}

  bool is_same_record(stiSharedRTMRec* rec) {
    if (_expire == 1) {
      if (_linkid == rec->_linkid)
        return true;
/*
      if (_linkid != rec->_linkid || _affect_lcnt != rec->_affect_lcnt) {
        return false;
      }
      if (_lon != rec->_lon || _lat != rec->_lat) {
        return false;
      }
      if (!memcmp(_desc, rec->_desc, sizeof(rec->_desc))) {
        return true;
      }
*/
    }
    return false;
  }

  /**
   * 주어진 시점에 활성화되는 컨텐츠 인가?
   * @param
   */
  bool is_active_data(uint32_t utc) {
    if (_stime == 0 || _etime == 0) {
      return false;
    }
    if (_stime <= utc && utc <= _etime) {
      return true;
    }
    return false;
  }
  
  /**
   * 해당 유고정보가 NO Pass(차단 및 통제)인가???
   */ 
  bool is_no_pass_rtm() {
    if (_rtm_tbl_no == RTM_TBL_NO47 &&
        _rtm_tbl_code == RTM_TBL_NO47_CODE018) {
      return true;
    }
    return false;
  }

  void decide_priority() {
    _rtm_priority = RTM_PRO_NOT_DEFINE; 
    _rtm_class = RTM_CL_NOT_DEFINE;
    /**
     * @thinkware 유형 분류.
     */
    if (_rtm_tbl_no == RTM_TBL_NO15) {
      if (_rtm_tbl_code == RTM_TBL_NO15_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO15_CODE002 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE003 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE004 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE005 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE006 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE007 ||
        _rtm_tbl_code == RTM_TBL_NO15_CODE255) {
        _rtm_class = RTM_CL_CROWDED;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO24) {
      if (_rtm_tbl_code == RTM_TBL_NO24_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO24_CODE002 ||
        _rtm_tbl_code == RTM_TBL_NO24_CODE003 ||
        _rtm_tbl_code == RTM_TBL_NO24_CODE004 ||
        _rtm_tbl_code == RTM_TBL_NO24_CODE005 ||
        _rtm_tbl_code == RTM_TBL_NO24_CODE255 ||
        _rtm_tbl_code == 0 /* ytn */ ) {
        _rtm_priority = RTM_PRO_ACTIVITY; ///< 행사.
        _rtm_class = RTM_CL_CROWDED;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO50) {
      if (_rtm_tbl_code == RTM_TBL_NO50_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO50_CODE002 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE003 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE004 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE005 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE006 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE007 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE011 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE014 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE015 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE018 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE019 ||
        _rtm_tbl_code == RTM_TBL_NO50_CODE255 ||
        _rtm_tbl_code == 0 /* ytn */ ) {
        _rtm_priority = RTM_PRO_NET_COND; ///< 공사.
        _rtm_class = RTM_CL_CROWDED;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO12) {
      if (_rtm_tbl_code == RTM_TBL_NO12_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO12_CODE010 ||
        _rtm_tbl_code == RTM_TBL_NO12_CODE011 ||
        _rtm_tbl_code == RTM_TBL_NO12_CODE012 ||
        _rtm_tbl_code == RTM_TBL_NO12_CODE013 ||
        _rtm_tbl_code == RTM_TBL_NO12_CODE014 ||
        _rtm_tbl_code == RTM_TBL_NO12_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 재해.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO13) {
      if (_rtm_tbl_code == RTM_TBL_NO13_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO13_CODE002 ||
        _rtm_tbl_code == RTM_TBL_NO13_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 시야 장애.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO17) {
      if (_rtm_tbl_code == RTM_TBL_NO17_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO17_CODE002 ||
        _rtm_tbl_code == RTM_TBL_NO17_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 주행 방해요인.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO18) {
      if (_rtm_tbl_code == RTM_TBL_NO18_CODE004 || 
        _rtm_tbl_code == RTM_TBL_NO18_CODE005 ||
        _rtm_tbl_code == RTM_TBL_NO18_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 노면상태.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO34) {
      if (_rtm_tbl_code == RTM_TBL_NO34_CODE001 || 
        _rtm_tbl_code == RTM_TBL_NO34_CODE003 ||
        _rtm_tbl_code == RTM_TBL_NO34_CODE005) {
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO36) {
      if (_rtm_tbl_code == RTM_TBL_NO36_CODE002 || 
        _rtm_tbl_code == RTM_TBL_NO36_CODE003 ||
        _rtm_tbl_code == RTM_TBL_NO36_CODE004 ||
        _rtm_tbl_code == RTM_TBL_NO36_CODE005 || 
        _rtm_tbl_code == RTM_TBL_NO36_CODE006 ||
        _rtm_tbl_code == RTM_TBL_NO36_CODE007 ||
        _rtm_tbl_code == RTM_TBL_NO36_CODE008 || 
        _rtm_tbl_code == RTM_TBL_NO36_CODE009 ||
        _rtm_tbl_code == RTM_TBL_NO36_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 보안 경보 종류.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO39) {
      if (_rtm_tbl_code == RTM_TBL_NO39_CODE007 || 
        _rtm_tbl_code == RTM_TBL_NO39_CODE008 ||
        _rtm_tbl_code == RTM_TBL_NO39_CODE014 ||
        _rtm_tbl_code == RTM_TBL_NO39_CODE017 || 
        _rtm_tbl_code == RTM_TBL_NO39_CODE020 ||
        _rtm_tbl_code == RTM_TBL_NO39_CODE023 ||
        _rtm_tbl_code == RTM_TBL_NO39_CODE255) {
        _rtm_priority = RTM_PRO_ROAD_COND; ///< 노면 상태.
        _rtm_class = RTM_CL_CAUTION;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO03) {
      if (_rtm_tbl_code == RTM_TBL_NO03_CODE003 || 
        _rtm_tbl_code == RTM_TBL_NO03_CODE004 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE008 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE017 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE019 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE021 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE022 ||
        _rtm_tbl_code == RTM_TBL_NO03_CODE255 ||
        _rtm_tbl_code == 0 /* ytn */ ) {
        _rtm_priority = RTM_PRO_ACCIDENT; ///< 사고.
        _rtm_class = RTM_CL_UNEXPECTED_ACC;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO20) {
      if (_rtm_tbl_code == RTM_TBL_NO20_CODE255) {
        _rtm_priority = RTM_PRO_ACCIDENT; ///< 사고.
        _rtm_class = RTM_CL_UNEXPECTED_ACC;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO43) {
      if (_rtm_tbl_code == RTM_TBL_NO43_CODE004) {
        _rtm_priority = RTM_PRO_ACCIDENT; ///< 사고
        _rtm_class = RTM_CL_UNEXPECTED_ACC;
      }
    } else if (_rtm_tbl_no == RTM_TBL_NO47) {
      if (_rtm_tbl_code == RTM_TBL_NO47_CODE008 || 
        _rtm_tbl_code == RTM_TBL_NO47_CODE018 ||
        _rtm_tbl_code == RTM_TBL_NO47_CODE255) {
        _rtm_priority = RTM_PRO_NO_PASS; ///< 통제.
        _rtm_class = RTM_CL_DETOUR;
      }
    }
  }

 public:
  uint8_t   _expire;        ///< 만료 여부 (0: 만료, 1: 유효)
  uint8_t   _type;          ///< 유고 정보 타입
  uint16_t  _rtm_id;        ///< 로컬 고유번호
  uint32_t  _gtime;         ///< 송출 시간
  uint32_t  _stime;         ///< 시작 시간
  uint32_t  _etime;         ///< 종료 시간
  uint32_t  _linkid;        ///< tpeg link id <<추가하자!!!
  uint16_t  _ctt_block_idx; ///< ctt block index
  uint16_t  _ctt_item_idx;  ///< ctt block item index
  uint16_t  _road_num;      ///< 도로 번호. (0: 없음)
  uint8_t   _affect_lcnt;   ///< affect link cnt
  uint8_t   _acc_cnt;       ///< 사고 건수
  uint8_t   _acc_car_cnt;   ///< 사고 차량 대수
  uint8_t   _weather;       ///< 날씨
  uint8_t   _lane_info;     ///< 차선 정보
  uint8_t   _viewing_dis;   ///< 시야 거리 (100m 단위)
  uint8_t   _rtm_tbl_no;    ///< RTM Table No
  uint8_t   _rtm_tbl_code;  ///< RTM Table Code
  char      _region_code;   ///< region code
  char      _rtm_priority;  ///< rtm 우선순위
  char      _rtm_class;     ///< 유고정보 유형
  char      _reserved;      ///< reserved
  uint16_t  _rtm_key;       ///< Tcon서버 고유번호
  uint32_t  _lat;           ///< WGS84 10 micro degree (0: 없음)
  uint32_t  _lon;           ///< WGS84 10 micro degree (0: 없음)
  char      _desc[200];     ///< description
};

/**
 * affect link shared!
 */
class stiSharedRTMAffectLinkInfo {
 public:
  enum { MAX_AFFECT_LINK_CNT = 80 };
  stiSharedRTMAffectLinkInfo() {
    clear();
  }
  void clear() {
    _is_share = 0;
    _rtm_idx = 0xff;
    _rtm_id = 0xffff;
  }
 public:
  uint8_t  _is_share;
  uint8_t  _rtm_idx;
  uint16_t _rtm_id;
};

class stiSharedRTMAffectLinkRecord {
 public:
  stiSharedRTMAffectLinkRecord() {
    clear();
  }
  void clear() {
    _affect_bidx = _affect_iidx = 0;
    _affect_link = 0;
  }
 public:
  uint16_t _affect_bidx;
  uint16_t _affect_iidx;
  ///< ID도 필요하고, block index, item index도 필요한데...
  uint32_t   _affect_link;
};

class stiSharedRTMAffectLinkList {
 public:
	enum { MAX_AFFECT_LINK_CNT = 80 };
  stiSharedRTMAffectLinkList() {
	  clear();
	}
	void clear() {
	  for (int i = 0; i < MAX_AFFECT_LINK_CNT; ++i) {
		  _link_id[i] = 0;
		}
	}
 public:
	uint32_t  _link_id[MAX_AFFECT_LINK_CNT]; // affect link
};

// for air rtm hash by jmpark129
class stiSharedRTMHashTable {
 public:
  enum { MAX_RTM_ID = 65536 };
  stiSharedRTMHashTable() {
    clear();
  }
  void clear() {
    _idx = 0xff;
  }
 public:
  uint8_t _idx;
}; 
/**
 * 뉴스정보 결과 레코드
 */
class stiSharedNWSRec {
 public:
  enum LENGTH { STI_NWS_TITLE_LEN = 60, 
    STI_NWS_DESC_LEN = 2400 };
 public:
  stiSharedNWSRec() {
    clear();
  }

  void clear() {
    _expire = 0;
    _class = 0;
    _time = 0;
    memset(_reserved, 0x00, sizeof(_reserved));
    memset(_title, 0x00, sizeof(_title));
    memset(_desc, 0x00, sizeof(_desc));
  }

  /**
   * scroll news의 경우 국제, 스포츠, 날씨, 연예, 주요뉴스의 단문뉴스가.
   * 20분 간격으로 수신되며, 컨텐츠의 내용은 같으나 길이가 다른경우가 존재.
   * 그래서 중복체크를 달리한다. 
   */
  bool is_same_contents(stiSharedNWSRec* rec, bool is_scroll) {
    if (_expire == 0) {
    return false;
    }
  if (_class != rec->_class) {
    return false;
  }
    int cmp1, cmp2;
  cmp1 = memcmp(_title, rec->_title, sizeof(rec->_title));
    if (is_scroll && !cmp1) {    
    return true;
    }
  cmp2 = memcmp(_desc, rec->_desc, sizeof(rec->_desc));
    if (!cmp1 && !cmp2) {
      return true;
    }
    return false;  
  }

 public:
  uint8_t _expire;           ///< 만료여부 (0: 만료, 1: 유효)
  uint8_t _class;            ///< 종류.
  char  _reserved[2];     
  uint32_t  _time;             ///< generation time(utc)
  char  _title[STI_NWS_TITLE_LEN]; 
  char  _desc[STI_NWS_DESC_LEN];       ///< 본문.
};

/**
 * 서비스 command의 경우 일부는 요청값과 결과 값이 존재하기도 한다. 
 * 아래 구조를 이용하여 함수를 호출한다. 
 * category의 경우 TPEG, IDIO, CDMA 별도로 사용한다. 
 * @todo 나중에 아래 enum값을 sti_define값을 사용하고, 아래 값은 제거하자.
 */
class service_param {
 public:
  service_param() {
    clear();
  }
  ~service_param() { }
  void clear() {
    _command = 0;
    for (int i = 0; i < 4; ++i) {
      _input[i] = 0;
      _output[i] = 0;
    }
  }
 public:
  int  _command;  ///< command value (각 서비스 별로 정의)
  int  _input[4];  ///< input value
  int  _output[4];  ///< output value
};

/**
 * sti status
 */



///////////////////////////////////////////////////////////////////////////////
// 교통정보 LINK SPEED (CTT) 데이터 검증 관련.
///////////////////////////////////////////////////////////////////////////////

/** 
 * tpeg 표준 노드 링크 중에서. 
 * 링크 정보만 로딩해서 관리.
 */
class std_tpegid_mgr {
 public:
  std_tpegid_mgr() {}
  ~std_tpegid_mgr() {
    clear();
  }
  void clear() {
    _idlistvec.clear();
  }

  /**
   * TPEG ID만 뽑아놓은 tpeg_idlist.txt파일을 열어서 로딩한다.
   */
  bool load_data(char* file_path) {
    thlib::thFile<> tpeg_id_list;
    if (tpeg_id_list.open(file_path, "r") != 0) {
      return false;
    }
    char strline[128], tmpstr[12];
    unsigned int id = 0;
    memset(strline, 0x00, sizeof(strline));
    /// 첫라인은 날린다. column name
    tpeg_id_list.readline(strline, 128);
    while (tpeg_id_list.readline(strline, 128) > 0) {
      memset(tmpstr, 0x00, sizeof(tmpstr));
      memcpy(tmpstr, strline, 10);
      id = atoi(tmpstr);
      if (id == 0) {
        break;
      }
      _idlistvec.push_back(id);
      memset(strline, 0x00, sizeof(strline));
    }
    return true;
  }

  bool find(uint32_t id) {
    int sIdx = 0;
    int eIdx = _idlistvec.size() - 1;
    int mIdx = 0;
    int find_idx = -1;
    /* binary search */
    while (1) {
      mIdx = (sIdx + eIdx) / 2;

      if (sIdx >= eIdx) {
        return false;
      }

      if (_idlistvec[mIdx] == id) {
        find_idx = mIdx;
      } else if (_idlistvec[sIdx] == id) {
        find_idx = sIdx;
      } else if (_idlistvec[eIdx] == id) {
        find_idx = eIdx;
      }

      if (find_idx != -1) {
        return true;
      }

      if (_idlistvec[mIdx] > id) {
        eIdx = mIdx - 1;
      } else if (_idlistvec[mIdx] < id) {
        sIdx = mIdx + 1;
      }     
    }
  }
 private:

 public:
  std::vector<unsigned int, thallocator<unsigned int> > _idlistvec;
};


class ctt_info {
 public:
  ctt_info() {
    _generation_time = _speed = 0;
  }
  ctt_info& operator=(const ctt_info& rhs) {
    _generation_time = rhs._generation_time;
    _speed = rhs._speed;
    return *this;
  }
 public:
  int      _generation_time;
  int      _speed;
};

class rtm_update_info {
 public:
  rtm_update_info() {
    _gstart_time = _gend_time = 0;
  _stime = _etime = 0;
    _type = 0;
    memset(_section, 0, sizeof(_section));
    memset(_description, 0, sizeof(_description));
    _lon = _lat = 0;
  }

 public:
  int      _gstart_time;            ///< 수신 시간 utc
  int      _gend_time;               ///< 최종 수신 시간 utc
  int   _stime; 
  int   _etime; 
  int      _type;                    ///< 유형.
  char    _section[40];          ///< 구간명.
  char    _description[120];          ///< 설명.
  int      _lat;                ///< 위도.
  int      _lon;                ///< 경도.
};


typedef std::multimap<uint32_t, ctt_info, std::less<uint32_t>, 
             thallocator<std::pair<uint32_t, ctt_info> > > MMAP_CTT_INFO;
typedef std::multimap<uint32_t, rtm_update_info, std::less<uint32_t>,
             thallocator<std::pair<uint32_t, rtm_update_info> > > 
                                                                MMAP_RTM_INFO;
typedef MMAP_CTT_INFO::iterator ciIterator;
typedef MMAP_RTM_INFO::iterator ruiIterator;

/**
 * CTT 검증.
 */
class ctt_verification {
 public:
  enum shared_mem_room { SMR_STATUS = 0, SMR_CTT, SMR_RTM, SMR_NWS };

  ctt_verification() {
    clear();
    for (int i = 0; i < 5; ++i) {
      _used_shared_mem[i] = 0;
    }
  }
  ~ctt_verification() {  
    if (_ctt_file.isOpen()) {
      _ctt_file.close();
    }
    _available_sti_cnt = 0;
    std::map<uint32_t, int, std::less<uint32_t>, 
             thallocator<std::pair<uint32_t, int> > >().swap(_not_exist_idlist);
    std::map<uint32_t, int, std::less<uint32_t>, 
             thallocator<std::pair<uint32_t, int> > >().swap(_exist_idlist);
    MMAP_CTT_INFO().swap(_id_speed_tbl);
    MMAP_RTM_INFO().swap(_rtm_up_info);
    _sti_cnt_vec.clear();
  }
  /** 
   * 검증 파일 Write할 Path.
   */
  void set_savedir(char *dir) {
    _savedir(dir);
    thlib::thString filename;
    filename.printf("%s/%s.txt", _savedir(), "ctt_verify");
    int ret = _ctt_file.open(filename(), "wb");
    (void) ret; /* unused value */
  }

  void clear() {
    std::map<uint32_t, int, std::less<uint32_t>, 
         thallocator<std::pair<uint32_t, int> > >().swap(_latest_update_time);
    _available_sti_cnt = 0;
    for (size_t i = 0; i< _sti_cnt_vec.size(); ++i) {
      _available_sti_cnt += _sti_cnt_vec[i];
    }
    if (_sti_cnt_vec.size() > 0) {
      _available_sti_cnt = 
        static_cast<int>(_available_sti_cnt/_sti_cnt_vec.size());
    }
  }

  void write_stream(char *str, int str_size) {
    if (_ctt_file.isOpen()) {
      _ctt_file.write(str, str_size);
    }
  }
  /**
   * 중복 체크 및 id list 저장.
   */
  bool push2not_exist_id(uint32_t realid, int generation_time) {
                     std::map<uint32_t, int, std::less<uint32_t>, 
                      thallocator<std::pair<uint32_t, int> > >::
                                                                   iterator it;
    it = _not_exist_idlist.find(realid);
    if (it == _not_exist_idlist.end()) {
      _not_exist_idlist.insert(std::map<uint32_t, int, std::less<uint32_t>, 
                               thallocator<std::pair<uint32_t, int> > >::
                                                        value_type(realid, 1));
      _latest_update_time.insert(std::map<uint32_t, int, std::less<uint32_t>, 
                                 thallocator<std::pair<uint32_t, int> > >::
                                                  value_type(realid, 
                                                             generation_time));
    } else {
      if (generation_time == _latest_update_time[realid]) {
        return false;
      }
      _latest_update_time[realid] = generation_time;
      ++_not_exist_idlist[realid];
    }    
    return true;
  }
  /**
   * 중복 체크 및 id list 저장.
   */
  bool push2exist_id(uint32_t realid, int generation_time) {
    std::map<uint32_t, int, std::less<uint32_t>, 
             thallocator<std::pair<uint32_t, int> > >::iterator it;
    it = _exist_idlist.find(realid);
    if (it == _exist_idlist.end()) {
      _exist_idlist.insert(std::map<uint32_t, int, std::less<uint32_t>, 
                                    thallocator<std::pair<uint32_t, int> > >
                           ::value_type(realid, 1));
      _latest_update_time.insert(std::map<uint32_t, int, std::less<uint32_t>, 
                                      thallocator<std::pair<uint32_t, int> > >
                                 ::value_type(realid, generation_time));
    } else {
      if (generation_time == _latest_update_time[realid]) {
        return false;
      }
      _latest_update_time[realid] = generation_time;
      ++_exist_idlist[realid];
    }
    
    return true;
  }
  void push2id_speed_tbl(uint32_t realid, int time, int speed) {
    ctt_info rec;
    rec._generation_time = time;
    rec._speed = speed;
    _id_speed_tbl.insert(MMAP_CTT_INFO::value_type(realid, rec));
  }

  void push2available_sti_cnt(int available_sti_cnt) {
    if (_sti_cnt_vec.size() < 100 && available_sti_cnt > 0) {
      _sti_cnt_vec.push_back(available_sti_cnt);
    }
  }

  void push2usemem(int type, int datasize) {
    switch (type) {
    case 1: type -= 1; break;
    case 2: type -= 1; break;
    case 3: type -= 1; break;
    case 4: type -= 1; break;
    case 6: type -= 2; break;
    }
    if (_used_shared_mem[type] < datasize) {
      _used_shared_mem[type] = datasize;
    } 
  }

  /**
   * rtm 정보 insert.
   */
  void push2rtminfo(int realid, int gtime, int stime, int etime, 
    int type, char* section, char* description, int lat, int lon) {
    if (realid < 0) 
      return;
    MMAP_RTM_INFO::iterator it;
    it = _rtm_up_info.find(realid);
    if (it == _rtm_up_info.end()) {
      rtm_update_info rec;
      rec._gstart_time = gtime;
      rec._type = type;
      rec._stime = stime;
      rec._etime = etime;
      if (strlen(section) <= sizeof(rec._section)) {
        memcpy(&rec._section, section, strlen(section));
      } else {
        memcpy(&rec._section, section, sizeof(rec._section));  
      }
      if (strlen(description) <= sizeof(rec._description)) {
        memcpy(&rec._description, description, strlen(description));
      } else {
        memcpy(&rec._description, description, sizeof(rec._description));
      }
      rec._lon = lon;
      rec._lat = lat;
      _rtm_up_info.insert(MMAP_RTM_INFO::value_type(realid, rec));
    } else {  /* 중복 확인 */
      std::pair<ruiIterator, ruiIterator> range = 
        _rtm_up_info.equal_range(realid);
      for (ruiIterator it = range.first; it != range.second; ++it) {
        if (!strncmp(section, it->second._section, 
                     strlen(it->second._section))) {
          it->second._gend_time = gtime;
        }
      }
    }
  }

  int avg_available_cnt() {
    return _available_sti_cnt;
  }

 public:
  ///< real id, update time///< real id, update time
  std::map<uint32_t, int, std::less<uint32_t>, 
           thallocator<std::pair<uint32_t, int> > >    _not_exist_idlist;
  ///< real id, update time
  std::map<uint32_t, int, std::less<uint32_t>,
           thallocator<std::pair<uint32_t, int> > >    _exist_idlist;
  MMAP_CTT_INFO      _id_speed_tbl;      ///< id별 speed정보.
  MMAP_RTM_INFO      _rtm_up_info;      ///< id별 유고정보.
  int            _available_sti_cnt;    ///< 가용 sti 개수 합.
  int            _used_shared_mem[5];  ///< shared Memory 최대 사용량.
 private:
  thlib::thString      _savedir;        ///< file로 Write할 path.
  thlib::thFile<>      _ctt_file;        ///< 데이터 기록.
  // # tmp value
  ///< real id, generation time
  std::map<uint32_t, int, std::less<uint32_t>,
           thallocator<std::pair<uint32_t, int> > >    _latest_update_time;
  ///< 사용가능한 sti_cnt개수.
  std::vector<int, thallocator<int> >    _sti_cnt_vec;
};

class ctt_update_info {
 public:
  ctt_update_info() {
    clear();
  }
  void clear() {
    _realid     = 0;
    _update_time = -1;
    _rtm_exist   = 0;
  }
 public:
  int      _realid;              ///< realid
  int16_t    _update_time;            ///< 갱신회수 (중복 제외)
  int16_t    _rtm_exist;              ///< 유고정보 여부.
};

class LockMemory {
  public:
    LockMemory() : _size(0), _mem(NULL) {}
    ~LockMemory() {
      destroy();
    }
    int init(uint32_t size) {
      _lock.entercs();
      if (_mem != NULL) {
        _lock.leavecs();
        return 0;
      }
      _mem = thlib::MemPolicyNew<char>::alloc(size);
      if (_mem == NULL) {
        _lock.leavecs();
        return -MSG_FAIL_MEMORY_ALLOC;
      }
      _size = size;
      memset(_mem, 0x00, _size);
      _lock.leavecs();
      return 0;
    }
    void destroy() {
      _lock.entercs();
      if (_mem != NULL) {
        thlib::MemPolicyNew<char>::memfree(_mem);
        _mem = NULL;
        _size = 0;
      }
      _lock.leavecs();
    }
    void clear() {
      if (_mem != NULL) {
        memset(_mem, 0x00, _size);
      }
    }
    char* buff() {
      return _mem;
    }
    int enter() {
      return _lock.entercs();
    }
    int leave() {
      return _lock.leavecs();
    }
  private:
    thlib::thLock _lock;
    uint32_t      _size;
    char*         _mem;
};

typedef std::vector<LockMemory*>  stiLockMemVec;

class stiSharedTTLCTTInfo {
 public:
  stiSharedTTLCTTInfo() {
    clear();
  }
  void clear() {
    _update_time = 0;
    _ctt_blk_cnt = 0;
  }
 public:
  unsigned int  _update_time; ///< 최근 만료처리 시간.(expire)
  uint16_t      _ctt_blk_cnt; ///< CTT 총 Block 시간
  char          _reserved[2]; ///< reserved 
};

class stiSharedTTLCTTRec {
 public:
  enum { NO_PASS = 0, NOT_RECEIVE = 255 };
 public:
  stiSharedTTLCTTRec() {
    clear();
  }
  ~stiSharedTTLCTTRec() {}

  void clear() {
    _time = 0;
    _speed = NOT_RECEIVE;
    _exist_rtm = 0;
  }

  stiSharedTTLCTTRec(const stiSharedTTLCTTRec& lhs) {
    _time = lhs._time;
    _speed = lhs._speed;
    _exist_rtm = lhs._exist_rtm;
  }

  stiSharedTTLCTTRec& operator=(const stiSharedTTLCTTRec& lhs) {
    if (this != &lhs) {
      _time = lhs._time;
      _speed = lhs._speed;
      _exist_rtm = lhs._exist_rtm;
    }
    return *this;
  }

 public:
  ///< clock tic count값의 minute값. (second로 바꿔서 계산해보자.)
  uint16_t   _time;
  uint8_t    _speed;    ///< speed 값.
  uint8_t    _exist_rtm;
};

class stiSharedTTLCTTBlock {
 public:
  void clear() {
    for (int i = 0; i < thTTLIDBlock::BLOCK_ITEM_CNT; ++i) {
      _tbl[i].clear();
    }
  }
 public:
  stiSharedTTLCTTRec _tbl[thTTLIDBlock::BLOCK_ITEM_CNT];
};

class stiSharedRotationCTTData {
 public:
  stiSharedRotationCTTData()
  : _speed(0xff), _time(0), _block_idx(0xffff), _item_idx(0xffff) {}

  void clear() {
    _speed = 0xff;
    _time = 0;
    _item_idx = _block_idx = 0xffff;
  }

  void clear_speed() {
    _speed = 0xff;
    _time = 0;
  }

  stiSharedRotationCTTData& operator=(const stiSharedRotationCTTData& lhs) {
    if (this != &lhs) {
      _speed      = lhs._speed;
      _reserved   = lhs._reserved;
      _time       = lhs._time;
      _block_idx  = lhs._block_idx;
      _item_idx   = lhs._item_idx;
    }
    return *this;
  }

public:
  uint8_t   _speed;
  uint8_t   _reserved;
  uint16_t  _time;
  uint16_t  _block_idx;
  uint16_t  _item_idx;
};

class stiSharedRotationCTTInfo {
 public:
  stiSharedRotationCTTInfo() {
    clear();
  }
  void clear() {
    _update_time = 0;
    _ctt_blk_cnt = 0;
  }
 public:
  unsigned int  _update_time; ///< 최근 갱신 시간
  uint16_t      _ctt_blk_cnt; ///< CTT 총 Block 개수
  char          _reserved[2]; ///< reserved 
};

class stiSharedRotationCTTRec {
 public:
  enum { MAX_ROTATE_CTT_COUNT = 16 };
 public:
  stiSharedRotationCTTRec() : _current_idx(0) {}
  ~stiSharedRotationCTTRec() {}

  void clear() {
    for (int i = 0; i < MAX_ROTATE_CTT_COUNT; ++i) {
      _data[i].clear();
    }
    _current_idx = 0;
  }

  stiSharedRotationCTTRec(const stiSharedRotationCTTRec& lhs) {
    for (int i = 0; i < MAX_ROTATE_CTT_COUNT; ++i) {
      _data[i] = lhs._data[i];
    }
    _current_idx = lhs._current_idx;
  }

  stiSharedRotationCTTRec& operator=(const stiSharedRotationCTTRec& lhs) {
    if (this != &lhs) {
      for (int i = 0; i < MAX_ROTATE_CTT_COUNT; ++i) {
        _data[i] = lhs._data[i];
      }
      _current_idx = lhs._current_idx;
    }
    return *this;
  }

 int insert(uint16_t bidx, uint16_t iidx, uint8_t speed, uint16_t time) {
   for (int i = 0; i < _current_idx; ++i) {
     if (_data[i]._block_idx == bidx &&
         _data[i]._item_idx == iidx) {
       _data[i]._speed = speed;
       _data[i]._time = time;
       return 0;
     }
   }

   if (_current_idx == MAX_ROTATE_CTT_COUNT) {
     return -1;
   }

   _data[_current_idx]._block_idx = bidx;
   _data[_current_idx]._item_idx = iidx;
   _data[_current_idx]._speed = speed;
   _data[_current_idx]._time = time;
   ++_current_idx;

   return 0;
 }

 uint8_t speed(uint16_t bidx, uint16_t iidx) {
   uint8_t speed = 0xff;
   for (int i = 0; i < _current_idx; ++i) {
     if (_data[i]._block_idx == bidx && _data[i]._item_idx == iidx) {
       speed = _data[i]._speed;
       break;
     }
   }
   return speed;
 }

 public:
  int _current_idx;
  stiSharedRotationCTTData _data[MAX_ROTATE_CTT_COUNT];
};

class stiSharedRotationCTTBlock {
 public:
  void clear() {
    for (int i = 0; i < thTrIDBlock::BLOCK_ITEM_CNT; ++i) {
      _tbl[i].clear();
    }
  }
 public:
  stiSharedRotationCTTRec _tbl[thTrIDBlock::BLOCK_ITEM_CNT];
};
/////////////////////////////////////////////////////////////////////////
#endif /* _STI_DATAST_H_ */
