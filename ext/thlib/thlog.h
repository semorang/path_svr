/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THLOG_
#define _THLOG_

#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #include "thusewindows.h"
  #if (_WIN32_WCE < 500)
  #include <errno.h>
  #endif
  #include "thsocket.h"

#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "thusewindows.h"
  #include <errno.h>
  #include "thsocket.h"
#else
  #include <errno.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include "thsocket.h"
  /**
   * 일단, 나머지는 여기에 
   */
#endif

#ifdef ANDROID
  #include <android/log.h>
#endif


#include "thtime.h"
#include <stdarg.h>
#include <map>
#include <iterator>
#include "thstring.h"
#include "thfile.h"
#include "thdir.h"
#include "thlock.h"
#include "thconfig.h"
#include "ththreads.h"

///< it is not used every where!
static thlib::thString g_pnm_thlib = "thlib";


namespace thlib {
  /**
   *  class LogRecord
   *    - manage log recording
   */
  class LogRecord  {
  public:
#if defined (S_IWOTH)
    enum { UMASKDIRMODE = S_IWOTH };
#else
    enum { UMASKDIRMODE = 0 };
#endif
  public:
    static int logloc(int uselast, struct tm* lastopen,
              thString& path, char* modulename) {
      time_t curtime = time(NULL);
      struct tm* curtimest = NULL;
      if (uselast) {
        curtimest = lastopen;
      } else {
        curtimest = localtime(&curtime);
      }
      if (curtimest == NULL) {
        return -1;
      }
      if ((uselast) || (lastopen == NULL) ||
          (lastopen->tm_year != curtimest->tm_year ||
          lastopen->tm_mon != curtimest->tm_mon ||
          lastopen->tm_mday != curtimest->tm_mday)) {
        thString bpath(path());
        thString pname;
        bpath.printf("%s/y%d", bpath(), curtimest->tm_year + 1900);
        if (thFS::makedir(bpath, UMASKDIRMODE) != 0) {
          return -1;
        }
        bpath.printf("%s/m%d", bpath(), curtimest->tm_mon + 1);
        if (thFS::makedir(bpath, UMASKDIRMODE) != 0) {
          return -1;
        }
        pname.printf("project_%s", modulename);
        bpath.printf("%s/%s", bpath(),  pname());
        if (thFS::makedir(bpath, UMASKDIRMODE) != 0) {
          return -1;
        }
        path.printf("%s/log_%s_d%d", bpath(), modulename, curtimest->tm_mday);
        if (lastopen != NULL) {
          memcpy((void*)lastopen, curtimest, sizeof(struct tm));
        }
        return 0;
      } else { 
        if (lastopen != NULL) {
          memcpy((void*)lastopen, curtimest, sizeof(struct tm));
        }
        return 1;
      }
    }  ///  end of function logloc
  };  ///  end of class LogRecord



  /**
   *  class WMFile
   *    - write log to file
   */
  class WMFile {
  public:
#if defined (S_IXUSR)
    enum { UMASKFILEMODE = S_IXUSR | S_IXGRP | S_IWOTH | S_IXOTH };
#else
    enum { UMASKFILEMODE = 0 };
#endif
  public:
    virtual ~WMFile()  { destroy(); }
  public:
    int init(char* loc) {
      if (loc != NULL) {
        _basepath(loc);
      }
      memset((void*)&_lastopen, 0, sizeof(_lastopen));
      return 0;
    }
    void destroy() {
      if (_logdest.isOpen()) {
        _logdest.close();
      }
    }
    int write(char* modulename, char* msg) {
      thString fpath(_basepath());
      int ret = LogRecord::logloc(false, &_lastopen, fpath, modulename);
      if (ret == -1) {
        return -1;
      } else if (ret == 0) {
        if (_logdest.isOpen()) {
          _logdest.close();
        }
        if (thFS::modeopen(_logdest, fpath, "ab", UMASKFILEMODE) != 0) {
          return -1;
        }
      }
      thString tmsg;
      tmsg.printf("%2d:%2d:%2d | %s\n", _lastopen.tm_hour,
          _lastopen.tm_min, _lastopen.tm_sec, msg);
      size_t len = tmsg.size();
      if (_logdest.write(tmsg, len) != len) {
        return -1;
      }
      if (_logdest.flush() != 0) {
        return -1;
      }

      return 0;
    }
  private:
    thFile<>  _logdest;
    thString  _basepath;
    struct tm  _lastopen;
  };  ///  end of class WMFile



  ///  forward declaration of thSockettcpclt

  #define  THLOG_DELIM    "ª"


  /**
   *  class WMSocket
   *    - write log to socket
   */
#if !defined(_WIN32_WCE)
  class WMSocket {
  public:
    WMSocket() : _plogdest(NULL) {}
    virtual ~WMSocket()  { destroy(); }
  public:
    int init(char* loc = NULL); 
    void destroy();
    int write(char* modulename, char* msg); 

  private:
    thSockettcpclt<thSingleThread>  *_plogdest;
    thlib::thString  _location;
  };  ///  end of WMSocket
#endif



  /**
   *  class thLog
   *    - write log using this class
   */  
#ifndef va_copy
  #define va_copy(d, s)  ((d) = (s))
#endif
  template<class WriteMethod>
  class thLog : public WriteMethod {
//  private:
  public:
    ///< prohibit from excution of default constructor
    thLog()  : _name("") {}
  public:
    thLog(char* logloc, char* name) {
//    void operator()(char* logloc, char* name) {
      WriteMethod::init(logloc);
      if (name != NULL)
        _name(name);
    }
    void operator()(char* logloc, char* name) {
      WriteMethod::init(logloc);
      if (name != NULL)
        _name(name);
    }
    virtual ~thLog() {
    }

  public:
    int write(const char* fmt, va_list list) {
      char *p, *r;
      thString buf, tmp, fname;

      ///  va_start have to be called in previous function
      //va_start(list, fmt);

      for (p = (char*)fmt; *p; ++p) {
        if (*p == '%') {
          ++p;
          size_t loc = 0;
          char plen[10];
          memset(plen, 0, 10);
          for (; *p; ++p, ++loc) {
            if (*p == 's') {
              ++p;
              break;
            } else {
              plen[loc] = *p;
            }
          }
          ///  be sure that first %s argument is __FILE__
          r = va_arg(list, char*);
          thString fname(r);
          fname = fname.substr(fname.rfind('/') + 1);
          fname = fname.substr(fname.rfind('\\') + 1);
          tmp.printf("%%%ss", plen);
          tmp.printf(tmp(), fname()); //NOLINT
          buf += tmp;
          break;
        } else {
          buf += *p;
        }
      }
      char *rembuf = thlib::MemPolicy<char>::alloc(2048);
      vsnprintf(rembuf, 2048, p, list);
      buf += rembuf;

      va_end(list);
      
      _lock.entercs();
      int ret =  WriteMethod::write(_name(), buf());
      _lock.leavecs();

      thlib::MemPolicy<char>::memfree(rembuf);
      return ret;
    }
  private:
    thString  _name;
    thLock  _lock;
  };  ///  end of class thLog



  /**
   *  class thLogMain
   *    - main class that manage all log object
   */
  class thLogMain {
  public:
  enum method { WMFILE, WMSOCKET };
  private:
    typedef std::map<thlib::thString, thLog<WMFile>*, std::less<thlib::thString>
      , thallocator<std::pair<thlib::thString, thLog<WMFile>*> > >   thLogListF;
#if !defined(_WIN32_WCE)
    typedef std::map<thlib::thString, thLog<WMSocket>*, 
            std::less<thlib::thString>, thallocator<std::pair<thlib::thString, 
            thLog<WMSocket>*> > >   thLogListS;
#endif
//  private:
  public:
    thLogMain()  { _instance = NULL; }
  public:
    ~thLogMain() { destroy(); }
    static thLogMain* instance() {
      if (_instance == NULL) {
        _instance = thlib::MemPolicyNew<thLogMain>::alloc(1);
        atexit(thlib::thLogMain::destroy_bridge);
      }
      return _instance;
    }
    ///< atexit를 call하기 위해서 __cdecl *형을 선언
    static void destroy_bridge() {
      if (thLogMain::_instance != NULL) {
        thLogMain::instance()->destroy();
        //thLogMain::_instance->~thLogMain();
        thlib::MemPolicyNew<thLogMain>::memfree(thLogMain::_instance);
        thLogMain::_instance = NULL;
      }
    }
    void destroy() {
      uint32_t i;
      for (i = 0; i < _loglistF.size(); ++i) {
        if (_loglistF[i] != NULL) {
          //delete _loglistF[i];
          //_loglistF[i]->~thLog<WMFile>();
          thlib::MemPolicyNew<thLog<WMFile> >::memfree(_loglistF[i]);
        }
      }
      _loglistF.clear();
#if !defined(_WIN32_WCE)
      for (i = 0; i < _loglistS.size(); ++i) {
        if (_loglistS[i] != NULL) {
          //_loglistS[i]->~thLog<WMSocket>();
          thlib::MemPolicyNew<thLog<WMSocket> >::memfree(_loglistS[i]);
          //delete _loglistS[i];
        }
      }
      _loglistS.clear();
#endif
    }
  public:
    int make(char* name, int method, char* logloc) {
      if (name == NULL || logloc == NULL) {
        return -EPARAM;
      }
      thString key(name);
      if (method == WMFILE) {
        thLogListF::iterator mi = _loglistF.find(key);
        if (mi != _loglistF.end()) {
          return -1;
        }
        //thLog<WMFile>* plogobj = new thLog<WMFile>(logloc, name);
        thLog<WMFile> *plogobj = thlib::MemPolicyNew<thLog<WMFile> >::alloc(1);
        (*plogobj)(logloc, name);
        _loglistF[key] = plogobj;
      } else if (method == WMSOCKET) {
#if !defined(_WIN32_WCE)
        thLogListS::iterator mi = _loglistS.find(key);
        if (mi != _loglistS.end()) {
          return -1;
        }
        //thLog<WMSocket>* plogobj = new thLog<WMSocket>(logloc, name);
        thLog<WMSocket>* plogobj = 
                   thlib::MemPolicyNew<thLog<WMSocket> >::alloc(1);
        (*plogobj)(logloc, name);
        _loglistS[key] = plogobj;
#endif
      } else {
        return -1;
      }

      return 0;
    }
    int write(thString& key, const char* fmt, ...) {
      va_list args;

      va_start(args, fmt);
      thLogListF::iterator mi = _loglistF.find(key);
      if (mi != _loglistF.end()) {
        return mi->second->write(fmt, args);
      }

#if !defined(_WIN32_WCE)
      thLogListS::iterator si = _loglistS.find(key);
      if (si != _loglistS.end()) {
        int ret = si->second->write(fmt, args);
        ///  if fail to write the log, reconnect to the server
        ///    and rewrite the log
        if (ret) {
          return ret;
        } else {
          si->second->init();
          return si->second->write(fmt, args);
        }
      }
#endif

      return -1;
    }

  public:
    static thLogMain *_instance;
  public:
    thLogListF  _loglistF;
#if !defined(_WIN32_WCE)
    thLogListS  _loglistS;
#endif
  };  ///  end of class thLogMain

#ifdef THUSELOG  
#ifdef ANDROID
#define  thMkLog(A, B, C)
#define  thWtLog0(key, A)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__); }
#define  thWtLog1(key, A, B)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B); }
#define  thWtLog2(key, A, B, C)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C); }
#define  thWtLog3(key, A, B, C, D)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D); }
#define  thWtLog4(key, A, B, C, D, E)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E); }
#define  thWtLog5(key, A, B, C, D, E, F)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F); }
#define  thWtLog6(key, A, B, C, D, E, F, G)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F, G); }
#define  thWtLog7(key, A, B, C, D, E, F, G, H)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H); }
#define  thWtLog8(key, A, B, C, D, E, F, G, H, I)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I); }
#define  thWtLog9(key, A, B, C, D, E, F, G, H, I, J)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I, J); }
#define  thWtLog10(key, A, B, C, D, E, F, G, H, I, J, K)  {\
    thlib::thString fmt("[%20s:%.5d]");\
    fmt.printf("%s%s", fmt(), A);\
    __android_log_print(ANDROID_LOG_INFO, "hw_engine", \
                 fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I, J, K); }
#else
#define thMkLog(A, B, C) thlib::thLogMain::instance()->make(A, B, C);
#define thWtLog0(key, A)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__); }
#define thWtLog1(key, A, B)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B); }
#define thWtLog2(key, A, B, C)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C); }
#define thWtLog3(key, A, B, C, D)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D); }
#define thWtLog4(key, A, B, C, D, E)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E); }
#define thWtLog5(key, A, B, C, D, E, F)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F); }
#define thWtLog6(key, A, B, C, D, E, F, G)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F, G); }
#define thWtLog7(key, A, B, C , D, E, F, G, H)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H); }
#define thWtLog8(key, A, B, C, D, E, F, G, H, I)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I); }
#define thWtLog9(key, A, B, C, D, E, F, G, H, I, J)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I, J); }
#define thWtLog10(key, A, B, C, D, E, F, G, H, I, J, K)  {\
    thlib::thString fmt("%20s:%.5d | ");\
    fmt = fmt + A;\
    thlib::thLogMain::instance()->write(key, fmt(), __FILE__, __LINE__, B, C, D, E, F, G, H, I, J, K); }
#endif  // end of #ifdef ANDROID
#else
#define  thMkLog(A, B, C)
#define  thWtLog0(key, A)
#define  thWtLog1(key, A, B)
#define  thWtLog2(key, A, B, C)
#define  thWtLog3(key, A, B, C, D)
#define  thWtLog4(key, A, B, C, D, E)
#define  thWtLog5(key, A, B, C, D, E, F)
#define  thWtLog6(key, A, B, C, D, E, F, G)
#define  thWtLog7(key, A, B, C, D, E, F, G, H)
#define  thWtLog8(key, A, B, C, D, E, F, G, H, I)
#define  thWtLog9(key, A, B, C, D, E, F, G, H, I, J)
#define  thWtLog10(key, A, B, C, D, E, F, G, H, I, J, K)

#endif // end of #ifdef THUSELOG

};  ///  end of namespace


#endif


