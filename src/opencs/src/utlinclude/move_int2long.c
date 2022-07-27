#ifndef __move_int2long_SOURCE_INCLUDED
#define __move_int2long_SOURCE_INCLUDED
/* move_int2long.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    move/copy and convert number arrays
 *    convert an int/long array into a long/int array.
 *
 *    check for a partial overlap, but allow source and target point to
 *    the same address!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: move_int2long.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
long *move_int2long(long *dst, const int *src, size_t n)
{
   if (
         ( (void *)dst == (void *)(src  ) ) ||
         ( (void *)dst >  (void *)(src+n) ) ||
         ( (void *)src >  (void *)(dst+n) )
      )
   {
      while(n-- > 0) dst[n] = (long)src[n];
      return dst;
   }
   return NULL; /* indicate a partial overlap to the caller */
}
#endif
