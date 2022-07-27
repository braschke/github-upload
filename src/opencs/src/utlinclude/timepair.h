#pragma once
#ifndef timepair_HEADER_INCLUDED
#define timepair_HEADER_INCLUDED
/* timepair.h
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
 *    $Id: timepair.h 5707 2017-11-30 10:47:48Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


#ifdef __cplusplus
extern "C" {
#endif

#define TPTSC_RATIO(tsc_a,tsc_b)  ((tsc_b>0) ? CAST_DOUBLE(tsc_a)/CAST_DOUBLE(tsc_b) : -1.0)

typedef struct _TIMEPAIR
{
   uint64_t tp_ucpu;     /* last measured user cpu time with OS max. resolution */
   uint64_t tp_scpu;     /* last measured system cpu time with OS max. resolution */
   uint64_t tp_real;     /* last measured wall clock real time with OS max. resolution */
   uint64_t tp_tick;     /* value of the TSC(time stamp counter) or other high resolution wall ticks */
   uint64_t tp_tkhz;     /* value of the TSC frequency in Hz (Intel processor only) */
   int      tp_nthreads; /* no. of parallel running (optional) OMP threads */
} TIMEPAIR;

#if IS_MSWIN

   /*
    * MSWin FILETIME unit is 100 nanosecond ticks,
    * however the clock accuracy is not better than 15.6 msec.
    */
   #define TIMEPAIR_ACCURACY     (1.0e-6)
   #define TIMEPAIR_real_scale() (1.0e-7)
   #define TIMEPAIR_real_ticks() FT_SCALE_SEC
   #define TIMEPAIR_cpu_scale()  (1.0e-7)

#else

   /*
    * Unix gettimeofday() unit is u-seconds.
    * The clock accuracy is not better than 1 millisecond
    */
   #define TIMEPAIR_ACCURACY     (1.0e-6)
   #define TIMEPAIR_real_scale() (1.0e-6)
   #define TIMEPAIR_real_ticks() 1000000L
   #undef  TIMEPAIR_cpu_scale    /* Implemented as a function */

#endif

#if !INCLUDE_STATIC

   extern uint64_t  TIMEPAIR_gettick   (void);
   extern TIMEPAIR *TIMEPAIR_get       (TIMEPAIR *tp);
   extern TIMEPAIR *TIMEPAIR_diff      (const TIMEPAIR *tp_stop, const TIMEPAIR *tp_start);

   #if !IS_MSWIN /* Unix only, depends on sysconf() */
      extern double TIMEPAIR_cpu_scale (void);
   #endif

   #if defined(__GNUC__) && !defined(__ICC) /* let gcc/g++ check the format string and arguments */
      extern void TIMEPAIR_fprintf(FILE *fp, TIMEPAIR *tp_start, const char *fmt, ...) __attribute__ ((format(printf,3,4)));
   #else
      extern void TIMEPAIR_fprintf(FILE *fp, TIMEPAIR *tp_start, const char *fmt, ...);
   #endif

   extern void TIMEPAIR_fprinttsc(FILE *fp, const uint64_t tsc, const char *stotal, const char *scount, const unsigned count);

#endif

#ifdef __cplusplus
}
#endif
#endif
