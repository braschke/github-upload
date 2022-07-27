#pragma once
#ifndef memrev_vptr_SOURCE_INCLUDED
#define memrev_vptr_SOURCE_INCLUDED
/* memrev_vptr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memrev(), but with objects of size void *
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memrev_vptr.c 5454 2017-08-04 14:17:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void **memrev_vptr(void *buf[], size_t n)
{
   void **head = buf;
   void **tail = buf + n-1;

   n /= 2;
   while(n-- > 0)
   {
      void *tmp = *head;
      *head++ = *tail;
      *tail-- = tmp;
   }
   return buf;
}
#endif
