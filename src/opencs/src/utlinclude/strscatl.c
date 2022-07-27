#ifndef strscatl_SOURCE_INCLUDED
#define strscatl_SOURCE_INCLUDED
/* strscatl.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Join multiple strings into a single destination string. This replaces calls like
 *
 *       strscat(dst,s1);
 *       strscat(dst,s2);
 *       ......
 *       strscat(dst,sn);
 *
 *    by
 *
 *       strscatl(dst,sizeof(dst),s1,s2,...sn,NULL);
 *
 *    do a securce strcat() - check for buffer overflow - and always make sure the
 *    last byte of target string is '\0' and the target string is properly terminated.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Dec/23: Carsten Dehning, Initial release
 *    $Id: strscatl.c 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <stdarg.h>

C_FUNC_PREFIX
TCHAR *strscatl(TCHAR *dst, size_t size, const TCHAR *src, ...)
{
   const TCHAR *s;
         TCHAR *d, *dEnd;
   size_t       dlen;
   va_list      ap;


   if (dst && size) /* got valid destination */
   {
      dlen = STRLEN(dst);
      dEnd = (d=dst+dlen) + size - dlen - 1;  /* -1 for trailing 0 */
      va_start(ap,src);

         for(s=src; s && d<dEnd; s=va_arg(ap,const TCHAR *))
         {
            while (*s && d<dEnd)
               *d++ = *s++;
         }
         *d = TEXT('\0');

      va_end(ap);
   }
   return dst;
}

#if 0
int main(void)
{
   char s[30];s[0] = '\0';
   puts(strscatl(s,sizeof(s),"Mein","Name","ist","Carsten","Dehning","und","dieser","Text","ist","viel","zu","lang",NULL));
   return 0;
}
#endif
#endif
