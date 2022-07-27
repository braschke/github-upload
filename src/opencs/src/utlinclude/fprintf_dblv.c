#ifndef fprintf_dblv_SOURCE_INCLUDED
#define fprintf_dblv_SOURCE_INCLUDED
/* fprintf_dblv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    formatted print variable length list of doubles (e.g. coords)
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Dec/01: Carsten Dehning, Initial release
 *    $Id: fprintf_dblv.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void fprintf_dblv(FILE *fp, const double *dbuf, int n, const char *trailerFmt, ...)
{
   int i;

   for (i=0; i<n; i++)
      fprintf(fp," %g", dbuf[i]);

   if (STRHASLEN(trailerFmt))
   {
      if (strchr(trailerFmt,'%'))
      {
         va_list ap;
         va_start(ap,trailerFmt);
         vfprintf(fp,trailerFmt,ap);
         va_end(ap);
      }
      else
      {
         fputs(trailerFmt,fp);
      }
   }
}
#endif
