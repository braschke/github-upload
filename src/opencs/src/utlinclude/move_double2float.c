#pragma once
#ifndef move_double2float_SOURCE_INCLUDED
#define move_double2float_SOURCE_INCLUDED
/* move_double2float.c
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
 *    $Id: move_double2float.c 4090 2016-03-24 10:31:37Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *move_double2float(float *dst, const double *src, size_t n)
{
   if (
         ( (void *)dst == (void *)(src  ) ) ||
         ( (void *)dst >  (void *)(src+n) ) ||
         ( (void *)src >  (void *)(dst+n) )
      )
   {
      size_t i;
      for (i=0; i<n; i++) dst[i] = CAST_FLOAT(src[i]);
      return dst;
   }
   return NULL; /* indicate a partial overlap to the caller */
}
#endif
