#ifndef fnextline_SOURCE_INCLUDED
#define fnextline_SOURCE_INCLUDED
/* fnextline.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Advance to the next line in a textfile.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fnextline.c 5634 2017-10-14 03:59:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int fnextline(FILE *fp)
{
   int c;

   while ((c = getc(fp)) != EOF)
      if (c == '\n') return 0;
   return EOF;
}
#endif
