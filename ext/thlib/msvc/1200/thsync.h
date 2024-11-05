/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THSYNC_MSVC_H__
#define _THSYNC_MSVC_H__



#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #include "thusewindows.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "../../thusewindows.h"
  #pragma warning(disable : 4786)
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif



namespace thlib {


class thSyncDef {
 public:
  enum mode { TH_SYNC_CREATE = 0, TH_SYNC_USE = 1 };
  enum control { TH_SYNC_PREFIX = 112 };
};

class thSyncErr {
 public:
  enum err { TH_SYNC_SUCCESS = 0, TH_SYNC_CFAIL = -1
      , TH_SYNC_ALREADY = -2, TH_SYNC_NOEXIST = -3
         , TH_SYNC_ATFAIL = -4, TH_SYNC_REMOVE = -5
         , TH_SYNC_FAIL = -6, TH_SYNC_INVARG = -7  };
};



/**
 *  no use semaphore
 */
class thNoSync {
 public:
  thNoSync()  : _handle(NULL)  {}
  virtual ~thNoSync()  { release(); }

 public:
  int init(int key, int nsems, int mode) {
    return 0;
  }
  int release() {
    return 0;
  }
  int enter() {
    return 0;
  }
  int leave() {
    return 0;
  }

 private:
  HANDLE _handle;
};


#if defined(WIN32) && !defined(_WIN32_WCE)
/**
 *  use semaphore for synchronization between processes
 */
class thSem {
 public:
  thSem()  : _handle(NULL)  {}
  virtual ~thSem()  { release(); }

 public:
  int init(int key, int nsems, int mode);
  int release();
  int enter();
  int leave();

 private:
  HANDLE _handle;
};



/**
 *  use mutex for synchronization between threads
 */
class thMutex {
 public:
  thMutex()  {}
  virtual ~thMutex()  { release(); }

 public:
  int init(int, int, int);
  int release();
  int enter();
  int leave();

 private:
};
#endif
};  //  end of namespace thlib





#endif

