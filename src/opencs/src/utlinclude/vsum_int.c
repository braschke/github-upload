#ifndef __vsum_int_SOURCE_INCLUDED
#define __vsum_int_SOURCE_INCLUDED
/* vsum_int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all values of array v[n] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vsum_int.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
long vsum_int(const int *v, const int n)
{
   long sum = 0;
   int i;
   for(i=0; i<n; i++) sum += CAST_LONG(v[i]);
   return sum;
}
#endif
