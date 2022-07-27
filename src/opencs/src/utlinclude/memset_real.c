#pragma once
#ifndef memset_real_SOURCE_INCLUDED
#define memset_real_SOURCE_INCLUDED
/* memset_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memset(), but with real's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memset_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "memset_double.c"
#else
   #include "memset_float.c"
#endif

#endif
