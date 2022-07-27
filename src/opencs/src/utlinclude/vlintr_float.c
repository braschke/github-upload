#ifndef vlintr_float_SOURCE_INCLUDED
#define vlintr_float_SOURCE_INCLUDED
/* vlintr_float.c
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
 *    $Id: vlintr_float.c 3543 2015-11-06 18:59:52Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *vlintr_float(float *v, const int n, const float scale, const float offs)
{
   int i;

   if (offs)
   {
      if (scale != 1.0f)
      {
         for(i=0; i<n; i++) v[i] = scale*v[i] + offs;
      }
      else
      {
         for(i=0; i<n; i++) v[i] += offs;
      }
   }
   else if (scale != 1.0f)
   {
      for(i=0; i<n; i++) v[i] *= scale;
   }
   return v;
}
#endif
