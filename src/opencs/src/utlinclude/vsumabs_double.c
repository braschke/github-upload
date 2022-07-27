#ifndef __vsumabs_double_SOURCE_INCLUDED
#define __vsumabs_double_SOURCE_INCLUDED
/* vsumabs_double.c
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
 *    $Id: vsumabs_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double vsumabs_double(const double *v, int n)
{
   double sum = 0.0;
   int i;
   for(i=0; i<n; i++) sum += fabs(v[i]);
   return sum;
}
#endif
