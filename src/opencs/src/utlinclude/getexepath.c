#pragma once
#ifndef getexepath_SOURCE_INCLUDED
#define getexepath_SOURCE_INCLUDED
/* getexepath.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Platform independent wrapper to get the full pathname of the current executable.
 *
 *    MSWin:
 *       Use GetModuleFileName(), argv[0] is a fallback solution
 *
 *    UNIX:
 *       Works only if there was no chdir() call before, so it should be called
 *       quite early inside main()
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Mar: Carsten Dehning, Initial release
 *    $Id: getexepath.c 4757 2016-06-14 09:16:41Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   C_FUNC_PREFIX
   int getexepath(const TCHAR *argv0, TCHAR *path, const size_t size)
   {
      if (!GetModuleFileName(NULL,path,(DWORD)size))
      {
         /* should never arrive here */
         if (!STRHASLEN(argv0))
            return -1;

         GetFullPathName(argv0,(DWORD)size,path,NULL);
      }

      /* convert possibly MSDOS 8.3 names into full pathnames */
      GetLongPathName(path,path,(DWORD)size); /* same buffer is allowed */

      /* remove the UNICODE indicator in front */
      if (!memcmp(path,TEXT("\\??\\"),4*sizeof(TCHAR)))
      {
         TCHAR *pp4 = path + 4;
         while((*path++=*pp4++) != 0);
      }
      return 0;
   }

#else

   #if INCLUDE_STATIC
      #include "cleanpath.c"
   #endif

   C_FUNC_PREFIX
   int getexepath(const char *argv0, char *path, const size_t size)
   {
      char *p     = path;
      char *plast = path + size-1;

      if (*argv0 != '/') /* not an absolute pathname: put the CWD in front of the argv[0] */
      {
         getcwd(path,size);
         p += strlen(path);
         *p++ = '/';
      }
      while(p<=plast && (*p++=*argv0++) != '\0')
         ;
      *plast = '\0';
      cleanpath(path);
      return 0;
   }

#endif

#endif
