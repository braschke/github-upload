#ifndef fpcat_SOURCE_INCLUDED
#define fpcat_SOURCE_INCLUDED
/* fpcat.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Append an open file 'fp_src' to a open file 'fp_dst'
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Nov/16: Carsten Dehning, Initial release
 *    $Id: fpcat.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int fpcat(FILE *fp_dst, FILE *fp_src, int rwnd)
{
   int c;

   if (!fp_src || !fp_dst)
      return EOF;

   if (rwnd)
      rewind(fp_src);

   while((c=fgetc(fp_src)) != EOF)
      fputc(c,fp_dst);

   return 0;
}
#endif
