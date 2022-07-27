#pragma once
#ifndef vsum_real_SOURCE_INCLUDED
#define vsum_real_SOURCE_INCLUDED
/* vsum_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all values of array v[n] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vsum_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vsum_double.c"
#else
   #include "vsum_float.c"
#endif

#endif
