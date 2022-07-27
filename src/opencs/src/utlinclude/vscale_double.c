#pragma once
#ifndef vscale_double_SOURCE_INCLUDED
#define vscale_double_SOURCE_INCLUDED
/* vscale_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale values:  V *= scale
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: vscale_double.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *vscale_double(double *v, const int n, const double scale)
{
   if (scale != 1.0)
   {
      int i;
      for(i=0; i<n; i++) v[i] *= scale;
   }
   return v;
}
#endif
