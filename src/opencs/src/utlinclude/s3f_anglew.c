#ifndef s3f_anglew_SOURCE_INCLUDED
#define s3f_anglew_SOURCE_INCLUDED
/* s3f_anglew.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    see s3f_angle.c, here we warn in case of invalid vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3f_anglew.c 2478 2014-01-14 07:51:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include <math.h>

C_FUNC_PREFIX
float s3f_anglew
(
   const float ux,
   const float uy,
   const float uz,
   const float vx,
   const float vy,
   const float vz
)
{
   float tmp = S3_LENGTH2(ux,uy,uz) * S3_LENGTH2(vx,vy,vz);

   if (tmp > 0)
   {
      /* tmp = cos(phi) */
      tmp = S3_SPROD(ux,uy,uz, vx,vy,vz) / sqrtf(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      return
      (
         (tmp >=  1) ? 0           :
         (tmp <= -1) ? (float)M_PI :
         acosf(tmp)
      );
   }

   XMSG_WARNING6
   (
      "** s3f_anglew(%g/%g/%g, %g/%g/%g): Invalid vectors supplied.\n",
      ux,uy,uz, vx,vy,vz
   );
   return 0;
}
#endif
