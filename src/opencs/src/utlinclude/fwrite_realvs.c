#pragma once
#ifndef fwrite_realvs_SOURCE_INCLUDED
#define fwrite_realvs_SOURCE_INCLUDED
/* fwrite_realvs.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Binary save a real array as floats.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/May/11: Carsten Dehning, Initial release
 *    $Id: fwrite_realvs.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void fwrite_realvs(FILE *fp, const real *rv, const int n, const int skip)
{
   if (n > 0)
   {
      size_t nv = (size_t)n;

      if (skip < 2) /* do not skip, save plain array */
      {
      #if REAL_IS_DOUBLE
         while(nv > 0) /* save 2K bunches of converted floats */
         {
            float        fv[2048];
            const size_t nout = (nv < countof(fv)) ? nv : countof(fv);
            size_t       i;

            for (i=0; i<nout; i++)
               fv[i] = (float)rv[i];

            fwrite(fv,sizeof(float),nout,fp);
            rv += nout;
            nv -= nout;
         }
      #else
         fwrite(rv,sizeof(float),n,fp);
      #endif
      }
      else
      {
         while(nv > 0) /* save 2K bunches of converted floats */
         {
            float        fv[2048];
            const size_t nout = (nv < countof(fv)) ? nv : countof(fv);
            size_t       i;

            for (i=0; i<nout; i++,rv+=skip)
               fv[i] = (float)*rv;

            fwrite(fv,sizeof(float),nout,fp);
            nv -= nout;
         }
      }
   }
}

#endif
