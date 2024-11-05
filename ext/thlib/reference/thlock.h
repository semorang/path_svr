/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THTHREAD_H__
#define _THTHREAD_H__


#include <pthread.h>



namespace thlib {


class thLock
{
 public:
  thLock() {
    pthread_mutex_init(&_mutex, NULL);
  }
  ~thLock() {
    pthread_mutex_destroy(&_mutex);
  }

 public:
  int entercs() {
    return pthread_mutex_lock(&_mutex);
  }
  int leavecs() {
    return pthread_mutex_unlock(&_mutex);
  }
  pthread_mutex_t* mutex() { return &_mutex; }

 private:
  pthread_mutex_t _mutex;
};
};  ///  end of namespace



#endif

