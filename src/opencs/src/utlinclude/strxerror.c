#pragma once
#ifndef strxerror_SOURCE_INCLUDED
#define strxerror_SOURCE_INCLUDED
/* strxerror.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    strxerror(errnum) is a wrapper to make the MSWin FormatMessage() and the UNIX
 *    strerror() compatible for multi platform code. It returns error message string for a
 *    given errnum or UNIX errno or the MSWin GetLastError().
 *    This routine always returns a filled valid string pointer regardless of the
 *    error number. Like strerror() the string has no trailing '\n' newline or '.'.
 *
 *    UNIX:
 *       errnum >  0: strerror(errnum)
 *       errnum <= 0: strerror(errno)
 *
 *    MSWin:
 *       errnum >  0: _tcserror(errnum)
 *       errnum == 0: FormatMessage(....from GetLastError()....)
 *       errnum <  0: _tcserror( map GetLastError() to errno )
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: strxerror.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


#if IS_MSWIN

/*
 * Code snippet below was stolen from VC CTR-lib stuff and modified,
 * see
 *
 *    int __cdecl _get_errno_from_oserr(unsigned long oserrno)
 *
 * in source VC ../crt/src/dosmap.c, syserr.c and more
 */

/****************************************************************************************/
static int _map_winerr_to_errno(DWORD winerr)
/****************************************************************************************/
{
   static struct error_mapping
   {
        DWORD windocode;   /* OS return value */
        int   errnocode;   /* System V error code */
   } errtable[] =
   {
        {  ERROR_INVALID_FUNCTION,       EINVAL       },  /* 1 */
        {  ERROR_FILE_NOT_FOUND,         ENOENT       },  /* 2 */
        {  ERROR_PATH_NOT_FOUND,         ENOENT       },  /* 3 */
        {  ERROR_TOO_MANY_OPEN_FILES,    EMFILE       },  /* 4 */
        {  ERROR_ACCESS_DENIED,          EACCES       },  /* 5 */
        {  ERROR_INVALID_HANDLE,         EBADF        },  /* 6 */
        {  ERROR_ARENA_TRASHED,          ENOMEM       },  /* 7 */
        {  ERROR_NOT_ENOUGH_MEMORY,      ENOMEM       },  /* 8 */
        {  ERROR_INVALID_BLOCK,          ENOMEM       },  /* 9 */
        {  ERROR_BAD_ENVIRONMENT,        E2BIG        },  /* 10 */
        {  ERROR_BAD_FORMAT,             ENOEXEC      },  /* 11 */
        {  ERROR_INVALID_ACCESS,         EINVAL       },  /* 12 */
        {  ERROR_INVALID_DATA,           EINVAL       },  /* 13 */
        {  ERROR_OUTOFMEMORY,            ENOMEM       },  /* 14 */
        {  ERROR_INVALID_DRIVE,          ENOENT       },  /* 15 */
        {  ERROR_CURRENT_DIRECTORY,      EACCES       },  /* 16 */
        {  ERROR_NOT_SAME_DEVICE,        EXDEV        },  /* 17 */
        {  ERROR_NO_MORE_FILES,          ENOENT       },  /* 18 */
        {  ERROR_LOCK_VIOLATION,         EACCES       },  /* 33 */
        {  ERROR_BAD_NETPATH,            ENOENT       },  /* 53 */
        {  ERROR_NETWORK_ACCESS_DENIED,  EACCES       },  /* 65 */
        {  ERROR_BAD_NET_NAME,           ENOENT       },  /* 67 */
        {  ERROR_FILE_EXISTS,            EEXIST       },  /* 80 */
        {  ERROR_CANNOT_MAKE,            EACCES       },  /* 82 */
        {  ERROR_FAIL_I24,               EACCES       },  /* 83 */
        {  ERROR_INVALID_PARAMETER,      EINVAL       },  /* 87 */
        {  ERROR_NO_PROC_SLOTS,          EAGAIN       },  /* 89 */
        {  ERROR_DRIVE_LOCKED,           EACCES       },  /* 108 */
        {  ERROR_BROKEN_PIPE,            EPIPE        },  /* 109 */
        {  ERROR_DISK_FULL,              ENOSPC       },  /* 112 */
        {  ERROR_INVALID_TARGET_HANDLE,  EBADF        },  /* 114 */
        {  ERROR_INSUFFICIENT_BUFFER,    ENOMEM       },  /* 122 */
        {  ERROR_INVALID_HANDLE,         EINVAL       },  /* 124 */
        {  ERROR_WAIT_NO_CHILDREN,       ECHILD       },  /* 128 */
        {  ERROR_CHILD_NOT_COMPLETE,     ECHILD       },  /* 129 */
        {  ERROR_DIRECT_ACCESS_HANDLE,   EBADF        },  /* 130 */
        {  ERROR_NEGATIVE_SEEK,          EINVAL       },  /* 131 */
        {  ERROR_SEEK_ON_DEVICE,         EACCES       },  /* 132 */
        {  ERROR_DIR_NOT_EMPTY,          ENOTEMPTY    },  /* 145 */
        {  ERROR_NOT_LOCKED,             EACCES       },  /* 158 */
        {  ERROR_BAD_PATHNAME,           ENOENT       },  /* 161 */
        {  ERROR_MAX_THRDS_REACHED,      EAGAIN       },  /* 164 */
        {  ERROR_LOCK_FAILED,            EACCES       },  /* 167 */
        {  ERROR_ALREADY_EXISTS,         EEXIST       },  /* 183 */
        {  ERROR_FILENAME_EXCED_RANGE,   ENAMETOOLONG },  /* 206 */
        {  ERROR_NESTING_NOT_ALLOWED,    EAGAIN       },  /* 215 */
        {  ERROR_NOT_ENOUGH_QUOTA,       ENOMEM       }  /* 1816 */
   };


   unsigned i;

   for (i=0; i<ucountof(errtable); i++)
      if (winerr == errtable[i].windocode)
         return errtable[i].errnocode;

   /* These are the low and high value in the range of errors that are exec failures. */
   if (winerr >= ERROR_WRITE_PROTECT && winerr <= ERROR_SHARING_BUFFER_EXCEEDED)
      return EACCES;

   /* These are the low and high value in the range of errors that are access violations */
   if (winerr >= ERROR_INVALID_STARTING_CODESEG && winerr <= ERROR_INFLOOP_IN_RELOC_CHAIN)
      return ENOEXEC;

   return -1;
}

/****************************************************************************************/
C_FUNC_PREFIX const TCHAR *strxerror_r(int unxerr, TCHAR *msg, size_t size)
/****************************************************************************************/
/*
 * multithreaded reentrant save version of strxerror()
 */
{
   DWORD  winerr = GetLastError();


   if (!msg || !size) /* got junk args */
      return msg;

   /*
    * if we have a winerr, but a negative unxerr:
    *    then try to map the winerror into UNIX errno
    */
   if (winerr && unxerr < 0)
   {
      unxerr = _map_winerr_to_errno(winerr);
      if (unxerr == -1)  /* -1 is a placeholder: in fact the mapping failed ... */
         unxerr = 0;     /* ... and we fall back to MSWin style */
   }

   if (!winerr || unxerr>0) /* must use unxerr */
   {
   #if (IS_MSWIN >= 1400 && !defined(IS_MINGW))

      /* first available unter VC 8.0, still NOT with MinGW */
      if (unxerr < 0) unxerr = 0;
      _tcserror_s(msg,size,unxerr);

   #else

      #if (IS_MSWIN <= 1200)
         #if defined(UNICODE) || defined(_UNICODE)
            #error The UNICODE version cannot be build with VC 6.0!
         #else
            #define _tcserror strerror
         #endif
      #endif

      const TCHAR *ep;

      if (unxerr < 0) unxerr = 0;
      ep = _tcserror(unxerr);
      if (STRHASLEN(ep))
      {
         _tcsncpy(msg,ep,size);
         msg[size-1] = 0;
      }
      else
         msg[0] = 0;

   #endif
   }
   else
   {
      /* we have winerr, but negative unxerr: use winerr */
      FormatMessage
      (
         FORMAT_MESSAGE_FROM_SYSTEM
         |FORMAT_MESSAGE_IGNORE_INSERTS,
         0,
         winerr,
         MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
         (LPTSTR)msg,
         (DWORD)size,
         NULL
      );

      /* make tail UNIX strerror() compatible without ".\n" */
      if (msg[0])
      {
         TCHAR *end;

         msg[size-1] = 0;
         STRENDNOSPACE(msg,end);
         if (ISDOT(*end)) *end = 0;
      }
   }

   if (!msg[0]) /* error message empty: make one */
      _sntprintf(msg,size,TEXT("Error #%d: No error message available"),unxerr);

   msg[size-1] = 0;
   return msg;
}

/****************************************************************************************/
C_FUNC_PREFIX const TCHAR *strxerror(int unxerr)
/****************************************************************************************/
{
   static TCHAR strxerror_msg[256]; /* static message buffer should be large enought */
   return strxerror_r(unxerr,strxerror_msg,countof(strxerror_msg));
}

/****************************************************************************************/

#else /* UNIX, what else */

/****************************************************************************************/
C_FUNC_PREFIX const char *strxerror(int unxerr)
/****************************************************************************************/
{
   static const char strxerror_fmt[] = "Error #%d - No error message available";
   static       char strxerror_sbuf[sizeof(strxerror_fmt)+20]; /* max. 20 digits for the error number */
          const char *msg;

   /* negative errno argument is relevant for MSWin and ignored under UNIX */
   if (unxerr <= 0) unxerr = errno;
   msg = strerror(unxerr);
   if (STRHASLEN(msg))
      return msg;

   /* strerror() message empty: make one */
   sprintf(strxerror_sbuf,strxerror_fmt,unxerr);
   return strxerror_sbuf;
}

/****************************************************************************************/

#endif
#endif
