#ifndef s3d_cross_SOURCE_INCLUDED
#define s3d_cross_SOURCE_INCLUDED
/* s3d_cross.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Cross product u x v
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3d_cross.c 3081 2014-08-12 14:03:30Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void s3d_cross
(
   double  ux, double  uy, double  uz,
   double  vx, double  vy, double  vz,
   double *xr, double *yr, double *zr
)
{
   S3_CROSS(ux,uy,uz, vx,vy,vz, *xr,*yr,*zr);
}

#endif
