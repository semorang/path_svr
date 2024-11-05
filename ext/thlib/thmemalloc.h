/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THMEM_H__
#define _THMEM_H__

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #include "thusewindows.h"
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "thusewindows.h"
#else
  /**
   * 일단, 나머지는 여기에 
   */
  #include <stdlib.h>
  #include <string.h>
#endif

#include "thconfig.h"

/**
 * 생성자가 불려지는 allocator에 대해서 정리가 필요하다. 일단 new는 생성자가 불려지고, 
 * 나머지 malloc과 HeapAlloc의 경우는 생성자가 불려지지 않는다. 
 * 개수가 많은 객체를 생성하고, 생서자가 필요 없다면 당연히 malloc과 HeapAlloc이 좋을 
 * 꺼고, 생성자 처리가 필요하다면 스스로 생성자를 호출하면 된다. 당연히 소멸자도 
 * 불러야 한다. 
 * 
 * ex) SomeClass *ptr = thlib::MemPolicy<SomeClass>::alloc(1)
 *     ptr->SomeClass::SomeClass(); 
 *     ptr->~SomeClass();
 */
namespace thlib {

  template<class T> 
  class thMalloc {
  public:
    thMalloc() : _mem(NULL), _size(0) {}
    thMalloc(size_t size) : _mem(NULL), _size(0) {//NOLINT
      alloc(size);
    }

    ~thMalloc() {
      memfree();
    }

    T* alloc(size_t size) {
      if (size == 0) return _mem;
      _mem = (T*)malloc(size);
      if (_mem != NULL)
        _size = size;
      return _mem;
    }

    //memory allocation
    T*  alloc(size_t size, T ic) {
      if (size <= _size) return _mem;
      if (size == 0)     return _mem;
      _mem = (T*)malloc(size);
      if (_mem != NULL) {
        _size = size;
        memset(_mem, ic, size);
      }
      return _mem;
    }

    T* alloc_zero(size_t size) {
      if (size == 0) return _mem;
      _mem = (T*)malloc(size);
      if (_mem != NULL) {
        _size = size;
        memset(_mem, 0x00, size);
      }
      return _mem;
    }

    //memory free
    void memfree() {
      if (_mem != NULL) {
        free(_mem);
        _mem = NULL;
      }
      _size = 0;
    }

    void clear() {
      if (_size != 0) {
        memset(_mem, 0, sizeof(T) * _size);
      }
    }

    T*     pointer() { return _mem; }
    T*     operator()() { return &_mem[0]; }
        operator  void*() { return _mem; }
//        operator  T*() { return _mem; }
    T&    operator[](size_t n) { return _mem[n]; }
    size_t  size()    { return _size; }
    size_t  binsize() { return _size * sizeof(T); }

  private:
    T*    _mem;
    size_t  _size;
  };

  /**
   * @caution this class don't class a constructor
   */
  template<class T> 
  class thNewArr {
  public:
    thNewArr() : _mem(NULL), _size(0)  {}
    thNewArr(size_t size) : _mem(NULL), _size(0) {  alloc(size); } //NOLINT
  private:
    thNewArr(const thNewArr<T>& rhs) {//NOLINT
      *this = rhs;
    }
  public:
    ~thNewArr() { 
      if (_mem != NULL) {
        free(_mem);
        _mem = NULL;
      }
      _size = 0;
    }

  public:
    thNewArr<T>& operator=(const thNewArr<T>& rhs) {
      if (rhs._size) {
        if (alloc(rhs._size) == NULL) {
          memfree();
          return *this;
        }
        memcpy(_mem, rhs._mem, sizeof(T) * rhs._size);
      } else {
        memfree();
      }
      return *this;
    }
    T* operator()() { return &_mem[0]; }
    operator void*() { return _mem; }
    T& operator[](int n) { return _mem[n]; }

    T* alloc(size_t size) { 
      memfree();
      if (size == 0) return NULL;
      _mem = (T*)malloc(sizeof(T) * size);
      if (_mem != NULL)
        _size = size;
      return _mem;
    }
    T* alloc_zero(size_t size) { 
      memfree();
      if (size == 0) return NULL;
      _mem = (T*)malloc(sizeof(T) * size);
      if (_mem != NULL) {
        _size = size;
        memset(_mem, 0x00, sizeof(T) * size);
      }
      return _mem;
    }
    void memfree() {
      if (_mem != NULL) {
        free(_mem);
        _mem = NULL;
      }
      _size = 0;
    }
    void clear() {
      if (_size != 0) {
        memset(_mem, 0, sizeof(T) * _size);
      }
    }

  public:
    int copy_ds(char* buf, size_t ds, size_t size) {
      thassert(binsize() >= ds + size);
      memcpy((char*)_mem + ds, buf, size);
      return 0;
    }
    T*       pointer() { return _mem; }
    size_t   size() { return _size; }
    size_t   binsize() { return _size*sizeof(T); }
  private:
    T*    _mem;
    size_t   _size;
  };

  template<class T> 
  class thNewArrCtor {
  public:
    thNewArrCtor() : _mem(NULL), _size(0)  {}
    thNewArrCtor(size_t size) : _mem(NULL), _size(0) {  alloc(size); }//NOLINT
  private:
    thNewArrCtor(const thNewArr<T>& rhs) {//NOLINT
      *this = rhs;
    }
  public:
    ~thNewArrCtor() { 
      if (_mem != NULL) {
        delete[] _mem; 
        _mem = NULL;
      }
      _size = 0;
    }

  public:
    thNewArrCtor<T>& operator=(const thNewArrCtor<T>& rhs) {
      if (rhs._size) {
        if (alloc(rhs._size) == NULL) {
          memfree();
          return *this;
        }
        memcpy(_mem, rhs._mem, sizeof(T) * rhs._size);
      } else {
        memfree();
      }
      return *this;
    }
    T* operator()() { return &_mem[0]; }
    operator void*() { return _mem; }
    T& operator[](int n) { return _mem[n]; }

    T* alloc(size_t size) { 
      memfree();
      if (size == 0)
        return NULL;
      _mem = new T[size];
      if (_mem != NULL)
        _size = size;
      return _mem;
    }
    T* alloc_zero(size_t size) { 
      memfree();
      if (size == 0)
        return NULL;
      _mem = new T[size];
      if (_mem != NULL) {
        _size = size;
        memset(_mem, 0x00, sizeof(T) * size);
      }
      return _mem;
    }
    void memfree() {
      if (_mem != NULL) {
        delete[] _mem;
        _mem = NULL;
      }
      _size = 0;
    }
    void clear() {
      if (_size != 0) {
        memset(_mem, 0, sizeof(T) * _size);
      }
    }

  public:
    int copy_ds(char* buf, size_t ds, size_t size) {
      thassert(binsize() >= ds + size);
      memcpy((char*)_mem + ds, buf, size);
      return 0;
    }
    T*       pointer() { return _mem; }
    size_t   size() { return _size; }
    size_t   binsize() { return _size*sizeof(T); }
  private:
    T*    _mem;
    size_t   _size;
  };

  template<class T>
  class MemMalloc {
  public:
    MemMalloc() {}
    ~MemMalloc() {}

    //memory allocation
    static T* alloc(size_t size) {
      return (T*)malloc(sizeof(T) * size);
    }
    static T* alloc(size_t size, char init) {
      T* mem = (T*)malloc(sizeof(T) * size);
      if (mem != NULL)
        memset(mem, init, sizeof(T) * size);
      return mem;
    }
    static T* alloc_zero(size_t size) {
      T* mem = (T*)malloc(sizeof(T) * size);
      if (mem != NULL)
        memset(mem, 0x00, sizeof(T) * size);
      return mem;
    }
    static void memfree(void* mem) {
      if (mem != NULL) {
        free(mem);
        mem = NULL;
      }
    }
  };

  template<typename T>
  class MemNew {
  public:
    MemNew() {}
    ~MemNew() {}

    //memory allocation
    static T* alloc(size_t size) {
      return new T[size];
    }
    static T* alloc(size_t size, char init) {
      T* mem = new T[size];
      if (mem != NULL)
        memset(mem, init, sizeof(T) * size);
      return mem;
    }
    static T* alloc_zero(size_t size) {
      T* mem = new T[size];
      if (mem != NULL)
        memset(mem, 0x00, sizeof(T) * size);
      return mem;
    }
    static void memfree(T* mem) {
      if (mem != NULL) {
        delete[] mem;
        mem = NULL;
      }
    }
  };



  /**
   *  @todo rrMemNew is temporary class which check memory status.
   *
   *
   */
#if defined(RR_MEMCHECK) && (defined(WIN32) || defined(_WIN32_WCE))
  class rrmn {
  private:
    rrmn() { _heap_hdl = NULL; }
  public:
    ~rrmn() {}
    static rrmn* instance() {
      if (_instance == NULL) {
        _instance = new rrmn;
        SYSTEM_INFO  system_info;
        GetSystemInfo(&system_info); 
        _instance->_heap_hdl = 
          ::HeapCreate(0, system_info.dwPageSize, 0);
        atexit(destroy_bridge);
      }
      return _instance;
    }
    static void destroy_bridge() {
      if (_instance != NULL) {
        if (_instance->_heap_hdl != NULL) {
          ::HeapDestroy(_instance->_heap_hdl);
          _instance->_heap_hdl = NULL;
        }
        delete _instance;
        _instance = NULL;
      }
    }
    HANDLE heap_hdl() { return _heap_hdl; }

  public:
    static rrmn* _instance;
    HANDLE _heap_hdl;
    static size_t _cur_mem;
    static size_t _peak_mem;
  };

  template<class T>
  class rrMemNew {
    static T* alloc(size_t size) {
      T* mem = (T*)HeapAlloc(rrmn::instance()->heap_hdl(), 
            HEAP_NO_SERIALIZE, sizeof(T) * size);
      if (mem) {
        rrmn::_cur_mem += size * sizeof(T);
        if (rrmn::_peak_mem < rrmn::_cur_mem) {
          rrmn::_peak_mem = rrmn::_cur_mem;
        }
      }
      return mem;
    }

    static T* alloc_zero(size_t size) {
      T* mem = (T*)HeapAlloc(rrmn::instance()->heap_hdl(), 
            HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(T) * size);
      if (mem) {
        rrmn::_cur_mem += size * sizeof(T);
        if (rrmn::_peak_mem < rrmn::_cur_mem) {
          rrmn::_peak_mem = rrmn::_cur_mem;
        }
      }
      return mem;
    }

    static void memfree(T* mem) {
      if (mem != NULL) {
        rrmn::_cur_mem -= ::HeapSize(rrmn::instance()->heap_hdl(), 0, mem);
        HeapFree(rrmn::instance()->heap_hdl(), 0, mem);
        mem = NULL;
      }
    }
  };  //  end of class rrMemNew
#else
  template<class T>
  class rrMemNew {
  public :
    static T* alloc(size_t size) {
      return new T[size];
    }

    static T* alloc_zero(size_t size) {
      T* mem = new T[size];
      if (mem != NULL) {
        memset(mem, 0x00, sizeof(T) * size);
      }
      return mem;
    }

    static void memfree(T* mem) {
      if (mem != NULL) {
        delete[] mem;
        mem = NULL;
      }
    }
  };  //  end of class rrMemNew
#endif


  /**
   *  @brief  class MemNew wrapper class
   */
#if defined(WIN32) || defined(_WIN32_WCE)
  struct MemNewWrapper {
    template <class __T>
    struct In { 
      typedef rrMemNew<__T> type; 
    };
  };
#else
  struct MemNewWrapper {
    template <class __T>
    struct In { 
      typedef MemNew<__T> type; 
    };
  };
#endif

/**
 * @brief only use in win32 api
 */
#if defined(WIN32) || defined(_WIN32_WCE)
  class HappyHeap {
  private:
    HappyHeap() { 
#if defined(THBUG_EAT) || defined(_DEBUG)
      _curalloced = 0;
      _peakmem = 0;
#endif
      _heap_handle = NULL; 
    }
  public:
    ~HappyHeap() { }

    HANDLE handle() { return _heap_handle; }
    void handle(HANDLE handle) { _heap_handle = handle; }

    static HappyHeap* instance() {
      if (_instance == NULL) {
        /// new HappyHeap()와 new HappyHeap은 서로 다른 의미 아닌가?? 
        _instance = new HappyHeap;
        ///< we don't use external memory alloc, anymore
//#if defined(MEM_HEAP_ALLOC) && defined(MAKE_DEFAULT_HEAP)
        ///< heap alloc을 사용하고, 외부에서 heap을 생성해서 주지 않으면 
        ///< 여기서 heap handle을 만들어 사용 
        //HANDLE heaphandle = NULL; ///< windows invalidate value
        SYSTEM_INFO  system_info;
        GetSystemInfo(&system_info); 
        _instance->_heap_handle = 
          ::HeapCreate(0, system_info.dwPageSize, 0);
//#endif
        atexit(thlib::HappyHeap::destroy_bridge);
        return _instance;
      }
      return _instance;
    }
    static void destroy_bridge() {
      if (HappyHeap::_instance != NULL) {
        ///< it time to destroy _heap_handle, it must be NULL
        ///< user delete
        if (HappyHeap::_instance->_heap_handle != NULL) {
          ::HeapDestroy(HappyHeap::_instance->_heap_handle);
          HappyHeap::_instance->_heap_handle = NULL;
        }
        delete HappyHeap::_instance;
        HappyHeap::_instance = NULL;
      }
    }
  public:
    HANDLE  _heap_handle;
    static  HappyHeap* _instance;

#if defined(THBUG_EAT) || defined(_DEBUG)
    static size_t _curalloced;
    static size_t _peakmem;
#endif
  };


  /**
   *  class MemHeapAlloc<>
   *  @brief  memory allocation class that use windows private heap handle
   *  @caution  the api about windows heap is protected from thread???
   */
  template<class T = char>
  class MemHeapAlloc {
  public:
    MemHeapAlloc() {}
    ~MemHeapAlloc() {}
    ///  memory allocation
    static T* alloc(size_t size) {
      thassert(size != 0);
      T* mem = (T*)HeapAlloc(HappyHeap::instance()->handle(), 
            HEAP_NO_SERIALIZE, sizeof(T) * size);

#if defined(THBUG_EAT) || defined(_DEBUG)
      if (mem != NULL) {
        size_t allocsize = sizeof(T) * size;
        //TRACE("happyheap alloced(%d)\n", allocsize);
        size_t heapsize = HeapSize(HappyHeap::instance()->handle(), 0, mem);
        HappyHeap::_curalloced += heapsize;

        if (HappyHeap::_peakmem < HappyHeap::_curalloced) {
          HappyHeap::_peakmem = HappyHeap::_curalloced;
          size_t ccj = HappyHeap::_peakmem;
        }
      }
      size_t testhaha = HappyHeap::_peakmem;
#endif
      return mem;
    }

    static T* alloc(size_t size, char init) {
      thassert(size != 0);
      int alloc_option = init == 0 ? 
                (HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY) : 
                HEAP_NO_SERIALIZE;
      T* mem = (T*)HeapAlloc(HappyHeap::instance()->handle(), 
            alloc_option, sizeof(T) * size);

#if defined(THBUG_EAT) || defined(_DEBUG)
      if (mem != NULL) {
        size_t allocsize = sizeof(T) * size;
        //TRACE("happyheap alloced(%d)\n", allocsize);
        size_t heapsize = HeapSize(HappyHeap::instance()->handle(), 0, mem);
        HappyHeap::_curalloced += heapsize;
        if (HappyHeap::_peakmem < HappyHeap::_curalloced) {
          HappyHeap::_peakmem = HappyHeap::_curalloced;
          size_t ccj = HappyHeap::_peakmem;
        }
      }
      size_t testhaha = HappyHeap::_peakmem;
#endif
      return mem;
    }

    static T* alloc_zero(size_t size) {
      thassert(size != 0);
      T* mem = (T*)HeapAlloc(HappyHeap::instance()->handle(), 
            HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY, sizeof(T) * size);

#if defined(THBUG_EAT) || defined(_DEBUG)
      if (mem != NULL) {
        size_t allocsize = sizeof(T) * size;
        //TRACE("happyheap alloced(%d)\n", allocsize);
        size_t heapsize = HeapSize(HappyHeap::instance()->handle(), 0, mem);
        HappyHeap::_curalloced += heapsize;

        if (HappyHeap::_peakmem < HappyHeap::_curalloced) {
          HappyHeap::_peakmem = HappyHeap::_curalloced;
          size_t ccj = HappyHeap::_peakmem;
        }
      }
      size_t testhaha = HappyHeap::_peakmem;
#endif
      return mem;
    }

    ///  memory free
    static void memfree(T* mem) {
      if (HappyHeap::_instance != NULL
          && HappyHeap::_instance->_heap_handle != NULL) {
#if defined(THBUG_EAT) || defined(_DEBUG)
        thassert(mem != NULL);
        size_t freedsize = HeapSize(HappyHeap::instance()->handle(), 0, mem);
        //TRACE("happyheap freed(%d)\n", freedsize);
        HappyHeap::_curalloced -= 
                     HeapSize(HappyHeap::instance()->handle(), 0, mem);
#endif
        HeapFree(HappyHeap::_instance->handle(), 0, mem);
      }
      mem = NULL;
    }
  };



/////////////////////////////////////////////////
  class hwVirtualHandle {
  private:
    hwVirtualHandle() { 
      _ptr_org = NULL;
      _ptr_cur = NULL;
      _last_alloc_size = 0;
    }
  public:
    ~hwVirtualHandle() { }

    static hwVirtualHandle* instance() {
      if (_instance == NULL) {
        /// new HappyHeap()와 new HappyHeap은 서로 다른 의미 아닌가?? 
        _instance = new hwVirtualHandle;
        _instance->_ptr_org = ::VirtualAlloc(NULL, 4 * 1024 * 1024, 
                                           MEM_RESERVE, PAGE_NOACCESS);
        if (_instance->_ptr_org == NULL) {
          thassert(0);
        }
        _instance->_ptr_org = 
          ::VirtualAlloc(_instance->_ptr_org, 4 * 1024 * 1024, 
                                           MEM_COMMIT, PAGE_READWRITE);
        if (_instance->_ptr_org == NULL) {
          thassert(0);
        }
        _instance->_ptr_cur = _instance->_ptr_org;
        ///< 마지막 alloc size를 기억
        _instance->_last_alloc_size = 0;
        /*
        SYSTEM_INFO  system_info;
        GetSystemInfo(&system_info); 
        _instance->_heap_handle = 
          ::HeapCreate(0, system_info.dwPageSize, 0);
        */

        atexit(thlib::hwVirtualHandle::destroy_bridge);
        return _instance;
      }
      return _instance;
    }
    static void destroy_bridge() {
      if (hwVirtualHandle::_instance != NULL) {
        if (hwVirtualHandle::_instance->_ptr_org != NULL) {
          ///< virtual free
          ::VirtualFree(hwVirtualHandle::_instance->_ptr_org, 0, MEM_RELEASE);
          hwVirtualHandle::_instance->_ptr_org = NULL;
          hwVirtualHandle::_instance->_ptr_cur = NULL;
        }
        delete hwVirtualHandle::_instance;
        hwVirtualHandle::_instance = NULL;
      }
    }
  public:
    void*  _ptr_org;
    void*  _ptr_cur;
    size_t  _last_alloc_size;
    static  hwVirtualHandle* _instance;
  };
  /**
   *  class 
   *  @brief  
   *  @caution ring buffer형식으로 삭제를 해야 한다. 삭제 시 주의 
   */
  template<class T = char>
  class MemVrtlAlloc {
  public:
    MemVrtlAlloc() {}
    ~MemVrtlAlloc() {}
    ///  memory allocation
    static T* alloc(size_t size) {
      thassert(size != 0);
      T* mem = (T*)hwVirtualHandle::instance()->_ptr_cur;
      size_t obj_size = sizeof(T) * size;
      size_t pad_size = 0, n = 1;
      while (obj_size > (pad_size = 1 << n)) {
        n++;
      }
      hwVirtualHandle::instance()->_ptr_cur = 
        (char*)hwVirtualHandle::instance()->_ptr_cur + pad_size;
      hwVirtualHandle::instance()->_last_alloc_size = pad_size;
      return mem;
    }
    static T* alloc(size_t size, char init) {
      thassert(size != 0);
      T* mem = (T*)hwVirtualHandle::instance()->_ptr_cur;
      size_t obj_size = sizeof(T) * size;
      size_t pad_size = 0, n = 1;
      while (obj_size > (pad_size = 1 << n)) {
        n++;
      }
      hwVirtualHandle::instance()->_ptr_cur = 
        (char*)hwVirtualHandle::instance()->_ptr_cur + pad_size;
      hwVirtualHandle::instance()->_last_alloc_size = pad_size;
      if (init == 0) {
        memset(mem, 0x00, pad_size);
      }
      return mem;
    }
    static T* alloc_zero(size_t size) {
      thassert(size != 0);
      T* mem = (T*)hwVirtualHandle::instance()->_ptr_cur;
      size_t obj_size = sizeof(T) * size;
      size_t pad_size = 0, n = 1;
      while (obj_size > (pad_size = 1 << n)) {
        n++;
      }
      hwVirtualHandle::instance()->_ptr_cur = 
        (char*)hwVirtualHandle::instance()->_ptr_cur + pad_size;
      hwVirtualHandle::instance()->_last_alloc_size = pad_size;
      memset(mem, 0x00, pad_size);
      return mem;
    }
    ///  memory free
    static void memfree(T* mem) {
      void* cur_mem = (char*)hwVirtualHandle::instance()->_ptr_cur - 
        hwVirtualHandle::instance()->_last_alloc_size;
//      thassert(mem == cur_mem);
      hwVirtualHandle::instance()->_ptr_cur = mem;
    }
  };


#endif ///< end WIN32 || _WIN32_WCE

#ifndef USE_TLSF
  #if defined(_WIN32_WCE)
    /**
     * EVC3.0 EVC4.0 등등 
     */
    #if defined(MEM_HEAP_ALLOC)
      template<class T>
      class MemPolicy : public MemHeapAlloc<T> {};
      template<class T>
      class MemPolicyNew : public MemHeapAlloc<T> {};
    #elif defined(MEM_MALLOC)
      template<class T>
      class MemPolicy : public MemMalloc<T> {};
      template<class T>
      class MemPolicyNew : public MemMalloc<T> {};
    #else
      template<class T>
      class MemPolicy : public MemNew<T> {};
      template<class T>
      class MemPolicyNew : public MemNew<T> {};
    #endif
  #elif defined(WIN32)
    /**
     * EVC가 아닌 것들중 windows 32bit인 놈들 
     */
    #if defined(MEM_HEAP_ALLOC)
      template<class T>
      class MemPolicy : public MemHeapAlloc<T> {};
      template<class T>
      class MemPolicyNew : public MemHeapAlloc<T> {};
    #elif defined(MEM_MALLOC)
      template<class T>
      class MemPolicy : public MemMalloc<T> {};
      template<class T>
      class MemPolicyNew : public MemMalloc<T> {};
    #else
      template<class T>
      class MemPolicy : public MemNew<T> {};
      template<class T>
      class MemPolicyNew : public MemNew<T> {};
    #endif
  #else
    /**
     * 일단, 나머지는 여기에 
     */
    #if defined(MEM_HEAP_ALLOC)
      ///< heap alloc은 win32쪽에만 있음, default로 가라 
      template<class T>
      class MemPolicy : public MemNew<T> {};
      template<class T>
      class MemPolicyNew : public MemNew<T> {};
    #elif defined(MEM_MALLOC)
      template<class T>
      class MemPolicy : public MemMalloc<T> {};
      template<class T>
      class MemPolicyNew : public MemMalloc<T> {};
    #else
      template<class T>
      class MemPolicy : public MemNew<T> {};
      template<class T>
      class MemPolicyNew : public MemNew<T> {};
    #endif
  #endif
#else
  template<class T>
  class MemPolicy : public MemMalloc<T> {
  public :
    static size_t peak_mem() {
      return 0;
    }
    static size_t cur_mem() {
      return 0;
    }
    static void init_max_memory() {
    }
  };

  template<class T>
  class MemPolicyNew : public MemNew<T> {};
#endif
} // end name space thlib

#endif

