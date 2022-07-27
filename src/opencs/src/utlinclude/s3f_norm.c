#ifndef __s3f_norm_SOURCE_INCLUDED
#define __s3f_norm_SOURCE_INCLUDED
/* s3f_norm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    normalize a vector
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3f_norm.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int s3f_norm(float *x, float *y, float *z)
{
   float len2 = S3_LENGTH2(*x,*y,*z);

   if (len2 > 0.0f)
   {
      float qlen = 1.0f/sqrtf(len2);
      *x *= qlen;
      *y *= qlen;
      *z *= qlen;
      return 0;
   }

   *x = *y = *z = 0.0f;
   return -1;
}
#endif
