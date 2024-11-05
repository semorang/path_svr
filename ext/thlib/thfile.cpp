/* Copyright 2011 <happyteam@thinkwaresys.com> */
///////////////////////////////////
// Generated header: thfile.cpp
// Forwards to the appropriate code
// that works on the detected compiler
// Generated on Mon Sep 30 23:14:48 2002
///////////////////////////////////
#include "thfile.h"
/** 
 * thlib::thStdFile은 windows, linux, wince에서 다 만들어야 한다. 
 */
#if 0


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  template class thlib::thFile<thlib::thWinFile>;

#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */

  template class thlib::thFile<thlib::thWinFile>;
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif


template class thlib::thFile<thlib::thStdFile>;
template <typename file_type>
size_t thlib::thFile<file_type>::thnpos = 0xffffffff;

#endif


