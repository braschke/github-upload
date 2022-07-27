#pragma once
#ifndef nancheck_real_SOURCE_INCLUDED
#define nancheck_real_SOURCE_INCLUDED
/* nancheck_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test a float/double array for nan's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Feb/07: Carsten Dehning, Initial release
 *    $Id: nancheck_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if REAL_IS_DOUBLE
   #include "nancheck_double.c"
#else
   #include "nancheck_float.c"
#endif

#endif
