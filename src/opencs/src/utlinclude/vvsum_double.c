#ifndef __vvsum_double_SOURCE_INCLUDED
#define __vvsum_double_SOURCE_INCLUDED
/* vvsum_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Sum all values of a packed array v[n*vdim] and return the vsum[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2013/May/22: Carsten Dehning, Initial release
 *    $Id: vvsum_double.c 946 2013-05-27 10:28:01Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "kahan_add.h"

C_FUNC_PREFIX
void vvsum_double(const double *v, const int n, double *vsum, const int vdim)
{
   int i;

#if USE_KAHAN_ADD

   double loss[MPCCI_QDIM_MAX];
   MEMZERO(loss,sizeof(loss));

#endif

   MEMZERO(vsum,vdim*sizeof(double));

#if USE_KAHAN_ADD

   switch(vdim)
   {
      case 1: /* scalar */
         for(i=0; i<n; i++)
         {
            KAHAN_ADD_DOUBLE(vsum[0],v[i],loss[0])
         }
         break;

      case 2: /* bi-scalar or 2D vector */
         for(i=0; i<n; i++,v+=2)
         {
            KAHAN_ADD_DOUBLE(vsum[0],v[0],loss[0])
            KAHAN_ADD_DOUBLE(vsum[2],v[1],loss[1])
         }
         break;

      case 3: /* vector */
         for(i=0; i<n; i++,v+=3)
         {
            KAHAN_ADD_DOUBLE(vsum[0],v[0],loss[0])
            KAHAN_ADD_DOUBLE(vsum[1],v[1],loss[1])
            KAHAN_ADD_DOUBLE(vsum[2],v[2],loss[2])
         }
         break;

      case 9: /* tensor */
         for(i=0; i<n; i++, v+=9)
         {
            KAHAN_ADD_DOUBLE(vsum[0],v[0],loss[0])
            KAHAN_ADD_DOUBLE(vsum[1],v[1],loss[1])
            KAHAN_ADD_DOUBLE(vsum[2],v[2],loss[2])
            KAHAN_ADD_DOUBLE(vsum[3],v[3],loss[3])
            KAHAN_ADD_DOUBLE(vsum[4],v[4],loss[4])
            KAHAN_ADD_DOUBLE(vsum[5],v[5],loss[5])
            KAHAN_ADD_DOUBLE(vsum[6],v[6],loss[6])
            KAHAN_ADD_DOUBLE(vsum[7],v[7],loss[7])
            KAHAN_ADD_DOUBLE(vsum[8],v[8],loss[8])
         }
         break;

      default:
         for(i=0; i<n; i++,v+=vdim)
         {
            int j;
            for(j=0; j<vdim; j++)
               KAHAN_ADD_DOUBLE(vsum[j],v[j],loss[j])
         }
         break;
   }

#else

   switch(vdim)
   {
      case 1: /* scalar */
         for(i=0; i<n; i++)
         {
            vsum[0] += v[i];
         }
         break;

      case 2: /* bi-scalar or 2D vector */
         for(i=0; i<n; i++,v+=2)
         {
            vsum[0] += v[0];
            vsum[1] += v[1];
         }
         break;

      case 3: /* vector */
         for(i=0; i<n; i++,v+=3)
         {
            vsum[0] += v[0];
            vsum[1] += v[1];
            vsum[2] += v[2];
         }
         break;

      case 9: /* tensor */
         for(i=0; i<n; i++,v+=9)
         {
            vsum[0] += v[0];
            vsum[1] += v[1];
            vsum[2] += v[2];
            vsum[3] += v[3];
            vsum[4] += v[4];
            vsum[5] += v[5];
            vsum[6] += v[6];
            vsum[7] += v[7];
            vsum[8] += v[8];
         }
         break;

      default:
         for(i=0; i<n; i++,v+=vdim)
         {
            int j;
            for(j=0; j<vdim; j++)
               vsum[j] += v[j];
         }
         break;
   }

#endif

}
#endif
