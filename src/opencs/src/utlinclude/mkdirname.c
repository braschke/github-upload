#pragma once
#ifndef mkdirname_SOURCE_INCLUDED
#define mkdirname_SOURCE_INCLUDED
/* mkdirname.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Strip the filename part of a pathname
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: mkdirname.c 5509 2017-08-19 10:10:10Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "getpathname.c"
#endif

C_FUNC_PREFIX
TCHAR *mkdirname(TCHAR *pathname)
{
   TCHAR *pBeg = getpathname(pathname);
   TCHAR *pEnd, *c;


   if (!STRHASLEN(pathname))
      return pBeg;

   pEnd = NULL;
   for(c=(TCHAR *)pBeg; *c; c++)
   {
      if (ISDIRSEP(*c)) pEnd = c;
   }

   if (pEnd == pBeg)
      pEnd[1] = 0; /* return "//server/" or "/" or "D:\" */
   else if (pEnd)
      pEnd[0] = 0;

   return pathname;
}
#endif
