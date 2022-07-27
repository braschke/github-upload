#ifndef __strfixs_SOURCE_INCLUDED
#define __strfixs_SOURCE_INCLUDED
/* strfixs.c , C.Dehning  July '90
 *
 *  alle mehrfachen leerzeichen/controls im string loeschen und dabei
 *  auf ISKOMMA() achten
 *
 *  ist dazu gedacht texte zu komprimieren, ohne den fluss zu unterbrechen
 *
 *  string utilities (c) C.Dehning  1990
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strfixs(TCHAR *s, const TCHAR *delmtr)
{
   register TCHAR *c, *p;

   for (c=p=s; *c; c++)
     if (ISGRAPH(c[0]) || ( ISGRAPH(c[1]) && !STRCHR(delmtr, c[1]) ) )
        *p++ = *c;

   *p = 0;
   return s;
}
#endif
