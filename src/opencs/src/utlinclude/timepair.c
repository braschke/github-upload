#pragma once
#ifndef timepair_SOURCE_INCLUDED
#define timepair_SOURCE_INCLUDED
/* timepair.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines a CPU + WCT (WallClockTime) + TSC (TimeStampCounter) time information.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/Jan/18: Carsten Dehning, Initial release
 *    $Id: timepair.c 5708 2017-11-30 13:00:51Z dehning $
 *
 *****************************************************************************************
 */
#include "timepair.h"

#ifdef _OPENMP
   #include <omp.h>
#endif

#if IS_MSWIN

   #define USE_MSWIN_PERFORMANCE_COUNTERS  1 /* MSWin only, remains a TODO */

#else

   /*
    * Need to have the definitions for
    *    'struct tms'
    *    'struct timeval'
    */
   #include <time.h>
   #include <sys/time.h>
   #include <sys/times.h>

   #if defined(IS_MACOSX)
      #include <sys/sysctl.h>
   #endif

#endif

#if INCLUDE_STATIC
   #include "dtscale.c"
   #include "ulong2adp.c"
#endif

/****************************************************************************************/

#if !IS_MSWIN

/****************************************************************************************/
C_FUNC_PREFIX double TIMEPAIR_cpu_scale(void)
/****************************************************************************************/
/*
 * Unix CPU times units are clock ticks
 *
 * CLK_TCK caused problems on Linux with gcc 3.3
 *
 *    a) may be function #define CLK_TCK ((__clock_t) __sysconf (2))
 *    b) may be unknown when compiling with -ansi.
 */
{
   static double qcticks = 0;
   #ifdef CLK_TCK
      if (!qcticks) qcticks = 1.0/CAST_DOUBLE(CLK_TCK);
   #else
      if (!qcticks) qcticks = 1.0/CAST_DOUBLE(sysconf(_SC_CLK_TCK));
   #endif

   return qcticks;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX uint64_t TIMEPAIR_gettick(void)
/****************************************************************************************/
/*
 * Returns the current time stamp counter value.
 */
{
#if IS_MSWIN

   #if USE_MSWIN_PERFORMANCE_COUNTERS
      /*
       * Measured with less than 30 nanoseconds resolution
       */
      LARGE_INTEGER wt;
      QueryPerformanceCounter(&wt);
      return wt.QuadPart;
   #else
      /*
       * Much faster, but not necessary reliable with power management & multi core hosts
       */
      return __rdtsc(); /* VC intrinsic function */
   #endif

#elif defined(CLOCK_MONOTONIC_RAW)

   /*
    * Works with Linux >= 2.6.28
    */
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC_RAW,&ts);
   return CAST_UINT64(ts.tv_sec)*1000000000L + ts.tv_nsec;

#elif defined(_POSIX_MONOTONIC_CLOCK)

   /*
    * Works on POSIX systems
    */
   struct timespec ts;
   clock_gettime(_POSIX_MONOTONIC_CLOCK,&ts);
   return CAST_UINT64(ts.tv_sec)*1000000000L + ts.tv_nsec;

#elif HAVE_RDTSC && (defined(IS_LINUX)||defined(IS_MACOSX))

   /*
    * Much faster, but not necessary reliable with power management & multi core hosts
    * Exception: '/proc/cpuinfo' shows the 'tsc_constant' or 'constant_tsc' processor flag
    */
   register uint32_t lo,hi;
   asm volatile ("rdtsc" : "=a" (lo), "=d" (hi));
   return (CAST_UINT64(hi) << 32) | lo;

#else

   /*
    * Always works, but with a low resolution of about one microsecond
    */
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return CAST_UINT64(tv.tv_sec)*1000000L + tv.tv_usec;

#endif
}

/****************************************************************************************/
C_FUNC_PREFIX TIMEPAIR *TIMEPAIR_get(TIMEPAIR *tp)
/****************************************************************************************/
/*
 * Init tp with current absolute time values
 */
{
#if IS_MSWIN

   union int64_union
   {
      uint64_t ui64;
      FILETIME ft;
   } creatTime,exitTime,cpuKernel,cpuUser;
   GetProcessTimes
   (
      GetCurrentProcess()
      ,&creatTime.ft
      ,&exitTime.ft
      ,&cpuKernel.ft
      ,&cpuUser.ft
   );
   GetSystemTimeAsFileTime(&creatTime.ft);
   tp->tp_scpu = cpuKernel.ui64;
   tp->tp_ucpu = cpuUser.ui64;
   tp->tp_real = creatTime.ui64;

#else

   struct tms     ts;
   struct timeval tv;

   times(&ts);
   gettimeofday(&tv,NULL);
   tp->tp_ucpu = ts.tms_utime;
   tp->tp_scpu = ts.tms_stime;
   tp->tp_real = (CAST_UINT64(tv.tv_sec))*1000000L + CAST_UINT64(tv.tv_usec);

#endif

   tp->tp_tick = TIMEPAIR_gettick();
   tp->tp_tkhz = 0; /* Evaluated later inside TIMEPAIR_diff() */

#ifdef _OPENMP
   tp->tp_nthreads = omp_get_max_threads();
#else
   tp->tp_nthreads = 1;
#endif

   return tp;
}

/****************************************************************************************/
C_FUNC_PREFIX TIMEPAIR *TIMEPAIR_diff(const TIMEPAIR *tp_stop, const TIMEPAIR *tp_start)
/****************************************************************************************/
/*
 * Calculates the difference of the stop time tp_stop - tp_start (tp_stop > tp_start !)
 * into the static tp_diff and returns a pointer to the static tp_diff.
 */
{
   static TIMEPAIR tp_diff;


   tp_diff.tp_ucpu = tp_stop->tp_ucpu - tp_start->tp_ucpu;
   tp_diff.tp_scpu = tp_stop->tp_scpu - tp_start->tp_scpu;
   tp_diff.tp_real = tp_stop->tp_real - tp_start->tp_real;
   tp_diff.tp_tick = tp_stop->tp_tick - tp_start->tp_tick;
   tp_diff.tp_tkhz = 0;

#if IS_MSWIN && USE_MSWIN_PERFORMANCE_COUNTERS
   {
      LARGE_INTEGER lifreq;
      QueryPerformanceFrequency(&lifreq);
      tp_diff.tp_tkhz = lifreq.QuadPart;
   }
#else
   if (tp_diff.tp_real && tp_diff.tp_tick)
   {
   #if 0
      tp_diff.tp_tkhz = CAST_UINT64(tp_diff.tp_tick/(tp_diff.tp_real*TIMEPAIR_real_scale()));
   #else
      tp_diff.tp_tkhz = (tp_diff.tp_tick*TIMEPAIR_real_ticks())/tp_diff.tp_real;
   #endif
   }
#endif

   tp_diff.tp_nthreads = tp_start->tp_nthreads;
   return &tp_diff;
}

/****************************************************************************************/
C_FUNC_PREFIX void TIMEPAIR_fprintf
(
   FILE       *fp,
   TIMEPAIR   *tp_start,
   const char *fmt,
   ...
)
/****************************************************************************************/
/*
 * Store back time differences and print
 */
{
   TIMEPAIR tp;
   double   cpu,wct;


   TIMEPAIR_get(&tp); /* Get the stop time ... */

   if (!fp)
      fp = stdout;

   if (STRHASLEN(fmt))
   {
      va_list ap;
      va_start(ap,fmt);
      vfprintf(fp,fmt,ap);
      va_end(ap);
   }
   else
   {
      fputs("Operation times: ",fp);
   }

   tp  = *TIMEPAIR_diff(&tp,tp_start); /* struct copy */
   cpu = (tp.tp_ucpu + tp.tp_scpu)*TIMEPAIR_cpu_scale();
   wct =  tp.tp_real              *TIMEPAIR_real_scale();

   /*
    * The clock accuracy is not better than TIMEPAIR_ACCURACY seconds
    */
   if (cpu < TIMEPAIR_ACCURACY || wct < TIMEPAIR_ACCURACY)
   {
      fputs("Less than one usec :)).\n",fp);
   }
   else
   {
      const char  *cpu_unit,*wct_unit;
      const double speedup = cpu/wct;

      if (tp.tp_nthreads > 1)
      {
         fprintf(fp,"CPU(%d x ",tp.tp_nthreads);

         /* Divide total CPU by the no. of threads */
         cpu /= tp.tp_nthreads;
      }
      else
      {
         fputs("CPU(",fp);
      }

      /* Get the times in an appropriate format */
      cpu_unit = dtscale(&cpu);
      wct_unit = dtscale(&wct);

      fprintf
      (
         fp
         ,"%.4f %s), WCT(%.4f %s), CPU/WCT(%.4f)"
         ,cpu
         ,cpu_unit
         ,wct
         ,wct_unit
         ,speedup
      );

      fprintf
      (
         fp
         ,", TSC(%s, ~%.2f GHz)"
         ,ulong2adp(tp.tp_tick,NULL,0)
         ,(tp.tp_tkhz/1.0e9)
      );

      fputs(".\n",fp);
   }
   fflush(fp);
}

/****************************************************************************************/
C_FUNC_PREFIX void TIMEPAIR_fprinttsc
(
   FILE           *fp,
   const uint64_t  wt,
   const char     *stotal,
   const char     *scount,
   const unsigned  count
)
/****************************************************************************************/
/*
 *  Print a given walltick
 */
{
   if (!fp)
      fp = stdout;

   if (STRHASLEN(scount) && count > 1)
   {
      char sw[32],sc[32];
      fprintf
      (
         fp,"%s(%s), %s(%s).\n"
         ,stotal
         ,ulong2adp(wt,sw,sizeof(sw))
         ,scount
         ,ulong2adp(wt/count,sc,sizeof(sc))
      );
   }
   else
   {
      char sw[32];
      fprintf
      (
         fp,"%s(%s).\n"
         ,stotal
         ,ulong2adp(wt,sw,sizeof(sw))
      );
   }

   fflush(fp);
}

/****************************************************************************************/

#undef USE_MSWIN_PERFORMANCE_COUNTERS

#endif
