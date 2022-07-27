#pragma once
#ifndef conv_double2float_SOURCE_INCLUDED
#define conv_double2float_SOURCE_INCLUDED
/* conv_double2float.c
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
 *    $Id: conv_double2float.c 4582 2016-06-01 10:39:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *conv_double2float(void *buf, const size_t n)
{
   float  *f = (float  *)buf;
   double *d = (double *)buf;
   size_t  i;

   /* avoid overlap and go from head to tail */
   for (i=0; i<n; i++) f[i] = CAST_FLOAT(d[i]);
   return f;
}
#endif
