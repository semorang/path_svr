/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THSYNC_REFERENCE_H__
#define _THSYNC_REFERENCE_H__

#include "../thdef.h"
#include <pthread.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>


namespace thlib {



#ifndef _THLIB_SEMUN_
#define _THLIB_SEMUN_
union semun {
  int val;          //<  value for SETVAL
  struct semid_ds* buf;    //< buffer for IPC_STAT, IPC_SET
  uint16_t *array;  //< array for GETALL, SETALL
  struct seminfo* __buf;    //< buffer for IPC_INFO
};
#endif



class thSyncDef {
 public:
  enum mode { TH_SYNC_CREATE = 0, TH_SYNC_USE = 1 };
  enum control { TH_SYNC_PREFIX = 112 };
  enum own_mode { TH_SYNC_NOOWN, TH_SYNC_THOWN, TH_SYNC_PROWN };
};

class thSyncErr {
 public:
  enum err { TH_SYNC_SUCCESS = 0, TH_SYNC_CFAIL = -1
      , TH_SYNC_ALREADY = -2, TH_SYNC_NOEXIST = -3
         , TH_SYNC_ATFAIL = -4, TH_SYNC_REMOVE = -5
         , TH_SYNC_FAIL = -6, TH_SYNC_INVARG = -7  };
};


/**
 *  don't use syncronization
 */
class thNoSync {
 public:
  thNoSync() : _key(-1)  {}
  virtual ~thNoSync()  { release(); }

 public:
  int init(size_t key, int nsems, int mode = thSyncDef::TH_SYNC_USE);
  int release();
  int enter();
  int leave();

 private:
  int _key;
};


/**
 *  use semaphore for synchronization between processes
 */
class thSem {
public:
  thSem() : _key(-1), _sem(NULL), _own_sem(thSyncDef::TH_SYNC_NOOWN) {}
  virtual ~thSem()  { release(); }

 public:
  int init(size_t key, int nsems, int mode = thSyncDef::TH_SYNC_USE);
  int release();
  int enter();
  int leave();

 private:
  int _key;
  sem_t* _sem;
  int _own_sem;
};


/**
 *  use mutex for synchronization between threads
 */
class thMutex {
 public:
  thMutex()  {}
  virtual ~thMutex()  {}

 public:
  int init(size_t, int, int);
  int release();
  int enter();
  int leave();

 private:
  pthread_mutex_t _mutex;
};
};  //  end of namespace thlib




#endif

