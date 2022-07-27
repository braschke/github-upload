#ifndef __vsum_float_SOURCE_INCLUDED
#define __vsum_float_SOURCE_INCLUDED
/* vsum_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all values of array v[n] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vsum_float.c 943 2013-05-24 15:58:49Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "kahan_add.h"

C_FUNC_PREFIX
double vsum_float(const float *v, const int n)
{
   double sum = 0;
   int i;

#if USE_KAHAN_ADD
   double loss = 0;
   for(i=0; i<n; i++)
      KAHAN_ADD_FLOAT(sum,v[i],loss)
#else
   for(i=0; i<n; i++) sum += CAST_DOUBLE(v[i]);
#endif

   return sum;
}
#endif
