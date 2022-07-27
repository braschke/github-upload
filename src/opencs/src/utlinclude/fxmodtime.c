#pragma once
#ifndef fxmodtime_SOURCE_INCLUDED
#define fxmodtime_SOURCE_INCLUDED
/* fxmodtime.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Return the file modification time (UNIX sec since '70) of a file
 *    or 0 - which I believe cannot happen nowadays - in case of error.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fxmodtime.c 4583 2016-06-01 10:40:51Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

C_FUNC_PREFIX
time_t fxmodtime(const TCHAR *pathname, const TCHAR *extension)
{
   /* convert from FILETIME to unsigned __int64 */
   union
   {
      int64_t  q;
      FILETIME f;
   } time64;

   WIN32_FILE_ATTRIBUTE_DATA fad;


   /* make filename + extension if extension is set */
   TCHAR pathExt[MAX_PATH];
   if (STRHASLEN(extension))
      pathname = _tcsncat(_tcsncpy(pathExt,pathname,countof(pathExt)),extension,countof(pathExt));

   if (!GetFileAttributesEx(pathname,GetFileExInfoStandard,&fad))
      return 0; /* stat error */

   /* get modification time */
   time64.f = fad.ftLastWriteTime;
   if (!time64.q)
      time64.f = fad.ftCreationTime; /* mtime not available, use ctime */

   return FILETIMETOTIME_T(time64.q);
}

#else /* IS_MSWIN */

#include <sys/types.h>
#include <sys/stat.h>

C_FUNC_PREFIX
time_t fxmodtime(const char *pathname, const char *extension)
{
   struct stat st;

   /* make filename + extension if extension is set */
   char pathExt[MAX_PATH];
   if (STRHASLEN(extension))
      pathname = strncat(strncpy(pathExt,pathname,sizeof(pathExt)),extension,sizeof(pathExt));

   return (stat(pathname,&st)) ? 0 : st.st_mtime;
}

#endif

#endif
