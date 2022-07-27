#pragma once
#ifndef vvsumabs_real_SOURCE_INCLUDED
#define vvsumabs_real_SOURCE_INCLUDED
/* vvsumabs_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all fabs(values) of array v[n*vdim] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vvsumabs_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vvsumabs_double.c"
#else
   #include "vvsumabs_float.c"
#endif

#endif
