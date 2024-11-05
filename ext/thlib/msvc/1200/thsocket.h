/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THLIB_THSOCKET_H__
#define _THLIB_THSOCKET_H__

#include <winsock.h>
#pragma warning(disable : 4786)

#if defined(WIN32) && !defined(_WIN32_WCE)
  #include <errno.h>
#else
  #error  support windows only
#endif

#if (_MSC_VER >= 1300)
  #include "thlib/reference/msvc_interface.h"
#endif
#if (_MSC_VER == 1200)
  #include "thlib/msvc/1200/msvc6helpers.h"
#else
#endif
#include "thlib/thtechnic.h"
#include "thlib/static_check.h"
#include "thlib/therrno.h"
#include "thlib/thusewindows.h"
#include "thlib/thstring.h"
#include "thlib/thmemalloc.h"
#include "thlib/ththreads.h"


namespace thlib {


/**
 *  basic define
 */
typedef SOCKET THSOCKET;      // define type of socket file description
#define  NLISTEN        1024  // max acceptable queue size of waiting clients
                              // if queue is full, the client may receive an error with ECONNREFUSED
                              // or the request may be ignored so that retries succeed.
#define THSOCK_TIMEOUT     5  // timeout



/**
 *  class thSocketmsghdr
 *    - when server and client read stream of variable size, 
 *      let they decide boundary.
 */
class thSocketmsghdr {
 public:
  int  _msgsize;
};


/**
 *  class thSocketsvr
 *    - socket server parent class. decide protocol by template argument
 *  @param  PT : accept thSockettcpsvr and thSocketudpsvr
 *        all interfaces of thSockettcpsvr and thSocketudpsvr are unified.
 */
template <class __ConnectionPolicy>
class thSocketsvr : public __ConnectionPolicy {
 public:
  thSocketsvr() {}
  virtual ~thSocketsvr() {}
};  ///  end of class thsocket


template <int __SOCKFAM>
class SockAddr {
 public:
  typedef Loki::NullType TYPE;
};
template <>
class SockAddr<AF_INET> {
 public:
  typedef sockaddr_in TYPE;
};

/**
 *  class thSockettcpsvr
 *    - tcp server. manage connection for client.
 */
template <int __SOCKFAM = AF_INET>
class thSockettcpsvr
{
  THSOCKET _sock;
  thString _host;
  int _port;

public :
  thSockettcpsvr() : _sock(INVALID_SOCKET), _host(""), _port(-1)  {}
  virtual ~thSockettcpsvr()  { release(); }
  void release() {
    if (_sock != INVALID_SOCKET) {
      closesocket(_sock);
    }
    _sock = INVALID_SOCKET;
    _host("");
    _port = -1;
  }
  int serverready(char* host, int port) {
    if (host != NULL) {
      _host(host);
    } else {
      _host("");
    }
    _port = port;

    typename SockAddr<__SOCKFAM>::TYPE sa;
    const int on = 1;
    WSADATA wsa_data;
    int ret = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
      return THSOCK_ERRCONN;
    }
    if (mkaddress(sa) != THSOCK_ERRSUC) {
      return ret;
    }
    if ((_sock = socket(__SOCKFAM, SOCK_STREAM, 0)) == INVALID_SOCKET) {
      return THSOCK_ERRINVALSOCK;
    }
    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on))) {
      closesocket(_sock);
      return THSOCK_ERROPT;
    }
    if (setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&on, sizeof(on))) {
      closesocket(_sock);
      return THSOCK_ERROPT;
    }
    if ((ret = bind(_sock, (struct sockaddr*)&sa, sizeof(sa))) != 0) {
      closesocket(_sock);
      return THSOCK_ERRBIND;
    }
    if (listen(_sock, NLISTEN)) {
      closesocket(_sock);
      return THSOCK_ERRLISTEN;
    }
    return THSOCK_ERRSUC;
  }
  THSOCKET accept(sockaddr_in* addr) {
    int addrsize = sizeof(*addr);
    return ::accept(_sock, (sockaddr*)addr, &addrsize);
  }
  THSOCKET sock() { return _sock; }
  THSOCKET* sock_pt() { return &_sock; }
  void sock(THSOCKET sock) { _sock = sock; }

private :
  int isvalidsock() { return (_sock == INVALID_SOCKET) ? 0 : 1; }
  int mkaddress(sockaddr_in& sa) {
    hostent* hp = NULL;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(_port);
    if (_host.size() != 0) {
      if (inet_addr(_host()) != INADDR_NONE) {
        hp = gethostbyname(_host());
        if (hp == NULL) {
          return THSOCK_ERRHOST;
        }
        sa.sin_addr = *(in_addr*)hp->h_addr;
      } else {
        return THSOCK_ERRHOST;
      }
    } else {
      sa.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    return THSOCK_ERRSUC;
  }
};  ///  end of class thSockettcpsvr



/**
 *  class thSockStream
 *    - manage read write
 */
class thSockStream {
 public:

  /**
   *  read : read stream unconditionally
   */
  static int read(THSOCKET sock, char* buf, const int buflen
      , const int timer = -1) {
    if (timer >= 0) {
      fd_set rfd;
      struct timeval tv;
      FD_ZERO(&rfd);
      FD_SET(sock, &rfd);
      tv.tv_sec = timer;
      tv.tv_usec = 0;
      int retsel = select(int(sock+1), &rfd, NULL, NULL, &tv);
      if (retsel < 0)
        return retsel;
      else if (retsel == 0)
        return THSOCK_TIMEOVER;
    }
    return recv(sock, buf, static_cast<int>(buflen), 0);
  }

  /**
   *  readn : read stream to buflen or to meeting EOF.
   */
  static int readn(THSOCKET sock, char* buf, const int buflen
      , const int timer = -1) {
    int cnt = 0;
    int ret = 0;
    cnt = buflen;
    while (cnt > 0) {
      ret = read(sock, buf, cnt, timer);
      if (ret < 0) {
#if defined(_WIN32_WCE)
        if (::GetLastError() == WSAEINTR) {  //<  interrupted?
          continue;      //<  restart read
        }
#else  //<  WIN32
        if (errno == EINTR) {  //<  interrupted?
          continue;      //<  restart read
        }
#endif
        return ret;
      } else if (ret == 0) {    ///  EOF?
        ///  connection is terminated
        return ret;
      }
      buf += ret;
      cnt -= static_cast<size_t>(ret);
    }
    return static_cast<int>(buflen);
  }

  /**
   *  readv : read stream to message length defined in header
   */
  static int readv(THSOCKET sock, thNewArr<char>& buf
      , const int timer = -1) {
    thSocketmsghdr msghdr;
    int ret = 0;

    ret = readn(sock, (char*)&msghdr, sizeof(thSocketmsghdr), timer);
    if (ret != sizeof(thSocketmsghdr)) {
      if (ret < 0) {
        ///  EINTR is processed in read function
        return ret;
      } else if (ret == 0) {
        ///  connection is terminated
        return ret;
      } else {
        return THSOCK_ERRETHDR;
      }
    }

    if (buf.alloc(msghdr._msgsize) == NULL) {
      ///  buf alloc fail... discard message of header, and return.
      clear_unread(sock, msghdr._msgsize, timer);
      return THSOCK_ERRMEM;
    }

    ///  read message to buffer
    ret = readn(sock, &buf[0], msghdr._msgsize, timer);
    if (ret != msghdr._msgsize) {
      if (ret < 0) {
        ///  EINTR is processed in read function
        return ret;
      } else {
        return THSOCK_ERRETHDR;
      }
    }

#ifdef  THSOCK_TESTPRINT
    char* printmsg = MemPolicy<char>::alloc(msghdr._msgsize+1);
    memcpy(printmsg, &buf[0], msghdr._msgsize);
    printmsg[msghdr._msgsize] = '\0';
    printf("thSockettcpclt::readv - msgsize(%d), size(%d), msg(%s)\n",
        msghdr._msgsize, ret, printmsg);
    MemPolicy<char>::memfree(printmsg);
#endif
    return ret;
  }

  static int write(THSOCKET sock, char* buf, const int buflen) {
    return send(sock, buf, buflen, 0);
  }

  static int writev(THSOCKET sock, thNewArr<char>& stm) {
    thSocketmsghdr msghdr;
    msghdr._msgsize = (int)stm.binsize();
    int ret = write(sock, (char*)&msghdr, sizeof(msghdr));
    if (ret < 0) {
      return ret;
    }
    ret = write(sock, stm(), stm.binsize());
    if (ret < 0) {
      return ret;
    }
    return ret;
  }

  static int clear_unread(THSOCKET sock, size_t size, const int timer) {
    int ret = 0;
    char tmpbuf[1024];
    size_t tmplen = 1024;
    size_t t_size = size;
    while (t_size > 0) {
      ret = readn(sock, tmpbuf, tmplen, timer);
      if (ret != static_cast<int>(tmplen)) {
        if (ret < 0) {
          ///  EINTR is processed in read function
          return ret;
        } else if (ret == 0) {
          ///  connection is terminated
          return ret;
        } else {
          return THSOCK_ERRETHDR;
        }
      }
      t_size -= tmplen;
      if (t_size < tmplen) {
        tmplen = t_size;
      }
    }

    return ret;
  }
};



/**
 *  class thSockettcpsvrs
 *    - this read and write with client directly.
 *    this is created by the acceptsvr method of thSockettcpsvr.
 *
 *  fucking m$ vc6 compiler!!!
 *    due to vc6 compiler's bug, I didn't use nested template.
 *    but below code occur internal compiler error too.
 *    so, use thObjectLock policy only in windows temporarily.
 */
template <class __ThreadPolicy = thObjectLock>
class thSockettcpsvrs : 
  public thObjectLock::In<thSockettcpsvrs<__ThreadPolicy> > {
  typedef typename thObjectLock::In<
        thSockettcpsvrs<__ThreadPolicy> >::Lock InLock;
 public:
  thSockettcpsvrs() : _sock(INVALID_SOCKET), _timer(-1) {}
  thSockettcpsvrs(THSOCKET sock) : _sock(sock), _timer(-1) {}//NOLINT
  ~thSockettcpsvrs()  { release(); }
 public:
  int init(THSOCKET sock) {
    InLock lock(*this);
    if (_sock == INVALID_SOCKET) {
      _sock = sock;
    } else {
      return THSOCK_ERREXIST;
    }
    return THSOCK_ERRSUC;
  }
  void release() {
    InLock lock(*this);
    if (_sock != INVALID_SOCKET) {
      close();
    }
    _timer = -1;
  }

 private:
  int close() {
    if (shutdown(_sock, 0x01) == SOCKET_ERROR)
      return THSOCK_ERRSHUTDOWN;
    char tmpbuf[1024];
    while (1) {
      int nb = read(tmpbuf, 1024);
      if (nb == SOCKET_ERROR)
        return nb;
      else if (nb == 0)
        break;
    }
    if (closesocket(_sock) == SOCKET_ERROR) {
      return THSOCK_ERRCLOSE;
    }
    _sock = INVALID_SOCKET;
    return THSOCK_ERRSUC;
  }

 public:
  /**
   *  read : read stream unconditionally
   */
  int read(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::read(_sock, buf, buflen);
  }
  /**
   *  readn : read stream to buflen or to meeting EOF.
   */
  int readn(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::readn(_sock, buf, buflen, _timer);
  }
  /**
   *   readn : read stream to structure size
   */
  template <class T>
  int readn(T& buf) {
    InLock lock(*this);
    return thSockStream::readn(_sock, (char*)&buf, sizeof(T), _timer);
  }
  /**
   *  readv : read stream to message length defined in header
   */
  int readv(thNewArr<char>& buf) {
    InLock lock(*this);
    return thSockStream::readv(_sock, buf, _timer);
  }
  template <class T>
  int write(T& buf) {
    InLock lock(*this);
    return thSockStream::write(_sock, (char*)&buf, sizeof(T));
  }
  int write(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::write(_sock, buf, buflen);
  }
  int writev(thNewArr<char>& stm) {
    InLock lock(*this);
    return thSockStream::writev(_sock, stm);
  }
  void  timer(int tm)  { _timer = tm; }
  int    timer()  { return _timer; }
  int   sock() { return _sock; }
 private:
  THSOCKET  _sock;
  int    _timer;
};  ///  end of class thSockettcpsvrs



/**
 *  class thSockettcpclt
 *    - tcp client
 *
 *  fucking m$ vc6 compiler!!!
 *    due to vc6 compiler's bug, I didn't use nested template.
 *    but below code occur internal compiler error too.
 *    so, use thObjectLock policy only in windows temporarily.
 */
template <class __ThreadPolicy = thObjectLock>
class thSockettcpclt : 
  public thObjectLock::In<thSockettcpclt<__ThreadPolicy> > {
  typedef typename thObjectLock::In<
        thSockettcpclt<__ThreadPolicy> >::Lock InLock;
 public:
  thSockettcpclt() : _sock(INVALID_SOCKET), _timer(-1), _port(-1), _host("")  {}
  ~thSockettcpclt()  { release(); }
 public:
  void release() {
    close();
    _timer = -1;
    _port = -1;
    _host("");
  }
  int isvalidsock() {
    InLock lock(*this);
    return (_sock == INVALID_SOCKET)?0:1;
  }
  int connect(char* host, int port) {
    InLock lock(*this);
    if (host != NULL)
      _host(host);
    else
      _host("");
    _port = port;

    WSADATA info;
    if (WSAStartup(MAKEWORD(2, 2), &info) != 0) {
      printf("cannot initialize winsock!");
      return THSOCK_ERRCONN;
    }

    u_long laddr;
    sockaddr_in sai;
    int ret = mkaddress(sai, laddr);
    if (ret != THSOCK_ERRSUC) {
      WSACleanup();
      return ret;
    }
    _sock = socket(AF_INET, SOCK_STREAM, 0);

    if (!isvalidsock()) {
      WSACleanup();
      return THSOCK_ERRINVALSOCK;
    }
    sai.sin_family = AF_INET;
    sai.sin_addr.s_addr = laddr;
    sai.sin_port = htons(_port);

    if (::connect(_sock, (sockaddr*)&sai, sizeof(sai)) == SOCKET_ERROR) {
      //int test = errno;
      close();
      WSACleanup();
      return THSOCK_ERRCONN;
    }
    return THSOCK_ERRSUC;
  }
  int connect(char* host, int port, int timeout) {
    InLock lock(*this);
    if (host != NULL)
      _host(host);
    else
      _host("");
    _port = port;

    WSADATA info;
    if (WSAStartup(MAKEWORD(2, 2), &info) != 0) {
      printf("cannot initialize winsock!");
      return THSOCK_ERRCONN;
    }

    u_long laddr;
    sockaddr_in sai;
    int ret = mkaddress(sai, laddr);
    if (ret != THSOCK_ERRSUC) {
      WSACleanup();
      return ret;
    }
    _sock = socket(AF_INET, SOCK_STREAM, 0);

    if (!isvalidsock()) {
      WSACleanup();
      return THSOCK_ERRINVALSOCK;
    }
    sai.sin_family = AF_INET;
    sai.sin_addr.s_addr = laddr;
    sai.sin_port = htons(_port);

    ULONG nonblk = true;
    if (ioctlsocket(_sock, FIONBIO, &nonblk) == SOCKET_ERROR) {
      return THSOCK_ERROPT;
    }

    ret = ::connect(_sock, (sockaddr*)&sai, sizeof(sai));
    if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
      close();
      WSACleanup();
      return THSOCK_ERRCONN;
    }

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(_sock, &rset);

    struct timeval tval;
    tval.tv_sec = timeout;
    tval.tv_usec = 0;

    if (select(0, NULL, &rset, NULL, &tval) == SOCKET_ERROR) {
      close();
      WSACleanup();
      return THSOCK_ERRRW;
    }

    if (!FD_ISSET(_sock, &rset)) {
      close();
      WSACleanup();
      return THSOCK_ERRRW;
    }

    nonblk = false;
    if (ioctlsocket(_sock, FIONBIO, &nonblk) == SOCKET_ERROR) {
      return THSOCK_ERROPT;
    }

    return THSOCK_ERRSUC;
  }
  int close() {
    InLock lock(*this);
    if (_sock == INVALID_SOCKET) {
      return THSOCK_ERRSUC;
    }
    /*
    if (shutdown(_sock, 0x01) == SOCKET_ERROR)
      return THSOCK_ERRSHUTDOWN;
    char tmpbuf[1024];
    while(1) {
      int nb = read(tmpbuf, 1024);
      if (nb == SOCKET_ERROR)
        return nb;
      else if (nb == 0)
        break;
    }
    */
    if (closesocket(_sock) == SOCKET_ERROR) {
      return THSOCK_ERRCLOSE;
    }
    _sock = INVALID_SOCKET;
    return THSOCK_ERRSUC;
  }

 private:
  int mkaddress(sockaddr_in& sai, u_long& laddr) {   
    hostent *hp = NULL;
    memset(&sai, 0, sizeof(sai));
    if (_host.size() != 0) {
      laddr = inet_addr(_host());
      if (laddr == INADDR_NONE) {
        hp = gethostbyname(_host());
        if (hp == NULL)
          return THSOCK_ERRHOST;
        laddr = *((u_long*)hp->h_addr_list[0]);
      }
    } else {
      sai.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    return THSOCK_ERRSUC;
  }

 public:
  /**
   *  read : read stream unconditionally
   */
  int read(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::read(_sock, buf, buflen);
  }
  /**
   *  readn : read stream to buflen or to meeting EOF.
   */
  int readn(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::readn(_sock, buf, buflen, _timer);
  }
  /**
   *   readn : read stream to structure size
   */
  template <class T>
  int readn(T& buf) {
    InLock lock(*this);
    return thSockStream::readn(_sock, (char*)&buf, sizeof(T), _timer);
  }
  /**
   *  readv : read stream to message length defined in header
   */
  int readv(thNewArr<char>& buf) {
    InLock lock(*this);
    return thSockStream::readv(_sock, buf, _timer);
  }
  template <class T>
  int write(T& buf) {
    InLock lock(*this);
    return thSockStream::write(_sock, (char*)&buf, sizeof(T));
  }
  int write(char* buf, const int buflen) {
    InLock lock(*this);
    return thSockStream::write(_sock, buf, buflen);
  }
  int writev(thNewArr<char>& stm) {
    InLock lock(*this);
    return thSockStream::writev(_sock, stm);
  }
  void  timer(int tm)  { _timer = tm; }
  int    timer()  { return _timer; }
  THSOCKET sock() { return _sock; }
  THSOCKET* sock_pt() { return &_sock; }

 private:
  THSOCKET  _sock;
  int      _timer;
  int      _port;
  thString  _host;
};



/**
 *  class thSocketudpsvr
 *    - udp server 
 */
template <class __ThreadPolicy = thSingleThread>
class thSocketudpsvr {
 public:
  thSocketudpsvr()  {}
  virtual ~thSocketudpsvr()  { release(); }
 public:
  void release()  {}
};  ///  end of class thSocketudpsvr
};  ///  end of namespace thlib


#endif

