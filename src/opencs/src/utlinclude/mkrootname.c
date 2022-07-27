#ifndef __mkrootname_SOURCE_INCLUDED
#define __mkrootname_SOURCE_INCLUDED
/* mkrootname.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    extract the root name (drive or node) from a pathname
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2008, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: mkrootname.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


C_FUNC_PREFIX
TCHAR *mkrootname(TCHAR *pathname)
{
   STRJUMPNOSPACE(pathname);
   if (!STRHASLEN(pathname))
      return pathname;

   if (pathname[1] == TEXT(':')) /* MSWin: return "D:\" */
      pathname[3] = 0;

   else if (ISDIRSEP(pathname[0]))
   {
      TCHAR *cp = PATH_ISUNC(pathname);

      if (cp) /* make "//server/share\0" */
      {
         for(cp++; *cp && !ISDIRSEP(*cp); cp++)
            ;
         *cp = 0;
      }
      else
         cp[1] = 0;
   }
   else
      pathname[0] = 0;

   return pathname;
}
#endif
