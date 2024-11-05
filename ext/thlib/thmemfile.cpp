/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thmemfile.h"

template class thlib::thMemFile<thlib::thStdFile>;


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 ��� 
   */
  template class thlib::thMemFile<thlib::thWinFile>;
  template<class file_type>
  char * thlib::thMemFile<file_type>::_mem_buf[512] = {NULL};
  template<class file_type>
  thlib::OpenFileList thlib::thMemFile<file_type>::_open_list;


#elif defined(WIN32)
  /**
   * EVC�� �ƴ� �͵��� windows 32bit�� ��� 
   */

  template class thlib::thMemFile<thlib::thWinFile>;
  template<class file_type>
    char * thlib::thMemFile<file_type>::_mem_buf[512] = {NULL};
  template<class file_type>
  thlib::OpenFileList thlib::thMemFile<file_type>::_open_list;
#else
  /**
   * �ϴ�, �������� ���⿡ 
   */
  template<typename file_type>
  char * thlib::thMemFile<file_type>::_mem_buf[512] = {NULL};
  template<typename file_type>
  thlib::OpenFileList thlib::thMemFile<file_type>::_open_list;
#endif


