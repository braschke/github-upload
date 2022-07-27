#ifndef __strcxchg_SOURCE_INCLUDED
#define __strcxchg_SOURCE_INCLUDED
/* strcxchg.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  ersetze zeichen um string, MSWin UNICODE save
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strcxchg(TCHAR *str, const TCHAR seek, const TCHAR replace)
{
   if (STRHASLEN(str))
   {
      int i;

      if (replace) /* true replacement */
      {
         for (i=0; str[i]; i++)
            if (str[i] == seek)
               str[i] = replace;
      }
      else /* remove seek char */
      {
         TCHAR *dst;

         for (i=0; str[i]; i++)
            if (str[i] == seek)
               break;

         dst = str + i;
         for (; str[i]; i++)
            if (str[i] != seek)
               *dst++ = str[i];

         *dst = TEXT('\0');
      }
   }
   return str;
}

#if 0
   #include <stdio.h>
   int main(int argc, char *argv[]){printf("=>\"%s\"\n",strcxchg(argv[1],TEXT('\\'),TEXT('\0')));}
#endif
#endif
