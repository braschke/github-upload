#ifndef s3f_length_SOURCE_INCLUDED
#define s3f_length_SOURCE_INCLUDED
/* s3f_length.c
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
 *    $Id: s3f_length.c 2482 2014-01-14 13:11:07Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX float s3f_length(const float x, const float y, const float z)
{
   return S3_LENGTHF(x,y,z);
}
#endif
