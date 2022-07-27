#pragma once
#ifndef tmat44_HEADER_INCLUDED
#define tmat44_HEADER_INCLUDED
/* tmat44.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Utilities for homogeneous coordinates/vector transformations based on a 4x4 matrix.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Mar: Carsten Dehning, Initial release
 *    $Id: tmat44.h 4592 2016-06-01 13:15:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TMAT44
{
   real  a11, a12, a13, a14;
   real  a21, a22, a23, a24;
   real  a31, a32, a33, a34;
   real  a41, a42, a43, a44;
} TMAT44;



#define TMAT44_TRANS_V1(type,tm,vect)\
{\
   vect[0] *= tm->a11;\
}

#define TMAT44_TRANS_V2(type,tm,vect)\
{\
   type _v0 = vect[0];\
   type _v1 = vect[1];\
   vect[0] = tm->a11*_v0 + tm->a12*_v1;\
   vect[1] = tm->a21*_v0 + tm->a22*_v1;\
}

#define TMAT44_TRANS_V3(type,tm,vect)\
{\
   type _v0 = vect[0];\
   type _v1 = vect[1];\
   type _v2 = vect[2];\
   vect[0] = tm->a11*_v0 + tm->a12*_v1 + tm->a13*_v2;\
   vect[1] = tm->a21*_v0 + tm->a22*_v1 + tm->a23*_v2;\
   vect[2] = tm->a31*_v0 + tm->a32*_v1 + tm->a33*_v2;\
}


#define TMAT44_TRANS_C1(type,tm,coor)\
{\
   coor[0] = tm->a11*coor[0] + tm->a14;\
}

#define TMAT44_TRANS_C2(type,tm,coor)\
{\
   type _c0 = coor[0];\
   type _c1 = coor[1];\
   coor[0] = tm->a11*_c0 + tm->a12*_c1  +  tm->a14;\
   coor[1] = tm->a21*_c0 + tm->a22*_c1  +  tm->a24;\
}

#define TMAT44_TRANS_C3(type,tm,coor)\
{\
   type _c0 = coor[0];\
   type _c1 = coor[1];\
   type _c2 = coor[2];\
   coor[0] = tm->a11*_c0 + tm->a12*_c1 + tm->a13*_c2  +  tm->a14;\
   coor[1] = tm->a21*_c0 + tm->a22*_c1 + tm->a23*_c2  +  tm->a24;\
   coor[2] = tm->a31*_c0 + tm->a32*_c1 + tm->a33*_c2  +  tm->a34;\
}

#ifndef    TMAT44_USE_PRINT
   #define TMAT44_USE_PRINT   0
#endif

#ifndef    TMAT44_USE_DET
   #define TMAT44_USE_DET     0
#endif

#ifndef    TMAT44_USE_SCALE
   #define TMAT44_USE_SCALE   0
#endif

#ifndef    TMAT44_USE_SWAP12
   #define TMAT44_USE_SWAP12  0
#endif

#ifndef    TMAT44_USE_INVERT
   #define TMAT44_USE_INVERT  0
#endif

#if !INCLUDE_STATIC
   extern real    TMAT44_det     (const TMAT44 *m);
   extern void    TMAT44_print   (const TMAT44 *m);
   extern TMAT44 *TMAT44_ident   (TMAT44 *m);
   extern TMAT44 *TMAT44_scale   (TMAT44 *m, real scale);
   extern TMAT44 *TMAT44_trans   (TMAT44 *m, real tx, real ty, real tz);
   extern TMAT44 *TMAT44_swap12  (TMAT44 *m);
   extern TMAT44 *TMAT44_mmult   (TMAT44 *m, const TMAT44 *ml, const TMAT44 *mr);
   extern TMAT44 *TMAT44_multr   (TMAT44 *m, const TMAT44 *mr);
   extern TMAT44 *TMAT44_multl   (TMAT44 *m, const TMAT44 *ml);
   extern TMAT44 *TMAT44_invert  (TMAT44 *minv, const TMAT44 *m);
   extern TMAT44 *TMAT44_rotax   (TMAT44 *m, double phi, const double axis[3], const double orig[3]);
#endif

#ifdef __cplusplus
}
#endif

#endif
