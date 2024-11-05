/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THTHREADS_H__
#define _THTHREADS_H__


#if defined(_WIN32_WCE)
  #include "thlib/thusewindows.h"

#elif defined(WIN32)
  #include "thlib/thusewindows.h"

#else

#endif


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
    CRITICAL_SECTION  _mutex;
  public:
    In() {
      ::InitializeCriticalSection(&_mutex);
    }
    ~In() {
      ::DeleteCriticalSection(&_mutex);
    }

    class Lock;
    friend class Lock;

    class Lock {
      thObjectLock::In<__HOST>& _host;

      Lock(const Lock&);
      Lock& operator=(const Lock&);
    public:
      Lock(thObjectLock::In<__HOST>& host) : _host(host) {//NOLINT
        ::EnterCriticalSection(&_host._mutex);
      }
      ~Lock() {
        ::LeaveCriticalSection(&_host._mutex);
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
      CRITICAL_SECTION _mutex;
      Initializer() {
        ::InitializeCriticalSection(&_mutex);
      }
      ~Initializer() {
        ::DeleteCriticalSection(&_mutex);
      }
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
        ::EnterCriticalSection(&_initializer._mutex);
      }
      Lock(thClassLock&) {//NOLINT
        ::EnterCriticalSection(&_initializer._mutex);
      }
      ~Lock() {
        ::LeaveCriticalSection(&_initializer._mutex);
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
