#ifndef __memset_long_SOURCE_INCLUDED
#define __memset_long_SOURCE_INCLUDED
/* memset_long.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memset(), but with long's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memset_long.c 942 2013-05-24 15:57:13Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
long *memset_long(long *buf, const size_t n, const long set)
{
   size_t i;
   for(i=0; i<n; i++) buf[i] = set;
   return buf;
}
#endif
