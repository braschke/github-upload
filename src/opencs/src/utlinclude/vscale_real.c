#pragma once
#ifndef vscale_real_SOURCE_INCLUDED
#define vscale_real_SOURCE_INCLUDED
/* vscale_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale values:  V *= scale
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: vscale_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "vscale_double.c"
#else
   #include "vscale_float.c"
#endif

#endif
