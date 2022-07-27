#pragma once
#ifndef getbasename_SOURCE_INCLUDED
#define getbasename_SOURCE_INCLUDED
/* getbasename.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Find the begin of the filename in a pathname and return a pointer to it.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getbasename.c 5509 2017-08-19 10:10:10Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *getbasename(const TCHAR *pathname)
{
   const TCHAR *pLast = pathname;

   for(; *pathname; pathname++)
   {
      if (ISDIRSEP(*pathname) && pathname[1])
         pLast = pathname + 1;
   }

   return CCAST_INTO(TCHAR *,pLast);
}
#endif
