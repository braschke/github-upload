#pragma once
#ifndef v3r_angle_SOURCE_INCLUDED
#define v3r_angle_SOURCE_INCLUDED
/* v3r_angle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    true signed angle (0..PI) between 2 oriented vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3r_angle.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "v3d_angle.c"
#else
   #include "v3f_angle.c"
#endif

#endif
