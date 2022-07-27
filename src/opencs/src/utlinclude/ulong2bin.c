#pragma once
#ifndef ulong2bin_SOURCE_INCLUDED
#define ulong2bin_SOURCE_INCLUDED
/* ulong2bin.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Print the bits of a 64 bit unsigned into a TCHAR string.
 *    This is a binary string "1011100010..." representation of a 64 bit unsigned.
 *    The string buffer is optional and may be NULL. If not NULL, the string buffer
 *    must have a size of at least 64+1(\0) TCHARS.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2017/Aug/8: Carsten Dehning, Initial release
 *    $Id: ulong2bin.c 5681 2017-11-10 12:30:48Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
const TCHAR *ulong2bin(const uint64_t uval, const unsigned minbits, TCHAR *s, const size_t count)
{
   static TCHAR sbuf[64+1];

   uint64_t v    = uval;
   uint64_t mask = CAST_UINT64(1) << 63;
   unsigned hb   = 64+1;  /* highest bit */
   unsigned nc   = 0;     /* No. of chars in the string */


   if (!s || count<(64+1))
   {
      /* Use local static buffer */
      s = sbuf;
   }

   /*
    * Make a bitmask starting with the highest bit in uval != 0,
    * but at least minbit bits.
    */
   while((--hb>minbits) && mask && (v&mask) == 0)
   {
      mask >>= 1;
   }

   do
   {
      s[nc++] = (v&mask) ? '1' : '0';
      mask >>= 1;
   } while(mask);

   s[nc] = 0;
   return s;
}

#endif
