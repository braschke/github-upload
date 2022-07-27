#ifndef memswapv_SOURCE_INCLUDED
#define memswapv_SOURCE_INCLUDED
/* memswapv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Change the endianess of a value of arbitrary size
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memswapv.c 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void *memswapv(void *val, size_t size)
{
#define BSWAPIJ(_i,_j) btmp=b[_i];b[_i]=b[_j];b[_j]=btmp

   unsigned char *b = (unsigned char *)val; /* convert into bytes */
   register unsigned char btmp;

   switch(size)
   {
      case 4:
         BSWAPIJ(0,3);
         BSWAPIJ(1,2);
         break;

      case 8:
         BSWAPIJ(0,7);
         BSWAPIJ(1,6);
         BSWAPIJ(2,5);
         BSWAPIJ(3,4);
         break;

      case 12:
         BSWAPIJ(0,11);
         BSWAPIJ(1,10);
         BSWAPIJ(2, 9);
         BSWAPIJ(3, 8);
         BSWAPIJ(4, 7);
         BSWAPIJ(5 ,6);
         break; /* long double == quad? */

      case 16:
         BSWAPIJ(0,15);
         BSWAPIJ(1,14);
         BSWAPIJ(2,13);
         BSWAPIJ(3,12);
         BSWAPIJ(4,11);
         BSWAPIJ(5,10);
         BSWAPIJ(6, 9);
         BSWAPIJ(7, 8);
         break; /* long double == quad? */

      case 2:
         BSWAPIJ(0,1);
         break;

      case 0: /* junk size */
      case 1:
         break;

      default:
         {
            size_t i, j;
            for(i=0,j=size-1; i<j; i++,j--)
            {
               BSWAPIJ(i,j);
            }
         }
         break;
   }
   return val;
#undef BSWAPIJ
}
#endif
