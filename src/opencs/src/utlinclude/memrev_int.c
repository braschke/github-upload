#pragma once
#ifndef memrev_int_SOURCE_INCLUDED
#define memrev_int_SOURCE_INCLUDED
/* memrev_int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memrev(), but with int objects
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memrev_int.c 5454 2017-08-04 14:17:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int *memrev_int(int *buf, size_t n)
{
   int *head = buf;
   int *tail = buf + n-1;

   n /= 2;
   while(n-- > 0)
   {
      int tmp = *head;
      *head++ = *tail;
      *tail-- = tmp;
   }
   return buf;
}
#endif
