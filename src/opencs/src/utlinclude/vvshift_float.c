#ifndef __vvshift_float_SOURCE_INCLUDED
#define __vvshift_float_SOURCE_INCLUDED
/* vvshift_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    shift all values of array v[n*vdim] by shift[vdim]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2010, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: vvshift_float.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *vvshift_float(float *buf, int n, const float *shift, int vdim)
{
   float *v = buf;
   int i;

   switch(vdim)
   {
      case 1: /* scalar */
         for(i=0; i<n; i++)
         {
            *v++ += shift[0];
         }
         break;

      case 2: /* bi-scalar or 2D vector */
         for(i=0; i<n; i++)
         {
            *v++ += shift[0];
            *v++ += shift[1];
         }
         break;

      case 3: /* vector */
         for(i=0; i<n; i++)
         {
            *v++ += shift[0];
            *v++ += shift[1];
            *v++ += shift[2];
         }
         break;

      case 9: /* tensor */
         for(i=0; i<n; i++)
         {
            *v++ += shift[0];
            *v++ += shift[1];
            *v++ += shift[2];
            *v++ += shift[3];
            *v++ += shift[4];
            *v++ += shift[5];
            *v++ += shift[6];
            *v++ += shift[7];
            *v++ += shift[8];
         }
         break;

      default:
         for(i=0; i<n; i++)
         {
            int j;
            for(j=0; j<vdim; j++)
               *v++ += shift[j];
         }
         break;
   }

   return buf;
}
#endif
