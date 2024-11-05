/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thusewindows_
#define _thusewindows_

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ���
   */
  #pragma warning(disable : 4786)
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    /**
     *  @note vs2008���� WinCE5.0�� ����� time_t ���� ���� ������ ���� 
     *        <crtdefs.h> �̳��� ���� include�ؾ� �Ѵ�.
     */
    #if (_MSC_VER >= 1300)
      #include <crtdefs.h>
    #endif

    #include <windows.h>
  #endif
#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ���
   */
  #pragma warning(disable : 4786)
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    #include <windows.h>
  #endif
#else
  /**
   * �ϴ�, �������� ���⿡
   */
#endif

#endif

