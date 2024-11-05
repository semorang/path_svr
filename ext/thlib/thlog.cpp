/* Copyright 2011 <happyteam@thinkwaresys.com> */
#include <setjmp.h>
#include "thlog.h"

jmp_buf exception_jump;

thlib::thLogMain* thlib::thLogMain::_instance = NULL;


#if !defined(_WIN32_WCE)
void thlib::WMSocket::destroy() {
  if (_plogdest) {
    //delete _plogdest;
    //_plogdest->~thSockettcpclt<thSingleThread>();
    thlib::MemPolicyNew<thSockettcpclt<thSingleThread> >::memfree(_plogdest);
    _plogdest = NULL;
  }
}

int thlib::WMSocket::init(char* loc) {
  if ((loc == NULL) && (_location.size() == 0)) {
    return -EPARAM;
  }
  if (_plogdest == NULL) {
    //_plogdest = new thSockettcpclt<thSingleThread>;
    _plogdest = thlib::MemPolicyNew<thSockettcpclt<thSingleThread> >::alloc(1);
  }
  if (_plogdest == NULL) {
    return -EMEM_ALLOC;
  }
  thString token;
  if (loc == NULL) {
    token = _location;
  } else {
    token = loc;
    _location = loc;
  }
  thString address = token.element(':');  
  thString port = token.element(':');

  _plogdest->release();
  _plogdest->timer(THSOCK_TIMEOUT);
  if (_plogdest->connect(address(), atoi(port())) != THSOCK_ERRSUC) {
    return -1;
  }
  return 0;
}

int thlib::WMSocket::write(char* modulename, char* msg) {
  thString delimeter(THLOG_DELIM);
  thString commsg;

  time_t curtime = time(NULL);
  commsg.printf("%s%s%d%s%s", modulename, delimeter(), curtime, 
                                                        delimeter(), msg);

  thNewArr<char> w_buf;
  if (w_buf.alloc(commsg.size() + 1) == NULL) {
    return -EMEM_ALLOC;
  }
  memcpy(w_buf(), commsg.pointer(), w_buf.binsize() - 1);
  w_buf[w_buf.binsize() - 1] = '\0';
  if (_plogdest->writev(w_buf) < 0) {
    return -1;
  }

  return 0;
}

#endif
