#ifndef s3f_sangle_SOURCE_INCLUDED
#define s3f_sangle_SOURCE_INCLUDED
/* s3f_sangle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Smallest angle (0..PI/2) between 2 vectors.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3f_sangle.c 2478 2014-01-14 07:51:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
float s3f_sangle
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
      if (tmp < 0) tmp = -tmp;
      return (tmp < 1) ? acosf(tmp) : 0;
   }
   return -1; /* signal any zero length vector with a negative angle */
}
#endif
