#pragma once
#ifndef conv_long2int_SOURCE_INCLUDED
#define conv_long2int_SOURCE_INCLUDED
/* conv_long2int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    In place convert a int/long array into a long/int array.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: conv_long2int.c 4582 2016-06-01 10:39:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int *conv_long2int(int64_t *buf, const size_t n)
{
   int64_t *lb = buf;
   int     *ib = CAST_INTP(buf);
   size_t   i;

   for (i=0; i<n; i++) ib[i] = CAST_INT(lb[i]);
   return CAST_INTP(buf);
}
#endif
