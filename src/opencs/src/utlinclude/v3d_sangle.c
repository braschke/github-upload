#pragma once
#ifndef v3d_sangle_SOURCE_INCLUDED
#define v3d_sangle_SOURCE_INCLUDED
/* v3d_sangle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    smallest angle (0..PI/2) between 2 vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3d_sangle.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
double v3d_sangle(double u[3], double v[3])
{
   double tmp = V3_LENGTH2(u) * V3_LENGTH2(v);

   if (tmp > 0.0)
   {
      /* tmp = cos(phi) */
      tmp = V3_SPROD(u,v) / sqrt(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      if (tmp < 0.0) tmp = -tmp;
      return (tmp < 1.0) ? acos(tmp) : 0.0;
   }
   return -1.0; /* signal any zero length vector with a negative angle */
}
#endif
