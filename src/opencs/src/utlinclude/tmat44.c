#pragma once
#ifndef tmat44_SOURCE_INCLUDED
#define tmat44_SOURCE_INCLUDED
/* tmat44.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Utilities for homogeneous coordinates/vector transformations
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Mar: Carsten Dehning, Initial release
 *    $Id: tmat44.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#define TMAT44_TEST 0

#if TMAT44_TEST
   #define TMAT44_USE_DET     1
   #define TMAT44_USE_PRINT   1
   #define REAL_IS_DOUBLE     1
#endif

#include "tmat44.h"


#if TMAT44_USE_DET
/****************************************************************************************/
C_FUNC_PREFIX real TMAT44_det(const TMAT44 *m)
/****************************************************************************************/
{
   return (real)S_DET3
                (
                  m->a11, m->a12, m->a13,
                  m->a21, m->a22, m->a23,
                  m->a31, m->a32, m->a33
                );
}

/****************************************************************************************/
#endif

#if TMAT44_USE_PRINT
/****************************************************************************************/
C_FUNC_PREFIX void TMAT44_print(const TMAT44 *m)
/****************************************************************************************/
{
   printf
   (
      "TMAT44: %12g, %12g, %12g, %12g\n"
      "        %12g, %12g, %12g, %12g\n"
      "        %12g, %12g, %12g, %12g\n"
      "        %12g, %12g, %12g, %12g\n"
      "DET33 : %12g\n",
      m->a11,   m->a12,   m->a13,   m->a14,
      m->a21,   m->a22,   m->a23,   m->a24,
      m->a31,   m->a32,   m->a33,   m->a34,
      m->a41,   m->a42,   m->a43,   m->a44,
      TMAT44_det(m)
   );
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_ident(TMAT44 *m)
/****************************************************************************************/
/*
 * identity matrix
 *    | 1 0 0 0 |
 *    | 0 1 0 0 |
 *    | 0 0 1 0 |
 *    | 0 0 0 1 |
 */
{
   MEMZERO(m,sizeof(TMAT44));
   m->a11 =
   m->a22 =
   m->a33 =
   m->a44 = 1;
   return m;
}

/****************************************************************************************/

#if TMAT44_USE_SCALE
/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_scale(TMAT44 *m, real scale)
/****************************************************************************************/
/*
 * scale matrix
 *    | s 0 0 0 |
 *    | 0 s 0 0 |
 *    | 0 0 s 0 |
 *    | 0 0 0 1 |
 */
{
   MEMZERO(m,sizeof(TMAT44));
   m->a11 =
   m->a22 =
   m->a33 = scale;
   m->a44 = 1;
   return m;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_trans(TMAT44 *m, real tx, real ty, real tz)
/****************************************************************************************/
/*
 * translation matrix
 *    | 1 0 0 x |
 *    | 0 1 0 y |
 *    | 0 0 1 z |
 *    | 0 0 0 1 |
 */
{
   TMAT44_ident(m);
   m->a14 = tx;
   m->a24 = ty;
   m->a34 = tz;
   return m;
}

/****************************************************************************************/

#if TMAT44_USE_SWAP12
/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_swap12(TMAT44 *m)
/****************************************************************************************/
/*
 * matrix to swap x and y values in a x/y/z vector
 *    | 0 1 0 0 |
 *    | 1 0 0 0 |
 *    | 0 0 1 0 |
 *    | 0 0 0 1 |
 */
{
   TMAT44_ident(m);
   m->a11 =
   m->a22 = 0;
   m->a12 =
   m->a21 = 1;
   return m;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_mmult(TMAT44 *m, const TMAT44 *ml, const TMAT44 *mr)
/****************************************************************************************/
/*
 * matrix multiply:  m = ml*mr;
 */
{
   m->a11 = ml->a11*mr->a11 + ml->a12*mr->a21 + ml->a13*mr->a31 + ml->a14*mr->a41;
   m->a12 = ml->a11*mr->a12 + ml->a12*mr->a22 + ml->a13*mr->a32 + ml->a14*mr->a42;
   m->a13 = ml->a11*mr->a13 + ml->a12*mr->a23 + ml->a13*mr->a33 + ml->a14*mr->a43;
   m->a14 = ml->a11*mr->a14 + ml->a12*mr->a24 + ml->a13*mr->a34 + ml->a14*mr->a44;

   m->a21 = ml->a21*mr->a11 + ml->a22*mr->a21 + ml->a23*mr->a31 + ml->a24*mr->a41;
   m->a22 = ml->a21*mr->a12 + ml->a22*mr->a22 + ml->a23*mr->a32 + ml->a24*mr->a42;
   m->a23 = ml->a21*mr->a13 + ml->a22*mr->a23 + ml->a23*mr->a33 + ml->a24*mr->a43;
   m->a24 = ml->a21*mr->a14 + ml->a22*mr->a24 + ml->a23*mr->a34 + ml->a24*mr->a44;

   m->a31 = ml->a31*mr->a11 + ml->a32*mr->a21 + ml->a33*mr->a31 + ml->a34*mr->a41;
   m->a32 = ml->a31*mr->a12 + ml->a32*mr->a22 + ml->a33*mr->a32 + ml->a34*mr->a42;
   m->a33 = ml->a31*mr->a13 + ml->a32*mr->a23 + ml->a33*mr->a33 + ml->a34*mr->a43;
   m->a34 = ml->a31*mr->a14 + ml->a32*mr->a24 + ml->a33*mr->a34 + ml->a34*mr->a44;

   m->a41 = ml->a41*mr->a11 + ml->a42*mr->a21 + ml->a43*mr->a31 + ml->a44*mr->a41;
   m->a42 = ml->a41*mr->a12 + ml->a42*mr->a22 + ml->a43*mr->a32 + ml->a44*mr->a42;
   m->a43 = ml->a41*mr->a13 + ml->a42*mr->a23 + ml->a43*mr->a33 + ml->a44*mr->a43;
   m->a44 = ml->a41*mr->a14 + ml->a42*mr->a24 + ml->a43*mr->a34 + ml->a44*mr->a44;

   return m;
}

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_multr(TMAT44 *m, const TMAT44 *mr)
/****************************************************************************************/
/*
 * matrix right multiply:  m = m*mr;
 */
{
   TMAT44 tmp = *m;
   return TMAT44_mmult(m,&tmp,mr);
}

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_multl(TMAT44 *m, const TMAT44 *ml)
/****************************************************************************************/
/*
 * matrix left multiply:  m = ml*m;
 */
{
   TMAT44 tmp = *m;
   return TMAT44_mmult(m,ml,&tmp);
}

/****************************************************************************************/

#if TMAT44_USE_INVERT
/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_invert(TMAT44 *minv, const TMAT44 *m)
/****************************************************************************************/
/*
 * Invert the matrix: minv = (m)^-1
 */
{
   /* rotation == transposed */
   minv->a11 =  m->a11;
   minv->a12 =  m->a21;
   minv->a21 =  m->a12;

   minv->a22 =  m->a22;
   minv->a13 =  m->a31;
   minv->a31 =  m->a13;

   minv->a33 =  m->a33;
   minv->a23 =  m->a32;
   minv->a32 =  m->a23;

   /* translation == negated */
   minv->a14 = -m->a14;
   minv->a24 = -m->a24;
   minv->a34 = -m->a34;
   return minv;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX TMAT44 *TMAT44_rotax
(
   TMAT44        *m,
         double   phi,
   const double   axis[3],
   const double   orig[3]
)
/****************************************************************************************/
/*
 * Rotation matrix for arbitrary 'axis' and angle 'phi' with origin 'orig'.
 *
 *    M = T(orig) * R(axis, phi) * T(-orig)
 */
{
   double len,sinphi,cosphi,x,y,z,qc;


   TMAT44_ident(m);
   if (phi && (len=V3_LENGTH2(axis)) > 0.0)
   {
      len    = 1.0/sqrt(len);
      x      = axis[0]*len;
      y      = axis[1]*len;
      z      = axis[2]*len;
      sinphi = sin(phi);
      cosphi = cos(phi);
      qc     = 1.0 - cosphi;

      m->a11 = (real)( x*x       +  cosphi * (1.0 - x*x) );
      m->a12 = (real)( x*y * qc  -  sinphi * z           );
      m->a13 = (real)( x*z * qc  +  sinphi * y           );

      m->a21 = (real)( y*x * qc  +  sinphi * z           );
      m->a22 = (real)( y*y       +  cosphi * (1.0 - y*y) );
      m->a23 = (real)( y*z * qc  -  sinphi * x           );

      m->a31 = (real)( z*x * qc  -  sinphi * y           );
      m->a32 = (real)( z*y * qc  +  sinphi * x           );
      m->a33 = (real)( z*z       +  cosphi * (1.0 - z*z) );

      if (orig)
      {
         /* m = T(orig) * m * T(-orig) */
         TMAT44 tmp;
         TMAT44_multl(m,TMAT44_trans(&tmp,(real) orig[0],(real) orig[1],(real) orig[2]));
         TMAT44_multr(m,TMAT44_trans(&tmp,(real)-orig[0],(real)-orig[1],(real)-orig[2]));
      }
   }

#if TMAT44_TEST
   printf
   (
      "TMAT44_rotax()\n"
      "ANGLE : %12g\n"
      "AXIS  : %12g, %12g, %12g\n"
      "ORIGIN: %12g, %12g, %12g\n",
      phi,
      axis[0],axis[1],axis[2],
      orig[0],orig[1],orig[2]
   );
   TMAT44_print(m);
#endif

   return m;
}

/****************************************************************************************/

#if TMAT44_TEST
int main(void)
{
   TMAT44 m1,m2;
   double axis[3],orig[3];

   axis[0] = 1.0;
   axis[1] = 1.0;
   axis[2] = 1.0;

   orig[0] = 1.0;
   orig[1] = 2.0;
   orig[2] = 3.0;

   printf("\n**SWAP12**\n");
   TMAT44_print(TMAT44_swap12(&m2));

   TMAT44_rotax(&m1,1.5,axis,orig);
   TMAT44_multr(&m1,TMAT44_trans(&m2,1,1,1));
   TMAT44_multr(&m1,TMAT44_scale(&m2,1));
   TMAT44_multr(&m1,TMAT44_swap12(&m2));
   TMAT44_multr(&m1,TMAT44_rotax(&m2,-1.5,axis,orig));
   m1 = *TMAT44_invert(&m2,&m1);

   printf("\n**FINAL**\n");
   TMAT44_print(&m1);
   return 0;
}
#endif

#endif
