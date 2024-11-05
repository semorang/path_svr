/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "thmemalloc.h"

#if defined(WIN32) || defined(_WIN32_WCE)
thlib::HappyHeap* thlib::HappyHeap::_instance = NULL;
thlib::hwVirtualHandle* thlib::hwVirtualHandle::_instance = NULL;

#if defined(THBUG_EAT) || defined(_DEBUG)
size_t thlib::HappyHeap::_curalloced = 0;
size_t thlib::HappyHeap::_peakmem = 0;
#endif  //  #if THBUG_EAT || _DEBUG

#if defined(RR_MEMCHECK)
thlib::rrmn* thlib::rrmn::_instance = NULL;
size_t thlib::rrmn::_cur_mem = 0;
size_t thlib::rrmn::_peak_mem = 0;
#endif


#endif


