/* Copyright 2011 <happyteam@thinkwaresys.com> */
///////////////////////////////////
// Generated header: thfile.cpp
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
///////////////////////////////////
#include "thfile.h"
/** 
 * thlib::thStdFile�� windows, linux, wince���� �� ������ �Ѵ�. 
 */
#if 0


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ��� 
   */
  template class thlib::thFile<thlib::thWinFile>;

#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */

  template class thlib::thFile<thlib::thWinFile>;
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
#endif


template class thlib::thFile<thlib::thStdFile>;
template <typename file_type>
size_t thlib::thFile<file_type>::thnpos = 0xffffffff;

#endif


