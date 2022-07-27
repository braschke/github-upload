#pragma once
#ifndef mkdirectory_SOURCE_INCLUDED
#define mkdirectory_SOURCE_INCLUDED
/* mkdirectory.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Replacement for UNIX mkdir(path,mode).
 *    mkdirectory(path,mode) makes a full directory tree instead of just
 *    the tail directory of a path.
 *
 *    Returns Unix style: 0=success, -1=failure
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: mkdirectory.c 5672 2017-10-25 19:21:01Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if !IS_MSWIN
   #include <sys/types.h>
   #include <sys/stat.h>
#endif

/****************************************************************************************/
static int _do_mkdirectory(const TCHAR *path, unsigned mode)
/****************************************************************************************/
/*
 * Returns
 *    EEXIST : FATAL : path exists, but is not a directory == no way out
 *    0      : directory exists or was successfully created
 *    -1     : directory does not exist and was not created
 */
{
#if IS_MSWIN

   DWORD attribs;

   #if 0
   _tprintf(TEXT("mkdirectory <%s>\n"),path);
   #endif

   if ((attribs=GetFileAttributes(path)) == INVALID_FILE_ATTRIBUTES)
   {
      #if 0

         /*
          * TODO: need to translate the UNIX type mode 'drwxrwxrwx' into security attributes
          */
         SECURITY_ATTRIBUTES securityAttributes;

         /* mode->securityAttributes conversion HERE */
         return (CreateDirectory(path,&securityAttributes)) ? 0 : -1;

      #else

         (void)mode; /* keep compiler happy */
         #if 0
         _tprintf(TEXT("mkdirectory <%s>\n"),path);
         #endif
         return (CreateDirectory(path,NULL)) ? 0 : -1;

      #endif
   }

   #if 0
   _tprintf(TEXT("exists <%s> %d\n"),path,(attribs&FILE_ATTRIBUTE_DIRECTORY)?0:EEXIST);
   #endif
   if (attribs & FILE_ATTRIBUTE_DIRECTORY) return 0;/* existing directory */
   SetLastError(ERROR_FILE_EXISTS);
   return EEXIST; /* existing file or other stuff */

#else

   struct stat st;

   #if 0
   printf("mkdirectory <%s>\n",path);
   #endif
   if (stat(path,&st))
   {
      #if 0
      printf("mkdirectory <%s>\n",path);
      #endif
      return mkdir(path,mode);
   }

   #if 0
   printf("exists <%s>\n",path);
   #endif
   if ((st.st_mode & S_IFMT) == S_IFDIR) return 0; /* existing directory */
   return errno = EEXIST; /* existing file or other stuff */

#endif
}

/****************************************************************************************/
C_FUNC_PREFIX int mkdirectory(TCHAR *path, unsigned mode)
/****************************************************************************************/
{
   TCHAR *pp;
   int    iret;


   if (!STRHASLEN(path))
   {
      #if IS_MSWIN
      SetLastError(ERROR_BAD_PATHNAME);
      #else
      errno = EINVAL;
      #endif
      return -1;
   }

   if (!mode)
        mode = 0744;

   switch(_do_mkdirectory(path,mode))
   {
      case 0:      /* directory already exist */
         return 0;

      case EEXIST: /* existing stuff, but not a directory */
         return -1;

      default:
         /*
          * does not exist and was not created, we need to walk down
          * and recursive mkdir the tree structure.
          */
         pp = path;

      #if IS_MSWIN
         /* could be "D:\path\to\file" */
         if ((pp[1]==':') && ISAZALPHA(pp[0]))
            pp += 2;
      #endif

         if (ISDIRSEP(*pp))
         {
            pp++;
            if (ISDIRSEP(*pp)) /* have "//server/share/path/to/file" */
            {
               /* jump to the path portion */
               pp++;
               while(*pp && !ISDIRSEP(*pp)) pp++;
               if (ISDIRSEP(*pp))
               {
                  pp++;
                  while(*pp && !ISDIRSEP(*pp)) pp++;
               }
            }
         }

         for(; *pp; pp++)
         {
            if (ISDIRSEP(*pp))
            {
               *pp  = '\0';
               iret = _do_mkdirectory(path,mode);
               *pp  = _C_DIRSEP;
               if (iret) return -1;
            }
         }

         /* create tail dir if tail is not just a '/' */
         if (pp>path && !ISDIRSEP(*(pp-1)))
            if (_do_mkdirectory(path,mode))
               return -1;
         break;
   }

   return 0;
}

/****************************************************************************************/

#endif
