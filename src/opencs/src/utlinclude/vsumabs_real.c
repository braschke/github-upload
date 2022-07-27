#pragma once
#ifndef vsumabs_real_SOURCE_INCLUDED
#define vsumabs_real_SOURCE_INCLUDED
/* vsumabs_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all fabs(values) of array v[n] and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vsumabs_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vsumabs_double.c"
#else
   #include "vsumabs_float.c"
#endif

#endif
