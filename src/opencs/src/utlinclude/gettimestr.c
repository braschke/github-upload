#pragma once
#ifndef gettimestr_SOURCE_INCLUDED
#define gettimestr_SOURCE_INCLUDED
/* gettimestr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the double dseconds as a time string: "10 days 1 min 5 hours 10.056 secs"
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2016/Aug/10: Carsten Dehning, Initial release
 *    $Id: gettimestr.c 5518 2017-08-22 17:55:02Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX const TCHAR *gettimestr(const double dseconds)
{
   static TCHAR sbuf[32];

   TCHAR   *cp = sbuf;
   uint64_t msec,secs;
   unsigned days,hour,mins;

   msec = CAST_UINT64(1000.0*dseconds);
   secs = msec/1000      ; msec %= 1000;
   days = (unsigned)(secs/(24*60*60)); secs %= (24*60*60);
   hour = (unsigned)(secs/(   60*60)); secs %= (   60*60);
   mins = (unsigned)(secs/(      60)); secs %= (      60);

   if (days)
   {
      sprintf(cp,TEXT("%u day%s "),days,plurals(days));
      cp += STRLEN(cp);
   }
   if (hour)
   {
      sprintf(cp,TEXT("%u hour%s "),hour,plurals(hour));
      cp += STRLEN(cp);
   }
   if (mins)
   {
      sprintf(cp,TEXT("%u min%s "),mins,plurals(mins));
      cp += STRLEN(cp);
   }

   sprintf(cp,TEXT("%u.%03u sec%s"),(unsigned)secs,(unsigned)msec,plurals(secs));

   return sbuf;
}
#endif
