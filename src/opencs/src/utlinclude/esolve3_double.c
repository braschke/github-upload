#ifndef esolve3_double_SOURCE_INCLUDED
#define esolve3_double_SOURCE_INCLUDED
/* esolve3_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Three equations A[3][3] * x[3] = b[3] direct solver. This solver does a simple
 *    explicit matrix inversion (no iteration) and then  x = A^-1 * b.
 *
 *    Returns the determinant of A: DET(A) == 0 is error.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/03: Carsten Dehning, Initial release
 *    $Id: esolve3_double.c 1593 2013-09-27 15:52:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define A11 A[0]
#define A12 A[1]
#define A13 A[2]
#define A21 A[3]
#define A22 A[4]
#define A23 A[5]
#define A31 A[6]
#define A32 A[7]
#define A33 A[8]

C_FUNC_PREFIX
double esolve3_double(const double *A, const double b[3], double x[3])
{
   double det = S_DET3(A11,A12,A13,
                       A21,A22,A23,
                       A31,A32,A33);

   if (det)
   {
      double qdet = 1.0/det;

      x[0] =  (
                  S_DET2(A22,A23,
                         A32,A33) * b[0]
                + S_DET2(A13,A12,
                         A33,A32) * b[1]
                + S_DET2(A12,A13,
                         A22,A23) * b[2]
               ) * qdet;

      x[1] =  (
                  S_DET2(A23,A21,
                         A33,A31) * b[0]
                + S_DET2(A11,A13,
                         A31,A33) * b[1]
                + S_DET2(A13,A11,
                         A23,A21) * b[2]
               ) * qdet;

      x[2] =  (
                  S_DET2(A21,A22,
                         A31,A32) * b[0]
                + S_DET2(A12,A11,
                         A32,A31) * b[1]
                + S_DET2(A11,A12,
                         A21,A22) * b[2]
               ) * qdet;
   }
   else
   {
      x[0] = x[1] = x[2] = 0;
   }
   return det;
}

#undef A11
#undef A12
#undef A13
#undef A21
#undef A22
#undef A23
#undef A31
#undef A32
#undef A33

#endif
