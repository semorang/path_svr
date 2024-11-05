/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THTHREADS_H__
#define _THTHREADS_H__



#include <pthread.h>



namespace thlib {


/**
 *  @brief
 *  thread policy classes
 *
 *
 *
 */


struct thSingleThread {
 public:
  template <class __HOST>
  struct In {
    typedef __HOST VolatileType;
    
    struct Lock {
      Lock()  {}
      Lock(const thSingleThread::In<__HOST>&)  {}//NOLINT
    };
  };
};


struct thObjectLock {
  template <class __HOST>
  struct In {
  private:
    pthread_mutex_t  _mutex;
  public:
    In() {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init(&attr);
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&_mutex, &attr);
    }
    ~In() {}

    class Lock;
    friend class Lock;

    class Lock {
      thObjectLock::In<__HOST>& _host;

      Lock(const Lock&);
      Lock& operator=(const Lock&);
    public:
      Lock(thObjectLock::In<__HOST>& host) : _host(host) {//NOLINT
        pthread_mutex_lock(&_host._mutex);
      }
      ~Lock() {
        pthread_mutex_unlock(&_host._mutex);
      }
    };  //  end of class Lock

    typedef volatile __HOST VolatileType;
  };  //  end of struct In
};  //  end of class thObjectLock


struct thClassLock {
  template <class __HOST>
  struct In {
  public:
    struct Initializer;
    friend struct Initializer;
    struct Initializer {
      pthread_mutex_t _mutex;
      Initializer() {
        pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_mutex, &attr);
      }
      ~Initializer() {}
    };  //  end of struct Initializer

    static Initializer _initializer;

  public:
    class Lock;
    friend class Lock;
    class Lock {
      Lock(const Lock&);
      Lock& operator=(const Lock&);
    public:
      Lock() {
        pthread_mutex_lock(&_initializer._mutex);
      }
      Lock(thClassLock&) {//NOLINT
        pthread_mutex_lock(&_initializer._mutex);
      }
      ~Lock() {
        pthread_mutex_unlock(&_initializer._mutex);
      }
    };  //  end of class Lock

    typedef volatile __HOST VolatileType;
  };  //  end of struct In
};  //  end of class thClassLock

template <class __HOST>
typename thClassLock::template In<__HOST>::Initializer 
  thClassLock::template In<__HOST>::_initializer;




};  /// end of namespace





#endif
