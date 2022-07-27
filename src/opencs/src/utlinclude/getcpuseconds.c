#pragma once
#ifndef getcpuseconds_SOURCE_INCLUDED
#define getcpuseconds_SOURCE_INCLUDED
/* getcpuseconds.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the CPU time used by this process and all its threads
 *    in seconds as a double.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Sep/04: Carsten Dehning, Initial release
 *    $Id: getcpuseconds.c 5672 2017-10-25 19:21:01Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if !IS_MSWIN /* UNIX, what else */
   #include <time.h>
   #include <sys/times.h>
#endif

C_FUNC_PREFIX double getcpuseconds(void)
{
#if IS_MSWIN

   union int64_union
   {
      FILETIME ft;
      uint64_t ui64;
   } creatTime,exitTime,cpuKernel,cpuUser;
   return (GetProcessTimes
            (
               GetCurrentProcess()
               ,&creatTime.ft
               ,&exitTime.ft
               ,&cpuKernel.ft
               ,&cpuUser.ft
            )
          )
      ? (cpuKernel.ui64 + cpuUser.ui64)*1.0e-7 /* filetime is given in 100 nanosecond ticks */
      : -1.0; /* indicate failure */

#else /* UNIX, what else */

   struct tms ts;

   /*
    * CLK_TCK caused problems on Linux with gcc 3.3
    *
    *    a) may be function #define CLK_TCK ((__clock_t) __sysconf (2))
    *    b) may be unknown when compiling with -ansi.
    */
   static double qcticks = 0;
   #ifdef CLK_TCK
      if (!qcticks) qcticks = 1.0/CAST_DOUBLE(CLK_TCK);
   #else
      if (!qcticks) qcticks = 1.0/CAST_DOUBLE(sysconf(_SC_CLK_TCK));
   #endif

   times(&ts);
   return (CAST_UINT64(ts.tms_stime) + CAST_UINT64(ts.tms_utime))*qcticks;

#endif

}
#endif
