#ifndef __fwrite_charv_SOURCE_INCLUDED
#define __fwrite_charv_SOURCE_INCLUDED
/* fwrite_charv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Binary save a char array as floats.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2012-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2012/May/11: Carsten Dehning, Initial release
 *    $Id: fwrite_charv.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void fwrite_charv(FILE *fp, const char *cv, const int n)
{
   if (n > 0)
   {
      size_t nv = (size_t)n;

      while(nv > 0) /* save 2K bunches of converted floats */
      {
         float        fv[2048];
         const size_t nout = (nv < countof(fv)) ? nv : countof(fv);
         size_t       i;

         for (i=0; i<nout; i++)
            fv[i] = (float)cv[i];

         fwrite(fv,sizeof(float),nout,fp);
         cv += nout;
         nv -= nout;
      }
   }
}

#endif
