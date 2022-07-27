#ifndef __memcount_int_SOURCE_INCLUDED
#define __memcount_int_SOURCE_INCLUDED
/* memcount_int.c
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
 *    $Id: memcount_int.c 940 2013-05-24 15:56:26Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memcount_int(const int *buf, const int n, const int seek)
{
   int i,count = 0;
   for(i=0; i<n; i++) if (buf[i] == seek) count++;
   return count;
}
#endif
