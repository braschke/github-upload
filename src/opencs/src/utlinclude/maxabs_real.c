#pragma once
#ifndef maxabs_real_SOURCE_INCLUDED
#define maxabs_real_SOURCE_INCLUDED
/* maxabs_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the maximum of the two absolute values.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2009/Jan/28: Carsten Dehning, Initial release
 *    $Id: maxabs_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "maxabs_double.c"
#else
   #include "maxabs_float.c"
#endif

#endif
