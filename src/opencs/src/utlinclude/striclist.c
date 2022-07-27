#ifndef __striclist_SOURCE_INCLUDED
#define __striclist_SOURCE_INCLUDED
/* striclist.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    ignore case find chars from *clist in string *str and return pointer
 *    to match in str or NULL.
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "strichr.c"
#endif

C_FUNC_PREFIX
TCHAR *striclist(const TCHAR *str, const TCHAR *clist)
{
   size_t slen = STRLENP(str);
   size_t clen = STRLENP(clist);


   if (slen && clen)
   {
      if (slen > clen) /* optimize no. of strichr() calls */
      {
         for (; *clist; clist++)
         {
            TCHAR *cp = strichr(str,*clist);
            if (cp) return cp;
         }
      }
      else /* short str, but large compare list */
      {
         for (; *str; str++)
            if (strichr(clist,*str))
               return (TCHAR *)str;
      }
   }
   return NULL;
}
#endif
