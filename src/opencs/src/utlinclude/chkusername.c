#pragma once
#ifndef chkusername_SOURCE_INCLUDED
#define chkusername_SOURCE_INCLUDED
/* chkusername.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Check users login name for bad chars and length range
 *    with rsh, the username must have at most 16 chars,
 *    but UNLEN must be allowed to log on remote MSWin.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Apr/12: Carsten Dehning, Initial release
 *    $Id: chkusername.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN
   #include <lmcons.h>
   #define MY_UNAME_MAX  (UNLEN+DNLEN+3) /* [domain\]user or user[@domain] */
#else
   #define MY_UNAME_MAX  64
#endif

C_FUNC_PREFIX
int chkusername(const TCHAR *name)
{
   int i;


   for(i=0; name[i]; i++)
   {
      TCHAR c = name[i];

      /* some specials are invalid and may be misinterpreted by /bin/sh or CMD.exe */
      if (c <= TEXT(' ') || c >= 127) goto EXIT_ERROR; /* no controls and spaces */
      if (STRCHR(TEXT("\"\'`´[]:|<>+=;?*"),c)) goto EXIT_ERROR;
   }

   if (i >= 1 && i <= MY_UNAME_MAX)
      return i; /* return length of name */

EXIT_ERROR:
#if IS_MSWIN
   SetLastError(ERROR_BAD_USERNAME);
#endif
   errno = EINVAL;
   return 0;
}

#undef MY_UNAME_MAX
#endif
