#ifndef s3f_spat_SOURCE_INCLUDED
#define s3f_spat_SOURCE_INCLUDED
/* s3f_spat.c
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
 *    $Id: s3f_spat.c 2480 2014-01-14 08:16:33Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float s3f_spat
(
   const float ux, const float uy, const float uz,
   const float vx, const float vy, const float vz,
   const float wx, const float wy, const float wz
)
{
   return S_DET3(ux,uy,uz, vx,vy,vz, wx,wy,wz);
}
#endif
