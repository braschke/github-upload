#pragma once
#ifndef quaternion_SOURCE_INCLUDED
#define quaternion_SOURCE_INCLUDED
/* quaternion.c
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
 *    $Id: quaternion.c 5681 2017-11-10 12:30:48Z dehning $
 *
 *****************************************************************************************
 */
#include "quaternion.h"


#if QUATERNION_USE_PRINT
/****************************************************************************************/
C_FUNC_PREFIX QUATERNION * QUATERNION_print(QUATERNION *q)
/****************************************************************************************/
{
   printf
   (
      "QUATERNION: %12g, %12g, %12g, %12g\n"
      "NORM2     : %12g\n"
      ,q->w,q->x,q->y,q->z
      ,QUATERNION_norm2(q)
   );
   return q;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_normalize(QUATERNION *q)
/****************************************************************************************/
{
   const double norm2 = QUATERNION_norm2(q);

   if (norm2 > 0)
   {
      const real qnorm = (real)(1.0/sqrt(norm2));
      q->w *= qnorm;
      q->x *= qnorm;
      q->y *= qnorm;
      q->z *= qnorm;
   }
   return q;
}

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_inverse(QUATERNION *qi, const QUATERNION *q)
/****************************************************************************************/
/*
 * Quaternion inverse: qi = q-conjugate/norm(q);
 */
{
   const double norm2 = QUATERNION_norm2(q);

   QUATERNION_conjugate(qi,q);
   if (norm2 > 0)
   {
      const real qnorm2 = (real)(1.0/norm2);
      qi->w *= qnorm2;
      qi->x *= qnorm2;
      qi->y *= qnorm2;
      qi->z *= qnorm2;
   }
   return qi;
}

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_qmult(QUATERNION *qm, const QUATERNION *ql, const QUATERNION *qr)
/****************************************************************************************/
/*
 * Quaternion multiplication: q = ql*qr;
 */
{
   qm->w = ql->w*qr->w - ql->x*qr->x - ql->y*qr->y - ql->z*qr->z;
   qm->x = ql->w*qr->x + ql->x*qr->w + ql->y*qr->z - ql->z*qr->y;
   qm->y = ql->w*qr->y - ql->x*qr->z + ql->y*qr->w + ql->z*qr->x;
   qm->z = ql->w*qr->z + ql->x*qr->y - ql->y*qr->x + ql->z*qr->w;
   QUATERNION_normalize(qm);
   return qm;
}

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_qmultr(QUATERNION *qm, const QUATERNION *qr)
/****************************************************************************************/
/*
 * Quaternion right multiply: qm = qm*qr;
 */
{
   QUATERNION tmp = *qm;
   return QUATERNION_qmult(qm,&tmp,qr);
}

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_qmultl(QUATERNION *qm, const QUATERNION *ql)
/****************************************************************************************/
/*
 * Quaternion left multiply:  qm = ql*qm;
 */
{
   QUATERNION tmp = *qm;
   return QUATERNION_qmult(qm,ql,&tmp);
}


/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_slerp
(
         QUATERNION *qs,
   const QUATERNION *q1,
   const QUATERNION *q2,
   const double      ts
)
/****************************************************************************************/
/*
 * Quaternion slerp between q1 ... q2 and with 0 <= ts <= 1
 */
{
   if (ts > 0)
   {
      *qs = *q2; /* struct copy for all t > 0 */

      if (ts < 1)
      {
         /* Need to interpolate */
         double costheta2 = QUATERNION_costheta2(q1,qs);
         double s1,s2;

         if (costheta2 < 0)
         {
            costheta2 = -costheta2;
            qs->w     = -qs->w;
            qs->x     = -qs->x;
            qs->y     = -qs->y;
            qs->z     = -qs->z;
         }

         if (costheta2 > 0.99999)
         {
            /*
             * Do a linear interpolation, since sin(theta2) ~ 0.0
             * may result in division errors.
             */
            s1 = 1.0 - ts;
            s2 = ts;
         }
         else
         {
            /*
             * Do the spherical interpolation (slerp)
             */
            const double theta2  = acos(costheta2);
            const double qsinang = 1.0/sqrt(1.0 - costheta2*costheta2); /* 1.0/sin(theta2); */
            s1 = sin((1.0-ts) * theta2) * qsinang;
            s2 = sin((    ts) * theta2) * qsinang;
         }

         qs->w = (real)(s1*q1->w + s2*qs->w);
         qs->x = (real)(s1*q1->x + s2*qs->x);
         qs->y = (real)(s1*q1->y + s2*qs->y);
         qs->z = (real)(s1*q1->z + s2*qs->z);
         QUATERNION_normalize(qs);
      }
   }
   else /* t <= 0 */
   {
      *qs = *q1; /* struct copy */
   }

   return qs;
}

/****************************************************************************************/
C_FUNC_PREFIX QUATERNION *QUATERNION_squad
(
         QUATERNION *qs,
   const QUATERNION *q0,
   const QUATERNION *q1,
   const QUATERNION *q2,
   const QUATERNION *q3,
   const double      ts
)
/****************************************************************************************/
/*
 * Quaternion squad between q1 ... q2 and with 0 <= ts <= 1
 */
{
   if (ts > 0)
   {
      *qs = *q2; /* struct copy for all t > 0 */

      if (ts < 1)
      {
         /* Need to interpolate */
         QUATERNION qs12,qs03;

         QUATERNION_slerp(&qs12,q1,q2,ts);
         QUATERNION_slerp(&qs03,q0,q3,ts);
         QUATERNION_slerp(qs,&qs12,&qs03,2*ts*(1-ts));
      }
   }
   else /* t <= 0 */
   {
      *qs = *q1; /* struct copy */
   }

   return qs;
}

/****************************************************************************************/

#endif
