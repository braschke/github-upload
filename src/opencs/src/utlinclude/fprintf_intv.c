#ifndef fprintf_intv_SOURCE_INCLUDED
#define fprintf_intv_SOURCE_INCLUDED
/* fprintf_intv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    formatted print variable length list of integers (e.g. nodes ids)
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fprintf_intv.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void fprintf_intv(FILE *fp, const int *ibuf, int n, const char *trailerFmt, ...)
{
   int i;

   for (i=0; i<n; i++)
      fprintf(fp," %d", ibuf[i]);

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
