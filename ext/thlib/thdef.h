/* Copyright 2011 <happyteam@thinkwaresys.com> */
/**
 * @mainpage thlib
 * @section intro �Ұ�
 *  - Ž�� ��⿡�� ���� �ִ� ����(?) ���̺귯����� �ұ�???
 * @section developer ������
 *   - cjchoi@thinkwaresys.com
 *   - nineye@thinkwaresys.com
 *   - purism01@thinkwaresys.com
 * @section history  �̷�
 *  - 2007.01 purism01 - thgeo�� TM ��ǥ ��ȯ �κ� �߰�.
 *
 * CopyRight (c) 2003, Thinkwaresys Navigation R&D division
 */
#ifndef  _THDEF_
#define  _THDEF_

///////////////////////////////////////


//#define _WINDOG_
#include "thconfig.h"
#include "thtype.h"

#define THSTRING_MAX          2048

// maximum value 
#define WORD_MAX             (0xFFFF)
#define DWORD_MAX             (0xFFFFFFFF)
//#define USHORT_MAX             (0xFFFF)


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ��� 
   */
  #ifndef USHORT_MAX              
    #define USHORT_MAX            (0xFFFF)
  #endif

  #if (_MSC_VER < 1900)
    #define snprintf _snprintf
  #endif
/*
  #if (_WIN32_WCE >= 400)
    #define VC_BROKEN_STD std
  #else
    #define VC_BROKEN_STD std
  #endif
  #define VC_USING_STD std
*/
//#pragma intrinsic (sqrt, memcpy, memset)

#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */
  #ifndef USHORT_MAX              
    #define USHORT_MAX            (0xFFFF)
  #endif

  #if (_MSC_VER < 1900)
    #define snprintf _snprintf
  #endif
/*
  #if (_MSC_VER <= 1200)
    #define VC_BROKEN_STD std
  #else
    #define VC_BROKEN_STD std
  #endif
  #define VC_USING_STD std
*/

#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
  #ifndef UCHAR_MAX
    #define UCHAR_MAX             (0xFF)
  #endif
  #ifndef USHORT_MAX              
    #define USHORT_MAX            (0xFFFF)
  #endif
  #ifndef UINT_MAX
    #define UINT_MAX             (0xFFFFFFFF)
  #endif
/*
  #define VC_USING_STD std
*/
#endif


/* 
   BEGIN_C_DECLS should be used at the begining of your declarations, 
   so that c++ compilers don't mangle their names. Use END_C_DECLS at
   the end of C declarations. 
*/
#undef BEGIN_C_DECLS
#undef END_C_DECLS
#ifdef __cplusplus
# define BEGIN_C_DECLS extern "C" {
# define END_C_DECLS }
#else
# define BEGIN_C_DECLS /* empty */
# define END_C_DECLS /* empty */
#endif

/*
    PARAMS is a macro used to wrap function prototypes, so that
    compilers that don't understand ANSI C prototypes still work,
    and ANSI C compilers can issue warnings about type mismatches.
*/
#undef PARAMS
#if defined(__STDC__) || defined(_AIX) \
        || (defined(__mips) && defined(_SYSTYPE_SVR4)) \
        || defined(WIN32) || defined(__cplusplus)
# define PARAMS(protos) protos
#else
# define PARAMS(protos) ()
#endif




#define SWAP(type, a, b)  \
    {          \
      type c;      \
      c = a;      \
      a = b;      \
      b = c;      \
    }          \




#define DISALLOW_COPY_AND_ASSIGN(__typename)  \
  __typename(const __typename&);        \
  void operator=(const _typename&)

#endif

