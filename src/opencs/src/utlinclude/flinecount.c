#ifndef flinecount_SOURCE_INCLUDED
#define flinecount_SOURCE_INCLUDED
/* flinecount.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    count newlines in a file from the current position and return that number.
 *    do not change the position of the filepointer.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: flinecount.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
size_t flinecount(FILE *fp)
{
   int    c;
   fpos_t fpos;
   size_t nline = 0;


   fgetpos(fp,&fpos); /* remember current file position */

   while ((c = getc(fp)) != EOF)
      if (c == '\n') nline++;

   fsetpos(fp,&fpos); /* reset current file position */

   return nline;
}
#endif
