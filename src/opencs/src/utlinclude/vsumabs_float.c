#ifndef __vsumabs_float_SOURCE_INCLUDED
#define __vsumabs_float_SOURCE_INCLUDED
/* vsumabs_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all fabs(values) of array v[n] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2010, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vsumabs_float.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double vsumabs_float(const float *v, int n)
{
   double sum = 0.0;
   int i;
   for(i=0; i<n; i++) sum += fabs(v[i]);
   return sum;
}
#endif
