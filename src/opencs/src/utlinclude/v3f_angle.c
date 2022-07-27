#pragma once
#ifndef v3f_angle_SOURCE_INCLUDED
#define v3f_angle_SOURCE_INCLUDED
/* v3f_angle.c
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
 *    $Id: v3f_angle.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
float v3f_angle(float u[3], float v[3])
{
   float tmp = V3_LENGTH2(u) * V3_LENGTH2(v);

   if (tmp > 0.0f)
   {
      /* tmp = cos(phi) */
      tmp = V3_SPROD(u,v) / sqrtf(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      return
      (
         (tmp >=  1.0f) ? 0.0f  :
         (tmp <= -1.0f) ? (float)M_PI :
         acosf(tmp)
      );
   }
   return -1.0f; /* signal any zero length vector with a negative angle */
}
#endif
