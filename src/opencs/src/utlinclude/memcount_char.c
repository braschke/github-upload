#ifndef __memcount_char_SOURCE_INCLUDED
#define __memcount_char_SOURCE_INCLUDED
/* memcount_char.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    count the no. of occurences of seek in buf[n]
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memcount_char.c 940 2013-05-24 15:56:26Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memcount_char(const void *buf, const int n, const int seek)
{
   const char *s = (const char *)buf;
   int i,count = 0;
   for(i=0; i<n; i++) if (CAST_INT(s[i]) == seek) count++;
   return count;
}
#endif
