/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THIPC_REFERENCE_H__
#define _THIPC_REFERENCE_H__


#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include "thlib/thtype.h"
#include "thlib/thstring.h"


namespace thlib {


/**
 *  define
 */
class thIpcDef {
 public:
  enum create_mode { TH_IPC_CREATE = 0, TH_IPC_USE = 1 };
  enum data_mode { TH_IPC_WAIT = 0, TH_IPC_RETURN = 1 };
  enum own_mode { TH_IPC_NOOWN, TH_IPC_THOWN, TH_IPC_PROWN };
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
 * inter-process communications with signal
 */
class thIpcSig {
 public:
  thIpcSig()  {}
  virtual ~thIpcSig()  {}

 public:
};


/**
 * inter-process communications with message trasfer
 */
class thIpcMsg {
 public:
  thIpcMsg()  {}
  virtual ~thIpcMsg()  {}

 public:
};

/**
 * inter-process communications with shared memory
 */
template <class __SyncMethod>
class thIpcSMMsg {
public:
  enum { THIS_SHMNG_SIZE = 256 };
public:
  class IpcHdr {
  public:
    enum valid { TH_IPCD_VALID = 0, TH_IPCD_INVALID = 1 };
  public:
    int _valid;
    int _msgsize;
  };
public:
  thIpcSMMsg() : _key(-1), _buff(NULL), _bsize(-1), _own_mem(0)  {}
  virtual ~thIpcSMMsg()  { close(); }

public:
  int open(const char* key, const int mode = thIpcDef::TH_IPC_USE
        , const int bsize = thIpcDef::TH_IPC_BUFFSIZE);
  int close();
  int read(char* buff, const int size
        , int dmode = thIpcDef::TH_IPC_WAIT);
  int write(const char* buff, const int size
        , int dmode = thIpcDef::TH_IPC_WAIT);
  /* setter and getter */
  int key() { return _key; }
  //int fd() { return _fd; }
  void* buff() { return _shbuff; }
  int bsize() { return _bsize; }
  int sync_enter() { return _sync.enter(); }
  int sync_leave() { return _sync.leave(); }

private:
  int _key;
  void* _buff;
  void* _shbuff;
  int _bsize;
  int _own_mem;      //  create shared memory directly?
  __SyncMethod _sync;
};



template <class __SyncMethod>
int thIpcSMMsg<__SyncMethod>::open(const char* key
          , const int mode, const int bsize) {
  if (key == NULL) {
    return thIpcErr::TH_IPC_INVARG;
  }
  thlib::thString tmpkey;
  tmpkey.printf("%d", thIpcDef::TH_IPC_PREFIX);
  tmpkey += key;
  int i_key = atoi(tmpkey());
  int o_key = atoi(key);

  if (_sync.init(o_key, 1, mode)) {
    return thIpcErr::TH_IPC_INIT;
  }
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  int ret = 0;
  void* ret_a = NULL;
  switch (mode) {
  case thIpcDef::TH_IPC_CREATE:
    ret = shmget(i_key, bsize + sizeof(IpcHdr), 0600 | IPC_CREAT | IPC_EXCL);
    if (ret == -1) {
      if (errno == EEXIST) {
        close();
        return thIpcErr::TH_IPC_ALREADY;
      }
      close();
      return thIpcErr::TH_IPC_CFAIL;
    }
    _key = ret;
    _own_mem = 1;
    break;
  case thIpcDef::TH_IPC_USE:
    ret = shmget(i_key, bsize + sizeof(IpcHdr), 0600);
    if (ret == -1) {
      if (errno == ENOENT) {
        close();
        return thIpcErr::TH_IPC_NOEXIST;
      }
      close();
      return thIpcErr::TH_IPC_FAIL;
    }
    _key = ret;
    _own_mem = 0;
    break;
  default:
    close();
    return thIpcErr::TH_IPC_INVARG;
  }

  ret_a = shmat(_key, 0, 0);
  if (ret_a == (char*)-1) {
    close();
    return thIpcErr::TH_IPC_ATFAIL;
  }
  _buff = ret_a;
  _shbuff = _buff + THIS_SHMNG_SIZE;
  _bsize = bsize;

  if (mode == thIpcDef::TH_IPC_CREATE) {
    IpcHdr tmphdr;
    tmphdr._valid = IpcHdr::TH_IPCD_INVALID;
    tmphdr._msgsize = 0;
    memcpy(_buff, &tmphdr, sizeof(IpcHdr));
  }

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  return thIpcErr::TH_IPC_SUCCESS;
}


template <class __SyncMethod>
int thIpcSMMsg<__SyncMethod>::close() {
  if (_buff != NULL) {
    if (shmdt(_buff) == -1) {
      return thIpcErr::TH_IPC_REMOVE;
    }
    _buff = NULL;
  }
  if (_key != -1) {
    if (_own_mem == 1) {
      if (shmctl(_key, IPC_RMID, NULL) == -1) {
        return thIpcErr::TH_IPC_REMOVE;
      }
      _own_mem = 0;
    }
    _key = -1;
  }
  _bsize = -1;
  return _sync.release();
}


template <class __SyncMethod>
int thIpcSMMsg<__SyncMethod>::read(char* buff
            , const int size, const int dmode) {
  IpcHdr tmphdr;
  while (1) {
    if (_sync.enter()) {
      return thIpcErr::TH_IPC_SYNC;
    }

    memcpy(&tmphdr, _buff, sizeof(IpcHdr));
    if (tmphdr._valid == IpcHdr::TH_IPCD_INVALID) {
      if (_sync.leave()) {
        return thIpcErr::TH_IPC_SYNC;
      }
      if (dmode == thIpcDef::TH_IPC_WAIT) {
        sleep(thIpcDef::TH_IPC_PULLTIME);
        continue;
      } else if (dmode == thIpcDef::TH_IPC_RETURN) {
        return thIpcErr::TH_IPC_DATANEXIST;
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

  memcpy(buff, (char*)_buff + sizeof(IpcHdr), r_size);
  tmphdr._valid = IpcHdr::TH_IPCD_INVALID;
  tmphdr._msgsize = 0;
  memcpy(_buff, &tmphdr, sizeof(IpcHdr));

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}


template <class __SyncMethod>
int thIpcSMMsg<__SyncMethod>::write(const char* buff
            , const int size, const int dmode) {
  IpcHdr tmphdr;
  while (1) {
    if (_sync.enter()) {
      return thIpcErr::TH_IPC_SYNC;
    }

    memcpy(&tmphdr, _buff, sizeof(IpcHdr));
    if (tmphdr._valid == IpcHdr::TH_IPCD_VALID) {
      if (_sync.leave()) {
        return thIpcErr::TH_IPC_SYNC;
      }
      if (dmode == thIpcDef::TH_IPC_WAIT) {
        sleep(thIpcDef::TH_IPC_PULLTIME);
        continue;
      } else if (dmode == thIpcDef::TH_IPC_RETURN) {
        return thIpcErr::TH_IPC_DATAEXIST;
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

  memcpy((char*)_buff + sizeof(IpcHdr), buff, size);
  tmphdr._valid = IpcHdr::TH_IPCD_VALID;
  tmphdr._msgsize = size;
  memcpy(_buff, &tmphdr, sizeof(IpcHdr));

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}




/**
 * inter-process communications with shared memory
 *  this shared memory is treated for direct access
 */
template <class __SyncMethod>
class thIpcSM {
public:
  enum { THIS_SHMNG_SIZE = 256 };
public:
  thIpcSM() : _key(-1), _buff(NULL), _shbuff(NULL), 
              _bsize(-1), _own_mem(thIpcDef::TH_IPC_NOOWN)	{}
  virtual ~thIpcSM()	{ destroy(); }

public:
  int release();
  int close() { return release(); }
  void destroy() { release(); }

public:
  int open(const char* key, const int mode, const int bsize);
  int share_in_thread(thIpcSM<__SyncMethod>& is);
  int clear();
  int read(char* buff, const int size, 
      int dmode = thIpcDef::TH_IPC_WAIT);
  int write(const char* buff, const int size, 
      int dmode = thIpcDef::TH_IPC_WAIT);
  int readm(char* buff, const int off, const size_t size);
  int writem(char* buff, const int off, const size_t size);
  template <typename __TTYPE>
  int mem_test(__TTYPE t, int cnt) {
    if (!_shbuff || (sizeof(__TTYPE) * cnt > shsize())) {
      return -1;
    }
    void* off = _shbuff;
    for (uint ci = 0; ci < cnt; ++ci, off += sizeof(__TTYPE)) {
      //__android_log_print(ANDROID_LOG_INFO, "hw_engine", "<thIpcSM::mem_test> "
      //    "buff[%d] : %d(size - %d)", ci, *((int*)off), sizeof(__TTYPE));
    }
    return 0;
  }

  /* setter and getter */
  int key() { return _key; }
  //int fd() { return _fd; }
  void* buff() { return _shbuff; }
  int bsize() { return _bsize; }
  int shsize() { return _bsize -THIS_SHMNG_SIZE; } 
  int sync_enter() { return _sync.enter(); }
  int sync_leave() { return _sync.leave(); }

 private:
  int _key;
  void* _buff;
  void* _shbuff;
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
  thlib::thString tmpkey;
  tmpkey.printf("%d", thIpcDef::TH_IPC_PREFIX);
  tmpkey += key;
//  int i_key = atoi(tmpkey());
  int o_key = atoi(key);
/*
  if (_sync.init(o_key, 1, mode)) {
    return thIpcErr::TH_IPC_INIT;
  }
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }
*/
  int ret = 0;
  void* ret_a = NULL;
  switch (mode) {
  case thIpcDef::TH_IPC_CREATE:
    ret = shmget(o_key, bsize + THIS_SHMNG_SIZE, 0600 | IPC_CREAT | IPC_EXCL);
    if (ret == -1) {
      if (errno == EEXIST) {
        printf("shmget error : EEXIST key(%d)\n", o_key);
        close();
        return thIpcErr::TH_IPC_ALREADY;
      } else if (errno == EINVAL) {
        perror("shmget error : EINVAL\n");
      } else if (errno == ENOSPC) {
        perror("shmget error : ENOSPC \n");
      } else if (errno == ENOMEM) {
        perror("shmget error : ENOMEM \n");
      }
      close();
      return thIpcErr::TH_IPC_CFAIL;
    }
    _key = ret;
    //_own_mem = 1;
    _own_mem = thIpcDef::TH_IPC_THOWN;
    break;
  case thIpcDef::TH_IPC_USE:
    ret = shmget(o_key, bsize + THIS_SHMNG_SIZE, 0600);
    if (ret == -1) {
      if (errno == ENOENT) {
        close();
        return thIpcErr::TH_IPC_NOEXIST;
      }
      close();
      return thIpcErr::TH_IPC_FAIL;
    }
    _key = ret;
    //_own_mem = 0;
    _own_mem = thIpcDef::TH_IPC_NOOWN;
    break;
  default:
    close();
    return thIpcErr::TH_IPC_INVARG;
  }

  ret_a = shmat(_key, 0, 0);
  if (ret_a == (void*)-1) {
    if (errno == EACCES) {
        perror("shmat error : EACCES \n");
    } else if (errno == EINVAL) {
        perror("shmat error : EINVAL \n");
    } else if (errno == ENOMEM) {
        perror("shmat error : ENOMEM \n");
    }
    close();
    return thIpcErr::TH_IPC_ATFAIL;
  }
  _buff = ret_a;
  _shbuff = (char*)_buff + THIS_SHMNG_SIZE;
  _bsize = bsize;

  if (_sync.init((size_t)_buff, 1, mode)) {
  //if (_sync.init(o_key, 1, mode)) {
    return thIpcErr::TH_IPC_INIT;
  }
/*
  if (mode == thIpcDef::TH_IPC_CREATE) {
    IpcHdr tmphdr;
    tmphdr._valid = IpcHdr::TH_IPCD_INVALID;
    tmphdr._msgsize = 0;
    memcpy(_buff, &tmphdr, sizeof(IpcHdr));
  }

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
*/
  return thIpcErr::TH_IPC_SUCCESS;
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::release() {
  if (_buff != NULL) {
    if (shmdt(_buff) == -1) {
      return thIpcErr::TH_IPC_REMOVE;
    }
    _buff = NULL;
  }
  if (_key != -1) {
    if (_own_mem == thIpcDef::TH_IPC_THOWN /* 1 */) {
      if (shmctl(_key, IPC_RMID, NULL) == -1) {
        return thIpcErr::TH_IPC_REMOVE;
      }
      _own_mem = thIpcDef::TH_IPC_NOOWN /* 0 */;
    }
    _key = -1;
  }
  _bsize = -1;
  return _sync.release();
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::share_in_thread(thIpcSM<__SyncMethod>& is) {
  int ret = release();
  if (ret) {
    return ret;
  }
  _key = is._key;
  _buff = is._buff;
  _shbuff = is._shbuff;
  _bsize = is._bsize;
  _own_mem = thIpcDef::TH_IPC_NOOWN;
  _sync.share_in_thread(is._sync);
  return 0;
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::clear() {
  int ret = 0;
  ret = sync_enter();
  if (ret) {
    return ret;
  }
  if (_shbuff) {
    memset(_shbuff, 0, _bsize);
  }
  ret = sync_leave();
  if (ret) {
    return ret;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::read(char* buff, 
    const int size, const int dmode) {
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }  
  
  if (size > _bsize) {
    return thIpcErr::TH_IPC_OVERFLOW;
  }

  memcpy(buff, (char*)_buff + THIS_SHMNG_SIZE, size);

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::write(const char* buff, 
    const int size, const int dmode) {
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  if (size > _bsize) {
    return thIpcErr::TH_IPC_OVERFLOW;
  }

  memcpy((char*)_buff + THIS_SHMNG_SIZE, buff, size);

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }
  return thIpcErr::TH_IPC_SUCCESS;

}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::readm(char* buff, const int off, const size_t size) {
  if (buff == NULL || off == -1 || size == 0 || ((off + size) > _bsize)) {
    return thIpcErr::TH_IPC_INVARG;
  }
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  memcpy(buff, (char*)_shbuff + off, size);

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  return thIpcErr::TH_IPC_SUCCESS;
}

template <class __SyncMethod>
int thIpcSM<__SyncMethod>::writem(char* buff, const int off, const size_t size) {
  if (buff == NULL || off == -1 || size == 0 || ((off + size) > _bsize)) {
    return thIpcErr::TH_IPC_INVARG;
  }
  if (_sync.enter()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  memcpy((char*)_shbuff + off, buff, size);

  if (_sync.leave()) {
    return thIpcErr::TH_IPC_SYNC;
  }

  return thIpcErr::TH_IPC_SUCCESS;
}



};  //  end of namespace thlib



#endif

