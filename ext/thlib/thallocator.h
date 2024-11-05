/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _thalloctor_
#define _thalloctor_

#include <setjmp.h>
#include "thmemalloc.h"
#include "therrno.h"

extern jmp_buf exception_jump;

template<typename T> class thallocator;

/** 
 * @brief void ���� Ưȭ �κ� 
 */
template<>
class thallocator<void> {
 public:
  typedef void* pointer;
  typedef const void* const_pointer;
  typedef void value_type;
  template<class U>
    struct rebind { 
      typedef thallocator<U> other; 
    };
};

/**
 * @brief �޸� ���� �� char*�� wchar_t�� �Ҹ��ڸ� �θ� �ʿ䰡 ����.
 */
namespace thstl_alloc {
  inline void destruct(char *pchar) {
    (void)pchar;
  }
  inline void destruct(wchar_t *pwchar) {
    (void)pwchar;
  }
  template <typename T>
    inline void destruct(T *t) { t->~T(); }
} // end namespae

template<typename T>
class thallocator {
 public:
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;

  template<class U>
    struct rebind { 
      typedef thallocator<U> other; 
    };
  thallocator() {}
  pointer address(reference x) const { return &x; }
  pointer allocate(size_type size, 
      thallocator<void>::const_pointer hint = 0) const {
    (void)hint;
    void *ret_address = NULL;
    ret_address = _mem.alloc(size);
#if defined(SUPPORT_MEMEXCEPTION)   
    // Exception process in C
    if (!ret_address) {
      longjmp(exception_jump, 1);
    }
#endif //SUPPORT_MEMEXCEPTION
    return static_cast<pointer>(ret_address);
  }

#if defined(WIN32) && !defined(_WIN32_WCE)
  char *_Charalloc(size_type n) { 
    return static_cast<char*>(_mem.alloc(n)); 
  }
#endif
  ///< ������ �ϴ� �ڵ�����??
  template<class U> thallocator(const thallocator<U>&) {}
  thallocator(const thallocator<T>&) {} //NOLINT
  void deallocate(pointer p, size_type n) const {
    (void)n; // make a warnning shut up
    //printf("deallocate p, size_type = [%d]\n", n);
    _mem.memfree(p);
  }
  void deallocate(void *p, size_type n) const {
    (void)n; // make a warnning shut up
    //printf("deallocate p, size_type specialize void[%d]\n", n);
    _mem.memfree(p);
  }
  size_type max_size() const { return size_t(-1) / sizeof(value_type); }
  void construct(pointer p, const T& val) {
    ///< �Ʒ� �ڵ�� �� �ϴ� ������?
    ///< ã�� ���� pointer p �� T(val)������ �޸𸮸� �����ض�.
    ///< �׷���, ���� ������ pointer p�� ������ �ش� ��ü�� �־��� ���̴�. 
    ///< ����� pointer�� �ٽ� init���ؼ� �츮�� ���̶�� �� �־���
    new(static_cast<void*>(p)) T(val);
  }
  void construct(pointer p) {
    new(static_cast<void*>(p)) T();
  }
  void destroy(pointer p) { thstl_alloc::destruct(p); }
  static void dump() { }
 private:
  static thlib::MemMalloc<T> _mem;
};

///< ???
template<typename T> thlib::MemMalloc<T> thallocator<T>::_mem;

template<typename T, typename U>
inline bool operator==(const thallocator<T>&, const thallocator<U>&) {
  return true;
}

template<typename T, typename U>
inline bool operator!=(const thallocator<T>&, const thallocator<U>&) {
  return true;
}

#ifdef _WIN32
#define ALLOC_CDECL __cdecl
#else
#define ALLOC_CDECL
#endif

/**
 * @brief �Ʒ� ������ stl�� ������ ���� �ֱ� ���� �ڵ� �ε�
 */
namespace std {
template<class _Tp1, class _Tp2>
inline thallocator<_Tp2>& ALLOC_CDECL
__stl_alloc_rebind(thallocator<_Tp1>& __a, const _Tp2*) {
  return (thallocator<_Tp2>&)(__a);
}

template<class _Tp1, class _Tp2>
inline thallocator<_Tp2> ALLOC_CDECL
__stl_alloc_create(const thallocator<_Tp1>& __a, const _Tp2*) {
  return thallocator<_Tp2>();
}


}/// namespace std

#endif


