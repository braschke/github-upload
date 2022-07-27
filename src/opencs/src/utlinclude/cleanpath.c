#pragma once
#ifndef cleanpath_SOURCE_INCLUDED
#define cleanpath_SOURCE_INCLUDED
/* cleanpath.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Make a canonical pathname but do not resolve links like realpath() does.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Mar: Carsten Dehning, Initial release
 *    $Id: cleanpath.c 4757 2016-06-14 09:16:41Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


C_FUNC_PREFIX
TCHAR *cleanpath(TCHAR *path)
{
   TCHAR *src,*dst;


   for(src=(dst=path)+1; *src; src++)
   {
      if (ISDIRSEP(dst[0]))
      {
         if (ISDIRSEP(src[0]) && dst>path)
         {
            /* replace "//" with "/" */
            continue;
         }

         if (ISDOT(src[0]) && (ISDIRSEP(src[1]) || !src[1]))
         {
            /* replace "/./" with "/" */
            src++;
            continue;
         }

         if (ISDOT(src[0]) && ISDOT(src[1]) && (ISDIRSEP(src[2]) || !src[2]))
         {
            /* replace "/name/../" with "/" */
            for(dst--; dst>=path && !ISDIRSEP(*dst); dst--)
               ;
            src += 2;
            if (dst < path) /* no leading DIRSEP found */
               *(dst=path) = *++src;
            continue;
         }
      }
      if (++dst < src)
         *dst = *src;
   }

   dst[1] = TEXT('\0');
   return path;
}

#if 0
int main(int argc, char *argv[])
{
printf("PATH=\"%s\"\n",argv[1]);
cleanpath(argv[1]);
printf("PATH=\"%s\"\n",argv[1]);
}
#endif

#endif
