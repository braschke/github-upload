#pragma once
#ifndef str2long_SOURCE_INCLUDED
#define str2long_SOURCE_INCLUDED
/* str2long.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in ganze zahl zahl,
 *  wenn moeglich. code ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"

C_FUNC_PREFIX
int str2long(const TCHAR *s, long *plong)
{
   long num = 0;


   if (!STRHASLEN(s))
      return -1; /* NULL or empty string */

   STRJUMPNOSPACE(s);

   if (s[0]==TEXT('0') && TOUPPER(s[1])==TEXT('X')) /* hexadecimal number */
   {
      size_t nch = 0;

      s += 2; /* first char behind 'X' */
      do
      {
         long c = TOUPPER(*s);

         if (++nch > 2*sizeof(long)) return -1; /* too many nibbles */

              if (c >= '0' && c <= '9') c = c - '0';
         else if (c >= 'A' && c <= 'Z') c = c - 'A' + 10;
         else return -1;
         num = (num << 4) | c;
      } while (*++s);
   }

   else if (TOUPPER(s[0])==TEXT('B')) /* binary number */
   {
      size_t nch = 0;
      s++; /* first char behind 'B' */
      do
      {
         if (++nch > 8*sizeof(long)) return -1; /* too many bits */
         num <<= 1;
         switch(*s)
         {
            case TEXT('1'): num |= 1;
            case TEXT('0'): break;
            default       : return -1;
         }
      } while (*++s);
   }

   else /* assume decimal number */
   {
      int neg = 0;
      switch(*s)
      {
         case 0        : return -1; /* null string */
         case TEXT('-'): neg=1;
         case TEXT('+'): s++  ;
         default       : break;
      }
      do
      {
         if (*s < TEXT('0') || *s > TEXT('9')) return -1;
         num = 10*num + (long)(*s - TEXT('0'));
         if (num < 0) return 1; /* integer overflow */
      } while (*++s);
      if (neg) num = -num;
   }

   if (plong) *plong = num;
   return 0;
}
#endif
