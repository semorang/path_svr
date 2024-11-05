/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thusewindows_
#define _thusewindows_

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등
   */
  #pragma warning(disable : 4786)
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    /**
     *  @note vs2008에서 WinCE5.0용 빌드시 time_t 관련 에러 방지를 위해 
     *        <crtdefs.h> 이놈을 먼저 include해야 한다.
     */
    #if (_MSC_VER >= 1300)
      #include <crtdefs.h>
    #endif

    #include <windows.h>
  #endif
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들
   */
  #pragma warning(disable : 4786)
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    #include <windows.h>
  #endif
#else
  /**
   * 일단, 나머지는 여기에
   */
#endif

#endif

