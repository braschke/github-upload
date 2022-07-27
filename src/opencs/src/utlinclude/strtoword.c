#ifndef __STRTOWORD_SOURCE_INCLUDED
#define __STRTOWORD_SOURCE_INCLUDED
/* strtoword.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  alle nicht wort zeichen im string loeschen
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strtoword(char *str)
{
   char *src, *dst;
   for (src=dst=str; *src; src++) if (ISWORD(*src)) *dst++ = *src;
   *dst = '\0';
   return str;
}
#endif
