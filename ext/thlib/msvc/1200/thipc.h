/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THIPC_MSVC_H__
#define _THIPC_MSVC_H__


#if defined(_WIN32_WCE)
  /**
   * EVC3.0 EVC4.0 등등 
   */
  #include "thusewindows.h"
  #pragma warning(disable : 4786)
#elif defined(WIN32)
  /**
   * EVC가 아닌 것들중 windows 32bit인 놈들 
   */
  #include "../../thusewindows.h"
  #pragma warning(disable : 4786)
#else
  /**
   * 일단, 나머지는 여기에 
   */
#endif


#include <assert.h>
#include "../../thmemalloc.h"
#include "../../thstring.h"


namespace thlib {



/**
 *  define
 */
class thIpcDef {
 public:
  enum create_mode { TH_IPC_CREATE = 0, TH_IPC_USE = 1 };
  //  0 : wait for data, 1 : return error, 2 : force writing
  enum data_mode { TH_IPC_WAIT = 0, TH_IPC_RETURN = 1
        , TH_IPC_IGNORE = 3 };
  enum control { TH_IPC_PREFIX = 111, TH_IPC_BUFFSIZE = 1048576
      , TH_IPC_PULLTIME = 1
  };
};


/**
 *  error code
 */
class thIpcErr {
 public:
  enum err { TH_IPC_SUCCESS = 0, TH_IPC_CFAIL = -1
      , TH_IPC_ALREADY = -2, TH_IPC_NOEXIST = -3
         , TH_IPC_ATFAIL = -4, TH_IPC_REMOVE = -5
         , TH_IPC_FAIL = -6, TH_IPC_INVARG = -7
      , TH_IPC_INIT = -8, TH_IPC_SYNC = -9
      , TH_IPC_OVERFLOW = -10
  };
  enum status { TH_IPC_DATAEXIST = 1, TH_IPC_DATANEXIST = 2 };
};




/**
 * inter-process communications with shared memory
 */
  template <class __SyncMethod>
class thIpcSM {
 public:
  class IpcHdr {
  public:
    enum valid { TH_IPCD_VALID = 0, TH_IPCD_INVALID = 1 };
  public:
    int _valid;
    int _msgsize;
  };
 public:
  thIpcSM() : _key(INVALID_HANDLE_VALUE), _buff(NULL)
        , _bsize(-1), _own_mem(0)  {}
  virtual ~thIpcSM()  { close(); }

 public:
  int open(const char* key, const int mode = thIpcDef::TH_IPC_CREATE
        , const int bsize = TH_IPC_BUFFSIZE);
  int isopen() { return (_key == INVALID_HANDLE_VALUE)?0:1; }
  int close();
  int read(char* buff, const int size
        , const int dmode = thIpcDef::TH_IPC_WAIT);
  int write(const char* buff, const int size
        , const int dmode = thIpcDef::TH_IPC_WAIT);
  char* smbuff() { return _buff; }
 private:
  HANDLE _key;
  char* _buff;
  int _bsize;
  int _own_mem;      //  create shared memory directly?
  __SyncMethod _sync;
};



template <class __SyncMethod>
int thIpcSM<__SyncMethod>::open(const char* key
          , const int mode, const int bsize) {
  if (key == NULL) {
    return thIpcErr::TH_IPC_INVARG;
  }
  thString key_str;
  //key_str.printf("%d", thIpcDef::TH_IPC_PREFIX);
  key_str += key;
  int o_key = atoi(key);
  int ret = 0;
  void* ret_a = NULL;

  if (_sync.init(o_key, 1, mode)) {
    return thIpcErr::TH_IPC_INIT;
  }
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }


#if defined(_UNICODE)
  wchar_t conv_string[1024] = { L"", };
  uint32_t acp = ::IsValidLocale(1042, LCID_INSTALLED)?949:GetACP();
  ::MultiByteToWideChar(acp, 0, key_str(), key_str.size()+1
      , conv_string, sizeof(conv_string)/sizeof(conv_string[0]));
#else
  char conv_string[256] = { 0, };
  snprintf(conv_string, sizeof(conv_string), "%s", key_str());
#endif
  HANDLE ret_h = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL
        , PAGE_READWRITE, 0, bsize + sizeof(IpcHdr), conv_string);
  if (ret_h == NULL) {
    close();
    return thIpcErr::TH_IPC_CFAIL;
  }
  if (mode == thIpcDef::TH_IPC_CREATE) {
    if (::GetLastError() == ERROR_ALREADY_EXISTS) {
      close();
      return thIpcErr::TH_IPC_ALREADY;
    }
    _own_mem = 1;
  } else if (mode == thIpcDef::TH_IPC_USE) {
    if (::GetLastError() != ERROR_ALREADY_EXISTS) {
      close();
      return thIpcErr::TH_IPC_NOEXIST;
    }
    _own_mem = 0;
  }
  _key = ret_h;
  ret_a = ::MapViewOfFile(_key, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (ret_a == NULL) {
    close();
    return thIpcErr::TH_IPC_ATFAIL;
  }
  _buff = (char*)ret_a;
  _bsize = bsize;

  if (mode == thIpcDef::TH_IPC_CREATE) {
    IpcHdr tmphdr;
    tmphdr._valid = IpcHdr::TH_IPCD_INVALID;
    tmphdr._msgsize = 0;
    memcpy(_buff + bsize, &tmphdr, sizeof(IpcHdr));
  }

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  return thIpcErr::TH_IPC_SUCCESS;
}


template <class __SyncMethod>
int thIpcSM<__SyncMethod>::close() {
  if (_buff != NULL) {
    if (!UnmapViewOfFile(_buff)) {
      return thIpcErr::TH_IPC_REMOVE;
    }
    _buff = NULL;
  }
  if (_key != INVALID_HANDLE_VALUE) {
    if (_own_mem == 1) {
      CloseHandle(_key);
      _own_mem = 0;
    }
    _key = INVALID_HANDLE_VALUE;
  }
  _bsize = -1;
  return _sync.release();
}


template <class __SyncMethod>
int thIpcSM<__SyncMethod>::read(char* buff
            , const int size, const int dmode) {
  IpcHdr tmphdr;
  while (1) {
    if (_sync.enter()) {
      return thIpcErr::TH_IPC_SYNC;
    }

    memcpy(&tmphdr, _buff + _bsize, sizeof(IpcHdr));
    if (tmphdr._valid == IpcHdr::TH_IPCD_INVALID) {
      if (_sync.leave()) {
        return thIpcErr::TH_IPC_SYNC;
      }
      if (dmode == thIpcDef::TH_IPC_WAIT) {
        ::Sleep(thIpcDef::TH_IPC_PULLTIME * 1000);
        continue;
      } else if (dmode == thIpcDef::TH_IPC_RETURN) {
        return thIpcErr::TH_IPC_DATANEXIST;
      } else if (dmode == thIpcDef::TH_IPC_IGNORE) {
        break;
      } else {
        thassert(false);
      }
    } else {
      break;
    }
  }  //  end of while
  
  int r_size = (size == -1)?tmphdr._msgsize:size;
  if (r_size > _bsize) {
    return thIpcErr::TH_IPC_OVERFLOW;
  }

  memcpy(buff, _buff, r_size);
  tmphdr._valid = IpcHdr::TH_IPCD_INVALID;
  tmphdr._msgsize = 0;
  memcpy(_buff + _bsize, &tmphdr, sizeof(IpcHdr));

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}


template <class __SyncMethod>
int thIpcSM<__SyncMethod>::write(const char* buff
            , const int size, const int dmode) {
  IpcHdr tmphdr;
  while (1) {
    if (_sync.enter()) {
      return thIpcErr::TH_IPC_SYNC;
    }

    memcpy(&tmphdr, _buff + _bsize, sizeof(IpcHdr));
    if (tmphdr._valid == IpcHdr::TH_IPCD_VALID) {
      if (_sync.leave()) {
        return thIpcErr::TH_IPC_SYNC;
      }
      if (dmode == thIpcDef::TH_IPC_WAIT) {
        ::Sleep(thIpcDef::TH_IPC_PULLTIME * 1000);
        continue;
      } else if (dmode == thIpcDef::TH_IPC_RETURN) {
        return thIpcErr::TH_IPC_DATAEXIST;
      } else if (dmode == thIpcDef::TH_IPC_IGNORE) {
        break;
      } else {
        thassert(false);
      }
    } else {
      break;
    }
  }  //  end of while

  if (size > _bsize) {
    return thIpcErr::TH_IPC_OVERFLOW;
  }

  memcpy(_buff, buff, size);
  tmphdr._valid = IpcHdr::TH_IPCD_VALID;
  tmphdr._msgsize = size;
  memcpy(_buff + _bsize, &tmphdr, sizeof(IpcHdr));

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}




};  //  end of namespace thlib



#endif

