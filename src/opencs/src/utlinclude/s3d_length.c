#ifndef s3d_length_SOURCE_INCLUDED
#define s3d_length_SOURCE_INCLUDED
/* s3d_length.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    vector length
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3d_length.c 2482 2014-01-14 13:11:07Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX double s3d_length(const double x, const double y, const double z)
{
   return S3_LENGTHD(x,y,z);
}
#endif
