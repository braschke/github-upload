#pragma once
#ifndef v3d_angle_SOURCE_INCLUDED
#define v3d_angle_SOURCE_INCLUDED
/* v3d_angle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    true signed angle (0..PI) between 2 oriented vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3d_angle.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
double v3d_angle(double u[3], double v[3])
{
   double tmp = V3_LENGTH2(u) * V3_LENGTH2(v);

   if (tmp > 0.0)
   {
      /* tmp = cos(phi) */
      tmp = V3_SPROD(u,v) / sqrt(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      return
      (
         (tmp >=  1.0) ? 0.0  :
         (tmp <= -1.0) ? M_PI :
         acos(tmp)
      );
   }
   return -1.0; /* signal any zero length vector with a negative angle */
}
#endif
