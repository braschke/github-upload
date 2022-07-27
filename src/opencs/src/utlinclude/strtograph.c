#ifndef __STRTOGRAPH_SOURCE_INCLUDED
#define __STRTOGRAPH_SOURCE_INCLUDED
/* strtograph.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 * alle leerzeichen/controls im string loeschen
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strtograph(char *str)
{
   char *src, *dst;
   for (src=dst=str; *src; src++) if (ISGRAPH(*src)) *dst++ = *src;
   *dst = '\0';
   return str;
}
#endif
