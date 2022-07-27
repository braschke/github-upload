#pragma once
#ifndef vlintr_real_SOURCE_INCLUDED
#define vlintr_real_SOURCE_INCLUDED
/* vlintr_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale values:  V = scale*V + offset
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: vlintr_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vlintr_double.c"
#else
   #include "vlintr_float.c"
#endif

#endif
