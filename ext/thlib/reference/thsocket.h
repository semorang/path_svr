/* Copyright 2011 <happyteam@thinkwaresys.com> */
#ifndef _THLIB_THSOCKET_H__
#define _THLIB_THSOCKET_H__

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "msvc_interface.h"
#include "thlib/thtechnic.h"
#include "thlib/static_check.h"
#include "thlib/therrno.h"
#include "thlib/thstring.h"
#include "thlib/thmemalloc.h"
#include "thlib/ththreads.h"
#ifdef ANDROID
  #include <linux/un.h>
  #include <android/log.h>
#endif


namespace thlib {


/**
 *  basic define
 */
typedef int THSOCKET;        ///  define type of socket file description
#define  NLISTEN        1024  ///  max acceptable queue size of waiting clients
    ///    if queue is full, the client may receive an error with ECONNREFUSED
    ///    or the request may be ignored so that retries succeed.
#define THSOCK_TIMEOUT    15    ///  timeout



/**
 *  class thSocketmsghdr
 *    - when server and client read stream of variable size, let they decide boundary.
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
template <>
class SockAddr<AF_UNIX> {
 public:
  typedef sockaddr_un TYPE;
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
  thSockettcpsvr() : _sock(-1), _host(""), _port(-1)  {}
  virtual ~thSockettcpsvr()  { release(); }
  void release() {
    if (_sock >= 0) {
      close(_sock);
    }
    _sock = -1;
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
    int ret = mkaddress(sa);
    if (ret != THSOCK_ERRSUC) {
      return ret;
    }
    _sock = socket(__SOCKFAM, SOCK_STREAM, 0);
    if (!isvalidsock()) {
      return THSOCK_ERRINVALSOCK;
    }
    if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) {
      close(_sock);
      return THSOCK_ERROPT;
    }
    if (setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on))) {
      close(_sock);
      return THSOCK_ERROPT;
    }
    if ((ret = bind(_sock, (struct sockaddr*)&sa, sizeof(sa))) != 0) {
      close(_sock);
      return THSOCK_ERRBIND;
    }
    if (listen(_sock, NLISTEN)) {
      close(_sock);
      return THSOCK_ERRLISTEN;
    }
    return THSOCK_ERRSUC;
  }
  THSOCKET accept(typename SockAddr<__SOCKFAM>::TYPE* addr) {
    int addrsize = sizeof(*addr);
    return ::accept(_sock, (struct sockaddr *)addr, (socklen_t*)&addrsize);
  }
  THSOCKET sock() { return _sock; }
  THSOCKET* sock_pt() { return &_sock; }
  void sock(THSOCKET sock) { _sock = sock; }

private :
  int isvalidsock() { return (_sock >= 0) ? 1 : 0; }
  int mkaddress(sockaddr_in& sa) {
    hostent *hp = NULL;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if (_host.size() != 0) {
      if (!inet_aton(_host(), &sa.sin_addr)) {
        hp = gethostbyname(_host());
        if (hp == NULL)
          return THSOCK_ERRHOST;
        sa.sin_addr = *(in_addr*)hp->h_addr;
      }
    } else {
      sa.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    sa.sin_port = htons(_port);
    return THSOCK_ERRSUC;
  }
  int mkaddress(sockaddr_un& sa) {
    if (_host.size() == 0 || _host.size() >= sizeof(sa.sun_path)) {
      return THSOCK_ERRHOST;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    sa.sun_path[0] = '\0';
    snprintf(sa.sun_path + 1, sizeof(sa.sun_path) - 1, "%s", _host.c_str());
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
      int retsel = select(sock+1, &rfd, NULL, NULL, &tv);
      if (retsel < 0)
        return retsel;
      else if (retsel == 0)
        return THSOCK_TIMEOVER;
    }
    return recv(sock, buf, buflen, 0);
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
        if (errno == EINTR)  ///  interrupted?
          continue;      ///  restart read
#ifdef THSOCK_TESTPRINT
        printf("err - [%s:%d]\n", __FILE__, __LINE__);
#endif
        return ret;
      } else if (ret == 0) {    ///  EOF?
        ///  connection is terminated
        return ret;
      }
      buf += ret;
      cnt -= ret;
    }
    return buflen;
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
    char *printmsg = MemPolicy<char>::alloc(msghdr._msgsize+1);
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
    msghdr._msgsize = stm.binsize();
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
  
  typedef union _cmsg_fd {
    struct cmsghdr cmsg;
    char data[CMSG_SPACE(sizeof(int))];
  } cmsg_fd;
  
  static int recvfd(THSOCKET sock, THSOCKET & fd) {
    struct msghdr msg;
    struct iovec iov;
    cmsg_fd cmsg;
    THSOCKET lfd = fd;

    iov.iov_base = (void *)&lfd;
    iov.iov_len = sizeof(lfd);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    msg.msg_flags = 0;

    int ret = recvmsg(sock, &msg, 0);
    if (ret < 0) {
      return -THE_UNEXPECTED;
    }

    const struct cmsghdr *cptr(CMSG_FIRSTHDR(&msg));
    if (!cptr) {
      return -THE_UNEXPECTED;
    }
    if (cptr->cmsg_level != SOL_SOCKET || cptr->cmsg_type != SCM_RIGHTS) {
      return -THE_UNEXPECTED;
    }
    memcpy(&fd, CMSG_DATA(cptr), sizeof(fd));

    return 0;
  }

  static int sendfd(THSOCKET sock, THSOCKET fd) {
    struct msghdr msg;
    struct iovec iov;
    cmsg_fd cmsg;
    THSOCKET lfd = fd;

    iov.iov_base = (void*)&lfd;
    iov.iov_len = sizeof(lfd);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof(cmsg);

    msg.msg_flags = 0;

    struct cmsghdr* cptr(CMSG_FIRSTHDR(&msg));
    if (!cptr) {
      return -THE_UNEXPECTED;
    }
    cptr->cmsg_len = CMSG_LEN(sizeof(THSOCKET));
    cptr->cmsg_level = SOL_SOCKET;
    cptr->cmsg_type = SCM_RIGHTS;

    memcpy(CMSG_DATA(cptr), &lfd, sizeof(THSOCKET));
    int ret = sendmsg(sock, &msg, 0);
    if (ret < 0) {
      return -THE_UNEXPECTED;
    }
    return 0;
  }
};



/**
 *  class thSockettcpsvrs
 *    - this read and write with client directly.
 *    this is created by the acceptsvr method of thSockettcpsvr.
 */
template <class __ThreadPolicy = thObjectLock>
class thSockettcpsvrs : public __ThreadPolicy::template In<
                thSockettcpsvrs<__ThreadPolicy> > {
  typedef typename __ThreadPolicy::template In<
            thSockettcpsvrs<__ThreadPolicy> >::Lock InLock;
 public:
  thSockettcpsvrs() : _sock(-1), _timer(-1) {}
  thSockettcpsvrs(THSOCKET sock) : _sock(sock), _timer(-1) {}//NOLINT
  ~thSockettcpsvrs()  { release(); }
 public:
  int init(THSOCKET sock) {
    InLock lock(*this);
    if (_sock == -1) {
      _sock = sock;
    } else {
      return THSOCK_ERREXIST;
    }
    return THSOCK_ERRSUC;
  }
  void release() {
    InLock lock(*this);
    if (_sock >= 0) {
      close();
    }
    _timer = -1;
  }

 private:
  int close() {
    /*
    if (shutdown(_sock, SHUT_WR) == -1)
      return THSOCK_ERRSHUTDOWN;
    char tmpbuf[1024];
    while(1) {
      int nb = read(tmpbuf, 1024);
      if (nb < 0)
        return nb;
      else if (nb == 0)
        break;
    }
    */
    if (::close(_sock) == -1) {
      return THSOCK_ERRCLOSE;
    }
    _sock = -1;
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
  int recvfd(THSOCKET& fd) {
    InLock lock(*this);
    return thSockStream::recvfd(_sock, fd);
  }
  int sendfd(THSOCKET fd) {
    InLock lock(*this);
    return thSockStream::sendfd(_sock, fd);
  }
  void  timer(int tm)  { _timer = tm; }
  int    timer()  { return _timer; }
  THSOCKET sockfd() { return _sock; }
  THSOCKET sock() { return _sock; }
  THSOCKET* sock_pt() { return &_sock; }
  void sock(THSOCKET sock) { _sock = sock; }


 private:
  THSOCKET _sock;
  int    _timer;
};  ///  end of class thSockettcpsvrs


/**
 *  class thSockettcpclt
 *    - tcp client
 */
template <class __ThreadPolicy = thObjectLock, int __SOCKFAM = AF_INET>
class thSockettcpclt : public __ThreadPolicy::template In<
                thSockettcpclt<__ThreadPolicy, __SOCKFAM> > {
  typedef typename __ThreadPolicy::template In<
            thSockettcpclt<__ThreadPolicy, __SOCKFAM> >::Lock InLock;
 public:
  thSockettcpclt() : _sock(-1), _timer(-1), _port(-1), _host("")  {}
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
    return (_sock >= 0) ? 1 : 0;
  }
  int connect(char* host, int port) {
    InLock lock(*this);
    if (host != NULL)
      _host(host);
    else
      _host("");
    _port = port;

    typename SockAddr<__SOCKFAM>::TYPE sa;
    int ret = mkaddress(sa);
    if (ret != THSOCK_ERRSUC) {
      return ret;
    }
    _sock = socket(__SOCKFAM, SOCK_STREAM, 0);

    if (!isvalidsock())
      return THSOCK_ERRINVALSOCK;
    if (::connect(_sock, (sockaddr*)&sa, sizeof(sa))) {
      close();
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

    typename SockAddr<__SOCKFAM>::TYPE sa;
    int ret = mkaddress(sa);
    if (ret != THSOCK_ERRSUC) {
      return ret;
    }
    _sock = socket(__SOCKFAM, SOCK_STREAM, 0);

    if (!isvalidsock())
      return THSOCK_ERRINVALSOCK;

    int flags = fcntl(_sock, F_GETFL, 0);
    fcntl(_sock, F_SETFL, flags | O_NONBLOCK);

    if ((ret = ::connect(_sock, (sockaddr*)&sa, sizeof(sa))) < 0) {
      if (errno != EINPROGRESS) {
        return THSOCK_ERRCONN;
      }
    }

    if (ret != 0) {
      fd_set rset, wset;
      struct timeval tval;

      FD_ZERO(&rset);
      FD_SET(_sock, &rset);
      wset = rset;
      tval.tv_sec = timeout;
      tval.tv_usec = 0;

      if ((ret = select(_sock+1, &rset, &wset, NULL,
                        &tval)) == 0) {
        // timeout
        close();
        return THSOCK_ERRCONN;
      }

      if (FD_ISSET(_sock, &rset) || FD_ISSET(_sock, &wset)) {
        socklen_t len = sizeof(errno);
        if (getsockopt(_sock, SOL_SOCKET, SO_ERROR, &errno, &len) < 0) {
          return THSOCK_ERROPT;
        }
      } else {
        return THSOCK_ERRINVALSOCK;
      }
    }

    fcntl(_sock, F_SETFL, flags);
    if (errno) {
      close();
      return THSOCK_ERRINVALSOCK;
    }
    return THSOCK_ERRSUC;
  }
  int close() {
    InLock lock(*this);
    if (_sock == -1) {
      return THSOCK_ERRSUC;
    }
    /*
    if (shutdown(_sock, SHUT_WR) == -1)
      return THSOCK_ERRSHUTDOWN;
    char tmpbuf[1024];
    while(1) {
      int nb = read(tmpbuf, 1024);
      if (nb < 0)
        return nb;
      else if (nb == 0)
        break;
    }
    */
    if (::close(_sock) == -1) {
      return THSOCK_ERRCLOSE;
    }
    _sock = -1;
    return THSOCK_ERRSUC;
  }

 private:
  int mkaddress(sockaddr_in& sa) {
    hostent *hp = NULL;

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    if (_host.size() != 0) {
      if (!inet_aton(_host(), &sa.sin_addr)) {
        hp = gethostbyname(_host());
        if (hp == NULL)
          return THSOCK_ERRHOST;
        sa.sin_addr = *(in_addr*)hp->h_addr;
      }
    } else {
      sa.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    sa.sin_port = htons(_port);
    return THSOCK_ERRSUC;
  }
  int mkaddress(sockaddr_un& sa) {
    if (_host.size() == 0 || _host.size() >= sizeof(sa.sun_path)) {
      return THSOCK_ERRHOST;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    sa.sun_path[0] = '\0';
    snprintf(sa.sun_path + 1, sizeof(sa.sun_path) - 1, "%s", _host.c_str());
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
  int recvfd(THSOCKET& fd) {
    InLock lock(*this);
    return thSockStream::recvfd(_sock, fd);
  }
  int sendfd(THSOCKET fd) {
    InLock lock(*this);
    return thSockStream::sendfd(_sock, fd);
  }
  void  timer(int tm)  { _timer = tm; }
  int    timer()  { return _timer; }
  THSOCKET sock() { return _sock; }
  THSOCKET* sock_pt() { return &_sock; }
  void sock(THSOCKET sock) { _sock = sock; }

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



template <class __ThreadPolicy = thObjectLock>
class thSocketudpsvr : public __ThreadPolicy::template In<
                thSocketudpsvr<__ThreadPolicy> > {
  typedef typename __ThreadPolicy::template In<
            thSocketudpsvr<__ThreadPolicy> >::Lock InLock;
 public:
  thSocketudpsvr()  {}
  virtual ~thSocketudpsvr()  { release(); }
 public:
  void release()  {}
};  ///  end of class thSocketudpsvr
};  ///  end of namespace thlib


#endif

