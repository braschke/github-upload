#ifndef __memsum2p_double_SOURCE_INCLUDED
#define __memsum2p_double_SOURCE_INCLUDED
/* memsum2p_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all values of array product buf1*buf2 and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: memsum2p_double.c 941 2013-05-24 15:56:43Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double memsum2p_double(const double *buf1, const double *buf2, size_t n)
{
   double sum = 0.0;
   size_t i;
   for(i=0; i<n; i++)
   {
      sum += buf1[i]*buf2[i];
   }
   return sum;
}
#endif
