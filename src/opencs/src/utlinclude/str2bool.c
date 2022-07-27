#ifndef __STR2BOOL_SOURCE_INCLUDED
#define __STR2BOOL_SOURCE_INCLUDED
/* str2bool.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in 0/1,
 *  wenn moeglich. return ist 0, wenns geklappt hat, ansonsten -1
 *
 *  MSWin UNICODE save
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
int str2bool(const TCHAR *str, int *pbool)
{
   if (!str)
      return -1;

   switch(str[0])
   {
      case TEXT('0'):
         if (!str[1]) goto IS_FALSE;
         break;

      case TEXT('1'):
         if (!str[1]) goto IS_TRUE;
         break;

      case TEXT('t'):
      case TEXT('T'):
         if (!STRICMP(str,TEXT("true"))) goto IS_TRUE;
         break;

      case TEXT('f'):
      case TEXT('F'):
         if (!STRICMP(str,TEXT("false"))) goto IS_FALSE;
         break;

      case TEXT('o'):
      case TEXT('O'):
         if (!STRICMP(str,TEXT("on")) ) goto IS_TRUE;
         if (!STRICMP(str,TEXT("off"))) goto IS_FALSE;
         break;

      default:
         break;
   }
   return -1;

IS_TRUE:
   if (pbool) *pbool = 1;
   return 0;

IS_FALSE:
   if (pbool) *pbool = 0;
   return 0;
}
#endif
