#ifndef esolve2_double_SOURCE_INCLUDED
#define esolve2_double_SOURCE_INCLUDED
/* esolve2_double.c
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
 *    $Id: esolve2_double.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define A11 A[0]
#define A12 A[1]
#define A21 A[2]
#define A22 A[3]

C_FUNC_PREFIX
double esolve2_double(const double *A, const double b[2], double x[2])
{
   double det = S_DET2(
                        A11,A12,
                        A21,A22
                      );

   if (det)
   {
      double qdet = 1.0/det;
      x[0] = (A22*b[0] - A12*b[1]) * qdet;
      x[1] = (A11*b[1] - A21*b[0]) * qdet;
   }
   return det;
}

#undef A11
#undef A12
#undef A21
#undef A22

#endif
