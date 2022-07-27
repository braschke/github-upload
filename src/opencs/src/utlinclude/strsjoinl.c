#pragma once
#ifndef strsjoinl_SOURCE_INCLUDED
#define strsjoinl_SOURCE_INCLUDED
/* strsjoinl.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    join multiple strings into a single destination string. This replaces calls like
 *
 *       strscpy(dst,s0);
 *       strscat(dst,sep);
 *       strscat(dst,s1);
 *       strscat(dst,sep);
 *       strscat(dst,s2);
 *       strscat(dst,sep);
 *       ......
 *       strscat(dst,sn);
 *
 *    by
 *
 *       strsjoinl(dst,sizeof(dst),sep,s0,s1,s2,...sn,NULL);
 *
 *    do a securce strcat() - check for buffer overflow - and always make sure the
 *    last byte of target string is '\0' and the target string is properly terminated.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Dec/23: Carsten Dehning, Initial release
 *    $Id: strsjoinl.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <stdarg.h>

C_FUNC_PREFIX
TCHAR *strsjoinl(TCHAR *dst, size_t size, const TCHAR *sep, const TCHAR *src, ...)
{
   const TCHAR *s;
         TCHAR *d, *dEnd;
   va_list      ap;


   if (dst && size) /* got valid destination */
   {
      if (sep && !sep[0]) /* non null empty separator */
         sep = NULL;

      dEnd = (d=dst) + size - 1; /* -1 for trailing 0 */
      va_start(ap,src);

         for(s=src; s && d<dEnd; s=va_arg(ap,const TCHAR *))
         {
            if (*s) /* completely skip empty strings */
            {
               if (sep && d>dst) /* append separator, but not as first */
               {
                  const TCHAR *sp = sep;
                  while (*sp && d<dEnd)
                     *d++ = *sp++;
               }

               /* append string */
               while (*s && d<dEnd)
                  *d++ = *s++;
            }
         }
         *d = TEXT('\0');

      va_end(ap);
   }
   return dst;
}

#if 0
int main(void)
{
   char s[30];
   puts(strsjoinl(s,sizeof(s)," ","My","name","is","Carsten","Dehning","and","this","text","is","far","too","long",NULL));
   return 0;
}
#endif
#endif
