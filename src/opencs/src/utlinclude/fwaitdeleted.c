#ifndef __fwaitdeleted_SOURCE_INCLUDED
#define __fwaitdeleted_SOURCE_INCLUDED
/* fwaitdeleted.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Wait max. of msec milliseconds until a file does not exists.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2011, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fwaitdeleted.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if !IS_MSWIN
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
#endif

#if INCLUDE_STATIC
   #include "millisleep.c"
#endif

C_FUNC_PREFIX
int fwaitdeleted(const TCHAR *pathname, int msec)
{
   do {
      if (!FEXISTS(pathname)) return 1; /* return true */
      MILLISLEEP(1);
   } while (msec < 0 || msec-- > 0);
   return 0; /* file still exists */
}
#endif
