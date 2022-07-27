#pragma once
#ifndef millisleep_SOURCE_INCLUDED
#define millisleep_SOURCE_INCLUDED
/* millisleep.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sleep for some milliseconds
 *       a) mswin32           : we have Sleep(msec) instead.
 *       b) OSF-Alpha/Solaris : select() instead of nanosleep() or link with -lrt
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: millisleep.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN
   #undef  millisleep
   #define millisleep(_msec)     Sleep(_msec)
#else

#if defined(_DONT_HAVE_NANOSLEEP) /* defined(IS_OSFALPHA) */

   #include <sys/time.h>

   C_FUNC_PREFIX void millisleep(long msec)
   {
      if (msec > 0)
      {
         struct timeval req;
         req.tv_sec  = msec/1000;
         req.tv_usec = 1000*(msec%1000);
         while(select(0,NULL,NULL,NULL,&req) < 0 && errno == EINTR)
            ;
      }
   }

#else

   #include <time.h>

   C_FUNC_PREFIX void millisleep(long msec)
   {
      if (msec > 0)
      {
         struct timespec req, rem;
         req.tv_sec  = SCAST_INTO(time_t,msec/1000);
         req.tv_nsec = 1000000*(msec%1000);
         while (nanosleep(&req,&rem) < 0 && errno == EINTR)
            req = rem;
      }
   }

#endif
#endif
#endif
