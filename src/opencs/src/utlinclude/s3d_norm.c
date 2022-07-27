#pragma once
#ifndef s3d_norm_SOURCE_INCLUDED
#define s3d_norm_SOURCE_INCLUDED
/* s3d_norm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Normalize a vector
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3d_norm.c 5535 2017-08-25 18:00:07Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int s3d_norm(double *x, double *y, double *z)
{
   double len2 = S3_LENGTH2(*x,*y,*z);

   if (len2 > 0.0)
   {
      double qlen = 1.0/sqrt(len2);
      *x *= qlen;
      *y *= qlen;
      *z *= qlen;
      return 0;
   }

   *x = *y = *z = 0.0;
   return -1;
}
#endif
