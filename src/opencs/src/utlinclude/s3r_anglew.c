#pragma once
#ifndef s3r_anglew_SOURCE_INCLUDED
#define s3r_anglew_SOURCE_INCLUDED
/* s3r_anglew.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    see s3r_anglew.c, here we warn in case of invalid vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3r_anglew.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "s3d_anglew.c"
#else
   #include "s3f_anglew.c"
#endif

#endif
