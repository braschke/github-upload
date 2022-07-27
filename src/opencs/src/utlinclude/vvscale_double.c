#ifndef __vvscale_double_SOURCE_INCLUDED
#define __vvscale_double_SOURCE_INCLUDED
/* vvscale_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale all values of array v[n*vdim] by scale[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vvscale_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *vvscale_double(double *buf, const int n, const double *scale, const int vdim)
{
   double *v = buf;
   int i;

   switch(vdim)
   {
      case 1: /* scalar */
         for(i=0; i<n; i++)
         {
            *v++ *= scale[0];
         }
         break;

      case 2: /* bi-scalar or 2D vector */
         for(i=0; i<n; i++)
         {
            *v++ *= scale[0];
            *v++ *= scale[1];
         }
         break;

      case 3: /* vector */
         for(i=0; i<n; i++)
         {
            *v++ *= scale[0];
            *v++ *= scale[1];
            *v++ *= scale[2];
         }
         break;

      case 9: /* tensor */
         for(i=0; i<n; i++)
         {
            *v++ *= scale[0];
            *v++ *= scale[1];
            *v++ *= scale[2];
            *v++ *= scale[3];
            *v++ *= scale[4];
            *v++ *= scale[5];
            *v++ *= scale[6];
            *v++ *= scale[7];
            *v++ *= scale[8];
         }
         break;

      default:
         for(i=0; i<n; i++)
         {
            int j;
            for(j=0; j<vdim; j++)
               *v++ *= scale[j];
         }
         break;
   }

   return buf;
}
#endif
