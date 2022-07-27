#ifndef s3d_spat_SOURCE_INCLUDED
#define s3d_spat_SOURCE_INCLUDED
/* s3d_spat.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    spat product
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3d_spat.c 2480 2014-01-14 08:16:33Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double s3d_spat
(
   const double ux, const double uy, const double uz,
   const double vx, const double vy, const double vz,
   const double wx, const double wy, const double wz
)
{
   return S_DET3(ux,uy,uz, vx,vy,vz, wx,wy,wz);
}
#endif
