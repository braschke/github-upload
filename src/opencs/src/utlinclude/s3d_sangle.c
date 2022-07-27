#ifndef s3d_sangle_SOURCE_INCLUDED
#define s3d_sangle_SOURCE_INCLUDED
/* s3d_sangle.c
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
 *    $Id: s3d_sangle.c 2478 2014-01-14 07:51:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

C_FUNC_PREFIX
double s3d_sangle
(
   const double ux,
   const double uy,
   const double uz,
   const double vx,
   const double vy,
   const double vz
)
{
   double tmp = S3_LENGTH2(ux,uy,uz) * S3_LENGTH2(vx,vy,vz);

   if (tmp > 0)
   {
      /* tmp = cos(phi) */
      tmp = S3_SPROD(ux,uy,uz, vx,vy,vz) / sqrt(tmp);

      /* handle rounding errors, don't catch a mathlib acos(1.1) NAN */
      if (tmp < 0) tmp = -tmp;
      return (tmp < 1) ? acos(tmp) : 0;
   }
   return -1; /* signal any zero length vector with a negative angle */
}
#endif
