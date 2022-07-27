#pragma once
#ifndef vvscale_real_SOURCE_INCLUDED
#define vvscale_real_SOURCE_INCLUDED
/* vvscale_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale all values of array v[n*vdim] by scale[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vvscale_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vvscale_double.c"
#else
   #include "vvscale_float.c"
#endif

#endif
