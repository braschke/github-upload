#pragma once
#ifndef streamsocket_SOURCE_INCLUDED
#define streamsocket_SOURCE_INCLUDED
/* streamsocket.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *   Implementation of a stream socket/pipe. The reason is that we can't use the preferred
 *   way of C stdio streams with fdopen()/freopen() with the winsock under MSWin!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: streamsocket.c 4926 2016-07-25 12:38:33Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"
#include "xmem.h"

#include <stdarg.h>

#if INCLUDE_STATIC
   #include "rawsocket.c"
   #include "getendianess.c"
#endif

/****************************************************************************************/
static int _ssock_do_handshake(SOCK_t *sock, SSOCK_HI_t *hi)
/****************************************************************************************/
/*
 * Send and receive a handshake message immediately after the connection is established.
 * Check if the message on both sides is identical and we can do a binary transfer.
 */
{
#define _SSOCK_HS_BUFSIZE    512
#define _SSOCK_HS_MSGSIZE    128 /* <= SSOCK_HELLO_MAX !!!!! */

   static struct magic_header_info_struct
   {
      const char *dsc; /* Description of the unsigned flag */
      unsigned    loc; /* Value of this flag in the local machine (setup done here and sent) */
      unsigned    rem; /* Value of this flag on the remote machine (setup received) */
   } magicInfo[] =
   {
      { "strlen(magic)"       , 0                   , 0 },
      { "strlen(hello)"       , 0                   , 0 },
      { "sizeof(short)"       , usizeof(short)      , 0 },
      { "sizeof(int)"         , usizeof(int)        , 0 },
      { "sizeof(long)"        , usizeof(long)       , 0 },
#if HAVE_LONGLONG
      { "sizeof(long long)"   , usizeof(long long)  , 0 },
#else
      { "sizeof(long long)"   , usizeof(long)*2     , 0 },
#endif
      { "sizeof(float)"       , usizeof(float)      , 0 },
      { "sizeof(double)"      , usizeof(double)     , 0 },
      { "sizeof(long double)" , usizeof(long double), 0 }
   };

                     /* Offsets =     2    5    8   11   14   17   20   23   26   29  */
   static const char fmtInfo[] = "%c%c:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:";
#define _SSOCK_HSOF_SIZEOF  9
#define _SSOCK_HSOF_MAGIC  30

   size_t   lenMagic, lenHello;
   long     timeout;
   int      i;
   char     locBuf[_SSOCK_HS_BUFSIZE], cLocEnd, cLocSys;
   char     remBuf[_SSOCK_HS_BUFSIZE], cRemEnd, cRemSys;


   /* Clip message, len must fit into one byte: < 256! */
   lenMagic = strlen(hi->magic);
   if (lenMagic > _SSOCK_HS_MSGSIZE) lenMagic = _SSOCK_HS_MSGSIZE;

   lenHello = strlen(hi->hello);
   if (lenHello > _SSOCK_HS_MSGSIZE) lenHello = _SSOCK_HS_MSGSIZE;

   switch(hi->endianess)
   {
      case 0:
         cLocEnd = CAST_CHAR(getendianess()); /* 'I'=intel | 'M'=motorola */
         break;

      case ENDIANESS_INTEL: /* hi->endianess overwrites the machines endianess */
      case ENDIANESS_MOTOROLA:
         cLocEnd = CAST_CHAR(hi->endianess);
         break;

      default:
         XMSG_WARNING1("Got a bad external endianess definition \'%c\'.\n",hi->endianess);
         return -1;
   }

#if IS_MSWIN /* 'W'=windows | 'U'=unix | 'M'=mac */
   cLocSys = 'W';
#else
   cLocSys = 'U';
#endif
   /* Update the local informations and fill the message buffer */
   magicInfo[0].loc = CAST_UINT(lenMagic);
   magicInfo[1].loc = CAST_UINT(lenHello);
   MEMZERO(locBuf,sizeof(locBuf));
   sprintf
   (
      locBuf, fmtInfo, cLocEnd, cLocSys,
      magicInfo[0].loc,
      magicInfo[1].loc,
      magicInfo[2].loc,
      magicInfo[3].loc,
      magicInfo[4].loc,
      magicInfo[5].loc,
      magicInfo[6].loc,
      magicInfo[7].loc,
      magicInfo[8].loc
   );

   /* Length of the header containing the byte information */
   if (lenMagic) memcpy(locBuf+_SSOCK_HSOF_MAGIC         ,hi->magic,lenMagic);
   if (lenHello) memcpy(locBuf+_SSOCK_HSOF_MAGIC+lenMagic,hi->hello,lenHello);

   /* Send the hello string */
   RSOCK_nodelay(sock,1);
   i = RSOCK_send(sock,locBuf,sizeof(locBuf),RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON);
   RSOCK_nodelay(sock,0);
   if (i <= 0)
   {
      XMSG_WARNING3
      (
         "%s:%d: Failed to send hello message: %s.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,(i) ? SOCKET_STRERROR() : "Lost connection"
      );
      return -1;
   }

   /*
    * Receive partners hello string: make it a non blocking receive with a proper timeout.
    */
   timeout = RSOCK_HELLO_TIMEOUT(sock);
   RSOCK_timeout(sock,timeout);
   i = RSOCK_recv(sock,remBuf,sizeof(remBuf),RSOCK_FLAG_ATOMIC|RSOCK_FLAG_NORECON|1);
   RSOCK_timeout(sock,0);
   if (i <= 0)
   {
      XMSG_WARNING3
      (
         "%s:%d: Failed to receive hello message within %ld milliseconds.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,timeout
      );
      return -1;
   }

   /* Set last byte to 0 for the printf */
   remBuf[_SSOCK_HS_BUFSIZE-1] = '\0';
   if (sscanf
       (
         remBuf, fmtInfo, &cRemEnd, &cRemSys,
         &(magicInfo[0].rem),
         &(magicInfo[1].rem),
         &(magicInfo[2].rem),
         &(magicInfo[3].rem),
         &(magicInfo[4].rem),
         &(magicInfo[5].rem),
         &(magicInfo[6].rem),
         &(magicInfo[7].rem),
         &(magicInfo[8].rem)
       ) != 11)
      goto EXIT_FATAL;

   /*
    * Check for endianess 'I' or 'M' or magic pattern mismatch.
    * int comparison to avoid compiler complaints
    */
   if (cRemEnd != ENDIANESS_INTEL && cRemEnd != ENDIANESS_MOTOROLA)
      goto EXIT_FATAL;

   if (cRemSys != 'U' && cRemSys != 'W' && cRemSys != 'M')
      goto EXIT_FATAL;

   if (magicInfo[0].rem != CAST_UINT(lenMagic))
      goto EXIT_FATAL;

   if (memcmp(locBuf+_SSOCK_HSOF_MAGIC,remBuf+_SSOCK_HSOF_MAGIC,lenMagic))
      goto EXIT_FATAL;

   if (memcmp
       (
          locBuf+_SSOCK_HSOF_SIZEOF,
          remBuf+_SSOCK_HSOF_SIZEOF,
          _SSOCK_HSOF_MAGIC-_SSOCK_HSOF_SIZEOF
       ))
   {
      /* No full binary match: check the sizeof(xxx) bytes in the header */
      int mismatch = 0;

      for (i=2; i<9; i++)
      {
         if (magicInfo[i].loc == magicInfo[i].rem) continue;
         if ((hi->match & (1<<(i-2))) == 0       ) continue;
         XMSG_WARNING3
         (
            "   %s not identical: local(%d bytes), remote(%d bytes).\n",
            magicInfo[i].dsc,magicInfo[i].loc,magicInfo[i].rem
         );
         mismatch = 1;

         /* NOT YET COMPLETE: need to set additional flags in the SOCK_t structure */
      }

      if (mismatch) /* No match: check the sizeof(xxx) bytes in the header */
         XMSG_FATAL2
         (
            "%s:%d: Can\'t use binary transfer.\n"
            ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         );
   }

   SSOCK_DOSWAP(sock) = (cRemEnd != cLocEnd); /* Need to swap endianness? */
   if (SSOCK_DOSWAP(sock))
   {
      XMSG_DEBUG4
      (
         "%s:%d: Converting endianess from \'%c\' to \'%c\'.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
         ,cRemEnd
         ,cLocEnd
      );
   }
   else
   {
      XMSG_DEBUG2
      (
         "%s:%d: Using binary transfer.\n"
         ,SSOCK_HOST(sock),SSOCK_PORT(sock)
      );
   }

   strncpy(hi->hello,remBuf+_SSOCK_HSOF_MAGIC+lenMagic,SSOCK_HELLO_MAX);
   hi->hello[SSOCK_HELLO_MAX-1] = '\0';
   return 0;


EXIT_FATAL:
   /* Clean up the received buffer for printout */
   for(i=0; remBuf[i]; i++)
      if (!ISPRINT(remBuf[i]) || ISSPACE(remBuf[i]))
         remBuf[i] = ' ';

   XMSG_WARNING1( "Socket platform information handshake failed:\n\n"
                  "   Magic sent: \"%s\"\n",locBuf);
   for(i=0; i<9; i++) XMSG_WARNING2("   %s = %d\n",magicInfo[i].dsc,magicInfo[i].loc);

   XMSG_WARNING1("\n   Magic received: \"%s\"\n",remBuf);
   for(i=0; i<9; i++) XMSG_WARNING2("   %s = %d\n",magicInfo[i].dsc,magicInfo[i].rem);
   XMSG_WARNING0("Uups, found bug: Call 1-800-spiderman and get rid of this bug!\n");
   return -1;

#undef _SSOCK_HS_MSGSIZE
#undef _SSOCK_HS_BUFSIZE
#undef _SSOCK_HSOF_SIZEOF
#undef _SSOCK_HSOF_MAGIC
}

/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_flush(SOCK_t *sock)
/****************************************************************************************/
/*
 * Flush all bytes in the stream buffer and reset the buffer pointers/status.
 */
{
   if (sock)
   {
      if (SSOCK_IS_WRITEMODE(sock) && sock->cnt > 0)
      {
         if (RSOCK_IS_PIPE(sock))
         {
            if (IS_VALID_HANDLE(sock->pd_w))
            {
               RSOCK_send(sock,sock->buf,sock->cnt,RSOCK_FLAG_ATOMIC);
            }
         }
         else
         {
            if (IS_VALID_SOCKET(sock->sd))
            {
               RSOCK_nodelay(sock,1);
               RSOCK_send   (sock,sock->buf,sock->cnt,RSOCK_FLAG_ATOMIC);
               RSOCK_nodelay(sock,0);
            }
         }
      }
      sock->ptr = sock->buf;  /* Point to start of buf */
      sock->cnt = 0;          /* Buffer now empty */
   }
}

/****************************************************************************************/

/* Normally not defined, only to avoid compile warnings in special scenarios */
#ifndef SSOCK_NO_CLOSE
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_close(SOCK_t **psock)
/****************************************************************************************/
/*
 * Flush and close stream device.
 */
{
   if (psock)
   {
      SSOCK_flush(*psock);
      RSOCK_close(psock);
   }
}

/****************************************************************************************/
#endif

#if SSOCK_USE_CLIENT
/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *SSOCK_connect
(
   const char *servername,
   int         port,
   long        delay,
   long        timeout,
   SSOCK_HI_t *hi
)
/****************************************************************************************/
/*
 * Connect to a server socket.
 */
{
   SOCK_t *sock = RSOCK_connect(servername,port,delay,timeout);

   if (sock) /* returns NULL if the connection failed */
   {
#if 0
      int n,sz=sizeof(n);
      getsockopt(sock->sd,SOL_SOCKET,SO_RCVBUF,(char *)&n,&sz);
      printf("SSOCK_connect: RCVBUF: %d\n",n);
      getsockopt(sock->sd,SOL_SOCKET,SO_SNDBUF,(char *)&n,&sz);
      printf("SSOCK_connect: SNDBUF: %d\n",n);
#endif

      sock->buf = CAST_CHARP(MALLOC(_SSOCK_BUFSIZE*sizeof(char)));
      SSOCK_DOSWAP(sock) = 0;    /* no little/big endian swapping */
      SSOCK_CLEARBUF(sock,'?')

      /* handshake only if the message is not a NULL pointer */
      if (hi)
      {
         if (RSOCK_IS_SOCK(sock))
            SSOCK_DOREPAIR(sock) = 1; /* allow reconnections */
         if (_ssock_do_handshake(sock,hi))
            RSOCK_close(&sock);
      }
   }

   return sock;
}

/****************************************************************************************/
#endif

#if SSOCK_USE_LISTEN
/****************************************************************************************/
C_FUNC_PREFIX SOCK_t *SSOCK_accept(SOCK_t *listener, SSOCK_HI_t *hi)
/****************************************************************************************/
/*
 * Accept an incoming connection on the listener socket 'listener'
 *    SOCK_t *server = RSOCK_listen(.....);
 *    SOCK_t *sock   = SSOCK_accept(server,&hi);
 */
{
   SOCK_t *sock = RSOCK_accept(listener);

   sock->buf = CAST_CHARP(MALLOC(_SSOCK_BUFSIZE*sizeof(char)));
   SSOCK_DOSWAP(sock) = 0;
   SSOCK_CLEARBUF(sock,'?')

   /* Handshake only if the message is not a NULL pointer */
   if (hi)
   {
      if (RSOCK_IS_SOCK(sock))
         SSOCK_DOREPAIR(sock) = 1; /* Allow reconnections */
      if (_ssock_do_handshake(sock,hi))
         RSOCK_close(&sock);
   }
   return sock;
}

/****************************************************************************************/
#endif

#if SSOCK_USE_SELECT
/****************************************************************************************/
C_FUNC_PREFIX int SSOCK_select(SOCK_t *sockv[], int nsock, unsigned request, long timeout)
/****************************************************************************************/
/*
 * select() wrapper. Returns the index of the first selectable device.
 * Does a select() call only on all client stream after the I/O buffer check failed.
 */
{
   unsigned checkRead, checkWrite, checkExcept;


   /* avoid a bad mask */
   if ((request & (RSOCK_SBIT_READ|RSOCK_SBIT_WRITE|RSOCK_SBIT_EXCEPT)) == 0)
        request = (RSOCK_SBIT_READ|RSOCK_SBIT_WRITE|RSOCK_SBIT_EXCEPT);

   checkRead   = (request & RSOCK_SBIT_READ);
   checkWrite  = (request & RSOCK_SBIT_WRITE);
   checkExcept = (request & RSOCK_SBIT_EXCEPT);

   while(nsock > 0)
   {
      int i;

      /* first repeat check from the last call */
      for (i=0; i<nsock; i++)
      {
         SOCK_t *sock = sockv[i];
         if (checkRead   && SSOCK_IS_READABLE(sock))           return i;
         if (checkWrite  && SSOCK_IS_WRITEABLE(sock))          return i;
         if (checkExcept && (sock->select&RSOCK_SBIT_EXCEPT))  return i;
      }

      /* select-patterns and buffers are empty: now really select() */
      if (RSOCK_select(sockv,nsock,request,timeout) <= 0)
         break;
   }

   return -1;
}

/****************************************************************************************/
#endif

#if SSOCK_USE_RSELECT
/****************************************************************************************/
C_FUNC_PREFIX int SSOCK_rselect(SOCK_t *sockv[], int nsock, long timeout)
/****************************************************************************************/
/*
 * Like SSOCK_select(), but only checks for readable devices.
 */
{
   while(nsock > 0)
   {
      int i;

      /* First repeat check from the last call */
      for (i=0; i<nsock; i++)
      {
         SOCK_t *sock = sockv[i];
         if (SSOCK_IS_READABLE(sock)) return i;
      }

      /* Select-patterns and buffers are empty: now really select() */
      if (RSOCK_rselect(sockv,nsock,timeout) <= 0)
         break;
   }
   return -1;
}

/****************************************************************************************/
#endif


/* Normally not defined, only to avoid compile warnings in special scenarios */
#ifndef SSOCK_NO_PUTRAW
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putraw(SOCK_t *sock, const void *buf, size_t n)
/****************************************************************************************/
/*
 * Put bytes to the stream device.
 * The "buf" argument may be NULL: we then just send 0's to fill up the message to
 * the requested length.
 */
{
   const char *cbuf = CAST_CCHARP(buf);
   size_t      nbyte;


   if (SSOCK_IOMODE(sock) != 'w')
   {
      /* Clear buffer and switch to write mode */
      SSOCK_CLEARBUF(sock,'w')
   }

   for(nbyte=0; n>0; n-=nbyte, cbuf+=nbyte)
   {
      /* Flush stream if we have no more space left */
      if (sock->cnt >= _SSOCK_BUFSIZE)
      {
         nbyte = RSOCK_send(sock,sock->buf,_SSOCK_BUFSIZE,0);
         if (nbyte < _SSOCK_BUFSIZE) /* Something left in the buffer */
         {
            memcpy(sock->buf,sock->buf+nbyte, _SSOCK_BUFSIZE-nbyte);
            sock->ptr -= nbyte;
            sock->cnt -= nbyte;
         }
         else
         {
            sock->ptr = sock->buf;
            sock->cnt = 0;
         }
      }

      /*
       * If the stream buffer is empty we can optimize the send operation:
       * a) avoid memcpy(): send 'cbuf' instead of filling the stream buffer
       * b) send in bunches of size 'N x _SSOCK_BLKSIZE'
       */
      if (!sock->cnt && cbuf && n >= _SSOCK_BLKSIZE)
      {
         nbyte = RSOCK_send(sock,cbuf,_SSOCK_BLKSIZE*(n/_SSOCK_BLKSIZE),0);
      }
      else
      {
         size_t nfree = _SSOCK_BUFSIZE - sock->cnt;
         nbyte = (n > nfree) ? nfree : n;
         if (cbuf) /* Copy the bytes into the stream buffer */
         {
            char *p = sock->ptr;
            switch(nbyte)
            {
               case 8:
                  p[7] = cbuf[7];
                  p[6] = cbuf[6];
                  p[5] = cbuf[5];
                  p[4] = cbuf[4];

               case 4:
                  p[3] = cbuf[3];
                  p[2] = cbuf[2];
                  p[1] = cbuf[1];

               case 1:
                  p[0] = cbuf[0];
                  break;

               default:
                  memcpy(p,cbuf,nbyte);
                  break;
            }
         }
         else /* Just fill up the stream buffer with 0 */
         {
            MEMZERO(sock->ptr,nbyte);
         }

         sock->ptr += nbyte;
         sock->cnt += nbyte;
      }
   }
}

/****************************************************************************************/
#endif

/* Normally not defined, only to avoid compile warnings in special scenarios */
#ifndef SSOCK_NO_GETRAW
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_getraw(SOCK_t *sock, void *buf, size_t n)
/****************************************************************************************/
/*
 * Get bytes from the stream.
 * The "buf" argument may be NULL: we then just read away the bytes
 */
{
   char  *cbuf  = CAST_CHARP(buf);
   size_t nbyte;


   if (SSOCK_IOMODE(sock) != 'r')
   {
      /* Flush write buffer and switch into read mode */
      SSOCK_flush(sock);
      SSOCK_IOMODE(sock) = 'r';
   }

   for(nbyte=0; n>0; n-=nbyte, cbuf+=nbyte)
   {
      if (!sock->cnt)
      {
         /*
          * The stream buffer is empty and we may optimize the receive operation:
          * a) avoid memcpy(): read direct into 'cbuf' if 'buf' is large enough.
          * b) read in bunches of size >= _SSOCK_BLKSIZE'
          */
         if (cbuf && n >= _SSOCK_BLKSIZE)
         {
            nbyte = RSOCK_recv(sock,cbuf,n,0);
            continue;
         }

         /* Read into the buffer and do a memcpy() below */
         sock->ptr = sock->buf;
         sock->cnt = RSOCK_recv(sock,sock->buf,_SSOCK_BUFSIZE,0);
      }

      /*
       * Copy stream buffer contents into 'cbuf'
       */
      nbyte = (n > sock->cnt) ? sock->cnt : n;
      if (buf) /* just read away if buf == NULL and do not really copy */
      {
         char *p = sock->ptr;
         switch(nbyte)
         {
            case 8:
               cbuf[7] = p[7];
               cbuf[6] = p[6];
               cbuf[5] = p[5];
               cbuf[4] = p[4];

            case 4:
               cbuf[3] = p[3];
               cbuf[2] = p[2];
               cbuf[1] = p[1];

            case 1:
               cbuf[0] = p[0];
               break;

            default:
               memcpy(cbuf,p,nbyte);
               break;
         }
      }

      sock->ptr += nbyte;
      sock->cnt -= nbyte;
   }
}

/****************************************************************************************/
#endif

#if SSOCK_USE_RCLEAR
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_rclear(SOCK_t *sock, const long timeout)
/****************************************************************************************/
/*
 * Clear the read buffer and the TCP stack for the incoming streams.
 */
{
   SOCKET sd = sock->sd;


   SSOCK_CLEARBUF(sock,'?')
   RSOCK_CLEAROOB(sock);

   RSOCK_timeout(sock,timeout);

   for(;;)
   {
      if (RSOCK_IS_PIPE(sock))
      {
      }
      else
      {
      #if RSOCK_USE_GETOOB && !IS_MSWIN
         /* First try to read any OOB message */
         char oob;
         if (recv(sd,&oob,1,MSG_OOB) == 1)
            sock->oobyte = CAST_INT(oob);
      #endif

         if (recv(sd,sock->buf,_SSOCK_BUFSIZE,0) <= 0)
            break; /* Connection down or buffers empty */
      }
   }

   RSOCK_timeout(sock,0);
}

/****************************************************************************************/
#endif

#if SSOCK_USE_PUTSYNC
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putsync(SOCK_t *sock, const char *csync, const size_t slen)
/****************************************************************************************/
/*
 * Send a stream of '#' chars plus a few sync-bytes[slen] at the tail.
 *
 * slen: should be > 1, and less than _SSOCK_BLKSIZE/4
 */
{
   RSOCK_CLEAROOB(sock);
   SSOCK_CLEARBUF(sock,'w')
   memset(sock->buf,'#',_SSOCK_BLKSIZE);
   memcpy(sock->buf+_SSOCK_BLKSIZE-slen,csync,slen);
   sock->cnt = _SSOCK_BLKSIZE;
   SSOCK_flush(sock);
}

/****************************************************************************************/
#endif

#if SSOCK_USE_GETSYNC
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_getsync(SOCK_t *sock, const char *csync, const size_t slen)
/****************************************************************************************/
/*
 * Read the stream until we find a series of '#' chars
 * plus a few sync-bytes csync[slen] at the tail.
 *
 * slen: should be > 1, and less than _SSOCK_BLKSIZE/4
 */
{
   unsigned i;
   char     hashes[_SSOCK_BLKSIZE/3]; /* The hash block has a size of _SSOCK_BLKSIZE/3 chars */


   RSOCK_CLEAROOB(sock);
   SSOCK_CLEARBUF(sock,'?')

GET_AGAIN:

   /* Get 1/3 of the _SSOCK_BLKSIZE. Must be filled 100% with '#' */
   SSOCK_getraw(sock,hashes,_SSOCK_BLKSIZE/3);
   for(i=0; i<(_SSOCK_BLKSIZE/3); i++)
   {
      if (hashes[i] != '#')
         goto GET_AGAIN; /* Try to get the next block */
   }

   /* Get '#' chars until we reach the end of the hash block */
   do
   {
      SSOCK_getraw(sock,hashes,1);
   } while(hashes[0] == '#');

   if (hashes[0] == csync[0]) /* Match with the 1st sync byte */
   {
      if (slen > 1)
         SSOCK_getraw(sock,hashes+1,slen-1); /* Get the rest of the sync bytes */

      if (!memcmp(hashes,csync,slen))
         return; /* Match with the end of the sync stream */
   }

   XMSG_FATAL1
   (
      "SSOCK_getsync(\"%s\"): Gave up after non matching sync sequence.\n"
      ,csync
   );
}

/****************************************************************************************/
#endif

#if SSOCK_USE_SWAP

#if INCLUDE_STATIC
   #include "memswapb.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_getany(SOCK_t *sock, void *buf, size_t n, size_t size)
/****************************************************************************************/
/*
 * Get 'n' objects each of 'size' bytes from the stream.
 * The "buf" argument may be NULL: we then just read away the bytes
 */
{
   SSOCK_getraw(sock,buf,n*size);
   if (SSOCK_DOSWAP(sock) && buf) memswapb(buf,n,size);
}

/****************************************************************************************/
#endif

#if SSOCK_USE_PUTSIZ
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putsiz(SOCK_t *sock, size_t size)
/****************************************************************************************/
/*
 * Send a size_t as an unsigned int.
 * On a 64 bit system size_t may be 64 bit, but unsigned still a 32 bit int.
 */
{
   const unsigned uval = CAST_UINT(size);
   if (size > UINT_MAX)
      XMSG_FATAL1("SSOCK_putsiz(): The size value is > %u.\n",UINT_MAX);
   SSOCK_putint(sock,uval);
}

/****************************************************************************************/
#endif

#if SSOCK_USE_GETSIZ
/****************************************************************************************/
C_FUNC_PREFIX size_t SSOCK_getsiz(SOCK_t *sock)
/****************************************************************************************/
{
   unsigned uval;
   SSOCK_getint(sock,&ival);
   return CAST_SIZE(uval);
}
/****************************************************************************************/
#endif

#if SSOCK_USE_PUTCHR
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putchr(SOCK_t *sock, int chr)
/****************************************************************************************/
{
   char c = CAST_CHAR(chr);
   SSOCK_putraw(sock,&c,1);
}
/****************************************************************************************/
#endif

#if SSOCK_USE_GETCHR
/****************************************************************************************/
C_FUNC_PREFIX int SSOCK_getchr(SOCK_t *sock)
/****************************************************************************************/
{
   unsigned char c;
   SSOCK_getraw(sock,&c,1);
   return CAST_INT(c) & 0xff;
}
/****************************************************************************************/
#endif

#if SSOCK_USE_PUTARR
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putarr
(
   SOCK_t     *sock,
   const void *buf,
   size_t      numitem,
   size_t      size
)
/****************************************************************************************/
{
   const unsigned n = (buf) ? CAST_UINT(numitem) : 0;
   SSOCK_putint(sock,n);
   if (n) SSOCK_putraw(sock,buf,numitem*size);
}
/****************************************************************************************/
#endif

#if SSOCK_USE_GETARR
/****************************************************************************************/
C_FUNC_PREFIX size_t SSOCK_getarr
(
   SOCK_t *sock,
   void   *buf,
   size_t  maxitem,
   size_t  size
)
/****************************************************************************************/
{
   size_t   numitem;
   unsigned n;


   SSOCK_getint(sock,&n);
   if (!n) return 0;

   numitem = n;
   if (!buf) /* allow a NULL pointer to read away the data */
   {
      maxitem = 0;
   }
   else
   {
      if (maxitem > numitem)
          maxitem = numitem;
      SSOCK_getraw(sock,buf,maxitem*size);

      /* do a swap for short .. double */
      if (SSOCK_DOSWAP(sock) && size>1 && size<=sizeof(double))
         memswapb(buf,maxitem,size);
   }

   if (maxitem < numitem) /* read away the rest in the stream */
      SSOCK_getraw(sock,NULL,size*(numitem-maxitem));

   return maxitem;
}
/****************************************************************************************/
#endif

#if SSOCK_USE_PUTSTR
/****************************************************************************************/
C_FUNC_PREFIX void SSOCK_putstr(SOCK_t *sock, const char *str)
/****************************************************************************************/
/*
 * send a string: len + string + '\0', with len>0 - includes the trailing 0
 */
{
   int len = ISTRLENP(str) + 1;
   SSOCK_putint(sock,len);
   SSOCK_putraw(sock,str,len);
}
/****************************************************************************************/
#endif

#if SSOCK_USE_GETSTR
/****************************************************************************************/
C_FUNC_PREFIX char *SSOCK_getstr(SOCK_t *sock, char *str, size_t maxSize)
/****************************************************************************************/
/*
 * recv string: len + string + '\0'
 */
{
   int len;

   SSOCK_getint(sock,&len);
   if (len <= 0)
      XMSG_FATAL1("SSOCK_getstr(): Received a string length %d <= 0.\n",len);

   if (!str || maxSize >= CAST_SIZE(len))
   {
      SSOCK_getraw(sock,str,len);   /* str==NULL or size large enought */
   }
   else /* our buffer is too small */
   {
      SSOCK_getraw(sock,str,maxSize);        /* read maxSize bytes */
      SSOCK_getraw(sock,NULL,(len-maxSize)); /* read away the chars left */
      str[maxSize-1] = '\0';
   }

   return str;
}
/****************************************************************************************/
#endif

#if 0

typedef struct _sock_rpcarg_struct
{
   const char  *type;
   union
   {
      void          *addr;
      int            ival;
      long           lval;
#if HAVE_LONGLONG
      long long      Lval;
#endif
      unsigned int   uval;
      unsigned long  Uval;
      float          fval;
      real           rval;
      double         dval;
      long double    qval;
      char           cval;
      unsigned char  bval;
      size_t         zval;
      char           xval[16];
   } v;
   size_t       size;
   int          nitems;
   int          send;
} _SSOCK_RPCARG_t;


/****************************************************************************************/
static int _ssock_make_argv( const char *module,
                             const char *fmt,
                             _SSOCK_RPCARG_t  *argv,
                             va_list     ap)
/****************************************************************************************/
{
   const char *f;
   _SSOCK_RPCARG_t  *argp = argv;

   int sendMode    =  1;
   int itemCounter = -1;

#if 1
printf("FORMAT=%s\n",fmt);
#endif

   for (f=fmt; *f; f++)
   {
      int    useVal = (itemCounter < 0 && sendMode);
      int    nItems = itemCounter;

      itemCounter = -1;
#if 1
printf("   FMT=%c\n",*f);
#endif

/*
 * some of the ... arguments are promoted to highter precision
 */
#define  _SET_ARG(_val,_target_type,_promoted_type) \
            argp->type = #_target_type;\
            argp->size = sizeof(_target_type);\
            if (useVal) argp->v._val = (_target_type)va_arg(ap,_promoted_type);\
            break

      switch(*f)
      {
         case 'i': _SET_ARG(ival, int           , int );
         case 'l': _SET_ARG(lval, long          , long);
#if HAVE_LONGLONG
         case 'L': _SET_ARG(Lval, long long     , long long);
#endif
         case 'u': _SET_ARG(uval, unsigned int  , unsigned int);
         case 'U': _SET_ARG(Uval, unsigned long , unsigned long);
         case 'f': _SET_ARG(fval, float         , double);
         case 'r': _SET_ARG(rval, real          , double);
         case 'd': _SET_ARG(dval, double        , double);
         case 'q': _SET_ARG(qvar, long double   , long double );
         case 'c': _SET_ARG(cval, char          , int);
         case 'b': _SET_ARG(bval, unsigned char , int);
         case 'z': _SET_ARG(zval, size_t        , size_t);

         case 'T':
            if (sendMode && nItems >= 0) goto FORMAT_ERROR; /* cannot send '*t' */
            argp->type = "text";
            argp->size = sizeof(char);
            argp->v.addr = va_arg(ap,char *);
            if (sendMode) nItems = ISTRLEN(argp->v.addr) + 1;
            useVal = 1; /* already assigned the address */
            break;

         case '*':
            if (nItems >= 0) goto FORMAT_ERROR; /* got a '**' */
            itemCounter = CAST_INT(va_arg(ap,size_t));
            if (itemCounter < 0)
               XMSG_FATAL2("%sItem counter %u overflow.\n",module,CAST_SIZE(itemCounter));
            continue;

         case ':': /* switch to receive mode */
            if (!sendMode || nItems >= 0) goto FORMAT_ERROR; /* got '::' or '*:' */
            sendMode = 0;
            continue;

         case '?':
            if (sendMode || nItems >= 0) goto FORMAT_ERROR;
            _SET_ARG(zval, size_t, size_t);

         default:
            goto FORMAT_ERROR;
      }

#undef _SET_ARG

      if (!useVal)
      {
         void *p = va_arg(ap,char *);
         if (!p && nItems > 0)
            XMSG_WARNING3
            (
               "%sGot a NULL pointer for a \"%s\" array with %d items.\n",
               module,argp->type,nItems
            );

         argp->v.addr = p;
      }
      argp->nitems = nItems;
      argp->send   = sendMode;
#if 1
printf("      SIZ(%u), ARG(%u), NIT(%d), ITC(%d) TYP(%s)\n",
         CAST_UINT((nItems<0) ? argp->size : argp->nitems * argp->size),
         CAST_UINT(argp->size),
         argp->nitems,
         itemCounter,
         argp->type
         );
         fflush(stdout);
#endif
      argp++;
   }
   va_end(ap);


   if (itemCounter >= 0)
      XMSG_FATAL1("%sMissing array argument type specifier following \'*\'.\n", module);

   argp->type = NULL;

   return CAST_INT(argp-argv);


FORMAT_ERROR:
   XMSG_FATAL2("%sInvalid argument type specifier \'%c\'.\n", module, *f);
   return -1; /* keep compiler happy */
}

#if INCLUDE_STATIC
   #include "strtograph.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX int SSOCK_rpcall(SOCK_t *sock, int funcID, const char *format, ...)
/****************************************************************************************/
/*
 * call a procedure on the remote side passing all IN/OUT arguments
 *
 * SSOCK_rpcall(sock,28," iiii *r t  : *d< ",
 *              IN   int      int1,
 *              IN   int      int2,
 *              IN   int      int3,
 *              IN   int      int4,
 *              IN   size_t   nreal,
 *              IN   real     rarr[],
 *              IN   char *   "hallo",
 *              IN   size_t   ndmax,
 *              OUT  double   darr[],
 *              OUT  size_t * ndoublereceived);
 */
{

   #define LOOP_ARGV for (argp=argv; argp->type; argp++)

   char    module[256];
   char    packedFormat[128];
   size_t  messageMaxSize;
   int     errorCode = 0;
   va_list ap;

   _SSOCK_RPCARG_t argv[128], *argp;

   strncpy(packedFormat,format,sizeof(packedFormat));
   packedFormat[sizeof(packedFormat)-1] = '\0';
   strtograph(packedFormat,format));
   sprintf(module,"SSOCK_RPCALL(%d,\"%s\",...): ",funcID, packedFormat);

   /*
    * find out the no. of bytes we need to send as function call arguments
    */
   va_start(ap,format);
   _ssock_make_argv(module,packedFormat,argv,ap);
   va_end(ap);

   messageMaxSize = 0;
   LOOP_ARGV
   {
      if (argp->send)
      {
         if (argp->nitems < 0) /* send a value */
            messageMaxSize += argp->size;
         else                    /* send nitems of an array */
            messageMaxSize += sizeof(int) + argp->nitems*argp->size;
      }
      else if (argp->nitems >= 0)
         messageMaxSize += sizeof(int); /* send max. items of array received */

#if 1
      printf("   ARG(%5u): size(%u), type(%s-%s), items(%d) send(%d)\n",
         CAST_UINT(messageMaxSize),
         CAST_UINT(argp->size),
         (argp->nitems<0&&argp->send)?"VAL":"PTR",
         argp->type,
         argp->nitems,
         argp->send);

      if (argp->send && !strcmp(argp->type,"text"))
         puts(argp->v.addr);
#endif
   }


printf("MSG=<%u>\n",CAST_UINT(messageMaxSize));

   /*
    * start again: now really sending the arguments, no further checks
    *
    * send function id, format string and size of all arguments
    */
#if 1
   SSOCK_putint(sock,funcID);
   SSOCK_putstr(sock,packedFormat);
   SSOCK_putsiz(sock,messageMaxSize);
#else
   printf("SEND-funcid=<%d>\n",funcID);
   printf("SEND-format=<%s>\n",packedFormat);
   printf("SEND-msgsiz=<%u>\n",CAST_UINT(messageMaxSize));
#endif

   /*
    * send all currently known arguments input in the order they appaer in the arg list
    */
   LOOP_ARGV
   {
      if (argp->send)
      {
         if (argp->nitems < 0) /* send a value */
         {
#if 0
            SSOCK_putraw(sock,argp->xval,argp->size);
#else
            printf("SEND-putval=<%s> <size=%u>\n",argp->type,(unsigned)(argp->size));
#endif
         }
         else                  /* send nitems of an array */
         {
#if 0
            SSOCK_putarr(sock,argp->v.addr,argp->nitems,argp->size);
#else
            printf("SEND-putarr=<%s> <size=%u> <nitem=%d>\n",argp->type, (unsigned)(argp->size), argp->nitems);
#endif
         }
      }
      else if (argp->nitems >= 0)
      {
#if 0
         SSOCK_putsiz(sock,argp->nitems); /* send max. items of array received */
#else
         printf("SEND-putsiz=<%s> <val=%d>\n",argp->type, argp->nitems);
#endif
      }
   }

   /*
    * get an error return code
    */
#if 0
   SSOCK_getint(sock,&errorCode);
#else
   printf("RECV-geterr=<%d>\n",errorCode);
#endif

   if (errorCode)
      return errorCode;

   LOOP_ARGV
   {
      if (argp->send) continue;
      if (argp->nitems < 0)
      {
#if 0
         SSOCK_getany(sock,argp->v.addr,1,argp->size);
#else
         printf("RECV-getval=<%s> <size=%u>\n",argp->type,(unsigned)(argp->size));
#endif
      }
      else
      {
#if 0
         SSOCK_getarr(sock,argp->v.addr,argp->nitems,argp->size);
#else
         printf("RECV-getarr=<%s> <size=%u> <nitem=%d>\n",argp->type,(unsigned)(argp->size),argp->nitems);
#endif
      }
   }

   return 0;
}

#endif

/****************************************************************************************/

#endif
