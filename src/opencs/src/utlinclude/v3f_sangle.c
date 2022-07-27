#pragma once
#ifndef v3f_sangle_SOURCE_INCLUDED
#define v3f_sangle_SOURCE_INCLUDED
/* v3f_sangle.c
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
 *    $Id: v3f_sangle.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
float v3f_sangle(float u[3], float v[3])
{
   float tmp = V3_LENGTH2(u) * V3_LENGTH2(v);

   if (tmp > 0.0f)
   {
      /* tmp = cos(phi) */
      tmp = V3_SPROD(u,v) / sqrtf(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      if (tmp < 0.0f) tmp = -tmp;
      return (tmp < 1.0f) ? acosf(tmp) : 0.0f;
   }
   return -1.0f; /* signal any zero length vector with a negative angle */
}
#endif
