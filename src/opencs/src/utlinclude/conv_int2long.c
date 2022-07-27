#pragma once
#ifndef conv_int2long_SOURCE_INCLUDED
#define conv_int2long_SOURCE_INCLUDED
/* conv_int2long.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    In place convert an int/long array into a long/int array.
 *    The int array must have a 64 bit alignment, otherwise
 *    we may run into addressing issues.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: conv_int2long.c 4582 2016-06-01 10:39:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int64_t *conv_int2long(int *buf, const size_t n)
{
   int64_t *lb = SCAST_INTO(int64_t *,buf);
   int     *ib = buf;
   size_t   i  = n;

   /*
    * int array must have the size n*sizeof(long)!!!
    * avoid overlap and go from tail to head
    */
   while(i-- > 0) lb[i] = SCAST_INTO(int64_t,ib[i]);
   return SCAST_INTO(int64_t *,buf);
}

#endif
