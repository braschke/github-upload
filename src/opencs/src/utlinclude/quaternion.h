#pragma once
#ifndef quaternion_HEADER_INCLUDED
#define quaternion_HEADER_INCLUDED
/* quaternion.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Utilities for quaternions.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2016/Aug: Carsten Dehning, Initial release
 *    $Id: quaternion.h 5191 2016-10-13 16:35:03Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Quaternion object. This structure may also be used as a compact real[4] array
 */
typedef struct _QUATERNION
{
   real w, x,y,z; /* w=real part + xyz=imaginary parts */
} QUATERNION;


#ifndef    QUATERNION_USE_PRINT
   #define QUATERNION_USE_PRINT   0
#endif


#define QUATERNION_ident(_q)           _q->w=_q->x =_q->y=_q->z=1
#define QUATERNION_dotproduct(_q1,_q2) _q1->w*_q2->w + _q1->x*_q2->x + _q1->y*_q2->y + _q1->z*_q2->z
#define QUATERNION_add(_qa,_q1,_q2)    _qa->w=_q1->w+q2->w; _qa->x=_q1->x+q2->x; _qa->y=_q1->y+q2->y; _qa->z=_q1->z+q2->z
#define QUATERNION_sub(_qs,_q1,_q2)    _qs->w=_q1->w-q2->w; _qs->x=_q1->x-q2->x; _qs->y=_q1->y-q2->y; _qs->z=_q1->z+q2->z

#define QUATERNION_costheta2           QUATERNION_dotproduct
#define QUATERNION_norm2(_q)           QUATERNION_dotproduct(_q,_q)
#define QUATERNION_negate(_qn,_q)      _qn->w=-_q->w; _qn->x=-_q->x; _qn->y=-_q->y; _qn->z=-_q->z
#define QUATERNION_conjugate(_qc,_q)   _qc->w= _q->w; _qc->x=-_q->x; _qc->y=-_q->y; _qc->z=-_q->z

#if !INCLUDE_STATIC
   extern QUATERNION *QUATERNION_print    (QUATERNION *q);
   extern QUATERNION *QUATERNION_normalize(QUATERNION *q);
   extern QUATERNION *QUATERNION_inverse  (QUATERNION *qi, const QUATERNION *q);
   extern QUATERNION *QUATERNION_qmult    (QUATERNION *qm, const QUATERNION *ql, const QUATERNION *qr);
   extern QUATERNION *QUATERNION_qmultr   (QUATERNION *qm, const QUATERNION *qr);
   extern QUATERNION *QUATERNION_qmultl   (QUATERNION *qm, const QUATERNION *ql);
   extern QUATERNION *QUATERNION_slerp    (QUATERNION *qs, const QUATERNION *ql, const QUATERNION *q2, const double t);
#endif

#ifdef __cplusplus
}
#endif

#endif
