#pragma once
#ifndef strlwr_SOURCE_INCLUDED
#define strlwr_SOURCE_INCLUDED
/* strlwr.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    make string lowercase
 *
 */
#include "stdheader.h"

#if !IS_MSWIN

C_FUNC_PREFIX
char *strlwr(char *str)
{
   char *c;
   for (c=str; *c; c++)  *c = CAST_CHAR(tolower(*c));
   return str;
}
#endif
#endif
