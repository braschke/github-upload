#pragma once
#ifndef STR2DOUBLE_SOURCE_INCLUDED
#define STR2DOUBLE_SOURCE_INCLUDED
/* str2double.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in double,
 *  wenn moeglich. return ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"


C_FUNC_PREFIX
int str2double(const TCHAR *str, double *pdouble)
{
   TCHAR *end;
   double dbl;

   if (!STRHASLEN(str))         return -1; /* NULL or empty string */
   STRJUMPNOSPACE(str);
   if (!*str)                   return -1;
   dbl = STRTOD(str,&end);
   if (end == str || *end != 0) return -1;
   if (pdouble) *pdouble = dbl;
   return 0;
}
#endif
