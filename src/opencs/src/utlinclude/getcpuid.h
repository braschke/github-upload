#pragma once
#ifndef getcpuid_HEADER_INCLUDED
#define getcpuid_HEADER_INCLUDED
/* getcpuid.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Execute the x86 CPUID instruction - if possible - and return eax,ebx,ecx,edx
 *    return true(1)==successful and false(0) if CPUID is not available.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Jan/20: Carsten Dehning, Initial release
 *    $Id: getcpuid.h 4232 2016-04-28 05:21:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define  MMX_FEATURE_BIT   0x00800000 /* CPUID 1, ->EDX(bit 23) */
#define  SSE_FEATURE_BIT   0x02000000 /* CPUID 1, ->EDX(bit 25) */
#define SSE2_FEATURE_BIT   0x04000000 /* CPUID 1, ->EDX(bit 26) */
#define SSE3_FEATURE_BIT   0x00000001 /* CPUID 1, ->ECX(bit  0) */
#define SSE4_FEATURE_BIT   0x00000200 /* CPUID 1, ->ECX(bit  9) */

#if !INCLUDE_STATIC
   #if HAVE_CPUID
      extern int getcpuid(const int op, int regs[4]);
      extern int getcpuSSE();
   #else
      /* default: no functions available, just return 0 */
      #define getcpuid(_o,_r)    0
      #define getcpuSSE()        0
   #endif
#endif
#endif
