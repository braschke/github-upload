#ifndef memset_int_SOURCE_INCLUDED
#define memset_int_SOURCE_INCLUDED
/* memset_int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memset(), but with int's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memset_int.c 3299 2014-11-12 17:15:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int *memset_int(int *buf, const size_t n, const int set)
{
   size_t i;
   for(i=0; i<n; i++) buf[i] = set;
   return buf;
}
#endif
