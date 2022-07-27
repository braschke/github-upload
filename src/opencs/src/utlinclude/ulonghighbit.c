#pragma once
#ifndef ulonghighbit_SOURCE_INCLUDED
#define ulonghighbit_SOURCE_INCLUDED
/* ulonghighbit.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the number of significant bits of 64 bit unsigned.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2017/Jul/26: Carsten Dehning, Initial release
 *    $Id: ulonghighbit.c 5471 2017-08-08 10:47:22Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX unsigned ulonghighbit(uint64_t uval)
{
   unsigned highbit = 0;

   while((uval>>=1))
   {
      highbit++;
   }
   return highbit;
}

#if 0
int main(int argc, char *argv[])
{
   unsigned val = atoi(argv[1]);
   printf("val=%u, 0x%08x, nbits=%u\n",val,val,ulonghighbit(val));
   return 0;
}
#endif

#endif
