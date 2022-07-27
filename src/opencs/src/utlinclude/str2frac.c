#ifndef __str2frac_SOURCE_INCLUDED
#define __str2frac_SOURCE_INCLUDED
/* str2frac.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s '+-zaehler[/+-nenner]' in zaehler m und optionalen
 *  nenner d, wenn moeglich. return code wie bei str2long
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "str2long.c"
#endif

C_FUNC_PREFIX
int str2frac(const TCHAR *str, long *pnom, long *pden)
{
   TCHAR *dp;
   int    negn, negd;
   long   n,d,div;


   if ((dp = STRCHR(str,TEXT('/'))) == NULL)
   {
      *pden = 1; /* nenner = 1, da kein nenner vorhanden ist */
      return str2long(str,pnom);
   }

   *dp  = '\0';               /* '/' mit 'ende-string' ersetzen */
   negn = str2long(str,pnom); /* hole zaehler */
   *dp  = '/';                /* ersetzung wieder rueckgangig machen */
   if (negn || (negn = str2long(dp+1,pden)) != 0)
      return negn;   /* zaehler-error */


   n = *pnom;
   d = *pden;

   /* vorzeichen check */
   negn = negd = 0;
   if (n < 0) { n = -n; negn = 1; }
   if (d < 0) { d = -d; negd = 1; }

   /* bruchzahl kuerzen */
   if ( n == d ) { n =     d = 1; goto EXIT_OKAY; }
   if (!(n % d)) { n /= d; d = 1; goto EXIT_OKAY; }

   /* durch gemeinsames vielfaches kuerzen */
   for (div=2; ((2*div)<=d) && ((2*div)<=n); div++)
   {
      while ((n % div) == 0 && (d % div) == 0)
      {
         n /= div; d /= div;
      }
   }

EXIT_OKAY:
   *pnom = (negn) ? -n : n;
   *pden = (negd) ? -d : d;
   return 0;
}
#endif
