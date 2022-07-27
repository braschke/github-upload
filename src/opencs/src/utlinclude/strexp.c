#ifndef __strexp_SOURCE_INCLUDED
#define __strexp_SOURCE_INCLUDED
/* strexp.c , C.Dehning  July '90
 *
 *
 *  ersetze im string s die ersten n zeichen durch string si
 *  string utilities (c) C.Dehning  1990
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strexp(TCHAR *s, size_t n, const TCHAR *si)
{
   TCHAR  *c;
   size_t  lstr = STRLEN(s);
   size_t  ldel;


   if (lstr <= n) /* ganzen string ersetzen */
      return STRCPY(s,si);

   ldel = STRLEN(si) - n;

   /* strlen(si) > n , string von n an um strlen(si)-n nach rechts schieben */
   if (ldel > 0)
   {
      for (c = s+lstr; c >= s+n; c--)
         *(c+ldel) = *c;
   }

   /* strlen(si) < n, string von n an um n-strlen(si) nach links schieben */
   else if (ldel < 0)
   {
      for (c = s+n; c <= s+lstr; c++)
         *(c+ldel) = *c;
   }

   for (c = s; *si; c++,si++) /* si einbauen */
      *c = *si;

   return s;
}
#endif
