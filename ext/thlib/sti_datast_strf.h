/* Copyright 2017 <happyteam@thinkwaresys.com> */
#ifndef _STI_DATAST_STRF_H_
#define _STI_DATAST_STRF_H_

#if !defined(WIN32)
  #include <arpa/inet.h>
#endif

/********************************************************************
	Partial Strf Filename
********************************************************************/
const static char *PARTIAL_STRF_FILENAME_TTL = "mrdata/partial_ttl_strf";
//const static char *PARTIAL_STRF_FILENAME_TTL_INFO = "mrdata/partial_ttl_strf_info.bin";
const static int   PARTIAL_STRF_VERSION_SIZE = 10;
// pattern version(10Bytes) * pattern cnt [Partial]
// pattern version(10Bytes) * pattern cnt [Full]

/********************************************************************
	Partial Strf Protocol ( hw <-> psctrl )
********************************************************************/
//class stiPSReqHeader {
//public:
//  enum { TYPE_MR_VERSION = 0, TYPE_PATTERN_VERSION, TYPE_PAUSE, TYPE_RESUME };
//public:
//  void hton() {
//    _type     = htonl(_type);
//    _bodysize = htonl(_bodysize);
//  }
//  void ntoh() {
//    _type     = ntohl(_type);
//    _bodysize = ntohl(_bodysize);
//  }
//public:
//  int _type;
//  int _bodysize;
//};
//
//class stiPSResHeader {
//public:
//  enum { ERR_SUCCESS = 0, ERR_WRONG_TYPE, ERR_WRONG_VERSION };
//public:
//  void hton() {
//    _type     = htonl(_type);
//    _error    = htonl(_error);
//    _bodysize = htonl(_bodysize);
//  }
//  void ntoh() {
//    _type     = ntohl(_type);
//    _error    = ntohl(_error);
//    _bodysize = ntohl(_bodysize);
//  }
//public:
//  int _type;
//  int _error;
//  int _bodysize;
//};
#endif
