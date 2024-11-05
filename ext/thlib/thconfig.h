/**
 * ȯ�漳�� ���� , thlib�� �ֻ��� header
 * - �޸� �Ҵ� ���� ��å
 * - BUG MODE ���� 
 * - ....
 */
#ifndef _thconfig_
#define _thconfig_

#include "thtype.h"
/**
 * debug mode set, �����Ϸ����� ���� �ϵ��� �Ѵ�. 
 * - debug mode�� compile�Ѵ�. 
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
 * assert��� ����
 */
//#ifndef THUSE_ASSERT
//#define THUSE_ASSERT				///< assert �� ��� 
//#else
//#undef USE_ASSERT 				///< assert�� ������ ���� 
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
 *	@note	�޸� ��å
 *
 */

#define USE_TLSF	1

#ifndef USE_TLSF
#if defined(_WIN32_WCE)
	#ifndef MEM_HEAP_ALLOC
		#define MEM_HEAP_ALLOC	///< windows heap alloc�� ����Ѵ�. 
	#endif
#elif defined(WIN32)
	#ifndef MEM_HEAP_ALLOC
		#define MEM_HEAP_ALLOC	///< windows heap alloc�� ����Ѵ�. 
	#endif
#else
	#ifndef MEM_NEW
		#define MEM_NEW			///< defalut new alloc
	#endif
#endif
#endif

//#define MEM_HEAP_ALLOC			///< windows heap alloc�� ����Ѵ�. 
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
//	#define MAKE_DEFAULT_HEAP		///< default heap�� �̿��ϵ��� �Ѵ�. 
	#define MAKE_EXTERN_HEAP		///< heap�� �ܺο��� ������̴�. 
#endif

///< �� setting�� ������ alloc�� �ؾ��� 
//#include "thmemalloc.h"


#endif

