#ifndef vlintr_double_SOURCE_INCLUDED
#define vlintr_double_SOURCE_INCLUDED
/* vlintr_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale values:  V = scale*V + offset
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: vlintr_double.c 3543 2015-11-06 18:59:52Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *vlintr_double(double *v, const int n, const double scale, const double offs)
{
   int i;

   if (offs)
   {
      if (scale != 1.0)
      {
         for(i=0; i<n; i++) v[i] = scale*v[i] + offs;
      }
      else
      {
         for(i=0; i<n; i++) v[i] += offs;
      }
   }
   else if (scale != 1.0)
   {
      for(i=0; i<n; i++) v[i] *= scale;
   }
   return v;
}
#endif
