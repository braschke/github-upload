#pragma once
#ifndef ulong2adp_SOURCE_INCLUDED
#define ulong2adp_SOURCE_INCLUDED
/* ulong2adp.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Print a 64 bit unsigned (with many digits) into a TCHAR string including a
 *    thousands separator and return a pointer to the first TCHAR of the buffer.
 *    The string buffer is optional and may be NULL. If not NULL, the string buffer
 *    must have a size of at least 32 TCHARS.
 *
 * Author:
 *    string utilities (c) C.Dehning  1990
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: ulong2adp.c 5293 2017-03-03 08:42:19Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN
#else
   #include <locale.h>
#endif


C_FUNC_PREFIX
const TCHAR *ulong2adp(const uint64_t uval, TCHAR *s, const size_t count)
{
   static TCHAR sepc = TEXT('\0');
   static TCHAR sbuf[32];

   uint64_t v;
   unsigned n;


   if (!sepc)
   {
      /* Get the 1K group separator from the locale */
   #if IS_MSWIN
      GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STHOUSAND,sbuf,countof(sbuf));
      sepc = sbuf[0];
   #else
      struct lconv *lcp = localeconv();
      if (lcp && lcp->thousands_sep) sepc = lcp->thousands_sep[0];
   #endif
    /*if (sepc==0) printf("1000-sep=0\n"); else printf("1000-sep=%c\n",sepc);*/
      if (!sepc) sepc = TEXT(','); /* ',' in the US, '.' in Europe */
   }

   if (!s || count<32)
   {
      /* Use local static buffer */
      s = sbuf + (countof(sbuf)-1);
   }
   else
   {
      s += count-1;
   }

   /* Fill buffer from right to left */
   *s   = TEXT('\0');
   *--s = CAST_TCHAR(uval%10) + TEXT('0');
   v    = uval/10;
   for (n=1; v; n++)
   {
      if ((n%3) == 0) /* Insert the sepc every three digits */
      {
         *--s = sepc;
      }

      *--s = CAST_TCHAR(v%10) + TEXT('0');
      v /= 10;
   }

#if 0
   printf("ulong2adp, s=%p\n",(void *)s);
#endif

   return s;
}

#endif
