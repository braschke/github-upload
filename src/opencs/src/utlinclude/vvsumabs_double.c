#ifndef __vvsumabs_double_SOURCE_INCLUDED
#define __vvsumabs_double_SOURCE_INCLUDED
/* vvsumabs_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    sum all fabs(values) of a packed array v[n*vdim] and return the vsum[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2010, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vvsumabs_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void vvsumabs_double(const double *v, int n, double *vsum, int vdim)
{
   int i;


   MEMZERO(vsum,vdim*sizeof(double));
   switch(vdim)
   {
      case 1: /* scalar */
         for(i=0; i<n; i++)
         {
            vsum[0] += fabs(v[i]);
         }
         break;

      case 2: /* bi-scalar or 2D vector */
         for(i=0; i<n; i++)
         {
            vsum[0] += fabs(*v++);
            vsum[1] += fabs(*v++);
         }
         break;

      case 3: /* vector */
         for(i=0; i<n; i++)
         {
            vsum[0] += fabs(*v++);
            vsum[1] += fabs(*v++);
            vsum[2] += fabs(*v++);
         }
         break;

      case 9: /* tensor */
         for(i=0; i<n; i++)
         {
            vsum[0] += fabs(*v++);
            vsum[1] += fabs(*v++);
            vsum[2] += fabs(*v++);
            vsum[3] += fabs(*v++);
            vsum[4] += fabs(*v++);
            vsum[5] += fabs(*v++);
            vsum[6] += fabs(*v++);
            vsum[7] += fabs(*v++);
            vsum[8] += fabs(*v++);
         }
         break;

      default:
         for(i=0; i<n; i++)
         {
            int j;
            for(j=0; j<vdim; j++)
               vsum[j] += fabs(*v++);
         }
         break;
   }
}
#endif
