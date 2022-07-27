#pragma once
#ifndef strupper_SOURCE_INCLUDED
#define strupper_SOURCE_INCLUDED
/* strupper.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    make string uppercase
 *
 */
#include "stdheader.h"

#if !IS_MSWIN

C_FUNC_PREFIX
TCHAR *strupper(TCHAR *str)
{
   TCHAR *c;
   for (c=str; *c; c++) *c = CAST_CHAR(TOUPPER(*c));
   return str;
}
#endif
#endif
