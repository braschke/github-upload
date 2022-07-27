#ifndef xmsg_SOURCE_INCLUDED
#define xmsg_SOURCE_INCLUDED
/* xmsg.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    printf wrapper
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: xmsg.c 2391 2014-01-07 16:59:16Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if 0
   #include "xmsg.h"
#endif

/* need stdarg.h for the va_list */
#include <stdarg.h>
#include <time.h>

#ifndef XMSG_USE_PERCENT
   #define XMSG_USE_PERCENT   0   /* 1: keep the % within the format string */
#endif

#if !XMSG_USE_PTR && !XMSG_USE_CIO
   #error XMSG: Neither C/IO allowed nor functions set defined
#endif

#if XMSG_USE_CIO
   #include <stdio.h>
#endif

#if INCLUDE_STATIC && XMSG_USE_TIME
   #include "getcpuseconds.c"
#endif

typedef struct _XMSGDATA
{
   #if XMSG_USE_PTR
      void (*fullMessage)(const char *str, int len);
      void (*fullWarning)(const char *str, int len);
      void (*fullFatal  )(const char *str, int len);
      void (*lineMessage)(const char *str, int len);
      void (*lineWarning)(const char *str, int len);
      void (*lineFatal  )(const char *str, int len);
   #endif

   #if XMSG_USE_ATEXIT
      void (*exitHandler)(void);
   #endif

   #if XMSG_USE_PREFIX
      size_t   prefixLen;        /* prefix string length: size may be arbitrary */
   #endif

      size_t   iclineLen;        /* length of the last incomplete line */
      int      locked;           /* printing fatal messages sets a lock */
      int      level;            /* 0...3 */
      char     iclineStr[512];   /* may contain the last incomplete line */

   #if XMSG_USE_PREFIX
      char  prefixStr[64];       /* prefix string: size may be arbitrary */
   #endif

} XMSGDATA;

static XMSGDATA _xmsg_data =
{
   #if XMSG_USE_PTR
       NULL,NULL,NULL, NULL,NULL,NULL,
   #endif

   #if XMSG_USE_ATEXIT
       NULL,
   #endif

   #if XMSG_USE_PREFIX
       0,    /* default message prefix len == empty */
   #endif

       0
      ,0    /* we are not yet locked */
      ,1    /* default print level */
      ,"\0" /* last line buffer == empty */

   #if XMSG_USE_PREFIX
     ,"\0" /* default message prefix == empty */
   #endif
};


#define PREFIX_LEN      _xmsg_data.prefixLen
#define PREFIX_STR      _xmsg_data.prefixStr
#define ICLINE_LEN      _xmsg_data.iclineLen
#define ICLINE_STR      _xmsg_data.iclineStr
#define XMSG_LOCKED     _xmsg_data.locked

#if XMSG_USE_PREFIX

/****************************************************************************************/
static int _xmsg_patch_prefix(char *msg, size_t size, size_t len)
/****************************************************************************************/
/*
 * in msg[size] replace all '\n' by '\n' + prefix
 */
{
   size_t i;
   size_t maxLen = size - PREFIX_LEN;


   if (len >= maxLen) return CAST_INT(len);

   if (!ICLINE_LEN)
   {
      memmove(msg+PREFIX_LEN,msg,len+1);
      memcpy (msg,PREFIX_STR,PREFIX_LEN);
      len += PREFIX_LEN;
   }

   for (i=0; i<len; i++)
   {
      if (msg[i] == '\n' && msg[i+1])
      {
         if (len < maxLen)
         {
            char *src = msg + i + 1;
            memmove(src+PREFIX_LEN,src,len-i+1);
            memcpy (src,PREFIX_STR,PREFIX_LEN);
            len += PREFIX_LEN;
            i   += PREFIX_LEN;
         }
         else
            break;
      }
   }
   return CAST_INT(len);
}

/****************************************************************************************/

#endif

#if XMSG_USE_ATEXIT

/****************************************************************************************/
C_FUNC_PREFIX void XMSG_atexit(void (*atExitHandler)(void))
/****************************************************************************************/
{
   _xmsg_data.exitHandler = atExitHandler;
}

/****************************************************************************************/

#endif

#if XMSG_USE_PTR

/****************************************************************************************/
static void _xmsg_lprint(void (*linePrinter)(const char *str, int len), char *msg)
/****************************************************************************************/
/*
 * split the message into separate lines without a trailing newline (remove the
 * newline) and print each line separately via the linePrintFunction() - which may
 * be a FORTRAN subroutine adding newlines automatically.
 *
 * print only FULL lines == lines terminated by a '\n'.
 * Do not call the line print function without unterminated lines!
 */
{
   char *head, *end, *next;


   for (head=end=next=msg; *head; head=next)
   {
      int len;

      STRJUMPCHARCP(head,end,'\n'); /* find the next '\n' or '\0' */

      if (*end)
      {
         next = end + 1;
         *end = '\0';   /* terminate the current string */
      }
      else  /* line without trailing \n - which should not happen */
      {
         next = end;
      }

      len = CAST_INT(end-head);        /* strlen() without the \n */
      if (len) linePrinter(head,len);  /* print the line */
      else     linePrinter(" " , 1 );  /* fortran does not like CHARACTER*(0) */
   }
}

/****************************************************************************************/
C_FUNC_PREFIX void XMSG_setfunct
 (
   void (*printFullMessage)(const char *str, int len),
   void (*printFullWarning)(const char *str, int len),
   void (*printFullFatal  )(const char *str, int len),
   void (*printLineMessage)(const char *str, int len),
   void (*printLineWarning)(const char *str, int len),
   void (*printLineFatal  )(const char *str, int len)
 )
/****************************************************************************************/
{
   _xmsg_data.fullMessage = printFullMessage;
   _xmsg_data.fullWarning = printFullWarning;
   _xmsg_data.fullFatal   = printFullFatal;

   _xmsg_data.lineMessage = printLineMessage;
   _xmsg_data.lineWarning = printLineWarning;
   _xmsg_data.lineFatal   = printLineFatal;
}

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_vmessage(int level, const char *fmt, va_list ap)
/****************************************************************************************/
{
   char    *mPtr;
   size_t   mSiz;
   int      len;
   char     msg[32767];


   if (level > _xmsg_data.level || XMSG_LOCKED || !STRHASLEN(fmt))
      return 0;


   if (ICLINE_LEN) /* first fill buffer with the last incomplete line */
   {
      mPtr = (char *)memcpy(msg,ICLINE_STR,ICLINE_LEN) + ICLINE_LEN;
      mSiz = sizeof(msg) - ICLINE_LEN;
   }
   else
   {
      mPtr = msg;
      mSiz = sizeof(msg);
   }

   vsnprintf(mPtr,mSiz,fmt,ap);

   /* get strlen(mPtr) && remove format keys % from message */
   for (len=0; mPtr[len]; len++)
#if XMSG_USE_PERCENT
      ;
#else
      if (mPtr[len] == '%') mPtr[len] = '?';
#endif

#if XMSG_USE_PREFIX
   if (PREFIX_LEN)
      len = _xmsg_patch_prefix(mPtr,mSiz,CAST_SIZE(len));
#endif
   len += CAST_INT(ICLINE_LEN);
   ICLINE_LEN = 0;

   if (msg[len-1] != '\n') /* line is not '\n' terminated */
   {
      int i;  /* do an strrchr(msg,'\n') */
      for(i=len-2; i>=0; i--) if (msg[i] == '\n') break;

      /* remember this line and output the rest */
      ICLINE_LEN = len - i - 1;
      len = i + 1;
      memcpy(ICLINE_STR,msg+len,ICLINE_LEN);
      msg[len] = '\0';  /* terminate msg behind the last '\n' */
      if (!len) /* we have no leading text any longer */
         return 0;
   }

   if (level > 0)
   {
      /* print message on stdout and return */
           if (_xmsg_data.fullMessage) _xmsg_data.fullMessage(msg,len);
      else if (_xmsg_data.lineMessage) _xmsg_lprint(_xmsg_data.lineMessage,msg);
      #if XMSG_USE_CIO
      else { fputs(msg,stdout); fflush(stdout); }
      #endif
      return len;
   }

   if (level == 0)
   {
      #if XMSG_USE_CIO
      fflush(stdout);
      #endif

      /* print warning on stderr and return */
           if (_xmsg_data.fullWarning) _xmsg_data.fullWarning(msg,len);
      else if (_xmsg_data.lineWarning) _xmsg_lprint(_xmsg_data.lineWarning,msg);
      #if XMSG_USE_CIO
      else { fputs(msg,stderr); fflush(stderr); }
      #endif
      return len;
   }

   /* level < 0) == no recovery */
   {
   #if XMSG_USE_TIME
      time_t realtm = time(NULL);
      snprintf
      (
         msg+len,
         sizeof(msg)-CAST_SIZE(len),
         "\nExiting due to fatal error on %s"
         "%g seconds CPU time used.\n",
         ctime(&realtm),
         getcpuseconds()
      );
      len = ISTRLEN(msg);
   #endif

      /* lock any calls from now on, avoid recursions within exit handlers */
      XMSG_LOCKED = 1;

      #if XMSG_USE_CIO
      fflush(stdout);
      #endif

      /* print fatal on stderr and exit */
           if (_xmsg_data.fullFatal) _xmsg_data.fullFatal(msg,len);
      else if (_xmsg_data.lineFatal) _xmsg_lprint(_xmsg_data.lineFatal,msg);
      #if XMSG_USE_CIO
      else { fputs(msg,stderr); fflush(stderr); }
      #endif
   }

#if XMSG_USE_ATEXIT
   if (_xmsg_data.exitHandler)
       _xmsg_data.exitHandler();
#endif

   /*
    * level is always < 0
    * avoid compiler complaints about: " exit(EXIT_FAILURE); return len; "
    */
   if (level < 0) exit(EXIT_FAILURE); /* always exit 1, never -1! */
   return len;
}

/****************************************************************************************/

#elif XMSG_USE_PREFIX

 /* XMSG_USE_PTR == 0 : use standard CIO streams */

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_vmessage(int level, const char *fmt, va_list ap)
/****************************************************************************************/
{
   char  msg[32767];
   int   len;

   if (level > _xmsg_data.level || XMSG_LOCKED || !STRHASLEN(fmt))
      return 0;


   vsnprintf(msg,sizeof(msg),fmt,ap);

   /* get strlen(msg) && remove format keys % from message */
   for (len=0; msg[len]; len++)
#if XMSG_USE_PERCENT
      ;
#else
      if (msg[len] == '%') msg[len] = '?';
#endif

   if (PREFIX_LEN)
      len = _xmsg_patch_prefix(msg,sizeof(msg),CAST_SIZE(len));
   ICLINE_LEN = 0;

   if (level > 0)
   {
      fputs(msg,stdout);
      fflush(stdout);
   }
   else
   {
      fflush(stdout);
      fputs(msg,stderr);
   }

   if (level < 0)
   {
#if XMSG_USE_TIME
      time_t realtm = time(NULL);
      fprintf
      (
         stderr,
         "\nExiting due to fatal error on %s"
         "%g seconds CPU time used.\n",
         ctime(&realtm),
         getcpuseconds()
      );
      fflush(stderr);
#endif

      /* lock any calls from now on, avoid recursions within exit handlers */
      XMSG_LOCKED = 1;

#if XMSG_USE_ATEXIT
      if (_xmsg_data.exitHandler)
          _xmsg_data.exitHandler();
#endif

      exit(EXIT_FAILURE); /* always exit 1, never -1! */
   }

   if (msg[len-1] != '\n') ICLINE_LEN = 1; /* set a mark */
   return len;
}

/****************************************************************************************/

#else

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_vmessage(int level, const char *fmt, va_list ap)
/****************************************************************************************/
{
   FILE *iostream;
   int   len;

   if (level > _xmsg_data.level || XMSG_LOCKED || !STRHASLEN(fmt))
      return 0;

   if (level > 0) { iostream = stdout; }
   else           { iostream = stderr; fflush(stdout); }

   len = (strchr(fmt,'%'))
      ? vfprintf(iostream,fmt,ap)   /* got a format string */
      : fputs(fmt,iostream);        /* no format: just strcpy() */
   fflush(iostream);

   if (level < 0)
   {
#if XMSG_USE_TIME
      time_t realtm = time(NULL);
      fprintf
      (
         stderr,
         "\nExiting due to fatal error on %s"
         "%g seconds CPU time used.\n",
         ctime(&realtm),
         getcpuseconds()
      );
      fflush(stderr);
#endif

      /* lock any calls from now on, avoid recursions within exit handlers */
      XMSG_LOCKED = 1;

#if XMSG_USE_ATEXIT
      if (_xmsg_data.exitHandler)
          _xmsg_data.exitHandler();
#endif

      exit(EXIT_FAILURE); /* always exit 1, never -1! */
   }
   return len;
}

/****************************************************************************************/

#endif

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_message(int level, const char *fmt, ...)
/****************************************************************************************/
{
   va_list ap;
   int     len;

   if (level > _xmsg_data.level)
      return 0;

   va_start(ap,fmt);
   len = XMSG_vmessage(level,fmt,ap);
   va_end(ap);
   return len;
}

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_getlevel(void)
/****************************************************************************************/
{
   return _xmsg_data.level;
}

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_setlevel(int level)
/****************************************************************************************/
/*
 * set new message level. return old level so that the caller may reconstruct old level
 * after a temporary change.
 */
{
   /*
    * do NOT ...
    *
    *  int old = _xmsg_data.level;
    *
    * call the function XMSG_getlevel() to avoid compiler/linker complaints about unused
    * XMSG_getlevel() in case of static including this file.
    */
   int old = XMSG_getlevel();
   _xmsg_data.level = (level < 0) ? 0 : level;
   return old;
}

/****************************************************************************************/

#if XMSG_USE_PREFIX

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_vsetprefix(const char *fmt, va_list ap)
/****************************************************************************************/
/*
 * define or delete the message prefix.
 */
{
   char *cp = PREFIX_STR;
   int   len;


   cp[0]      = '\0'; /* empty the current prefix string */
   PREFIX_LEN = 0;
   if (!STRHASLEN(fmt)) /* remove the prefix */
      return 0;

   /* set a new prefix */
   vsnprintf(cp,sizeof(PREFIX_STR),fmt,ap);
   cp[sizeof(PREFIX_STR)-1] = '\0';

   /* get strlen(PREFIX_STR) && remove format keys % from header */
   for(len=0; cp[len]; len++)
      if (cp[len] == '%')
          cp[len] = '?';

   PREFIX_LEN = (size_t)len;
   return len;
}

/****************************************************************************************/
C_FUNC_PREFIX int XMSG_setprefix(const char *fmt,...)
/****************************************************************************************/
/*
 * define or delete the message prefix.
 */
{
   va_list ap;
   int     len;

   va_start(ap,fmt);
   len = XMSG_vsetprefix(fmt,ap);
   va_end(ap);
   return len;
}

/****************************************************************************************/

#endif

#undef PREFIX_LEN
#undef PREFIX_STR
#undef ICLINE_LEN
#undef ICLINE_STR
#undef XMSG_LOCKED

#endif
