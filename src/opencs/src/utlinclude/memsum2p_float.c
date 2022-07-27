#ifndef __memsum2p_float_SOURCE_INCLUDED
#define __memsum2p_float_SOURCE_INCLUDED
/* memsum2p_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all values of array product buf1*buf2 and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2008, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: memsum2p_float.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double memsum2p_float(const float *buf1, const float *buf2, size_t n)
{
   double sum = 0.0;
   size_t i;
   for(i=0; i<n; i++) sum += (double)(buf1[i]*buf2[i]);
   return sum;
}
#endif
