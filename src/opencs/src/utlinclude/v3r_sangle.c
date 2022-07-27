#pragma once
#ifndef v3r_sangle_SOURCE_INCLUDED
#define v3r_sangle_SOURCE_INCLUDED
/* v3r_sangle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    see v3r_sangle.c, here we warn in case of invalid vectors
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3r_sangle.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "v3d_sangle.c"
#else
   #include "v3f_sangle.c"
#endif

#endif
