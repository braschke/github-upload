#pragma once
#ifndef memswapb_SOURCE_INCLUDED
#define memswapb_SOURCE_INCLUDED
/* memswapb.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Change the endianess of an array of arbitrary sized types.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memswapb.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define BSWAPIJ(_i,_j) btmp=b[_i];b[_i]=b[_j];b[_j]=btmp

C_FUNC_PREFIX
void *memswapb(void *buf, size_t n, size_t size)
{
   unsigned char *b = CAST_UCHARP(buf); /* convert into bytes */

   switch(size)
   {
      case 4:
         for(;n-- > 0; b+=4)
         {
            register unsigned char btmp;
            BSWAPIJ(0,3);
            BSWAPIJ(1,2);
         }
         break;

      case 8:
         for(;n-- > 0; b+=8)
         {
            register unsigned char btmp;
            BSWAPIJ(0,7);
            BSWAPIJ(1,6);
            BSWAPIJ(2,5);
            BSWAPIJ(3,4);
         }
         break;

      case 12:
         for(;n-- > 0; b+=12)
         {
            register unsigned char btmp;
            BSWAPIJ(0,11);
            BSWAPIJ(1,10);
            BSWAPIJ(2, 9);
            BSWAPIJ(3, 8);
            BSWAPIJ(4, 7);
            BSWAPIJ(5 ,6);
         }
         break; /* long double == quad? */

      case 16:
         for(;n-- > 0; b+=16)
         {
            register unsigned char btmp;
            BSWAPIJ(0,15);
            BSWAPIJ(1,14);
            BSWAPIJ(2,13);
            BSWAPIJ(3,12);
            BSWAPIJ(4,11);
            BSWAPIJ(5,10);
            BSWAPIJ(6, 9);
            BSWAPIJ(7, 8);
         }
         break; /* long double == quad?*/

      case 2:
         for(;n-- > 0; b+=2)
         {
            register unsigned char btmp;
            BSWAPIJ(0,1);
         }
         break;

      case 0: /* junk size */
      case 1: /* nothing to do */
         break;

      default:
         for(;n-- > 0; b+=size)
         {
            size_t i, j;
            for(i=0,j=size-1; i<j; i++,j--)
            {
               register unsigned char btmp;
               BSWAPIJ(i,j);
            }
         }
         break;
   }
   return buf;
}
#undef BSWAPIJ
#endif
