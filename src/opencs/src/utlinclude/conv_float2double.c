#pragma once
#ifndef conv_float2double_SOURCE_INCLUDED
#define conv_float2double_SOURCE_INCLUDED
/* conv_float2double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    In place convert a float/real/double array into a double/real/float array.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: conv_float2double.c 4582 2016-06-01 10:39:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *conv_float2double(void *buf, const size_t n)
{
   float  *f = (float  *)buf;
   double *d = (double *)buf;
   size_t  i = n;

   /*
    * float array must have the size n*sizeof(double)!!!
    * avoid overlap and go from tail to head
    */
   while(i-- > 0) d[i] = CAST_DOUBLE(f[i]);
   return d;
}
#endif
