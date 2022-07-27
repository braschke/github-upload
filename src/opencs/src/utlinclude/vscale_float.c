#pragma once
#ifndef vscale_float_SOURCE_INCLUDED
#define vscale_float_SOURCE_INCLUDED
/* vscale_float.c
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
 *    $Id: vscale_float.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *vscale_float(float *v, const int n, const float scale)
{
   if (scale != 1.0f)
   {
      int i;
      for(i=0; i<n; i++) v[i] *= scale;
   }
   return v;
}
#endif
