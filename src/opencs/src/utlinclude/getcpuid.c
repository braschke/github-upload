#ifndef getcpuid_SOURCE_INCLUDED
#define getcpuid_SOURCE_INCLUDED
/* getcpuid.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    execute the x86 CPUID instruction - if possible - and return eax,ebx,ecx,edx
 *    return true=1 if we have the CPUID
 *    return false=0 if CPUID is not available (sun/irix/hpux etc.)
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Jan/20: Carsten Dehning, Initial release
 *    $Id: getcpuid.c 3767 2016-01-07 06:29:18Z dehning $
 *
 *****************************************************************************************
 */
#include "getcpuid.h"

/* default: no functions available, just return 0 */
#ifndef getcpuid
   #define getcpuid(_o,_r)    0
#endif

#ifndef getcpuSSE
   #define getcpuSSE()     0
#endif

#if HAVE_CPUID

#undef getcpuid

/****************************************************************************************/
C_FUNC_PREFIX int getcpuid(const int op, int regs[4])
/****************************************************************************************/
{
   #if IS_MSWIN
      __cpuid(regs,op); /* intrinsic __cpuid() available since VC 8.0 */
   #else
      __asm__
      (
         "cpuid"
         : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
         : "0" (op)
      );
   #endif
   return 1; /* CPUID successful */
}

/****************************************************************************************/

#if USE_SSE

#undef getcpuSSE

/****************************************************************************************/
C_FUNC_PREFIX int getcpuSSE(void)
/****************************************************************************************/
/*
 * Execute CPUID with eax=1 and check the feature bits in ecx and edx
 * return the level of SSE=1,SSE2=2,SSE3=3,MMX=-1, otherwise 0
 */
{
   int regs[4]; /* eax, ebx, ecx, edx */

   if (getcpuid(1,regs)) /* CPUID successful? */
   {
      if (regs[2] & SSE4_FEATURE_BIT) return  4; /* ecx */
      if (regs[2] & SSE3_FEATURE_BIT) return  3; /* ecx */
      if (regs[3] & SSE2_FEATURE_BIT) return  2; /* edx */
      if (regs[3] &  SSE_FEATURE_BIT) return  1; /* edx */
      if (regs[3] &  MMX_FEATURE_BIT) return -1; /* edx */
   }
   return 0;
}

/****************************************************************************************/

#endif
#endif

   #if 0 /* test stuff */
      #include <stdio.h>
      int main(void){printf("SSE=%d\n",getcpuSSE());return 0;}
   #endif

#endif
