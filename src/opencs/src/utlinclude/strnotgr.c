#ifndef __strnotgraph_SOURCE_INCLUDED
#define __strnotgraph_SOURCE_INCLUDED
/* strnotgraph.c , C.Dehning  July '90
 *
 *  finde ersten NICHT graphischen char
 *
 *  string utilities (c) C.Dehning  1990
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strnotgraph(const TCHAR *s)
{
   if (STRHASLEN(s))
   {
      while (ISGRAPH(*s)) s++;
      if (*s) return (TCHAR *)s;
   }
   return NULL;
}
#endif
