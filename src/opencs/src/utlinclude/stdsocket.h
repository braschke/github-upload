#pragma once
#ifndef stdsocket_HEADER_INCLUDED
#define stdsocket_HEADER_INCLUDED
/* stdsocket.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines the raw-socket & stream-socket stuff
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: stdsocket.h 4755 2016-06-14 09:04:32Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifndef    SSOCK_USE_LISTEN
   #define SSOCK_USE_LISTEN      0  /* compile stuff for a server? */
#endif

#ifndef    SSOCK_USE_SELECT
   #define SSOCK_USE_SELECT      0 /* compile stuff for a server? */
#endif

#ifndef    SSOCK_USE_RSELECT
   #define SSOCK_USE_RSELECT     0 /* compile stuff for a server? */
#endif

#ifndef    SSOCK_USE_CLIENT
   #define SSOCK_USE_CLIENT      0 /* compile stuff for a client? */
#endif

#ifndef    SSOCK_USE_SWAP
   #define SSOCK_USE_SWAP        0 /* compile without endianess swapping */
#endif

#ifndef    SSOCK_USE_RCLEAR
   #define SSOCK_USE_RCLEAR      0
#endif

#ifndef    SSOCK_USE_PUTSYNC
   #define SSOCK_USE_PUTSYNC     0
#endif

#ifndef    SSOCK_USE_GETSYNC
   #define SSOCK_USE_GETSYNC     0
#endif

#ifndef    SSOCK_USE_PUTSIZ
   #define SSOCK_USE_PUTSIZ      0
#endif
#ifndef    SSOCK_USE_GETSIZ
   #define SSOCK_USE_GETSIZ      0
#endif

#ifndef    SSOCK_USE_PUTCHR
   #define SSOCK_USE_PUTCHR      0
#endif
#ifndef    SSOCK_USE_GETCHR
   #define SSOCK_USE_GETCHR      0
#endif

#ifndef    SSOCK_USE_PUTARR
   #define SSOCK_USE_PUTARR      0
#endif
#ifndef    SSOCK_USE_GETARR
   #define SSOCK_USE_GETARR      0
#endif

#ifndef    SSOCK_USE_PUTSTR
   #define SSOCK_USE_PUTSTR      0
#endif
#ifndef    SSOCK_USE_GETSTR
   #define SSOCK_USE_GETSTR      0
#endif

#ifndef    SSOCK_USE_GETINT
   #define SSOCK_USE_GETINT      0
#endif

#ifndef    SSOCK_USE_GETLONG
   #define SSOCK_USE_GETLONG     0
#endif

#ifndef    SSOCK_USE_GETFLOAT
   #define SSOCK_USE_GETFLOAT    0
#endif

#ifndef    SSOCK_USE_GETREAL
   #define SSOCK_USE_GETREAL     0
#endif

#ifndef    SSOCK_USE_GETDOUBLE
   #define SSOCK_USE_GETDOUBLE   0
#endif

#ifndef    SSOCK_USE_GETOOB
   #define SSOCK_USE_GETOOB      0 /* compile without RSOCK_getoob included */
#endif

#ifndef    SSOCK_USE_PUTOOB
   #define SSOCK_USE_PUTOOB      0 /* compile without RSOCK_putoob included */
#endif

#ifndef    SSOCK_USE_DEBUG
   #define SSOCK_USE_DEBUG       0 /* compile without RSOCK_debug included */
#endif

#if !SSOCK_USE_LISTEN && !SSOCK_USE_CLIENT
/*   #error Neither SSOCK_USE_LISTEN nor SSOCK_USE_CLIENT are true */
#endif



/*
 * Set the RSOCK usage flags
 */
#if SSOCK_USE_LISTEN
   #undef   RSOCK_USE_LISTEN
   #define  RSOCK_USE_LISTEN     1
#endif

#if SSOCK_USE_SELECT
   #undef   RSOCK_USE_SELECT
   #define  RSOCK_USE_SELECT     1
#endif

#if SSOCK_USE_RSELECT
   #undef   RSOCK_USE_RSELECT
   #define  RSOCK_USE_RSELECT    1
#endif

#if SSOCK_USE_CLIENT
   #undef   RSOCK_USE_CLIENT
   #define  RSOCK_USE_CLIENT     1
#endif

#if SSOCK_USE_GETOOB
   #undef   RSOCK_USE_GETOOB
   #define  RSOCK_USE_GETOOB     1
#endif

#if SSOCK_USE_PUTOOB
   #undef   RSOCK_USE_PUTOOB
   #define  RSOCK_USE_PUTOOB     1
#endif

#if SSOCK_USE_DEBUG
   #undef   RSOCK_USE_DEBUG
   #define  RSOCK_USE_DEBUG      1
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if !IS_MSWIN

   /*
    * Make UNIX compatible to MSWin WSA
    */

   typedef int SOCKET;  /* MSWin defines SOCKET as a HANDLE */
   typedef int HANDLE;

   #define closesocket(_sd)   close(_sd)

   /* shutdown(s,...) arguments */
   #ifndef SD_RECEIVE
      #define SD_RECEIVE   SHUT_RD
   #endif
   #ifndef SD_SEND
      #define SD_SEND      SHUT_WR
   #endif
   #ifndef SD_BOTH
      #define SD_BOTH      SHUT_RDWR
   #endif

#endif

#if IS_MSWIN

   #ifndef IS_MINGW
      #pragma comment(lib,"ws2_32")
   #endif
   #if INCLUDE_STATIC
      #include "WSAStrerror.c"
   #endif
   #include <wininet.h>
   #include <ws2tcpip.h>

   /*
    * According to the MSDN a socket handle can be any value 0...INVALID_SOCKET-1
    */
   #ifndef INVALID_SOCKET
      #define INVALID_SOCKET  (SOCKET)(~0) /* missing with old MSVC versions */
   #endif

   #define IS_VALID_SOCKET(_s)   ( (_s) != INVALID_SOCKET )
   #define IS_INVALID_SOCKET(_s) ( (_s) == INVALID_SOCKET )

   /*
    * winsock2.h is not BSD compatible and uses a separate errno/socket types
    */
   #define SOCKET_STRERROR()              WSAStrerror()
   #define SOCKET_SET_ERRNO(_sockerrno)   const int _sockerrno = WSAGetLastError()
   #define SOCKET_DO_AGAIN(_sockerrno)    (\
                                             _sockerrno == WSAEINTR        || \
                                             _sockerrno == WSAEINPROGRESS  || \
                                             _sockerrno == WSAECONNREFUSED || \
                                             _sockerrno == WSAEWOULDBLOCK     \
                                          )

#else

   #include <netdb.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <netinet/tcp.h>
   #include <arpa/inet.h>
   #include <sys/time.h>

   #ifndef INVALID_SOCKET
      #define INVALID_SOCKET              -1
   #endif
   #define IS_VALID_SOCKET(_s)            (  (_s) >= 0 )
   #define IS_INVALID_SOCKET(_s)          (  (_s) <  0 )

   #define SOCKET_STRERROR()              strerror(errno)
   #define SOCKET_SET_ERRNO(_sockerrno)   const int _sockerrno = errno
   #define SOCKET_DO_AGAIN(_sockerrno)    (\
                                             _sockerrno == EINTR       || \
                                             _sockerrno == EAGAIN      || \
                                             _sockerrno == EWOULDBLOCK || \
                                             _sockerrno == ECONNREFUSED   \
                                          )

#endif

#define ATOMAR_SOCKET_LOOP \
{\
   SOCKET_SET_ERRNO(_tmp_socket_errno);\
   if (!SOCKET_DO_AGAIN(_tmp_socket_errno)) break;\
   MILLISLEEP(1);\
}

/*
 * General purpose union which holds IPv4 as well as IPv6 socket structures.
 */
typedef union _USOCKADDR
{
   unsigned short       af;      /* Address family used to distinguish AF_INET & AF_INET6 */
   struct sockaddr_in6  saddr_ipv6;
   struct sockaddr_in   saddr_ipv4;
   struct sockaddr      saddr;   /* Widely used as a function arg ... to avoid compiler complaints */
} USOCKADDR;


/*
 * Define the socket structure
 */
#define SOCK_T_DEFINED  1  /* Can be used to check if SOCK_t is defined */

typedef struct _SOCK_t  SOCK_t;

struct _SOCK_t
{
   int (*handler)(SOCK_t *sock, const char *cdir, const char *msg); /* pointer to an optional error handler */
   SOCK_t  *listener;   /* !=NULL: this is the listen() master socket */
   char    *hostname;   /* malloc'ed resolved hostname */
   char    *pipename;   /* malloc'ed name of the pipe used: !NULL, then we use the pipes for communication */
   char    *ipibname;   /* address string of an optional infiniband IP over IB address */
   char    *ibfabric;   /* list of hostname within the server infiniband fabric */

   char    *buf;        /* pointer to the OPTIONAL(!) malloc'ed stream buffer */
   char    *ptr;        /* current pointer position in buffer */
   size_t   cnt;        /* chars still in buffer */

   SOCKET   sd;         /* socket descriptor, UNIX int or MSWin handle */
#if IS_MSWIN
   HANDLE   se;         /* event handle assigned to the socket fo WSAWaitForMultipleEvents() */
#endif
   HANDLE   pd_r;       /* read pipe handle */
   HANDLE   pd_w;       /* write pipe handle */
   HANDLE   od_r;       /* read oob pipe handle */
   HANDLE   od_w;       /* write oob pipe handle */

   long     delay;      /* delay during connection retries */
   long     timeout;    /* timeout until connection give up */
   int      af;         /* AF_UNSPEC, AF_INET6, AF_INET */
   int      port;       /* port used to reconnect */
   unsigned select;     /* bitpatterns set after a select call */
   int      oobyte;     /* OOB byte received: UNIX only */
   u_char   iomode;     /* char: last r/w modus 'L', '?', 'w', 'r' */
   u_char   doswap;     /* bool: swap endianess of received data if true */
   u_char   dorepair;   /* bool: reconnect in case of failures */
};

/*
 * Special timeout values for RSOCK_connect()
 */
#define SSOCK_TIMEOUT_NEVER   -1
#define SSOCK_TIMEOUT_TRIAL   -2

/*
 * Bitpatterns: which variables need to match between local and remote host
 */
#define SSOCK_MATCH_SHORT     0x0001
#define SSOCK_MATCH_INT       0x0002
#define SSOCK_MATCH_LONG      0x0004
#define SSOCK_MATCH_LONGLONG  0x0008
#define SSOCK_MATCH_FLOAT     0x0010
#define SSOCK_MATCH_DOUBLE    0x0020
#define SSOCK_MATCH_QUAD      0x0040

#define SSOCK_HELLO_MAX       256 /* Max. no of chars for the hello string */

typedef struct _SSOCK_HI_t
{
   unsigned match;                  /* Number check bitpattern */
   int      endianess;              /* M|I, then the machine endianess is overwritten */
   char     magic[SSOCK_HELLO_MAX]; /* Magic string */
   char     hello[SSOCK_HELLO_MAX]; /* Hello string = identifier */
} SSOCK_HI_t;


#define _SSOCK_BLKSIZE     1024 /* Not for public use */
#define _SSOCK_BUFSIZE     (4*_SSOCK_BLKSIZE)

/*
 * SSOCK_select() bitpatterns
 */
#define RSOCK_SBIT_READ          0x01
#define RSOCK_SBIT_WRITE         0x02
#define RSOCK_SBIT_EXCEPT        0x04

#define RSOCK_FLAG_ATOMIC        0x00010000
#define RSOCK_FLAG_NORECON       0x00020000
#define RSOCK_MASK_TIMEOUT       0x0000ffff

/*
 * RSOCK general macros
 */

#define RSOCK_HAS_OOB(_sock)        ( (_sock)->oobyte >= 0 )
#define RSOCKAF_IPVX(_af)           ( ((_af)==AF_INET)?"IPv4":((_af)==AF_INET6)?"IPv6":"IPvX" )
#define RSOCK_CLEAROOB(_sock)         (_sock)->oobyte = -1
#define RSOCK_IS_PIPE(_sock)        ( (_sock)->pipename != NULL )
#define RSOCK_IS_SOCK(_sock)        ( (_sock)->pipename == NULL )
#define RSOCK_IS_LISTEN(_sock)      ( (_sock)->listener == (_sock) )
#define RSOCK_IS_CLIENT(_sock)      ( (_sock)->listener == NULL )
#define RSOCK_IS_SERVER(_sock)      ( (_sock)->listener != NULL && (_sock)->listener != (_sock) )
#define RSOCK_IS_VALID(_sock)       ( (_sock) != NULL && (RSOCK_IS_PIPE(_sock) ?   IS_VALID_HANDLE((_sock)->pd_r) :   IS_VALID_SOCKET((_sock)->sd)) )
#define RSOCK_IS_INVALID(_sock)     ( (_sock) == NULL || (RSOCK_IS_PIPE(_sock) ? IS_INVALID_HANDLE((_sock)->pd_r) : IS_INVALID_SOCKET((_sock)->sd)) )
#define RSOCK_CTYP(_sock)           ( RSOCK_IS_PIPE(_sock) ? "pipe" : RSOCKAF_IPVX((_sock)->af) )


/*
 * During the initial connection dialogs (PIPES vs. SOCKET) and sending the hello string
 * the connection is a non blocking receive with a timeout (in milliseconds).
 * The server: should get an immediate response from any client.
 * The client: timeout is much larger, since the client may be connected to the server
 * and has sent his hello string, but the server may be busy with other connections
 * for a while.
 *
 * Practical experience:
 *    Client timeout: 60 seconds
 *    Server timeout   6 seconds.
 */
#define RSOCK_HELLO_TIMEOUT(_sock)  (RSOCK_IS_CLIENT(_sock) ? 60000 : 6000)


/*
 * SSOCK general macros
 */
#define SSOCK_CLEARBUF(_sock,_iomode) { (_sock)->iomode=_iomode;(_sock)->cnt=0;(_sock)->ptr=(_sock)->buf; }



#define SSOCK_HANDLER(_sock)          (_sock)->handler
#define SSOCK_HOST(_sock)             (_sock)->hostname
#define SSOCK_PIPE(_sock)             (_sock)->pipename
#define SSOCK_IPIB(_sock)             (_sock)->ipibname
#define SSOCK_PORT(_sock)             (_sock)->port
#define SSOCK_IOMODE(_sock)           (_sock)->iomode
#define SSOCK_AF(_sock)               (_sock)->af
#define SSOCK_CLEARSELECT(_sock)      (_sock)->select = 0
#define SSOCK_HOWMANY(_sock)        ( (_sock)->cnt         )
#define SSOCK_DOSWAP(_sock)           (_sock)->doswap
#define SSOCK_DOREPAIR(_sock)         (_sock)->dorepair

#define SSOCK_IS_READMODE(_sock)    ( (_sock)->iomode == 'r' )
#define SSOCK_IS_WRITEMODE(_sock)   ( (_sock)->iomode == 'w' )
#define SSOCK_IS_READABLE(_sock)    ( ( SSOCK_IS_READMODE(_sock) && SSOCK_HOWMANY(_sock) > 0)  || ((_sock)->select & RSOCK_SBIT_READ ) != 0 )
#define SSOCK_IS_WRITEABLE(_sock)   ( (_sock)->iomode != 'w' || (_sock)->cnt < _SSOCK_BUFSIZE || ((_sock)->select & RSOCK_SBIT_WRITE) != 0 )

/*
 * send to SOCKET_t
 */
#define SSOCK_putint(_sock,_val)          SSOCK_putraw(_sock,&_val,sizeof(int))
#define SSOCK_putlong(_sock,_val)         SSOCK_putraw(_sock,&_val,sizeof(long))
#if HAVE_LONGLONG
#define SSOCK_putlonglong(_sock,_val)     SSOCK_putraw(_sock,&_val,sizeof(long long))
#endif

#define SSOCK_putfloat(_sock,_val)        SSOCK_putraw(_sock,&_val,sizeof(float))
#define SSOCK_putreal(_sock,_val)         SSOCK_putraw(_sock,&_val,sizeof(real))
#define SSOCK_putdouble(_sock,_val)       SSOCK_putraw(_sock,&_val,sizeof(double))
#define SSOCK_putquad(_sock,_val)         SSOCK_putraw(_sock,&_val,sizeof(quad))
#define SSOCK_putcmd(_sock,_cmd)          SSOCK_putraw(_sock,_cmd,4)
#define SSOCK_putnul(_sock)               SSOCK_putraw(_sock,NULL,sizeof(int))



/*
 * receive from SOCKET_t
 */
#define SSOCK_getint(_sock,_pval)         SSOCK_getany(_sock,_pval,1,sizeof(int))
#define SSOCK_getlong(_sock,_pval)        SSOCK_getany(_sock,_pval,1,sizeof(long))
#if HAVE_LONGLONG
#define SSOCK_getlonglong(_sock,_pval)    SSOCK_getany(_sock,_pval,1,sizeof(long long))
#endif

#define SSOCK_getfloat(_sock,_pval)       SSOCK_getany(_sock,_pval,1,sizeof(float))
#define SSOCK_getreal(_sock,_pval)        SSOCK_getany(_sock,_pval,1,sizeof(real))
#define SSOCK_getdouble(_sock,_pval)      SSOCK_getany(_sock,_pval,1,sizeof(double))
#define SSOCK_getquad(_sock,_pval)        SSOCK_getany(_sock,_pval,1,sizeof(quad))

#define SSOCK_getsstr(_sock,_str)         SSOCK_getstr(_sock,_str,sizeof(_str))

#define SSOCK_puttype1(_sock,_type,_arg0)\
{\
   _type _buf[1];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype2(_sock,_type,_arg0,_arg1)\
{\
   _type _buf[2];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype3(_sock,_type,_arg0,_arg1,_arg2)\
{\
   _type _buf[3];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype4(_sock,_type,_arg0,_arg1,_arg2,_arg3)\
{\
   _type _buf[4];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype5(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4)\
{\
   _type _buf[5];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   _buf[4] = SCAST_INTO(_type,_arg4);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype6(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5)\
{\
   _type _buf[6];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   _buf[4] = SCAST_INTO(_type,_arg4);\
   _buf[5] = SCAST_INTO(_type,_arg5);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype7(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6)\
{\
   _type _buf[7];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   _buf[4] = SCAST_INTO(_type,_arg4);\
   _buf[5] = SCAST_INTO(_type,_arg5);\
   _buf[6] = SCAST_INTO(_type,_arg6);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype8(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6,_arg7)\
{\
   _type _buf[8];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   _buf[4] = SCAST_INTO(_type,_arg4);\
   _buf[5] = SCAST_INTO(_type,_arg5);\
   _buf[6] = SCAST_INTO(_type,_arg6);\
   _buf[7] = SCAST_INTO(_type,_arg7);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}

#define SSOCK_puttype9(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6,_arg7,_arg8)\
{\
   _type _buf[9];\
   _buf[0] = SCAST_INTO(_type,_arg0);\
   _buf[1] = SCAST_INTO(_type,_arg1);\
   _buf[2] = SCAST_INTO(_type,_arg2);\
   _buf[3] = SCAST_INTO(_type,_arg3);\
   _buf[4] = SCAST_INTO(_type,_arg4);\
   _buf[5] = SCAST_INTO(_type,_arg5);\
   _buf[6] = SCAST_INTO(_type,_arg6);\
   _buf[7] = SCAST_INTO(_type,_arg7);\
   _buf[8] = SCAST_INTO(_type,_arg8);\
   SSOCK_putraw(_sock,_buf,sizeof(_buf));\
}


#define SSOCK_putint2(_sock,_a1,_a2)                                 SSOCK_puttype2(_sock,int   ,_a1,_a2)
#define SSOCK_putint3(_sock,_a1,_a2,_a3)                             SSOCK_puttype3(_sock,int   ,_a1,_a2,_a3)
#define SSOCK_putint4(_sock,_a1,_a2,_a3,_a4)                         SSOCK_puttype4(_sock,int   ,_a1,_a2,_a3,_a4)
#define SSOCK_putint5(_sock,_a1,_a2,_a3,_a4,_a5)                     SSOCK_puttype5(_sock,int   ,_a1,_a2,_a3,_a4,_a5)
#define SSOCK_putint6(_sock,_a1,_a2,_a3,_a4,_a5,_a6)                 SSOCK_puttype6(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6)
#define SSOCK_putint7(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7)             SSOCK_puttype7(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7)
#define SSOCK_putint8(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)         SSOCK_puttype8(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)
#define SSOCK_putint9(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)     SSOCK_puttype9(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)

#define SSOCK_putdouble2(_sock,_a1,_a2)                              SSOCK_puttype2(_sock,double,_a1,_a2)
#define SSOCK_putdouble3(_sock,_a1,_a2,_a3)                          SSOCK_puttype3(_sock,double,_a1,_a2,_a3)
#define SSOCK_putdouble4(_sock,_a1,_a2,_a3,_a4)                      SSOCK_puttype4(_sock,double,_a1,_a2,_a3,_a4)
#define SSOCK_putdouble5(_sock,_a1,_a2,_a3,_a4,_a5)                  SSOCK_puttype5(_sock,double,_a1,_a2,_a3,_a4,_a5)
#define SSOCK_putdouble6(_sock,_a1,_a2,_a3,_a4,_a5,_a6)              SSOCK_puttype6(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6)
#define SSOCK_putdouble7(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7)          SSOCK_puttype7(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7)
#define SSOCK_putdouble8(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)      SSOCK_puttype8(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)
#define SSOCK_putdouble9(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)  SSOCK_puttype9(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)


#define SSOCK_gettype1(_sock,_type,_arg0)\
{\
   _type _buf[1];\
   SSOCK_getany(_sock,_buf,1,sizeof(_type));\
   _arg0 = _buf[0];\
}

#define SSOCK_gettype2(_sock,_type,_arg0,_arg1)\
{\
   _type _buf[2];\
   SSOCK_getany(_sock,_buf,2,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
}

#define SSOCK_gettype3(_sock,_type,_arg0,_arg1,_arg2)\
{\
   _type _buf[3];\
   SSOCK_getany(_sock,_buf,3,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
}

#define SSOCK_gettype4(_sock,_type,_arg0,_arg1,_arg2,_arg3)\
{\
   _type _buf[4];\
   SSOCK_getany(_sock,_buf,4,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
}

#define SSOCK_gettype5(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4)\
{\
   _type _buf[5];\
   SSOCK_getany(_sock,_buf,5,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
   _arg4 = _buf[4];\
}

#define SSOCK_gettype6(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5)\
{\
   _type _buf[6];\
   SSOCK_getany(_sock,_buf,6,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
   _arg4 = _buf[4];\
   _arg5 = _buf[5];\
}

#define SSOCK_gettype7(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6)\
{\
   _type _buf[7];\
   SSOCK_getany(_sock,_buf,7,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
   _arg4 = _buf[4];\
   _arg5 = _buf[5];\
   _arg6 = _buf[6];\
}

#define SSOCK_gettype8(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6,_arg7)\
{\
   _type _buf[8];\
   SSOCK_getany(_sock,_buf,8,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
   _arg4 = _buf[4];\
   _arg5 = _buf[5];\
   _arg6 = _buf[6];\
   _arg7 = _buf[7];\
}

#define SSOCK_gettype9(_sock,_type,_arg0,_arg1,_arg2,_arg3,_arg4,_arg5,_arg6,_arg7,_arg8)\
{\
   _type _buf[9];\
   SSOCK_getany(_sock,_buf,9,sizeof(_type));\
   _arg0 = _buf[0];\
   _arg1 = _buf[1];\
   _arg2 = _buf[2];\
   _arg3 = _buf[3];\
   _arg4 = _buf[4];\
   _arg5 = _buf[5];\
   _arg6 = _buf[6];\
   _arg7 = _buf[7];\
   _arg8 = _buf[8];\
}


#define SSOCK_getint2(_sock,_a1,_a2)                                 SSOCK_gettype2(_sock,int   ,_a1,_a2)
#define SSOCK_getint3(_sock,_a1,_a2,_a3)                             SSOCK_gettype3(_sock,int   ,_a1,_a2,_a3)
#define SSOCK_getint4(_sock,_a1,_a2,_a3,_a4)                         SSOCK_gettype4(_sock,int   ,_a1,_a2,_a3,_a4)
#define SSOCK_getint5(_sock,_a1,_a2,_a3,_a4,_a5)                     SSOCK_gettype5(_sock,int   ,_a1,_a2,_a3,_a4,_a5)
#define SSOCK_getint6(_sock,_a1,_a2,_a3,_a4,_a5,_a6)                 SSOCK_gettype6(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6)
#define SSOCK_getint7(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7)             SSOCK_gettype7(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7)
#define SSOCK_getint8(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)         SSOCK_gettype8(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)
#define SSOCK_getint9(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)     SSOCK_gettype9(_sock,int   ,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)

#define SSOCK_getdouble2(_sock,_a1,_a2)                              SSOCK_gettype2(_sock,double,_a1,_a2)
#define SSOCK_getdouble3(_sock,_a1,_a2,_a3)                          SSOCK_gettype3(_sock,double,_a1,_a2,_a3)
#define SSOCK_getdouble4(_sock,_a1,_a2,_a3,_a4)                      SSOCK_gettype4(_sock,double,_a1,_a2,_a3,_a4)
#define SSOCK_getdouble5(_sock,_a1,_a2,_a3,_a4,_a5)                  SSOCK_gettype5(_sock,double,_a1,_a2,_a3,_a4,_a5)
#define SSOCK_getdouble6(_sock,_a1,_a2,_a3,_a4,_a5,_a6)              SSOCK_gettype6(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6)
#define SSOCK_getdouble7(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7)          SSOCK_gettype7(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7)
#define SSOCK_getdouble8(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)      SSOCK_gettype8(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8)
#define SSOCK_getdouble9(_sock,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)  SSOCK_gettype9(_sock,double,_a1,_a2,_a3,_a4,_a5,_a6,_a7,_a8,_a9)


#if !INCLUDE_STATIC

/*
 * rawsocket.c functions
 */
extern int         RSOCK_init    (void);
extern int         RSOCK_islisten(const int port);
extern SOCK_t     *RSOCK_listen  (const int af, const int port, long delay, long timeout);
extern SOCK_t     *RSOCK_connect (const char *servername, int port, long delay, long timeout);
extern SOCK_t     *RSOCK_accept  (SOCK_t *listener);
extern char       *RSOCK_prefix  (const char *prefix);
extern int         RSOCK_select  (SOCK_t *sockv[], const int nsock, unsigned request, const long timeout);
extern int         RSOCK_rselect (SOCK_t *sockv[], const int nsock,                   const long timeout);
extern void        RSOCK_close   (SOCK_t **psock);
extern int         RSOCK_recv    (SOCK_t *sock,       void *buf, const size_t maxlen, const unsigned flags);
extern int         RSOCK_send    (SOCK_t *sock, const void *buf, const size_t numlen, const unsigned flags);
extern int         RSOCK_timeout (SOCK_t *sock, const long timeout);
extern int         RSOCK_nodelay (SOCK_t *sock, unsigned on);
extern int         RSOCK_getoob  (SOCK_t *sock);

/*
 * streamsocket.c functions
 */
extern SOCK_t    *SSOCK_connect  (const char *servername, int port, long delay, long timeout, SSOCK_HI_t *hi);
extern SOCK_t    *SSOCK_accept   (SOCK_t *listener, SSOCK_HI_t *hi);
extern int        SSOCK_select   (SOCK_t *sockv[], int nsock, unsigned request, long timeout);
extern int        SSOCK_rselect  (SOCK_t *sockv[], int nsock,                   long timeout);
extern void       SSOCK_close    (SOCK_t **psock);
extern void       SSOCK_flush    (SOCK_t *sock);
extern void       SSOCK_putraw   (SOCK_t *sock, const void *buf, size_t n);
extern void       SSOCK_getraw   (SOCK_t *sock,       void *buf, size_t n);
extern void       SSOCK_rclear   (SOCK_t *sock, const long timeout);
extern void       SSOCK_putsync  (SOCK_t *sock, const char *csync, const size_t slen);
extern void       SSOCK_getsync  (SOCK_t *sock, const char *csync, const size_t slen);
extern void       SSOCK_getany   (SOCK_t *sock,       void *buf, size_t n, size_t size);
extern void       SSOCK_putsiz   (SOCK_t *sock, size_t size);
extern size_t     SSOCK_getsiz   (SOCK_t *sock);
extern void       SSOCK_putchr   (SOCK_t *sock, int chr);
extern int        SSOCK_getchr   (SOCK_t *sock);
extern void       SSOCK_putarr   (SOCK_t *sock, const void *buf, size_t numitem, size_t size);
extern size_t     SSOCK_getarr   (SOCK_t *sock,       void *buf, size_t maxitem, size_t size);
extern void       SSOCK_putstr   (SOCK_t *sock, const char *str);
extern char      *SSOCK_getstr   (SOCK_t *sock,       char *str, size_t maxSize);
extern int        SSOCK_rpcall   (SOCK_t *sock, int funcid, const char *fmt, ...);
extern void     **SSOCK_rpexec   (SOCK_t *sock);

/*
 * multi-platform helper functions
 */
extern TCHAR      *bytes2hex           (const void *barray, const unsigned blen, const int upper, const int hyphen, TCHAR *hexstr);
extern TCHAR     **getmacaddrv         (const int hyphen, const int upcase);
extern char      **getifnamev          (void);
extern const char *splitporthost       (const char *porthost, int *port, int warn);
extern const char *getlocalhostname    (char *lhostname);
extern int         islocalhost         (const char *hostname);
extern int         getinfinibandaddr   (const int af, USOCKADDR *usaddr);
extern char       *getinfinibandhosts  (const int af, USOCKADDR *usaddr, char *hostlist, size_t *phsize);

extern int         usockaddr_isequal   (const USOCKADDR *usaddr1, const USOCKADDR *usaddr2);
extern int         usockaddr_islocal   (const USOCKADDR *usaddr);
extern int         usockaddr_isloopback(const USOCKADDR *usaddr);
extern int         usockaddr_unmap     (      USOCKADDR *usaddr);
extern const char *usockaddr_ntop      (const USOCKADDR *usaddr, char *dst, const size_t size);
extern int         usockaddr_pton      (const char *devname, USOCKADDR *usaddr);
extern int         getnetdeviceaddr    (const char *devname, USOCKADDR *usaddr);

#if IS_MSWIN
   #include <iphlpapi.h>
   extern const IP_ADAPTER_ADDRESSES *getipadapters(const unsigned dofree);
#else
   #include <ifaddrs.h>
   extern const struct ifaddrs *getipadapters(const unsigned dofree);
#endif

#endif /* INCLUDE_STATIC */

#ifdef __cplusplus
}
#endif

#endif
