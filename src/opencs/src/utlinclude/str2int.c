#ifndef __str2int_SOURCE_INCLUDED
#define __str2int_SOURCE_INCLUDED
/* str2int.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in ganze zahl zahl,
 *  wenn moeglich. code ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"
#include <limits.h>

#if INCLUDE_STATIC
   #include "str2long.c"
#endif

C_FUNC_PREFIX
int str2int(const TCHAR *str, int *pint)
{
   long num = 0;
   int  ret = str2long(str,&num);

   if (ret) return ret;
#if (INT_MAX < LONG_MAX)
   if (num < -INT_MAX || num > INT_MAX) return 1; /* got an uint overflow */
#endif
   if (pint) *pint = CAST_INT(num);
   return 0;
}
#endif
