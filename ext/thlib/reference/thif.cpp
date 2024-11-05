/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "../thif.h"

int thlib::thIf::createthread(void* tid, void* arg, int cflag
    , void* (*tfunc)(void*), void* tfarg) {
  return pthread_create((pthread_t*)tid, (pthread_attr_t*)arg, tfunc, tfarg);
}

