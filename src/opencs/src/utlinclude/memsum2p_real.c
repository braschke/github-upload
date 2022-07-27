#pragma once
#ifndef memsum2p_real_SOURCE_INCLUDED
#define memsum2p_real_SOURCE_INCLUDED
/* memsum2p_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all values of array product buf1*buf2 and return the sum
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: memsum2p_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "memsum2p_double.c"
#else
   #include "memsum2p_float.c"
#endif

#endif
