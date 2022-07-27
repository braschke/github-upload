#pragma once
#ifndef getpathname_SOURCE_INCLUDED
#define getpathname_SOURCE_INCLUDED
/* getpathname.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Find the begin of the pathname in a
 *
 *       [user@host:][drive:|UNC]pathname
 *
 *    and return a pointer to it
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getpathname.c 5509 2017-08-19 10:10:10Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *getpathname(const TCHAR *pathname)
{
   static TCHAR rootPath[2] = {_C_DIRSEP,0};
   const TCHAR *cp;


   STRJUMPNOSPACE(pathname);
   if (!STRHASLEN(pathname))
      return rootPath;

   /* could be "user@host:/path/to/file" or even user@host:D:\path\to\file */
   if ((cp=STRCHR(pathname,TEXT(':'))) != NULL)
      pathname = cp + 1;

   if (pathname[1] == TEXT(':')) /* MSWin: could be "D:\path\to\file" */
      pathname += 2;

   else if (PATH_ISUNC(pathname)) /* have "//server/share/path/to/file" */
   {
      pathname += 2;
      while(*pathname && !ISDIRSEP(*pathname)) pathname++;
      if (ISDIRSEP(*pathname))
      {
         pathname++;
         while(*pathname && !ISDIRSEP(*pathname)) pathname++;
      }
   }

   return (TCHAR *)pathname;
}
#endif
