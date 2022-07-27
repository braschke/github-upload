#pragma once
#ifndef v3d_anglew_SOURCE_INCLUDED
#define v3d_anglew_SOURCE_INCLUDED
/* v3d_anglew.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    see v3d_angle.c, here we warn in case of invalid vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3d_anglew.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include <math.h>

C_FUNC_PREFIX
double v3d_anglew(double u[3], double v[3])
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

   XMSG_WARNING6
   (
      "** v3d_anglew(%g/%g/%g, %g/%g/%g): Invalid vectors supplied.\n",
      u[0],u[1],u[2],v[0],v[1],v[2]
   );
   return 0.0;
}
#endif
