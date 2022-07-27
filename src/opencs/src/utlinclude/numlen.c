#ifndef __NUMLEN_SOURCE_INCLUDED
#define __NUMLEN_SOURCE_INCLUDED
/* numlen.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    wieviele zeichen brauchen wir, um 'value' mit 'radix 'formatiert
 *    auszugeben
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
size_t numlen(long value, int radix)
{
   size_t count;
   long   rad = (radix < 2) ? 2 : radix;

   if (value < 0)
   {
      count = 2;
      value = -value;
   }
   else
      count = 1; /* 1 zeichen IMMER !! */

   while ( (value /= rad) > 0)
      count++;

   return count;
}
#endif
