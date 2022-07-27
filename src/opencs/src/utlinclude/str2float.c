#ifndef __STR2FLOAT_SOURCE_INCLUDED
#define __STR2FLOAT_SOURCE_INCLUDED
/* str2float.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in double,
 *  wenn moeglich. return ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "str2double.c"
#endif


C_FUNC_PREFIX
int str2float(const TCHAR *str, float *pfloat)
{
   double dbl = 0.0;
   int    ret = str2double(str,&dbl);

   if (ret) return ret;
   if (dbl > FLT_MAX || dbl < -FLT_MAX) return 1;
   if (pfloat) *pfloat = (float)dbl;
   return 0;
}
#endif
