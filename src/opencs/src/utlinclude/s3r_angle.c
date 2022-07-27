#pragma once
#ifndef s3r_angle_SOURCE_INCLUDED
#define s3r_angle_SOURCE_INCLUDED
/* s3r_angle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    True signed angle (0..PI) between 2 oriented vectors.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3r_angle.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "s3d_angle.c"
#else
   #include "s3f_angle.c"
#endif

#endif
