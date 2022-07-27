#pragma once
#ifndef move_float2double_SOURCE_INCLUDED
#define move_float2double_SOURCE_INCLUDED
/* move_float2double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    move/copy and convert number arrays
 *    convert a float/real/double array into a double/real/float array.
 *
 *    check for a partial overlap, but allow source and target point to
 *    the same address!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: move_float2double.c 4090 2016-03-24 10:31:37Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *move_float2double(double *dst, const float *src, size_t n)
{
   if (
         ( (void *)dst == (void *)(src  ) ) ||
         ( (void *)dst >  (void *)(src+n) ) ||
         ( (void *)src >  (void *)(dst+n) )
      )
   {
      while(n-- > 0) dst[n] = (double)src[n];
      return dst;
   }
   return NULL; /* indicate a partial overlap to the caller */
}
#endif
