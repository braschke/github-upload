#pragma once
#ifndef fisdirectory_SOURCE_INCLUDED
#define fisdirectory_SOURCE_INCLUDED
/* fisdirectory.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Check if a path is an existing directory
 *    return: -1=error/does not exist, 1=directory, 0=other file type
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fisdirectory.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   C_FUNC_PREFIX int fisdirectory(const TCHAR *path)
   {
      DWORD attribs;
      return ((attribs=GetFileAttributes(path)) == INVALID_FILE_ATTRIBUTES)
         ? -1 : ((attribs & FILE_ATTRIBUTE_DIRECTORY) != 0);
   }

#else

   #include <sys/types.h>
   #include <sys/stat.h>

   C_FUNC_PREFIX int fisdirectory(const char *path)
   {
      struct stat st;
      return (stat(path,&st))
         ? -1 : ((st.st_mode & S_IFMT) == S_IFDIR) ? 1 : 0;
   }

#endif
#endif
