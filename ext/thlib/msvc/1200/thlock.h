/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THTHREAD_H__
#define _THTHREAD_H__


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ��� 
   */
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    #include "thusewindows.h"
  #endif
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */
  #if defined(STL_USE_MFC) || defined(MODULE_USE_MFC)
    #include <afx.h>
  #else
    #include "thusewindows.h"
  #endif
  #pragma warning(disable : 4786)
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
#endif


namespace thlib {

class thLock {
 public:
  thLock() {
    ::InitializeCriticalSection(&_mutex);
  }
  ~thLock() {
    ::DeleteCriticalSection(&_mutex);
  }

 public:
  int entercs() {
    ::EnterCriticalSection(&_mutex);
    return 0;
  }
  int leavecs() {
    ::LeaveCriticalSection(&_mutex);
    return 0;
  }

 private:
  CRITICAL_SECTION _mutex;
};

};  ///  end of namespace

#endif
