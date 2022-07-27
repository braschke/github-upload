#pragma once
#ifndef esolve2_real_SOURCE_INCLUDED
#define esolve2_real_SOURCE_INCLUDED
/* esolve2_real.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Two equations A[2][2] * x[2] = b[2] direct solver. This solver does a simple
 *    explicit matrix inversion (no iteration) and then  x = A^-1 * b.
 *
 *    Returns the determinant of A: DET(A) == 0 is error.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: esolve2_real.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "esolve2_double.c"
#else
   #include "esolve2_float.c"
#endif

#endif
