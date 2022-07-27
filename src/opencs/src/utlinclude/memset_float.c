#ifndef __memset_float_SOURCE_INCLUDED
#define __memset_float_SOURCE_INCLUDED
/* memset_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memset(), but with float's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memset_float.c 942 2013-05-24 15:57:13Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float *memset_float(float *buf, const size_t n, const float set)
{
   size_t i;
   for(i=0; i<n; i++) buf[i] = set;
   return buf;
}
#endif
