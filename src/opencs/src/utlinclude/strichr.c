#pragma once
#ifndef strichr_SOURCE_INCLUDED
#define strichr_SOURCE_INCLUDED
/* strichr.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    ignore case strchr()
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strichr(const TCHAR *str, int c)
{
   if (STRHASLEN(str))
   {
      c = CAST_INT(TOLOWER(c));
      do { if (TOLOWER(*str) == c) return CAST_TCHARP(str); } while (*++str);
   }
   return NULL;
}
#endif
