#pragma once
#ifndef strlower_SOURCE_INCLUDED
#define strlower_SOURCE_INCLUDED
/* strlower.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    make string lowercase
 *
 */
#include "stdheader.h"

#if !IS_MSWIN

C_FUNC_PREFIX
TCHAR *strlower(TCHAR *str)
{
   TCHAR *c;
   for (c=str; *c; c++)  *c = CAST_TCHAR(TOLOWER(*c));
   return str;
}
#endif
#endif
