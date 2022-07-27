#ifndef folines_SOURCE_INCLUDED
#define folines_SOURCE_INCLUDED
/* folines.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    fopen a file and count newlines and return that number and filepointer.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: folines.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "flinecount.c"
#endif

C_FUNC_PREFIX
FILE *folines(const char *pathname, long *pnlines)
{
   FILE  *fp = fopen(pathname,"r");
   if (fp && pnlines) *pnlines = (long)flinecount(fp);
   return fp;
}
#endif
