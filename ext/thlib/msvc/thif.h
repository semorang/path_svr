/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THIF_H__
#define _THIF_H__

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0
   */
#elif defined(WIN32)
  /**
   * windows 32bit
   */
#else
  #ifdef ANDROID
    #include <pthread.h>
  #else
  #endif
#endif

namespace thlib {

class thIf {
 public:
  static int createthread(void* tid, void* arg, int cflag, 
                          void* (*tfunc)(void*), void* tfarg);
};

};  //  end of namespace thlib




#endif

