#pragma once
#ifndef v3f_anglew_SOURCE_INCLUDED
#define v3f_anglew_SOURCE_INCLUDED
/* v3f_anglew.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    see v3f_angle.c, here we warn in case of invalid vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3f_anglew.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include <math.h>

C_FUNC_PREFIX
float v3f_anglew(float u[3], float v[3])
{
   float tmp = V3_LENGTH2(u) * V3_LENGTH2(v);

   if (tmp > 0.0f)
   {
      /* tmp = cos(phi) */
      tmp = V3_SPROD(u,v) / sqrtf(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      return
      (
         (tmp >=  1.0f) ? 0.0f :
         (tmp <= -1.0f) ? (float)M_PI :
         acosf(tmp)
      );
   }

   XMSG_WARNING6
   (
      "** v3f_anglew(%g/%g/%g, %g/%g/%g): Invalid vectors supplied.\n",
      u[0],u[1],u[2],v[0],v[1],v[2]
   );
   return 0.0f;
}
#endif
