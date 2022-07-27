#pragma once
#ifndef rawsocket_SOURCE_INCLUDED
#define rawsocket_SOURCE_INCLUDED
/* rawsocket.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Raw socket/pipe/infiniband stuff platform independent wrappers.
 *    On this RAW level: no buffering, no endianess check and not translation.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: rawsocket.c 5432 2017-07-31 12:37:02Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include <limits.h>
#include <time.h>

#include "xmsg.h"
#include "xmem.h"

#ifndef    RSOCK_USE_LISTEN
   #define RSOCK_USE_LISTEN   0 /* Compile stuff for a server? */
#endif

#ifndef    RSOCK_USE_ISLISTEN
   #define RSOCK_USE_ISLISTEN 0 /* Compile stuff for a server? */
#endif

#ifndef    RSOCK_USE_CLIENT
   #define RSOCK_USE_CLIENT   0 /* Compile stuff for a client? */
#endif

#if !RSOCK_USE_LISTEN && !RSOCK_USE_CLIENT
/*   #error Neither RSOCK_USE_LISTEN nor RSOCK_USE_CLIENT are true. Am I a server or client? */
#endif

#ifndef    RSOCK_USE_SELECT
   #define RSOCK_USE_SELECT   0 /* Compile stuff for a server? */
#endif

#ifndef    RSOCK_USE_RSELECT
   #define RSOCK_USE_RSELECT  0 /* Compile stuff for a server? */
#endif

#ifndef    RSOCK_USE_CLONE
   #define RSOCK_USE_CLONE    0 /* Use the RSOCK_clone function? */
#endif

#ifndef    RSOCK_USE_GETOOB
   #define RSOCK_USE_GETOOB   0 /* Use the RSOCK_getoob function? */
#endif

#ifndef    RSOCK_USE_DEBUG
   #define RSOCK_USE_DEBUG    0 /* Use the RSOCK_debug function? */
#endif

#ifndef    RSOCK_DEBUG
   #define RSOCK_DEBUG        0
#endif

#if INCLUDE_STATIC

   #include "millisleep.c"
   #include "getmstime.c"
   #include "getlocalhostname.c"
   #include "getinfinibandaddr.c"
   #include "usockaddr_unmap.c"
   #include "usockaddr_ntop.c"
   #if RSOCK_USE_CLIENT
      #include "splitporthost.c"
   #endif

#endif


/****************************************************************************************/
static void _rsock_timeout_msg(const SOCK_t *sock, const long timeout)
/****************************************************************************************/
{
   XMSG_FATAL3
   (
      "%s:%d: No response within %ld milliseconds.\n"
      ,SSOCK_HOST(sock),SSOCK_PORT(sock)
      ,timeout
   );
}

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_timeout(SOCK_t *sock, const long timeout)
/****************************************************************************************/
/*
 * Set the recv() timeout value on a socket: 'long timeout' is milliseconds
 */
{
   int ret = 0;

   if (RSOCK_IS_PIPE(sock))
   {
      /*
       * Change the read timeouts only, not the write timeouts
       */
      #if IS_MSWIN

         COMMTIMEOUTS tout;

         if (GetCommTimeouts(sock->pd_r,&tout))
         {
            if (timeout > 0)
            {
               tout.ReadIntervalTimeout         = 0;
               tout.ReadTotalTimeoutMultiplier  = 0;
               tout.ReadTotalTimeoutConstant    = timeout;
            #if 0
               tout.WriteTotalTimeoutMultiplier = 0;
               tout.WriteTotalTimeoutConstant   = 0;
            #endif
            }
            else /* No timeout at all: full blocking */
            {
               tout.ReadIntervalTimeout         =
               tout.ReadTotalTimeoutMultiplier  =
               tout.ReadTotalTimeoutConstant    = 0;
            #if 0
               tout.WriteTotalTimeoutMultiplier = 0;
               tout.WriteTotalTimeoutConstant   = 0;
            #endif
            }
            SetCommTimeouts(sock->pd_r,&tout);
         }

      #else

         /*fcntl(sock->pd_r,....;*/

      #endif
   }
   else
   {
   #if IS_MSWIN
      DWORD tout = (DWORD)((timeout<0) ? 0 : timeout); /* MSWin wants milliseconds */
   #else
      struct timeval tout;

      if (timeout > 0)
      {
         tout.tv_sec  = CAST_INT(timeout/1000);
         tout.tv_usec = 1000*(timeout%1000);
      }
      else /* No timeout at all: full blocking */
      {
         tout.tv_sec  = 0;
         tout.tv_usec = 0;
      }
   #endif

      ret = setsockopt(sock->sd,SOL_SOCKET,SO_RCVTIMEO,(char *)(&tout),sizeof(tout));
      if (ret < 0)
      {
         XMSG_WARNING2
         (
            "Can\'t setsockopt(SO_RCVTIMEO=%ld): %s.\n",
            timeout,SOCKET_STRERROR()
         );
      }
   }

   return ret;
}

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_nodelay(SOCK_t *sock, unsigned on)
/****************************************************************************************/
/*
 * Switch on/off the 'Nagle Algorithm'
 */
{
   int ret = 0;

   if (RSOCK_IS_PIPE(sock))
   {
   }
   else
   {
      on &= 1; /* Must be 0||1 */
      ret = setsockopt(sock->sd,IPPROTO_TCP,TCP_NODELAY,(char *)(&on),sizeof(on));
      if (ret < 0)
      {
         XMSG_WARNING2
         (
            "Can\'t setsockopt(TCP_NODELAY=%u): %s.\n",
            on,SOCKET_STRERROR()
         );
      }
   }
   return ret;
}

/****************************************************************************************/



/****************************************************************************************/
static void _rsock_close_pipes(SOCK_t *sock)
/****************************************************************************************/
/*
 * Close all possible opened pipes.
 */
{
#if IS_MSWIN
   if (IS_VALID_HANDLE(sock->pd_r)) CloseHandle(sock->pd_r);
   if (IS_VALID_HANDLE(sock->od_r)) CloseHandle(sock->od_r);
#else
   if (IS_VALID_HANDLE(sock->pd_w)) close(sock->pd_w);
   if (IS_VALID_HANDLE(sock->pd_r)) close(sock->pd_r);
   if (IS_VALID_HANDLE(sock->od_w)) close(sock->od_w);
   if (IS_VALID_HANDLE(sock->od_r)) close(sock->od_r);
#endif

   sock->pd_w = sock->pd_r =
   sock->od_w = sock->od_r = INVALID_HANDLE_VALUE;
}

/****************************************************************************************/

/*
 * Suffixes for the pipe names under MSWin (dual pipes) and UNIX (single pipes only :-( )
 */
#define PIPE_SUFFIX_IOB          "-iob"   /* Standard read & write IO stream */
#define PIPE_SUFFIX_IOB_READ     "-iob-r"
#define PIPE_SUFFIX_IOB_WRITE    "-iob-w"

#define PIPE_SUFFIX_OOB          "-oob"   /* OOB urgent message read & write pipes */
#define PIPE_SUFFIX_OOB_READ     "-oob-r"
#define PIPE_SUFFIX_OOB_WRITE    "-oob-w"

#if !IS_MSWIN

/*
 * Unix pipes are a bit different comapred to MSWin
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/****************************************************************************************/
static int _unix_open_pipe(char *pname, char *ptail, const char *sfx, const int oflags)
/****************************************************************************************/
/*
 * Opens a fifo file as a pipe.
 * The file descriptor must not be inherited to any child process.
 */
{
   int fd;


   strcpy(ptail,sfx);

#ifdef O_CLOEXEC /* Since Linux 2.6.23 */
   if ((fd=open(pname,oflags|O_CLOEXEC)) < 0)
#else
   if ((fd=open(pname,oflags)) < 0)
#endif
   {
      XMSG_WARNING2
      (
         "Failed to open pipe \"%s\": %s.\n"
         ,pname
         ,strerror(errno)
      );
   }
#ifndef O_CLOEXEC
   else
   {
      int flags = fcntl(fd,F_GETFD);
      if (flags != -1)
      {
         flags |= FD_CLOEXEC;
         fcntl(fd,F_SETFD,flags);
      }
   }
#endif

   return fd;
}

/****************************************************************************************/

#endif /* !IS_MSWIN */

/****************************************************************************************/
static SOCKET _rsock_do_create_af(const int af)
/****************************************************************************************/
/*
 * Create a new IPv6 or IPv4 socket.
 * The socket descriptor must not be inherited to any child process.
 */
{
   SOCKET sd = socket(af,SOCK_STREAM,IPPROTO_TCP);
   if (IS_INVALID_SOCKET(sd))
   {
   #if IS_UNICODE
      XMSG_MESSAGE
      (
         (af == AF_INET6) ? XMSG_LEVEL_WARNING : XMSG_LEVEL_FATAL
         ,"Can\'t create %s socket:\n%S.\n"
         ,RSOCKAF_IPVX(af)
         ,SOCKET_STRERROR()
      );
   #else
      XMSG_MESSAGE
      (
         (af == AF_INET) ? XMSG_LEVEL_FATAL : XMSG_LEVEL_WARNING
         ,"Can\'t create %s socket:\n%s.\n"
         ,RSOCKAF_IPVX(af)
         ,SOCKET_STRERROR()
      );
   #endif
   }
#if !IS_MSWIN
   else
   {
      int flags = fcntl(sd,F_GETFD);
      if (flags != -1)
      {
         flags |= FD_CLOEXEC;
         fcntl(sd,F_SETFD,flags);
      }
   }
#endif

   return sd;
}

/****************************************************************************************/
static int _rsock_bind_sd(SOCKET sd, USOCKADDR *usaddr)
/****************************************************************************************/
{
   switch(usaddr->saddr.sa_family)
   {
      case AF_INET:
         return bind(sd,&(usaddr->saddr),sizeof(struct sockaddr_in));

      case AF_INET6:
         return bind(sd,&(usaddr->saddr),sizeof(struct sockaddr_in6));

      default:
         break;
   }
   return -1;
}

/****************************************************************************************/
static int _rsock_connect_sd(SOCKET sd, USOCKADDR *usaddr)
/****************************************************************************************/
{
   switch(usaddr->saddr.sa_family)
   {
      case AF_INET:
         return connect(sd,&(usaddr->saddr),sizeof(struct sockaddr_in));

      case AF_INET6:
         return connect(sd,&(usaddr->saddr),sizeof(struct sockaddr_in6));

      default:
         break;
   }
   return -1;
}

/****************************************************************************************/
static SOCKET _rsock_do_accept(SOCK_t *sock, USOCKADDR *paddress)
/****************************************************************************************/
{
   USOCKADDR usaddr;
   SOCKET    newsd;
#if IS_MSWIN || defined(IS_OSFALPHA) || defined(IS_HPUX11)
   int       addrSize = sizeof(USOCKADDR);
#else
   socklen_t addrSize = sizeof(USOCKADDR);
#endif


   XMSG_INFO3
   (
      "Accepting incoming %s connections on \"%s:%d\"...\n"
      ,RSOCKAF_IPVX(sock->af)
      ,SSOCK_HOST(sock),SSOCK_PORT(sock)
   );

#if IS_MSWIN

   while((newsd = accept(sock->sd,&(usaddr.saddr),&addrSize)) == INVALID_SOCKET)
      ATOMAR_SOCKET_LOOP

   /*
    * Need to clear the FD_ACCEPT event, otherwise WSAWaitForMultipleEvents()
    * returns with an event seen on this listener socket :-(, although it was
    * already properly handled.
    */
   WSAResetEvent(sock->se);

#else

   while((newsd = accept(sock->sd,&(usaddr.saddr),&addrSize)) < 0)
      ATOMAR_SOCKET_LOOP

#endif

   if (IS_INVALID_SOCKET(newsd))
   {
      XMSG_FATAL3
      (
         "Can\'t accept() on listener socket \"%s:%d\":\n%s.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,SOCKET_STRERROR()
      );
   }

   if (paddress)
   {
      usockaddr_unmap(&usaddr);
      *paddress = usaddr; /* Struct copy */
   }
   return newsd;
}

/****************************************************************************************/

#define MSG_INTERVAL   4000L  /* Message interval is 4 seconds */

/****************************************************************************************/
static SOCKET _rsock_do_connect
(
   const char  *hostname,
   const int    port,
         int   *af,
         long   delay,
         long   timeout,
         char **rhostname
)
/****************************************************************************************/
/*
 * Try to connect to a host 'hostname' on port 'port'.
 */
{
   const char      *ai_canonname = hostname;
   struct addrinfo *aihead = NULL;
   struct addrinfo *ai = NULL;
   struct addrinfo  hints;
   USOCKADDR        ibaddr; /* socket address of the infiniband device */
   USOCKADDR        usaddr;
   int              gotib,ierr,addrc;
   char             service[16];


   XMSG_ACTION3
   (
      "Connecting to %s \"%s:%d\" ...\n"
      ,RSOCKAF_IPVX(*af)
      ,hostname,port
   );

   /*
    * Timeout:
    *    < 0: special value
    *    >=0: at least 10 seconds
    */
   if (timeout < 0)
   {
      if (timeout != SSOCK_TIMEOUT_NEVER && timeout != SSOCK_TIMEOUT_TRIAL)
      {
         XMSG_FATAL3
         (
            "Failed to connect with \"%s:%d\": Invalid special timeout value(%ld).\n"
            ,hostname,port
            ,timeout
         );
      }
   }
   else if (timeout < 10000)
   {
      timeout = 10000; /* Min. 10 seconds */
   }

   if (delay < 500)
   {
       delay = 500; /* Min 1/2 second */
   }

   gotib = 0; /* false */
   if (!getinfinibandaddr(*af,&ibaddr))
   {
      /* Got an infiniband device and do IP on IB */
      char inastr[INET6_ADDRSTRLEN+1];
      usockaddr_ntop(&ibaddr,inastr,sizeof(inastr));
      XMSG_INFO1
      (
         "RSOCK_connect: Using local infiniband network device address \"%s\".\n"
         ,inastr
      );
      gotib = 1; /* true */
   }

   /* Get the canonical name of the server host */
   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = *af;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_CANONNAME|AI_V4MAPPED|AI_NUMERICSERV;
   sprintf(service,"%d",port);
   if ((ierr=getaddrinfo(hostname,service,&hints,&aihead)) != 0)
   {
      XMSG_FATAL2
      (
         "Can\'t resolve hostname \"%s\": %s.\n"
         ,hostname
         ,gai_strerror(ierr)
      );
   }

   ai = aihead;
   if (ai->ai_canonname)
   {
      ai_canonname = ai->ai_canonname;
      if (XMSG_GETLEVEL()>=XMSG_LEVEL_DEBUG && strcmp(hostname,ai_canonname))
      {
         XMSG_DEBUG3
         (
            "RSOCK_connect: Using %s host connection \"%s:%d\" ...\n"
            ,RSOCKAF_IPVX(ai->ai_family)
            ,ai_canonname,port
         );
      }
   }

   MEMZERO(&usaddr,sizeof(USOCKADDR));
   addrc = 0;
   for(ai=aihead; ai; ai=ai->ai_next)
   {
      uint64_t currTime,trialTime,deathTime,nextmTime;

      memcpy(&usaddr,ai->ai_addr,ai->ai_addrlen);
      switch(usaddr.saddr.sa_family)
      {
         case AF_INET:
         case AF_INET6:
            break;

         default: /* Skip unix/netlink/packet etc. sockets */
#if 0
      XMSG_DEBUG1
      (
         "RSOCK_connect: got AF=%d ...\n"
         ,usaddr.saddr.sa_family
      );
#endif
            continue;
      }

      addrc++; /* One more address */
      XMSG_DEBUG3
      (
         "RSOCK_connect: Try %s connection to \"%s:%d\" ...\n"
         ,RSOCKAF_IPVX(usaddr.saddr.sa_family)
         ,ai_canonname,port
      );

      trialTime = getmstime();
      deathTime = (timeout >= 0) ? trialTime + SCAST_INTO(uint64_t,timeout) : 0;
      nextmTime = trialTime + MSG_INTERVAL;

      /*
       * Repeat: Try to connect() until the timeout is reached
       */
      for(;;)
      {
         /* Create a socket and connect to the server */
         SOCKET sd = _rsock_do_create_af(usaddr.saddr.sa_family);
         if (gotib) /* Bind the socket to the infiniband device */
         {
            if (_rsock_bind_sd(sd,&ibaddr) < 0)
            {
               closesocket(sd);
               XMSG_FATAL2
               (
                  "Can\'t bind %s client socket to infiniband device:\n%s.\n"
                  ,RSOCKAF_IPVX(ibaddr.saddr.sa_family)
                  ,SOCKET_STRERROR()
               );
               goto EXIT_FAIL;
            }
         }

         if (!_rsock_connect_sd(sd,&usaddr))
         {
            /* Success */
            XMSG_DEBUG3
            (
               "RSOCK_connect: Connected to %s host \"%s:%d\"...\n"
               ,RSOCKAF_IPVX(usaddr.saddr.sa_family)
               ,ai_canonname,port
            );

            *af = usaddr.saddr.sa_family;
            if (rhostname)
            {
               *rhostname = CAST_CHARP(MEMDUP(ai_canonname,strlen(ai_canonname)+1));
            }

            freeaddrinfo(aihead);
            return sd;
         }

      #if IS_MSWIN
         /*
          * MSWin access rights trouble: IPv6 access violations might be a VPN tunnel
          * issue. Try the next address info, and at least with IPv4 it should work.
          */
         if (usaddr.saddr.sa_family == AF_INET6 && WSAGetLastError() == WSAEACCES)
         {
            closesocket(sd);
            break; /* Try the next address info */
         }
      #endif

         /*
          * There is a bug at least on HPUX: the socket is no longer reusable after
          * the connection timed out once, so we always close the socket and create a
          * new fresh socket if connect() failed.
          */
         closesocket(sd);

         if (timeout == SSOCK_TIMEOUT_TRIAL)
         {
            /* Just one trial, no delay and no timeout */
            goto EXIT_FAIL;
         }


         /*
          * The difference between currTime & trialTime is the time spent inside connect()
          */
         currTime = getmstime();
         if (deathTime && currTime >= deathTime)
            goto EXIT_FAIL; /* Give up due to a time out */

         if (XMSG_GETLEVEL() >= XMSG_LEVEL_INFO && currTime > nextmTime)
         {
            /* Show info at end of each message interval */
            const char *mh   = "   Connection failed, retry every ";
            double delay_sec = CAST_DOUBLE(delay)/1000.0;

            if (deathTime)
            {
               XMSG_INFO3
               (
                  "%s%g seconds for %u seconds.\n"
                  ,mh
                  ,delay_sec
                  ,(CAST_UINT(deathTime-currTime)+999)/1000
               );
            }
            else
            {
               XMSG_INFO2
               (
                  "%s%g seconds.\n"
                  ,mh
                  ,delay_sec
               );
            }
            nextmTime = currTime + MSG_INTERVAL;
         }

         if ((trialTime+=delay) > currTime)
         {
            MILLISLEEP(CAST_LONG(trialTime-currTime));
         }

         trialTime = getmstime();
      }
   }

EXIT_FAIL:
   XMSG_WARNING5
   (
      "Tried %d addresses, but could not connect to %s host \"%s:%d\":\n%s.\n"
      ,addrc
      ,RSOCKAF_IPVX(usaddr.saddr.sa_family)
      ,ai_canonname,port
      ,SOCKET_STRERROR()
   );

   freeaddrinfo(aihead);
   return INVALID_SOCKET;
}

/****************************************************************************************/

#undef MSG_INTERVAL

/****************************************************************************************/
static int _rsock_do_reconnect(SOCK_t *sock, const char *cdir, const char *errmsg)
/****************************************************************************************/
/*
 * Try to reconnect a dead socket connection or call a handler function ...
 * ... which might throw a a C++ exception and do a longjump().
 *
 * return -1 in case of errors, else 0
 *
 * Note:
 *    XMSG_FATAL may return in case of recursive calls with an exit handler.
 *    In case of a lost socket connection the exit handler may close other socket
 *    connections which may also be lost.
 */
{
   if (IS_VALID_SOCKET(sock->sd))
   {
      closesocket(sock->sd);
      sock->sd = INVALID_SOCKET;
   }

   _rsock_close_pipes(sock);

   /* Check if the repair flag is set */
   if (!SSOCK_DOREPAIR(sock))
   {
      /* Call the optional handler - which might throw a C++ exception */
      if (sock->handler)
      {
         sock->handler(sock,cdir,errmsg);
      }
      goto GIVE_UP;
   }

   XMSG_WARNING5
   (
      "%s error on %s \"%s:%d\":\n"
      "%s.\n"
      "Trying to repair connection ...\n"
      ,cdir
      ,RSOCK_CTYP(sock)
      ,SSOCK_HOST(sock),SSOCK_PORT(sock)
      ,errmsg
   );

   if (RSOCK_IS_CLIENT(sock))
   {
      /*
       * We are client socket:
       *    try to re-reconnect to the server port@host
       */
      sock->sd = _rsock_do_connect
                 (
                     SSOCK_HOST(sock),SSOCK_PORT(sock),&(SSOCK_AF(sock)),
                     sock->delay,
                     sock->timeout,
                     NULL
                 );
   }
   else if (RSOCK_IS_SERVER(sock))
   {
      /*
       * We are listener socket:
       *    try to accept a reconnection from the client on the listener socket
       */
      if (IS_VALID_SOCKET(sock->listener->sd))
         sock->sd = _rsock_do_accept(sock->listener,NULL);
   }
   else /* Should never happen */
   {
      XMSG_FATAL2
      (
         "Repair connection of \"%s:%d\": Can\'t repair a listener socket.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
      );
      return -1;
   }

   if (IS_VALID_SOCKET(sock->sd))
      return 0;


GIVE_UP:
   XMSG_FATAL6
   (
      "%s error on %s \"%s:%d\":\n"
      "%s.\n"
      "Connection repair %s. Giving up ...\n"
      ,cdir
      ,RSOCK_CTYP(sock)
      ,SSOCK_HOST(sock),SSOCK_PORT(sock)
      ,errmsg
      ,(SSOCK_DOREPAIR(sock)) ? "failed" : "not activated"
   );
   return -1;
}

/****************************************************************************************/

/*
 * Public functions
 */

/****************************************************************************************/
C_FUNC_PREFIX void RSOCK_close(SOCK_t **psock)
/****************************************************************************************/
/*
 * Shutdown and close the socket and free allocated memory in reverse order of allocation.
 */
{
   SOCK_t *sock;


   if (!psock)
      return;

   sock = *psock;
   *psock = NULL;

   if (!sock)
      return;

   if (RSOCK_IS_PIPE(sock))
   {
   #if IS_MSWIN
      if (IS_VALID_HANDLE(sock->pd_w))
         FlushFileBuffers(sock->pd_w);
   #else
      if (IS_VALID_HANDLE(sock->pd_w))
         fsync(sock->pd_w);
   #endif
      _rsock_close_pipes(sock);
   }
   else
   {
      SOCKET sd = sock->sd;
      if (IS_VALID_SOCKET(sd))
      {
      #if IS_MSWIN
         WSAEventSelect(sd,NULL,0);
         if (IS_VALID_HANDLE(sock->se))
            WSACloseEvent(sock->se);
      #endif

         if (!RSOCK_IS_LISTEN(sock)) /* Then I am not a listener socket... */
         {
            int ret;

            while((ret=shutdown(sd,SD_BOTH)) != 0) ATOMAR_SOCKET_LOOP
         #if IS_MSWIN
            if (ret && WSAGetLastError() != WSAENOTCONN)
         #else
            if (ret && errno != ENOTCONN)
         #endif
               XMSG_WARNING3
               (
                  "Can\'t shutdown socket \"%s:%d\": %s.\n"
                  ,SSOCK_HOST(sock),SSOCK_PORT(sock)
                  ,SOCKET_STRERROR()
               );
         }

         if (closesocket(sd))
         {
            XMSG_WARNING3
            (
               "Can\'t close socket \"%s:%d\": %s.\n"
               ,SSOCK_HOST(sock),SSOCK_PORT(sock)
               ,SOCKET_STRERROR()
            );
         }
      }
   }

   /*
    * These malloc'ed sock->buf is set on the SSOCK_xxx level, but because
    * we free(sock) we also need to free them here.
    */
   if (sock->buf     ) FREE(sock->buf);
   if (sock->pipename) FREE(sock->pipename);
   if (sock->ibfabric) FREE(sock->ibfabric);
   if (sock->ipibname) FREE(sock->ipibname);
   if (sock->hostname) FREE(sock->hostname);

   MEMZERO(sock,sizeof(SOCK_t));
   FREE(sock);
}

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_recv
(
   SOCK_t        *sock,
   void          *buf,
   const size_t   size,
   const unsigned flags
)
/****************************************************************************************/
{
   char    *cbuf    = CAST_CHARP(buf);
   size_t   nleft   = size;
   unsigned timeout = flags&RSOCK_MASK_TIMEOUT; /* Timeout in seconds */
   SOCKET   sd      = sock->sd;
   int      nrecv;


   if (timeout)
   {
      timeout += CAST_UINT(time(NULL));
   }

   #if RSOCK_DEBUG
      XMSG_WARNING4
      (
         "RSOCK_recv(%s:%d, %s max. size=%u\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,RSOCK_CTYP(sock)
         ,(unsigned)size
      );
   #endif

   while (nleft > 0)
   {
      if (RSOCK_IS_PIPE(sock))
      {
         #if IS_MSWIN
            DWORD nr;
            if (!ReadFile(sock->pd_r,cbuf,(DWORD)nleft,&nr,NULL) || !nr)
            {
               switch(GetLastError())
               {
                  case ERROR_HANDLE_EOF:
                  case ERROR_BROKEN_PIPE:
                  case ERROR_NO_DATA:
                     nrecv = 0; /* Pipe closed */
                     break;

                  default:
                     nrecv = -1;
                     break;
               }
            }
            else
            {
               nrecv = nr;
            }
         #else
            nrecv = read(sock->pd_r,cbuf,nleft);
         #endif
      }
      else
      {
         #if RSOCK_USE_GETOOB && !IS_MSWIN
            /* First try to read any OOB message */
            char oob;
            if (recv(sd,&oob,1,MSG_OOB) == 1)
               sock->oobyte = CAST_INT(oob);
         #endif

         #if IS_MSWIN || defined(IS_IRIX65) || defined(IS_OSFALPHA)
            nrecv = recv(sd,cbuf,CAST_INT(nleft),0);
         #else
            nrecv = recv(sd,cbuf,nleft,0);
         #endif
      }

   #if RSOCK_DEBUG
      XMSG_WARNING4
      (
         "   %s:%d: %s nrecv=%d\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,RSOCK_CTYP(sock)
         ,nrecv
      );
   #endif

      if (nrecv > 0) /* 99.99% true ratio */
      {
         nleft -= CAST_UINT(nrecv);
         if (!(flags&RSOCK_FLAG_ATOMIC)) break; /* NOT atomic */
         cbuf  += nrecv;
         continue;
      }

      if (nrecv < 0) /* Error, may be EAGAIN in nonblocking mode */
      {
         SOCKET_SET_ERRNO(_socket_errno);
         if (SOCKET_DO_AGAIN(_socket_errno)) /* Non blocking receive failed */
         {
            if (timeout && timeout<CAST_UINT(time(NULL)))
               return nrecv;
            MILLISLEEP(1);
            continue;
         }
      }

      /* Other error and/or connection down/lost */
      if (flags&RSOCK_FLAG_NORECON /* Skip reconnect */
         ||_rsock_do_reconnect(sock,"Receive",(nrecv) ? SOCKET_STRERROR() : "Lost connection"))
         return nrecv;
   }

   return CAST_INT(size-nleft);
}

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_send
(
   SOCK_t        *sock,
   const void    *buf,
   const size_t   size,
   const unsigned flags
)
/****************************************************************************************/
{
   const char *cbuf  = CAST_CCHARP(buf);
   size_t      nleft = size;
   int         nsent;

/*
 * On Linux (since 2.2) a remotely closed send() connection causes a SIGPIPE.
 * In case MSG_NOSIGNAL is not defined the signal is not implemented and flags can be 0.
 */
#ifndef MSG_NOSIGNAL
   #define MSG_NOSIGNAL 0
#endif

   #if RSOCK_DEBUG
      XMSG_WARNING4
      (
         "RSOCK_send(%s:%d, %s size=%u\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,RSOCK_CTYP(sock)
         ,(unsigned)size
      );
   #endif

   while (nleft > 0)
   {
      if (RSOCK_IS_PIPE(sock))
      {
         #if IS_MSWIN
            DWORD nw;
            if (!WriteFile(sock->pd_w,cbuf,(DWORD)nleft,&nw,NULL)||!nw)
            {
               switch(GetLastError())
               {
                  case ERROR_HANDLE_EOF:
                  case ERROR_BROKEN_PIPE:
                  case ERROR_NO_DATA:
                     nsent = 0; /* pipe closed */
                     break;

                  default:
                     nsent = -1;
                     break;
               }
            }
            else
            {
               nsent = nw;
            }
         #else
            typedef void (SIGFUNC)(int);
            SIGFUNC *sighandler = signal(SIGPIPE,SIG_IGN);
            while ((nsent=write(sock->pd_w,cbuf,CAST_INT(nleft))) < 0)
               ATOMAR_SYSCALL_LOOP
            signal(SIGPIPE,sighandler);
         #endif
      }
      else
      {
         #if IS_MSWIN || defined(IS_IRIX65) || defined(IS_OSFALPHA)
            while((nsent=send(sock->sd,cbuf,CAST_INT(nleft),MSG_NOSIGNAL)) < 0)
         #else
            while((nsent=send(sock->sd,cbuf,         nleft ,MSG_NOSIGNAL)) < 0)
         #endif
               ATOMAR_SOCKET_LOOP
      }

   #if RSOCK_DEBUG
      XMSG_WARNING4
      (
         "   %s:%d: %s nsent=%d\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,RSOCK_CTYP(sock)
         ,nsent
      );
   #endif
      if (nsent <= 0)
      {
         if (flags&RSOCK_FLAG_NORECON /* Skip reconnect */
            ||_rsock_do_reconnect(sock,"Send",(nsent) ? SOCKET_STRERROR() : "Lost connection"))
            return nsent;
         continue;
      }

      nleft -= CAST_UINT(nsent);
      if (!(flags&RSOCK_FLAG_ATOMIC)) break; /* NOT atomic */
      cbuf += nsent;
   }

   return CAST_INT(size-nleft);
}

/****************************************************************************************/

#if RSOCK_USE_ISLISTEN

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_islisten(const int port)
/****************************************************************************************/
/*
 * Test if 'port' is used as a listener port on the local host. Returns
 *   -1 Error
 *    0 No one is listening
 *    1 Already used as a listener
 */
{
   struct addrinfo *aihead = NULL;
   struct addrinfo *ai = NULL;
   struct addrinfo  hints;
   USOCKADDR        usaddr;
   SOCKET           sd;
   int              inuse = 0; /* boolean */
   int              ierr;
   char             service[16];



   /* Get the canonical name of the server host */
   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_ADDRCONFIG|AI_V4MAPPED|AI_NUMERICSERV;
   sprintf(service,"%d",port);
   if ((ierr=getaddrinfo("localhost",service,&hints,&aihead)) != 0)
   {
      XMSG_WARNING1
      (
         "Can\'t resolve local host address: %s.\n"
         ,gai_strerror(ierr)
      );
      return -1;
   }

   MEMZERO(&usaddr,sizeof(USOCKADDR));
   for(ai=aihead; !inuse && ai; ai=ai->ai_next)
   {
      memcpy(&usaddr,ai->ai_addr,ai->ai_addrlen);
      switch(usaddr.saddr.sa_family)
      {
         case AF_INET:
         case AF_INET6:
            /* Contact the listener port */
            sd = _rsock_do_create_af(usaddr.saddr.sa_family);
            if (IS_VALID_SOCKET(sd))
            {
               inuse = !_rsock_connect_sd(sd,&usaddr);
               closesocket(sd);
            }
            break;

         default: /* Skip unix/netlink/packet etc. sockets */
            continue;
      }
   }

   freeaddrinfo(aihead);
   return inuse;
}

/****************************************************************************************/

#endif

#if RSOCK_USE_LISTEN

/****************************************************************************************/
C_FUNC_PREFIX char *RSOCK_prefix(const char *prefix)
/****************************************************************************************/
/*
 * Sets and returns an optional prefix name for the pipes.
 */
{
   static char pipe_prefix[64] = "";


   if (STRHASLEN(prefix))
   {
      strncpy(pipe_prefix,prefix,sizeof(pipe_prefix));
      pipe_prefix[sizeof(pipe_prefix)-1] = '\0';
   }
   return pipe_prefix;
}

/****************************************************************************************/

#if INCLUDE_STATIC
   #include "getinfinibandhosts.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *RSOCK_listen
(
   const int   afwanted,
   const int   port,
   long        delay,
   long        timeout
)
/****************************************************************************************/
/*
 * Create a listener socket which is used for accept()ing incomping connections.
 * The socket for this server may be bound to the IP address of a specific device.
 */
{
   const char *hostname;
   SOCK_t     *sock = NULL;
   USOCKADDR   usaddr;
   SOCKET      sd;
   size_t      hsize;
   int         af = afwanted;


   if (timeout < 10000)
   {
      timeout = (timeout <= 0)
         ? LONG_MAX  /* Retry for ever? */
         : 10000;    /* Min. 10 seconds */
   }

   if (delay < 500)
      delay = 500; /* Min 1/2 second */


CREATE_LISTENER_SOCKET:

   MEMZERO(&usaddr,sizeof(USOCKADDR));
   switch(af)
   {
      case AF_UNSPEC:
      case AF_INET6:
         af = usaddr.saddr.sa_family      = AF_INET6;
              usaddr.saddr_ipv6.sin6_port = htons((unsigned short)port);
         break;

      case AF_INET:
      default:
         af = usaddr.saddr.sa_family     = AF_INET;
              usaddr.saddr_ipv4.sin_port = htons((unsigned short)port);
         break;
   }

   sd = _rsock_do_create_af(af);
   if (IS_INVALID_SOCKET(sd))
   {
      af = AF_INET;
      goto CREATE_LISTENER_SOCKET;
   }

   if (af == AF_INET6)
   {
      /* Switch to dual stack mode */
      int tmp = 0;
      if (setsockopt(sd,IPPROTO_IPV6,IPV6_V6ONLY,(const char *)&tmp,sizeof(tmp)))
      {
         closesocket(sd);
         XMSG_WARNING1
         (
            "Can\'t set IPv6 dual stack mode: %s.\n"
            ,SOCKET_STRERROR()
         );
         af = AF_INET;
         goto CREATE_LISTENER_SOCKET;
      }
   }

   {
      int tmp = 1;
      if (setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,(const char *)&tmp,sizeof(tmp)))
      {
         closesocket(sd);
         XMSG_WARNING1
         (
            "Can\'t set SO_REUSEADDR socket option: %s.\n"
            ,SOCKET_STRERROR()
         );
         af = AF_INET;
         goto CREATE_LISTENER_SOCKET;
      }
   }

   hostname = getlocalhostname(NULL);
   if (_rsock_bind_sd(sd,&usaddr))
   {
      closesocket(sd);
      XMSG_MESSAGE
      (
         (af == AF_INET) ? XMSG_LEVEL_FATAL : XMSG_LEVEL_WARNING
         ,"Can\'t bind %s listener socket \"%s:%d\":\n%s.\n"
         ,RSOCKAF_IPVX(af)
         ,hostname,port
         ,SOCKET_STRERROR()
      );

      af = AF_INET;
      goto CREATE_LISTENER_SOCKET;
   }

   if (listen(sd,SOMAXCONN))
   {
      closesocket(sd);
      XMSG_MESSAGE
      (
         (af == AF_INET) ? XMSG_LEVEL_FATAL : XMSG_LEVEL_WARNING
         ,"Can\'t listen on %s socket \"%s:%d\":\n%s.\n"
         ,RSOCKAF_IPVX(af)
         ,hostname,port
         ,SOCKET_STRERROR()
      );

      af = AF_INET;
      goto CREATE_LISTENER_SOCKET;
   }

   sock           = SCAST_INTO(SOCK_t *,MALLOC(sizeof(SOCK_t))); MEMZERO(sock,sizeof(SOCK_t));
   sock->pd_r     =
   sock->pd_w     =
   sock->od_r     =
   sock->od_w     = INVALID_HANDLE_VALUE;
   sock->hostname = CAST_CHARP(MEMDUP(hostname,strlen(hostname)+1));
   sock->port     = port;
   sock->af       = af;
   sock->listener = sock; /* We are a listener socket: let us point to ourself */
   sock->delay    = delay;
   sock->timeout  = timeout;
   sock->sd       = sd;
   sock->iomode   = 'L'; /* Listener mode */
   RSOCK_CLEAROOB(sock); /* No OOB received */

   /* IPv6 is always dual mode stack */
   if (sock->af == AF_INET6)
       sock->af = AF_UNSPEC;

   hsize = 0;
   getinfinibandhosts(afwanted,&usaddr,NULL,&hsize);
   if (hsize > 0)
   {
      char *ibfabric = CAST_CHARP(MALLOC(hsize));
      if (getinfinibandhosts(afwanted,&usaddr,ibfabric,&hsize))
      {
         /* Got an infiniband device and can do IP on IB */
         char ipvxname[INET6_ADDRSTRLEN+1];

         usockaddr_ntop(&usaddr,ipvxname,sizeof(ipvxname));
         XMSG_INFO1
         (
            "RSOCK_listen: Found infiniband device with address \"%s\".\n"
            ,ipvxname
         );
         sock->ipibname = CAST_CHARP(MEMDUP(ipvxname,strlen(ipvxname)+1));
         sock->ibfabric = ibfabric;
      }
   }

#if IS_MSWIN
   sock->se = WSACreateEvent();
   WSAEventSelect(sock->sd,sock->se,FD_ACCEPT);
#endif
   return sock;
}

/****************************************************************************************/

#if INCLUDE_STATIC
   #include "getusername.c"
   #include "getmstime.c"
   #include "usockaddr_islocal.c"
#endif

#if IS_MSWIN

/****************************************************************************************/
static HANDLE _mswin_create_pipe(char *pname, char *ptail, const char *sfx)
/****************************************************************************************/
/*
 * Server: create a fifo. Returns the pipe handle.
 */
{
   HANDLE h;


   strcpy(ptail,sfx);
   h = CreateNamedPipeA
      (
        pname,
        PIPE_ACCESS_DUPLEX|FILE_FLAG_FIRST_PIPE_INSTANCE|WRITE_DAC,
        PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT|PIPE_REJECT_REMOTE_CLIENTS,
        1,
        _SSOCK_BUFSIZE,
        _SSOCK_BUFSIZE,
        20000,
        NULL
      );
   if (IS_INVALID_HANDLE(h))
   {
      XMSG_WARNING2
      (
         "Could not create pipe \"%s\": %s.\n"
         ,pname
         ,WSAStrerror()
      );
   }
   return h;
}

/****************************************************************************************/

#else

/****************************************************************************************/
static int _unix_create_pipes(char *pname, char *ptail)
/****************************************************************************************/
/*
 * Server: create a fifo. Returns -1/0=error/success
 */
{
   strcpy(ptail,PIPE_SUFFIX_IOB_WRITE); if (mkfifo(pname,0600)) goto EXIT_FAIL;
   strcpy(ptail,PIPE_SUFFIX_IOB_READ ); if (mkfifo(pname,0600)) goto EXIT_FAIL;
   strcpy(ptail,PIPE_SUFFIX_OOB_WRITE); if (mkfifo(pname,0600)) goto EXIT_FAIL;
   strcpy(ptail,PIPE_SUFFIX_OOB_READ ); if (mkfifo(pname,0600)) goto EXIT_FAIL;
   return 0;

EXIT_FAIL:
   XMSG_WARNING2
   (
      "Could not create pipe \"%s\": %s.\n"
      ,pname
      ,strerror(errno)
   );
   return -1;
}

/****************************************************************************************/
static void _unix_unlink_pipes(char *pname, char *ptail)
/****************************************************************************************/
/*
 * Remove all fifo's created.
 */
{
   strcpy(ptail,PIPE_SUFFIX_IOB_WRITE); unlink(pname);
   strcpy(ptail,PIPE_SUFFIX_IOB_READ ); unlink(pname);
   strcpy(ptail,PIPE_SUFFIX_OOB_WRITE); unlink(pname);
   strcpy(ptail,PIPE_SUFFIX_OOB_READ ); unlink(pname);
}

/****************************************************************************************/

#endif

/****************************************************************************************/
static int _switch_server_to_pipe(SOCK_t *sock)
/****************************************************************************************/
/*
 * Server creates the named pipe(s). Returns 0/1=false/true
 */
{
   const char *prefix; /* Optional prefix */
   char    *pname;
   char    *ptail;  /* End of the basename of the pipename */
   SOCKET   sd = sock->sd;
   long     timeout;
   unsigned len;
   char     response;
   char     buffer[2*MAX_PATH];


   prefix = RSOCK_prefix(NULL); /* Get an optional prefix */
   if (!STRHASLEN(prefix))
      prefix = "pipe";

   /* The first two bytes of the buffer are 'P' or 'I' or 'C' + length */
   pname = buffer + 2;

#if IS_MSWIN

   sprintf
   (
      pname
      #if IS_UNICODE
         ,"\\\\.\\pipe\\%S-rcom-%s-%I64u-%u"
      #else
         ,"\\\\.\\pipe\\%s-rcom-%s-%I64u-%u"
      #endif
      ,getusername()
      ,prefix
      ,getmstime()
      ,(unsigned)sock->port
   );

   ptail = pname + strlen(pname);
   sock->pd_w =
   sock->pd_r = _mswin_create_pipe(pname,ptail,PIPE_SUFFIX_IOB);
   sock->od_w =
   sock->od_r = _mswin_create_pipe(pname,ptail,PIPE_SUFFIX_OOB);
   if (IS_INVALID_HANDLE(sock->pd_r) || IS_INVALID_HANDLE(sock->od_r))
   {
      _rsock_close_pipes(sock);
      return 0;
   }

#else

   /* mkfifo /tmp/unique-name */
   sprintf
   (
      pname
      ,"/tmp/%s-rcom-%s-%lu-%u"
      ,getusername()
      ,prefix
      ,(unsigned long)getmstime()
      ,(unsigned)sock->port
   );

   ptail = pname + strlen(pname);
   if (_unix_create_pipes(pname,ptail))
   {
      _unix_unlink_pipes(pname,ptail);
      return 0;
   }

#endif

   *ptail = '\0';
   XMSG_INFO1("Try pipe communication: \"%s\".\n",pname);
   len = (int)strlen(pname)+1;
   buffer[0] = 'P';
   buffer[1] = (unsigned char)len;
   RSOCK_nodelay(sock,1);
   RSOCK_send   (sock,buffer,2+len,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   RSOCK_nodelay(sock,0);

#if !IS_MSWIN

   /* Read and write end are swapped on the server side */
   sock->pd_w = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_IOB_READ ,O_WRONLY);
   sock->pd_r = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_IOB_WRITE,O_RDONLY);
   if (IS_INVALID_HANDLE(sock->pd_r) || IS_INVALID_HANDLE(sock->pd_w))
   {
      _rsock_close_pipes(sock);
      _unix_unlink_pipes(pname,ptail);
      return 1;
   }

   /* Read and write end are swapped on the server side */
   sock->od_w = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_OOB_READ ,O_WRONLY);
   sock->od_r = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_OOB_WRITE,O_RDONLY|O_NONBLOCK);
   if (IS_INVALID_HANDLE(sock->od_r) || IS_INVALID_HANDLE(sock->od_w))
   {
      _rsock_close_pipes(sock);
      _unix_unlink_pipes(pname,ptail);
      return 1;
   }

#endif

   timeout = RSOCK_HELLO_TIMEOUT(sock);
   XMSG_INFO1
   (
      "Awaiting clients response within %ld milliseconds...\n"
      ,timeout
   );
   RSOCK_timeout(sock,timeout);
   len = RSOCK_recv(sock,&response,1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   RSOCK_timeout(sock,0);
   if (len <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 1;
   }

   switch(response)
   {
      case 'P': case 'p': /* Finally use the pipe */
         shutdown(sd,SD_RECEIVE);
         closesocket(sd);

         *ptail = '\0';
         sock->sd       = INVALID_SOCKET;
         sock->pipename = CAST_CHARP(MEMDUP(pname,strlen(pname)+1));
         XMSG_INFO1("Switched to named pipe: \"%s\".\n",pname);
      #if !IS_MSWIN
         /*
          * Since both, client & server have successfully opened the pipes
          * we can remove them now to avoid any junk left in /tmp
          */
         _unix_unlink_pipes(pname,ptail);
      #endif
         return 1;

      case 'C': case 'c':
         XMSG_INFO0("Continue with sockets.\n");
         _rsock_close_pipes(sock);
      #if !IS_MSWIN
         _unix_unlink_pipes(pname,ptail);
      #endif
         return 1;

      default:
         XMSG_FATAL1("Got a bad response: \'%c\'.\n",response);
         return 1;
   }
}

/****************************************************************************************/
static int _switch_server_to_ibnd(SOCK_t *sock, const char *ipibname)
/****************************************************************************************/
/*
 * Server creates the named pipe(s). Returns 0/1=false/true
 */
{
   long     timeout;
   unsigned len;
   char     response;
   char     buffer[2*MAX_PATH];


   XMSG_INFO1("Try ibnd communication: \"%s\".\n",ipibname);
   len = (int)strlen(ipibname)+1;
   buffer[0] = 'I';
   buffer[1] = (unsigned char)len;
   strcpy(buffer+2,ipibname);
   RSOCK_nodelay(sock,1);
   RSOCK_send   (sock,buffer,2+len,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   RSOCK_nodelay(sock,0);

   timeout = RSOCK_HELLO_TIMEOUT(sock);
   XMSG_INFO1
   (
      "Awaiting clients response within %ld milliseconds...\n"
      ,timeout
   );
   RSOCK_timeout(sock,timeout);
   len = RSOCK_recv(sock,&response,1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   RSOCK_timeout(sock,0);
   if (len <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 1;
   }

   switch(response)
   {
      case 'I': case 'i': /* Finally use the infiniband */
         return 1;

      case 'C': case 'c':
         XMSG_INFO0("Continue with sockets.\n");
         return 1;

      default:
         XMSG_FATAL1("Got a bad response: \'%c\'.\n",response);
         return 1;
   }
}

/****************************************************************************************/

#if INCLUDE_STATIC
   #include "stristr_a.c"
   #include "strlwr.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *RSOCK_accept(SOCK_t *listener)
/****************************************************************************************/
/*
 * Accept incoming connections for the server listener socket and return a new socket
 */
{
   SOCK_t   *sock = NULL;
   USOCKADDR usaddr;
   char      response;
   char      rhostname[HOST_NAME_MAX];


   if (!RSOCK_IS_LISTEN(listener))
   {
      XMSG_FATAL0("RSOCK_accept(): The socket is not a listener socket.\n");
   }

   sock       = SCAST_INTO(SOCK_t *,MALLOC(sizeof(SOCK_t))); MEMZERO(sock,sizeof(SOCK_t));
   sock->pd_r =
   sock->pd_w =
   sock->od_r =
   sock->od_w = INVALID_HANDLE_VALUE;
   sock->sd   = _rsock_do_accept(listener,&usaddr);

   /* Get the hostname of the connected client IP address by a reverse DNS lookup */
   if (getnameinfo(&(usaddr.saddr),sizeof(USOCKADDR),rhostname,sizeof(rhostname),NULL,0,NI_NAMEREQD))
   {
      /* rhostname is an address string */
      usockaddr_ntop(&usaddr,rhostname,sizeof(rhostname));
   }
   else
   {
      /* rhostname in lowercase */
      strlwr(rhostname);
   }

   sock->port     = (usaddr.saddr.sa_family == AF_INET6)
                     ? CAST_INT(ntohs(usaddr.saddr_ipv6.sin6_port))
                     : CAST_INT(ntohs(usaddr.saddr_ipv4.sin_port ));
   sock->af       = usaddr.saddr.sa_family;
   sock->listener = listener; /* We have a listener socket associated */
   sock->hostname = CAST_CHARP(MEMDUP(rhostname,strlen(rhostname)+1));
   sock->delay    = listener->delay;
   sock->timeout  = listener->timeout;
   RSOCK_CLEAROOB(sock);   /* No OOB received */

   response = 'C';
   if (usockaddr_islocal(&usaddr) > 0)
   {
      /* Client & server process run on the same node */
      if (_switch_server_to_pipe(sock))
         response = 0; /* Switched into pipe mode */
   }

   if (response
      && listener->ipibname
      && listener->ibfabric
      && stristr_a(listener->ibfabric,rhostname))
   {
      /* Client & server process run within the same infiniband fabric */
      if (_switch_server_to_ibnd(sock,listener->ipibname))
         response = 0; /* Switched into pipe mode */
   }

   if (response)
   {
      /* Continue the connection as is */
      RSOCK_nodelay(sock,1);
      RSOCK_send   (sock,&response,1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
      RSOCK_nodelay(sock,0);
   }

#if IS_MSWIN
   if (RSOCK_IS_SOCK(sock))
   {
      sock->se = WSACreateEvent();
      WSAEventSelect(sock->sd,sock->se,FD_READ|FD_CLOSE);
   }
#endif

   XMSG_DEBUG3
   (
      "Accepted %s connection from \"%s:%d\".\n"
      ,RSOCK_CTYP(sock)
      ,SSOCK_HOST(sock),SSOCK_PORT(sock)
   );
   return sock;
}

/****************************************************************************************/

#endif

#if RSOCK_USE_CLONE

/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *RSOCK_clone(SOCK_t *sock, SOCK_t *snew, char *buf)
/****************************************************************************************/
/*
 * Clone a device: The clone is identical with its twin except
 * the buffer and current status ... its brain.
 */
{
   memcpy(snew,sock,sizeof(SOCK_t));
   snew->buf =
   snew->ptr = buf; /* The cloned device has its own buffer */
   snew->cnt = 0;
   SSOCK_IOMODE(snew) = '?';
   return snew;
}

/****************************************************************************************/

#endif

#if RSOCK_USE_SELECT

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_select
(
   SOCK_t    *sockv[],
   const int  nsock,
   unsigned   request,
   const long timeout
)
/****************************************************************************************/
/*
 * Do a select() on all sockets in the sockv[] and return a bitpattern for
 * each entry in sockv[], not for the file descriptors used within the sockv[].
 * FD_XXX() and FD_SETSIZE logic is different unter MSWin and Unix.
 */
{
   struct timeval tout, *pTout;
   fd_set   fds_read,   *pRead;
   fd_set   fds_write,  *pWrite;
   fd_set   fds_except, *pExcept;
   int      i,ret;
   SOCKET   maxfd; /* == int under UNIX */


   if (nsock <= 0)
      return -1;

#if IS_MSWIN
   if (nsock > FD_SETSIZE)
   {
      XMSG_FATAL2
      (
         "RSOCK_select(): Currently FD_SETSIZE=%d is the max. no. of sockets for select().\n"
         "                Recompile the code with FD_SETSIZE >= %d.\n",
         FD_SETSIZE,nsock
      );
   }
#endif

   /* Avoid a bad mask */
   if ((request & (RSOCK_SBIT_READ|RSOCK_SBIT_WRITE|RSOCK_SBIT_EXCEPT)) == 0)
        request = (RSOCK_SBIT_READ|RSOCK_SBIT_WRITE|RSOCK_SBIT_EXCEPT);


   pRead   = (request & RSOCK_SBIT_READ  ) ? &fds_read   : NULL;
   pWrite  = (request & RSOCK_SBIT_WRITE ) ? &fds_write  : NULL;
   pExcept = (request & RSOCK_SBIT_EXCEPT) ? &fds_except : NULL;

   FD_ZERO(&fds_read  );
   FD_ZERO(&fds_write );
   FD_ZERO(&fds_except);

   maxfd = 0; /* 0=stdin */
   for (i=0; i<nsock; i++)
   {
      SOCK_t *sock = sockv[i];
      SOCKET  sd   = sock->sd;

   #if !IS_MSWIN
      if (sd > FD_SETSIZE)
      {
         XMSG_FATAL2
         (
            "RSOCK_select(): Currently FD_SETSIZE=%d is the highest file descriptor for select().\n"
            "                Recompile the code with FD_SETSIZE >= %d.\n",
            FD_SETSIZE,sd
         );
      }
   #endif

      if (sd > maxfd)
         maxfd = sd;

      /* Delete the bits in the select pattern. do not clear select! */
      sock->select &= ~request;

   #if IS_MSWIN
      #pragma warning(disable:4127)
   #endif
      FD_SET(sd,&fds_read  );
      FD_SET(sd,&fds_write );
      FD_SET(sd,&fds_except);
   #if IS_MSWIN
      #pragma warning(default:4127)
   #endif
   }

   pTout = NULL; /* Default: assume no timeout == infinite wait */
   if (timeout >= 0)  /* Wait 'timeout' milliseconds, timeout may be 0 == do not wait */
   {
      tout.tv_sec  = CAST_INT(timeout/1000);
      tout.tv_usec = 1000*(timeout%1000);
      pTout        = &tout;
   }

   while((ret=select(CAST_INT(maxfd+1),pRead,pWrite,pExcept,pTout)) < 0)
      ATOMAR_SOCKET_LOOP

   /* -1=error,  0=timeout,  >0=no. of selections */

   if (ret < 0)
   {
      XMSG_WARNING1("Can\'t select: %s.\n",SOCKET_STRERROR());
   }

   else if (ret > 0)
   {
      for (i=0; i<nsock; i++)
      {
         SOCK_t *sock = sockv[i];
         SOCKET  sd   = sock->sd;

         if (FD_ISSET(sd,&fds_read  )) sock->select |= RSOCK_SBIT_READ;
         if (FD_ISSET(sd,&fds_write )) sock->select |= RSOCK_SBIT_WRITE;
         if (FD_ISSET(sd,&fds_except)) sock->select |= RSOCK_SBIT_EXCEPT;
      }
   }

   return ret;
}

/****************************************************************************************/

#endif

#if RSOCK_USE_RSELECT

#if IS_MSWIN
   #include "WSAWaitForMultipleEvents4K.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_rselect(SOCK_t *sockv[], const int nsock, const long timeout)
/****************************************************************************************/
/*
 * Like RSOCK_select(), but only RSOCK_SBIT_READ is handled
 */
{
#if IS_MSWIN

   HANDLE eventv[FD_SETSIZE];
   DWORD  result;
   int    i,npipes;


   if (nsock <= 0)
      return -1;

   if (nsock > FD_SETSIZE)
   {
      XMSG_FATAL2
      (
         "RSOCK_rselect(): Currently FD_SETSIZE=%d is the max. no. of sockets for select().\n"
         "                 Recompile the code with FD_SETSIZE >= %d.\n",
         FD_SETSIZE,nsock
      );
   }

   /*
    * Create the array of events and count the no. of pipes (not sockets)
    */
   for (npipes=i=0; i<nsock; i++)
   {
      SOCK_t *sock = sockv[i];

      if (RSOCK_IS_PIPE(sock))
      {
         eventv[i] = sock->pd_r;
         npipes++;
      }
      else
      {
         eventv[i] = sock->se;
      }

      /* Delete the bits in the select pattern. do not clear select! */
      sock->select &= ~RSOCK_SBIT_READ;
   }

   /*
    * Select on the events array
    */
   result = (nsock > WSA_MAXIMUM_WAIT_EVENTS)
      ? WSAWaitForMultipleEvents4K(nsock,eventv,FALSE,(timeout>=0)?timeout:WSA_INFINITE,FALSE)
      : WSAWaitForMultipleEvents  (nsock,eventv,FALSE,(timeout>=0)?timeout:WSA_INFINITE,FALSE);

   switch(result)
   {
      case WSA_WAIT_TIMEOUT:
         return 0;

      default:
         if (result >= WSA_WAIT_EVENT_0 && result < (WSA_WAIT_EVENT_0+nsock))
         {
            SOCK_t *sock;
            int     nselect = 1;

            result -= WSA_WAIT_EVENT_0;

            sock = sockv[result];
            sock->select |= RSOCK_SBIT_READ;
            if (RSOCK_IS_SOCK(sock))
            {
               WSAResetEvent(sock->se);
            }
            else
            {
               npipes--;
            }

            if (npipes > 0)
            {
               for(i=0; i<nsock; i++)
               {
                  if ((DWORD)i != result)
                  {
                     sock = sockv[i];
                     if (RSOCK_IS_PIPE(sock))
                     {
                        DWORD nb;
                        char  buf;
                        if (PeekNamedPipe(sock->pd_r,&buf,1,NULL,&nb,NULL) && nb)
                        {
                           sock->select |= RSOCK_SBIT_READ;
                           nselect++;
                        }
                     }
                  }
               }
            }

            return nselect;
         }
         /*FALLTHROUGH*/

      case WSA_WAIT_FAILED:
         XMSG_WARNING1
         (
            "Can\'t select: %s.\n"
            ,WSAStrerror()
         );
         break; /* returns -1 */
   }
   return -1;


#else

   struct timeval tout, *pTout;
   fd_set   fds_read;
   int      i,ret,maxfd;


   if (nsock <= 0)
      return -1;

   FD_ZERO(&fds_read);
   maxfd = 0; /* 0=stdin */
   for (i=0; i<nsock; i++)
   {
      SOCK_t *sock = sockv[i];
      const int fd = (RSOCK_IS_PIPE(sock)) ? sock->pd_r : sock->sd;

      if (fd > FD_SETSIZE)
      {
         XMSG_FATAL2
         (
            "RSOCK_rselect(): Currently FD_SETSIZE=%d is the highest file descriptor for select().\n"
            "                 Recompile the code with FD_SETSIZE >= %d.\n",
            FD_SETSIZE,fd
         );
      }

      if (fd > maxfd)
         maxfd = fd;

      /* Delete the bits in the select pattern. do not clear select! */
      sock->select &= ~RSOCK_SBIT_READ;
      FD_SET(fd,&fds_read);
   }

   pTout = NULL; /* Default: assume no timeout == infinite wait */
   if (timeout >= 0)  /* Wait 'timeout' milliseconds, timeout may be 0 == do not wait */
   {
      tout.tv_sec  = CAST_INT(timeout/1000);
      tout.tv_usec = 1000*(timeout%1000);
      pTout        = &tout;
   }

#if 0
XMSG_WARNING1("RSOCK_rselect(nsock=%d) ....\n",nsock);
#endif
   while((ret=select(CAST_INT(maxfd+1),&fds_read,NULL,NULL,pTout)) < 0)
      ATOMAR_SOCKET_LOOP

   /* -1=error,  0=timeout,  >0=no. of selections */
   if (ret > 0)
   {
      for (i=0; i<nsock; i++)
      {
         SOCK_t *sock = sockv[i];
         const int fd = (RSOCK_IS_PIPE(sock)) ? sock->pd_r : sock->sd;
         if (FD_ISSET(fd,&fds_read)) sock->select |= RSOCK_SBIT_READ;
      }
   }
   else if (ret < 0)
   {
      XMSG_WARNING1
      (
         "Can\'t select: %s.\n"
         ,SOCKET_STRERROR()
      );
   }
#if 0
XMSG_WARNING1("RSOCK_rselect(ret=%d) ....\n",ret);
#endif
   return ret;

#endif

}

/****************************************************************************************/

#endif

#if RSOCK_USE_CLIENT

#if IS_MSWIN
/****************************************************************************************/
static HANDLE _mswin_open_pipe(char *pname, char *ptail, const char *sfx)
/****************************************************************************************/
{
   HANDLE h;


   strcpy(ptail,sfx);
   h = CreateFileA
       (
            pname,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
       );
   if (IS_INVALID_HANDLE(h))
   {
      XMSG_WARNING2
      (
         "Failed to open pipe \"%s\": %s.\n"
         ,pname
         ,WSAStrerror()
      );
   }
   return h;
}

/****************************************************************************************/
#else
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
#endif

/****************************************************************************************/
static int _open_client_pipe(SOCK_t *sock)
/****************************************************************************************/
/*
 * Client opens the named pipe(s). Returns 0/1=false/true
 */
{
   char    *ptail; /* End of the basename of the pipename */
   long     timeout = RSOCK_HELLO_TIMEOUT(sock);
   char     pname[2*MAX_PATH];
   unsigned len;


   /* Receive the length (y<=255) of the servers pipename */
   if (RSOCK_recv(sock,pname,sizeof(char),RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON) <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 0;
   }

   len = (unsigned)pname[0];
   if (RSOCK_recv(sock,pname,len,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON) <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 0;
   }

   XMSG_INFO1("Try pipe communication: \"%s\".\n",pname);

   ptail = pname + strlen(pname); /* End of the basename of the pipename */

#if IS_MSWIN

   sock->pd_w =
   sock->pd_r = _mswin_open_pipe(pname,ptail,PIPE_SUFFIX_IOB);
   if (IS_INVALID_HANDLE(sock->pd_r))
      return 0;

   sock->od_w =
   sock->od_r = _mswin_open_pipe(pname,ptail,PIPE_SUFFIX_OOB);
   if (IS_INVALID_HANDLE(sock->od_r))
   {
      _rsock_close_pipes(sock);
      return 0;
   }

#else

   /* Read and write end are swapped on the server side: write-iob & read-iob */
   sock->pd_r = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_IOB_READ ,O_RDONLY);
   sock->pd_w = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_IOB_WRITE,O_WRONLY);
   if (IS_INVALID_HANDLE(sock->pd_r) || IS_INVALID_HANDLE(sock->pd_w))
   {
      _rsock_close_pipes(sock);
      return 0;
   }

   sock->od_r = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_OOB_READ ,O_RDONLY|O_NONBLOCK);
   sock->od_w = _unix_open_pipe(pname,ptail,PIPE_SUFFIX_OOB_WRITE,O_WRONLY);
   if (IS_INVALID_HANDLE(sock->od_r) || IS_INVALID_HANDLE(sock->od_w))
   {
      _rsock_close_pipes(sock);
      return 0;
   }

#endif

   /* Send back the response */
   RSOCK_nodelay(sock,1);
   RSOCK_send(sock,"P",1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   shutdown(sock->sd,SD_BOTH);
   closesocket(sock->sd);
   *ptail = '\0';
   sock->sd       = INVALID_SOCKET;
   sock->pipename = CAST_CHARP(MEMDUP(pname,strlen(pname)+1));
   XMSG_INFO1("Switched to named pipe: \"%s\".\n",pname);
   return 1;
}

/****************************************************************************************/
static int _open_client_ibnd(SOCK_t *sock)
/****************************************************************************************/
/*
 * Client (re)connects via IP over IB. Returns 0/1=false/true
 */
{
   long     timeout = RSOCK_HELLO_TIMEOUT(sock);
   char     pname[2*MAX_PATH];
   unsigned len;


   /* Receive the length (y<=255) of the servers pipename */
   if (RSOCK_recv(sock,pname,sizeof(char),RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON) <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 0;
   }

   len = (unsigned)pname[0];
   if (RSOCK_recv(sock,pname,len,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON) <= 0)
   {
      _rsock_timeout_msg(sock,timeout);
      return 0;
   }

   XMSG_INFO1("Try ibnd communication: \"%s\".\n",pname);
   RSOCK_nodelay(sock,1);
   RSOCK_send(sock,"I",1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   XMSG_INFO1("Switched to ibnd: \"%s\".\n",pname);
   return 1;
}

/****************************************************************************************/

/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *RSOCK_connect
(
   const char *servername,
         int   port,
         long  delay,
         long  timeout
)
/****************************************************************************************/
{
   const char *hostname;
   char       *rhostname = NULL;
   SOCK_t     *sock = NULL;
   SOCKET      sd;
   int         af;


   if (!STRHASLEN(servername))
   {
      hostname = getlocalhostname(NULL);
   }
   else if ((hostname=splitporthost(servername,&port,1)) == NULL)
   {
      XMSG_FATAL1("Bad server name \"%s\".\n",servername);
   }

   af = AF_UNSPEC;
   sd = _rsock_do_connect(hostname,port,&af,delay,timeout,&rhostname);
   if (IS_VALID_SOCKET(sd))
   {
      char response;

      sock = SCAST_INTO(SOCK_t *,MALLOC(sizeof(SOCK_t))); MEMZERO(sock,sizeof(SOCK_t));
      sock->pd_r     =
      sock->pd_w     =
      sock->od_r     =
      sock->od_w     = INVALID_HANDLE_VALUE;
      sock->sd       = sd;
      sock->listener = NULL;  /* We are a client */
      sock->hostname = rhostname;
      sock->port     = port;
      sock->af       = af;
      sock->delay    = delay;
      sock->timeout  = timeout;
      RSOCK_CLEAROOB(sock);  /* No OOB received */

      /*
       * Receive servers response
       */
      timeout = RSOCK_HELLO_TIMEOUT(sock);
      XMSG_INFO1("Awaiting response within %ld milliseconds...\n",timeout);

      RSOCK_timeout(sock,timeout);
      if (RSOCK_recv(sock,&response,1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON) <= 0)
      {
         _rsock_timeout_msg(sock,timeout);
         RSOCK_timeout(sock,0);
         return sock;
      }

      XMSG_INFO1("Got response: \'%c\'.\n",response);
      switch(response)
      {
         case 'P': case 'p':
            /*
             * Client and server process run on the same node.
             * Switch into pipe mode
             */
            response = (_open_client_pipe(sock))
               ? 0      /* Success: response already sent out */
               : 'C';   /* Failed: continue as it is */
            break;

         case 'I': case 'i':
            /*
             * Client and server process run on the same infiniband fabric.
             * Switch to IP on IB mode.
             */
            response = (_open_client_ibnd(sock))
               ? 0      /* Success: response already sent out */
               : 'C';   /* Failed: continue as it is */
            break;

         case 'C': case 'c':
            /*
             * No speedup possible. Keep on going as it is.
             */
            response = 0;
            break;

         default:
            /*
             * Bad response from the server
             */
            XMSG_FATAL1("Got a bad response: \'%c\'.\n",response);
            break;
      }

      RSOCK_timeout(sock,0);
      if (response)
      {
         RSOCK_nodelay(sock,1);
         RSOCK_send   (sock,&response,1,RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
         RSOCK_nodelay(sock,0);
      }

   #if IS_MSWIN
      if (RSOCK_IS_SOCK(sock))
      {
         sock->se = WSACreateEvent();
         WSAEventSelect(sock->sd,sock->se,FD_READ|FD_CLOSE);
      }
   #endif
   }
   return sock;
}

/****************************************************************************************/

#endif

#if RSOCK_USE_GETOOB

#if !IS_MSWIN
   #ifdef IS_SUNOS
      #include <sys/sockio.h>
   #endif
   #include <sys/ioctl.h>
#endif

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_getoob(SOCK_t *sock)
/****************************************************************************************/
/*
 * Get a single OOB byte and return the (int)byte or -1 if the
 * OOB byte is not available.
 */
{
   char resp;

   if (RSOCK_IS_PIPE(sock))
   {
      HANDLE fd = sock->od_r;
      if (IS_VALID_HANDLE(fd))
      {
      #if IS_MSWIN

         DWORD nr;
         if (!PeekNamedPipe(fd,&resp,1,NULL,&nr,NULL) || !nr)
            return -1;

         ReadFile(fd,&resp,1,&nr,NULL);
         return CAST_INT(CAST_UCHAR(resp));

      #else

         return (read(fd,&resp,1) == 1)
            ? CAST_INT(CAST_UCHAR(resp))
            : -1;

      #endif
      }
   }
   else
   {
      SOCKET sd = sock->sd;
      if (IS_VALID_SOCKET(sd))
      {
      #if IS_MSWIN

         int    isempty = 1;
         DWORD  nr;

         /*
          * If WSAIoctl() returns a BOOL FALSE, then OOB was not read
          * and the following recv returns the OOB data
          */
         if (WSAIoctl(sd,SIOCATMARK,NULL,0,&isempty,sizeof(isempty),&nr,NULL,NULL) /* ioctl error */
               || isempty) return -1; /* No OOB data available */

         /* Get the OOB byte and return it as an int */
         resp = '\0';
         return (recv(sd,&resp,1,MSG_OOB) == 1)
            ? CAST_INT(CAST_UCHAR(resp))
            : -1;

      #else /* UNIX, what else */

         /*
          * Under Unix we try to read the OOB byte before each receive
          * and store it in the sock->oobyte
          */
         int oob = sock->oobyte;
         if (oob >= 0)
         {
            RSOCK_CLEAROOB(sock); /* Reset the oobyte */
         }
         else /* Try to receive now */
         {
            if (recv(sd,&resp,1,MSG_OOB) == 1)
               oob = CAST_INT(CAST_UCHAR(resp));
         }
         return oob; /* Return the byte we got */

      #endif
      }
   }

   return -1;

}

/****************************************************************************************/

#endif


#if RSOCK_USE_PUTOOB

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_putoob(SOCK_t *sock, const char oob)
/****************************************************************************************/
/*
 * Send a single oobyte as an urgent message. Returns the no. of bytes sent (1,0,-1)
 */
{
   int ret = 1;

   if (RSOCK_IS_PIPE(sock))
   {
      if (IS_VALID_HANDLE(sock->od_w))
      {
      #if IS_MSWIN
         DWORD nw;
         ret = (WriteFile(sock->od_w,&oob,1,&nw,NULL)) ? nw : -1;
      #else
         ret = write(sock->od_w,&oob,1);
      #endif
      }
   }
   else
   {
      ret = send(sock->sd,&oob,1,MSG_OOB);
   }
   return ret;
}

/****************************************************************************************/

#endif


#ifndef RSOCK_NO_INIT

/****************************************************************************************/
C_FUNC_PREFIX int RSOCK_init(void)
/****************************************************************************************/
/*
 * MSWin requires the socket dll ws2_32.dll library to be loaded and initialised.
 * If we are linked to a third party code which may already have initialized
 * a specific winsock version we avoid a second initialisation. The test is whether
 * we can create a socket or not. If not, we WSAStartup().
 */
{
#if IS_MSWIN

   SOCKET         sock;
   struct WSAData wsaData;


   /* Try to create a socket and get the reason for the failure */
   sock = WSASocket(AF_INET,SOCK_STREAM,IPPROTO_TCP,NULL,0,0);
   if (IS_VALID_SOCKET(sock)) /* We're OK */
   {
      closesocket(sock);
      return 0;
   }

   if (WSAGetLastError() != WSANOTINITIALISED)
      return -1;

   if (WSAStartup(WINSOCK_VERSION,&wsaData))
   {
      XMSG_FATAL1
      (
         "Can\'t initialize the Windows socket adapter: %s.\n",
         WSAStrerror()
      );
      return -1;
   }

   XMSG_DEBUG2
   (
      "Initialized Windows socket adapter version %d.%d.\n",
      LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion)
   );

#endif

   return 0;
}

#endif

/****************************************************************************************/

#endif

#if RSOCK_USE_DEBUG

/****************************************************************************************/
C_FUNC_PREFIX void RSOCK_debug(SOCK_t *sock, FILE *fp)
/****************************************************************************************/
/*
 * Debug a SOCK_t
 */
{
   union
   {
      const void *vp;
      int (*hp)(SOCK_t*, const char*, const char*);
   } handler;

   if (!fp)
      return;

   if (sock)
   {
      handler.hp = sock->handler;

      fprintf
      (
         fp,
         "Socket information: \"%s\":\n"
         "   Hostname: \"%s\"\n"
         "   Port    : %d\n"
         "   Listener: %p\n"
         "   Handler : %p\n"
      #if IS_MSWIN
         "   sd      : %u\n"
      #else
         "   sd      : %d\n"
      #endif
         "   Valid   : %d\n"
         "   Delay   : %ld\n"
         "   Timeout : %ld\n"
         "   Swap    : %d\n"
         "   Repair  : %d\n"
         "   Select  : 0x%08x\n"
         "   Mode    : %c\n"
         "   Buffer  : %p\n"
         "   Position: %u\n"
         "   Counter : %u\n"
         ,CAST_VOIDP(sock)
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,CAST_VOIDP(sock->listener)
         ,handler.vp
      #if IS_MSWIN
         ,CAST_UINT(sock->sd)
      #else
         ,sock->sd
      #endif
         ,IS_VALID_SOCKET(sock->sd)
         ,SSOCK_DELAY(sock)
         ,SSOCK_TIMEOUT(sock)
         ,SSOCK_DOSWAP(sock)
         ,SSOCK_DOREPAIR(sock)
         ,SSOCK_SELECT(sock)
         ,SSOCK_IOMOODE(sock)
         ,CAST_VOIDP(sock->buf)
         ,CAST_UINT(sock->ptr - sock->buf)
         ,CAST_UINT(sock->cnt)
      );
   }
   else
   {
      fprintf
      (
         fp,
         "Socket information: NULL pointer\n"
      );
   }
}

/****************************************************************************************/

#undef PIPE_SUFFIX_IOB
#undef PIPE_SUFFIX_IOB_READ
#undef PIPE_SUFFIX_IOB_WRITE

#undef PIPE_SUFFIX_OOB
#undef PIPE_SUFFIX_OOB_READ
#undef PIPE_SUFFIX_OOB_WRITE

#endif
