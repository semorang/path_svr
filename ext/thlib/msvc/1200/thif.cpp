/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include "../thif.h"

int thlib::thIf::createthread(void* tid, void* arg, int cflag
    void* (*tfunc)(void*), void* tfarg) {
  return ::CreateThread(arg, 0, tfunc, tfarg, cflag, tid);
}

