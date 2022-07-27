#pragma once
#ifndef bytes2hex_SOURCE_INCLUDED
#define bytes2hex_SOURCE_INCLUDED
/* bytes2hex.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Convert a bytes[blen] array into a 0-terminated hex string + hyphen chars
 *
 *    Dimension of hexstr[]:
 *       Need space for 2 x blen hex chars + (blen-1) x hyphens + '\0'
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Mar/02: Carsten Dehning, Initial release
 *    $Id: bytes2hex.c 5491 2017-08-17 18:09:52Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


C_FUNC_PREFIX TCHAR *bytes2hex
(
   const void    *barray,  /* bytes array (NOT string, may contain \0) */
   const unsigned blen,    /* no. of bytes in barray[] */
   const int      upper,   /* if true use upper case 'A'...'Z' */
   const int      hyphen,  /* hyphen char between hexes or 0 */
         TCHAR   *hexstr   /* \0 terminated hex char string */
)
{
   const unsigned char *bytes = CCAST_INTO(const unsigned char *,barray);
   unsigned i,n;
   const unsigned char aoffset = (upper) ? 'A'-10 : 'a'-10;


   for(n=i=0; i<blen; i++)
   {
      unsigned char b;

      if (i && hyphen)
         hexstr[n++] = CAST_TCHAR(hyphen);

      b = (bytes[i]>>4) & 0x0f; hexstr[n++] = CAST_TCHAR(b + ((b>9) ? aoffset : '0'));
      b =  bytes[i]     & 0x0f; hexstr[n++] = CAST_TCHAR(b + ((b>9) ? aoffset : '0'));
   }
   hexstr[n] = 0;

   return hexstr;
}
#endif
