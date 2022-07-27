#ifndef __strnotchr_SOURCE_INCLUDED
#define __strnotchr_SOURCE_INCLUDED
/* strnotchr.c , C.Dehning  July '90
 *
 *  invertiertes strchr(), finde ersten char != 'c'
 *
 *  string utilities (c) C.Dehning  1990
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strnotchr(const TCHAR *s, int c)
{
   if (STRHASLEN(s) && c)
   {
      while(*s == c) s++;
      if (*s) return (TCHAR *)s;
   }
   return NULL;
}
#endif
