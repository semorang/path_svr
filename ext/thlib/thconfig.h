/**
 * 환경설정 파일 , thlib의 최상위 header
 * - 메모리 할당 해제 정책
 * - BUG MODE 설정 
 * - ....
 */
#ifndef _thconfig_
#define _thconfig_

#include "thtype.h"
/**
 * debug mode set, 컴파일러에서 선언 하도록 한다. 
 * - debug mode로 compile한다. 
 */
//#define THBUG_EAT

//#ifndef THUSELOG
//#define THUSELOG
//#endif

#define SUPPORT_MEMEXCEPTION //junsung

//#define SUPPORT_THLOG //junsung
#if defined(SUPPORT_THLOG)
#define THUSELOG //junsung
#endif

/**
 * assert사용 여부
 */
//#ifndef THUSE_ASSERT
//#define THUSE_ASSERT				///< assert 를 사용 
//#else
//#undef USE_ASSERT 				///< assert를 강제로 삭제 
//#endif

/**
 *	define assert function that used thlib
 */
#if defined(THBUG_EAT) || defined(THUSE_ASSERT)
	#include <assert.h>
	#define thassert(_Expression)     assert(_Expression)
#else
	#define thassert(_Expression)     ((void)0)
#endif


/**
 *
 *	@note	메모리 정책
 *
 */

#define USE_TLSF	1

#ifndef USE_TLSF
#if defined(_WIN32_WCE)
	#ifndef MEM_HEAP_ALLOC
		#define MEM_HEAP_ALLOC	///< windows heap alloc을 사용한다. 
	#endif
#elif defined(WIN32)
	#ifndef MEM_HEAP_ALLOC
		#define MEM_HEAP_ALLOC	///< windows heap alloc을 사용한다. 
	#endif
#else
	#ifndef MEM_NEW
		#define MEM_NEW			///< defalut new alloc
	#endif
#endif
#endif

//#define MEM_HEAP_ALLOC			///< windows heap alloc을 사용한다. 
//#define MEM_NEW					///< defalut new alloc
//#define MEM_MALLOC				///< use malloc

/** 
 * check, is set memory policy ?
 */
#if defined(MEM_HEAP_ALLOC) || defined(MEM_NEW) || defined(MEM_MALLOC)
	//#pragma message("OK, Memory Policy was setting")
#else 
	///< if you don't set anything, the default is MEM_NEW
	#define MEM_NEW	
#endif

#if defined(MEM_HEAP_ALLOC)
//	#define MAKE_DEFAULT_HEAP		///< default heap을 이용하도록 한다. 
	#define MAKE_EXTERN_HEAP		///< heap을 외부에서 만들것이다. 
#endif

///< 위 setting을 가지고 alloc을 해야함 
//#include "thmemalloc.h"


#endif

