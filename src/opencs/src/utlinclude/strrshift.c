#ifndef __STRRSHIFT_SOURCE_INCLUDED
#define __STRRSHIFT_SOURCE_INCLUDED
/* strrshift.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    schiebe die zeichen des strings um 'n' nach rechts. links wird der
 *    string mit leerzeichen aufgefuellt
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strrshift(char *str, size_t n)
{
   if (n)
   {
      size_t len = strlen(str);
      if (n < len) memmove(str+n,str,len-n);
      else         n = len;
      memset(str,' ',n);
   }
   return str;
}
#endif
