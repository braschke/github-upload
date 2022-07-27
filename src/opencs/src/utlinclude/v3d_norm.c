#pragma once
#ifndef v3d_norm_SOURCE_INCLUDED
#define v3d_norm_SOURCE_INCLUDED
/* v3d_norm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    normalize a vector
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3d_norm.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int v3d_norm(double v[3])
{
   double len2 = V3_LENGTH2(v);

   if (len2 > 0.0)
   {
      double qlen = 1.0/sqrt(len2);
      v[0] *= qlen;
      v[1] *= qlen;
      v[2] *= qlen;
      return 0;
   }

   v[0] = v[1] = v[2] = 0.0;
   return -1;
}
#endif
