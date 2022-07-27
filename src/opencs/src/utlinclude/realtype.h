#pragma once
#ifndef REALTYPE_DEFINED
#define REALTYPE_DEFINED
/* realtype.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Template to define the 'real' type and REAL_MAX
 *
 *    This file may be copied to a local directory and modified and then
 *    #included (base on the cc -I search rules) instead of this template version.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: realtype.h 5691 2017-11-22 12:29:26Z dehning $
 *
 *****************************************************************************************
 */
#ifndef DOUBLE_PRECISION
   #error DOUBLE_PRECISION not defined: must be 0(=float) or 1(=double)
#endif

#include <float.h>
#include <math.h>


#if DOUBLE_PRECISION

   typedef double real;

   #define REAL_TYPENAME   "double"
   #define REAL_TYPECHAR   "d"
   #define REAL_IS_DOUBLE  1

   #ifdef DBL_MAX
      #define REAL_MAX  DBL_MAX
   #else
      #define REAL_MAX  1.0e100 /* unclear, but large enought */
   #endif

   /* avoid compiler complaints about double/float conversion */
   #define REAL_LARGE      1.0e100
   #define REAL_ZERO       0.0
   #define REAL_ONE        1.0
   #define REAL_TWO        2.0
   #define REAL_THREE      3.0
   #define REAL_FOUR       4.0
   #define REAL_FIVE       5.0
   #define REAL_TEN       10.0
   #define REAL_HALF       0.5
   #define REAL_THIRD      (1.0/3.0)
   #define REAL_QUARTER    0.25

   /* real versions == double versions */
   #define fabsr  fabs
   #define sqrtr  sqrt
   #define cbrtr  cbrt
   #define acosr  acos
   #define sinr   sin
   #define cosr   cos
   #define atan2r atan2
   #define powr   pow

#else

   typedef float real;

   #define REAL_TYPENAME   "single"
   #define REAL_TYPECHAR   "s"
   #define REAL_IS_DOUBLE  0

   #ifdef FLT_MAX
      #define REAL_MAX  FLT_MAX
   #else
      #define REAL_MAX  1.175494351E38f /* assume IEEE format */
   #endif

   /* avoid compiler complaints about double/float conversion */
   #define REAL_LARGE      1.0e38f
   #define REAL_ZERO       0.0f
   #define REAL_ONE        1.0f
   #define REAL_TWO        2.0f
   #define REAL_THREE      3.0f
   #define REAL_FOUR       4.0f
   #define REAL_FIVE       5.0f
   #define REAL_TEN       10.0f
   #define REAL_HALF       0.5f
   #define REAL_THIRD      (1.0f/3.0f)
   #define REAL_QUARTER    0.25f

   /* real versions == float versions */
   #define fabsr  fabsf
   #define sqrtr  sqrtf
   #define cbrtr  cbrtf
   #define acosr  acosf
   #define sinr   sinf
   #define cosr   cosf
   #define atan2r atan2f
   #define powr   powf

#endif

#if 0 /* not yet really used */
typedef long double qreal;  /* sounds better and shorter */
#endif

#undef DOUBLE_PRECISION

#endif
