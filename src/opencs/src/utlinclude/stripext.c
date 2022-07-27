#ifndef __STRIPEXT_SOURCE_INCLUDED
#define __STRIPEXT_SOURCE_INCLUDED
/* stripext.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    strip filename extension and optional replace
 *
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "getextname.c"
#endif

C_FUNC_PREFIX
TCHAR *stripext(TCHAR *path, const TCHAR *newext)
{
   TCHAR *oldext = getextname(path); /* get the last dot */
   if (oldext) *oldext = '\0';
   return (STRHASLEN(newext)) ? STRCAT(path,newext) : path;
}
#endif
