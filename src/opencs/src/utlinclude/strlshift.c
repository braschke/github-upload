#ifndef __STRLSHIFT_SOURCE_INCLUDED
#define __STRLSHIFT_SOURCE_INCLUDED
/* strlshift.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    loesche im string 's' die ersten n zeichen bzw. schiebe die zeichen
 *    des strings um 'n' nach links.
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strlshift(char *str, size_t n)
{
   if (n)
   {
      size_t len = strlen(str);
      if (n < len) return strcpy(str,str+n); /* stringende nachziehen */
      *str = '\0';                           /* ganzen string loeschen */
   }
   return str;
}
#endif
