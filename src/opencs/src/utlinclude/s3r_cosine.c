#ifndef __s3r_cosine_SOURCE_INCLUDED
#define __s3r_cosine_SOURCE_INCLUDED
/* s3r_cosine.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    return cosine of angle between two vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3r_cosine.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include <math.h>

C_FUNC_PREFIX
real s3r_cosine(real x1, real y1, real z1, real x2, real y2, real z2)
{
   real tmp = S3_LENGTH2(x1,y1,z1) * S3_LENGTH2(x2,y2,z2);

   return (tmp > 0.0) ? S3_SPROD(x1,y1,z1, x2,y2,z2) / sqrtr(tmp) : REAL_ZERO;
}
#endif
