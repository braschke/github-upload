#ifndef __str2ushort_SOURCE_INCLUDED
#define __str2ushort_SOURCE_INCLUDED
/* str2ushort.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in unsigned short,
 *  wenn moeglich. code ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"
#include <limits.h>

#if INCLUDE_STATIC
   #include "str2long.c"
#endif

C_FUNC_PREFIX
int str2ushort(const TCHAR *str, int *pint)
{
   long num = 0;
   int  ret = str2long(str,&num);

   if (ret) return ret;
   if (num < 0 || num > USHRT_MAX) return 1; /* got an ushort overflow */
   if (pint) *pint = CAST_INT(num);
   return 0;
}
#endif
