#pragma once
#ifndef vvsum_real_SOURCE_INCLUDED
#define vvsum_real_SOURCE_INCLUDED
/* vvsum_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all values of array v[n*vdim] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vvsum_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vvsum_double.c"
#else
   #include "vvsum_float.c"
#endif

#endif
