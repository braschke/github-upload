#ifndef __fislink_SOURCE_INCLUDED
#define __fislink_SOURCE_INCLUDED
/* fislink.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Check if a path is symbolic link resp. reparse point under MSWin
 *    return: -1=error/does not exist, 1=islink, 0=other file type
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fislink.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   C_FUNC_PREFIX int fislink(const TCHAR *path)
   {
      DWORD attribs;
      return ((attribs=GetFileAttributes(path)) == INVALID_FILE_ATTRIBUTES)
         ? -1 : ((attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0);
   }

#else

   #include <sys/types.h>
   #include <sys/stat.h>

   C_FUNC_PREFIX int fislink(const char *path)
   {
      struct stat st;
      return (stat(path,&st))
         ? -1 : ((st.st_mode & S_IFMT) == S_IFLNK) ? 1 : 0;
   }

#endif
#endif
