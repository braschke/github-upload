#pragma once
#ifndef fgetsize_SOURCE_INCLUDED
#define fgetsize_SOURCE_INCLUDED
/* fgetsize.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the size of a file or directory.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Dec/11: Carsten Dehning, Initial release
 *    $Id: fgetsize.c 4581 2016-06-01 10:39:03Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   C_FUNC_PREFIX size_t fgetsize(const TCHAR *path)
   {
      WIN32_FILE_ATTRIBUTE_DATA fad;
      return (GetFileAttributesEx(path,GetFileExInfoStandard,&fad))
         ? CAST_SIZE( (__int64)fad.nFileSizeHigh * 0x100000000i64 + (__int64)fad.nFileSizeLow )
         : CAST_SIZE(~0);
   }

#else /* UNIX, what else */

   #include <sys/types.h>
   #include <sys/stat.h>
   C_FUNC_PREFIX size_t fgetsize(const char *path)
   {
      struct stat st;
      return (stat(path,&st)) : CAST_SIZE(~0) : st.st_size;
   }

#endif
#endif
