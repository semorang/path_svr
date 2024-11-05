/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef  _THTYPE_
#define  _THTYPE_

/**
 *  EVC4
 */
#if defined(_WIN32_WCE)
  #pragma warning(disable : 4786)
  //#include "thlib/msvc/inttypes.h"
  #include "./msvc/inttypes.h"	// testtt
/**
 *  Win32
 */
#elif defined(WIN32)
  #pragma warning(disable : 4786)
  //#include "thlib/msvc/inttypes.h"
  #include "./msvc/inttypes.h"	// testtt
#else
  /**
   *  etc.. such as linux
   */
  #include <stddef.h>
  #include <inttypes.h>
#endif

#endif

