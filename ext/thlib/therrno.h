/* Copyright 2011 <happyteam@thinkwaresys.com> */
/**
 * thlib error number define
 * used for thlib..
 * Don't be duplicate other application.
 */
#ifndef _therrno_
#define _therrno_


#ifndef ETHINVAL
#define ETHINVAL        (1)
#endif

#define EPARAM            (0x08)    ///< wrong parameter
#define EBUF_SIZE         (0x0D)    ///< buffer size overflow

#define EMEM_ALLOC        (0x11)    ///< memory alloc fail
#define EFILE_OPEN        (0x12)    ///< file open error
#define EFILE_READ        (0x13)    ///< file read err
#define EFILE_WRITE       (0x14)    ///< file write err

#define THEOVERFLOW       (0x15)    ///< value overflows in max boundary
#define THE_UNEXPECTED    (0x16)    ///< unexpected error
///< exceed limitation which is defined in each
#define THE_EXCEEDLIMIT   (0x17)
#define THE_VERSION       (0x18)    ///< version doesn't match
#define THE_TIMEDOUT      (0x19)   ///< cond_timedwait error
#define THE_SUCCESS       (0x00)    ///< success



namespace thlib {


/**
 *  @brief  thsocket error number define
 */
#define THSOCK_ERRSUC       (0)     /// success
#define THSOCK_ERRADDRESS   (-502)    /// designating address is failed
#define THSOCK_ERRINVALSOCK (-503)    /// socket descriptor is invalid
#define THSOCK_ERROPT       (-504)    /// socket option setting is failed
#define THSOCK_ERRBIND      (-505)    /// socket binding is failed
#define THSOCK_ERRLISTEN    (-506)    /// socket listenning is failed
#define THSOCK_ERRHOST      (-507)    /// get host fail
#define THSOCK_ERRETHDR     (-508)    /// can't read entire header
#define THSOCK_ERRMEM       (-509)    /// memory allocation fail
#define THSOCK_ERRCONN      (-510)   /// connection failed
#define THSOCK_TIMEOVER     (-511)   /// exceed time designated by timer
#define THSOCK_ERRSHUTDOWN  (-512)   /// shutdown fail
#define THSOCK_ERRCLOSE     (-513)   /// socket close fail`
#define THSOCK_ERREXIST    (-514)  /// socket is already allocated
#define THSOCK_ERRRW        (-515)
};





#endif

