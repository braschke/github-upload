#pragma once
#ifndef vvshift_real_SOURCE_INCLUDED
#define vvshift_real_SOURCE_INCLUDED
/* vvshift_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    shift all values of array v[n*vdim] by shift[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vvshift_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vvshift_double.c"
#else
   #include "vvshift_float.c"
#endif

#endif
