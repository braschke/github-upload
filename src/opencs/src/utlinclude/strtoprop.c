#ifndef strtoprop_SOURCE_INCLUDED
#define strtoprop_SOURCE_INCLUDED
/* strtoprop.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  alle nicht property zeichen im string loeschen
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strtoprop(TCHAR *str)
{
   TCHAR *src,*dst;

   for (src=dst=str; *src; src++)
   {
      if (ISAZALPHA(*src)||ISDIGIT(*src))
      {
         *dst++ = *src;
      }
      else
      {
         switch(*src)
         {
            case '+':
            case '-':
            case '_':
            case '.':
               *dst++ = *src;
               break;

            default:
               break;
         }
      }
   }

   *dst = 0;
   return str;
}
#endif
