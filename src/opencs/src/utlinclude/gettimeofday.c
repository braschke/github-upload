#pragma once
#ifndef gettimeofday_SOURCE_INCLUDED
#define gettimeofday_SOURCE_INCLUDED
/* gettimeofday.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    MSWin implementation of Unix gettimeofday().
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Dec/10: Carsten Dehning, Initial release
 *    $Id: gettimeofday.c 4583 2016-06-01 10:40:51Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

#if INCLUDE_STATIC
   struct timezone
   {
      int tz_minuteswest; /* minutes W of Greenwich */
      int tz_dsttime;     /* type of dst correction */
   };
#endif

#if 0 /* MSWIN version from <winsock2.h>: just for the documentation */
   struct timeval
   {
      long tv_sec;   /* seconds */
      long tv_usec;  /* and microseconds */
   };
#endif

C_FUNC_PREFIX
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
   static BOOL tzokay = FALSE;


   if (tv)
   {
      UINT64 ft;

      GetSystemTimeAsFileTime((FILETIME *)&ft);
      ft         -= FT_EPOCH_BIAS;                           /* make time Unix epoch 1970/1/1 based */
      tv->tv_sec  = (long) (ft/FT_SCALE_SEC);                /* get the seconds */
      tv->tv_usec = (long)((ft%FT_SCALE_SEC)/FT_SCALE_USEC); /* get the microns */
   }

   if (tz)
   {
      if (!tzokay) /* first set global vars _timezone & _daylight */
      {
         _tzset();
         tzokay = TRUE;
      }
      tz->tz_minuteswest = _timezone / 60;
      tz->tz_dsttime     = _daylight;
   }

   return 0;
}

#endif
#endif
