#pragma once
#ifndef wallticks_SOURCE_INCLUDED
#define wallticks_SOURCE_INCLUDED
/* wallticks.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines a unitless and platform independent Wall-Clock-Ticks with a resolution
 *    far less than 1 millisecond, plus utilities.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2015/Dec/10: Carsten Dehning, Initial release
 *    $Id: wallticks.c 5672 2017-10-25 19:21:01Z dehning $
 *
 *****************************************************************************************
 */
#include "wallticks.h"

#if INCLUDE_STATIC
   #include "ulong2adp.c"
#endif

#if IS_MSWIN

   #define USE_MSWIN_PERFORMANCE_COUNTERS  1 /* MSWin only, is a TODO */

#else
   /*
    * Need to have the definitions for
    *    'struct timeval'
    */
   #include <sys/time.h>

   #if defined(IS_MACOSX)
      #include <sys/sysctl.h>
   #endif

#endif

#if HAVE_RDTSC && defined(IS_LINUX)

/****************************************************************************************/
static uint64_t get_cpu_freq_linux(void)
/****************************************************************************************/
{
   FILE    *fp;
   uint64_t freq = 1;


   fp = fopen("/proc/cpuinfo","r");
   if (fp)
   {
      char line[1024];

      while(fgets(line,sizeof(line),fp))
      {
         const char *cp;

         #if 0
            printf("Line = %s",line);
         #endif

         if ((cp=strstr(line,"cpu MHz")) != NULL && (cp=strchr(cp+7,':')))
         {
         #if 1
            char    *sp;
            uint64_t mega;
            unsigned ndec = 0;
            char     sbuf[64];


            cp++;
            STRJUMPNOSPACE(cp);

            /* Copy into a string, but without decimals and thousands */
            for(sp=sbuf; *cp && !ISSPACE(*cp); cp++)
            {
               switch(*cp)
               {
                  case '.': ndec = 1; break;
                  case ',':           break;
                  default :
                     if (ndec) ndec++;
                     *sp++ = *cp;
                     break;
               }
            }
            *sp = '\0';

            #if 0
               printf("sbuf = <%s> ndec=<%u>\n",sbuf,ndec);
            #endif

            /* Reduce mega by the no. of decimals */
            for (mega=1000000L; ndec>1; ndec--)
               mega /= 10L;

            freq = CAST_UINT64(atol(sbuf))*mega;
            #if 0
               printf("freq<%lu>\n",freq);
            #endif

            break;

         #else

            float ff;
            if (sscanf(cp+1,"%f",&ff) == 1)
            {
               freq = CAST_UINT64(ff*1000000.0);
               break;
            }

         #endif

         }

         else if ((cp=strstr(line,"Cpu0ClkTck")) != NULL && (cp=strchr(cp+10,':')))
         {
            unsigned uf;
            if (sscanf(cp+1,"%x",&uf) == 1)
            {
               freq = CAST_UINT64(uf);
               break;
            }
         }

         else if ((cp=strstr(line,"timebase")) != NULL && (cp=strchr(cp+8,':')))
         {
            unsigned uf;
            if (sscanf(cp+1,"%u",&uf) == 1)
            {
               freq = CAST_UINT64(uf);
               break;
            }
         }
      }

      fclose(fp);
   }
   else
   {
      printf("failed to open /proc/cpuinfo: %s.\n",strerror(errno));
   }

   return freq;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX uint64_t WALLTICKS_freq(unsigned *pvari)
/****************************************************************************************/
/*
 * Returns the current tick frequency (ticks/sec) + the information whether the
 * frequency is constant or only the current frequency.
 */
{
   static uint64_t freq = 0;
   static unsigned vari = 0;


   if (!freq)
   {
   #if IS_MSWIN

      #if USE_MSWIN_PERFORMANCE_COUNTERS
         LARGE_INTEGER lifreq;
         QueryPerformanceFrequency(&lifreq);
         freq = lifreq.QuadPart;
      #else
         freq = 1; /* TODO */
         vari = 1;
      #endif

   #elif HAVE_RDTSC && defined(IS_LINUX)

      freq = get_cpu_freq_linux();
      vari = 1;

   #elif HAVE_RDTSC && defined(IS_MACOSX)

      size_t   usize = sizeof(unsigned);
      unsigned uhertz;
      int      mib[2];

      mib[0] = CTL_HW;
      mib[1] = HW_CPU_FREQ;
      sysctl(mib,2,&uhertz,&usize,NULL,0);
      freq = CAST_UINT64(uhertz);

   #else

      /* gettimeofday() usec resolution */
      freq = 1000000L;

   #endif
   }

   if (pvari) *pvari = vari;
   return freq;
}

/****************************************************************************************/
C_FUNC_PREFIX uint64_t WALLTICKS_get(void)
/****************************************************************************************/
/*
 * Returns the current tick counter
 */
{
#if IS_MSWIN

   #if USE_MSWIN_PERFORMANCE_COUNTERS
      /*
       * Measured with less than 30 nanosecond resolution
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
   return (CAST_UINT64(tv.tv_sec)*1000000L + tv.tv_usec;

#endif
}

/****************************************************************************************/
C_FUNC_PREFIX void WALLTICKS_print
(
   const uint64_t  wt,
   const char     *stotal,
   const char     *scount,
   const unsigned  count,
   FILE           *fp
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
