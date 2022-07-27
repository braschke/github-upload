#pragma once
#ifndef fxopen_SOURCE_INCLUDED
#define fxopen_SOURCE_INCLUDED
/* fxopen.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Atomar fopen(), capture failures due to interrupts.
 *    handle optional filename extension
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fxopen.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "millisleep.c"
   #include "strsjoinl.c"
   #if IS_MSWIN
      #include "stristr.c"
   #endif
#endif

C_FUNC_PREFIX
FILE *fxopen(const TCHAR *pathname, const TCHAR *mode, const TCHAR *extension)
{
   FILE  *fp;
   size_t len = STRLENP(extension);
   TCHAR  pathExt[MAX_PATH];

   if (len > 0)
   {
      /* we got an extension: check if filname does not already has this extension */
   #if IS_MSWIN /* MSWin is case insensitive */
      #if IS_UNICODE
         const TCHAR *cp = stristr_w(pathname,extension);
      #else
         const TCHAR *cp = stristr_a(pathname,extension);
      #endif
   #else
      const TCHAR *cp = STRSTR(pathname,extension);
   #endif
      if (!cp || cp[len])
      {
         /* make filename + extension */
         pathname = strsjoinl(pathExt,countof(pathExt),NULL,pathname,extension,NULL);
      }
   }

#if IS_MSWIN
   while((fp=_tfopen(pathname,mode)) == NULL) ATOMAR_SYSCALL_LOOP;
#else
   while((fp=fopen(pathname,mode)) == NULL) ATOMAR_SYSCALL_LOOP;
#endif
   return fp;
}

#endif
