#pragma once
#ifndef s3r_sangle_SOURCE_INCLUDED
#define s3r_sangle_SOURCE_INCLUDED
/* s3r_sangle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Smallest angle (0..PI/2) between 2 vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: s3r_sangle.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "s3d_sangle.c"
#else
   #include "s3f_sangle.c"
#endif

#endif
