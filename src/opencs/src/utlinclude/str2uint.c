#ifndef __str2uint_SOURCE_INCLUDED
#define __str2uint_SOURCE_INCLUDED
/* str2uint.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in unsigned int,
 *  wenn moeglich. code ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"
#include <limits.h>

#if INCLUDE_STATIC
   #include "str2long.c"
#endif

C_FUNC_PREFIX
int str2uint(const TCHAR *str, unsigned *pint)
{
   long num = 0;
   int  ret = str2long(str,&num);

   if (ret) return ret;
#if (UINT_MAX < LONG_MAX)
   if (num < 0 || num > UINT_MAX) return 1; /* got an uint overflow */
#endif

   if (pint) *pint = CAST_UINT(num);
   return 0;
}
#endif
