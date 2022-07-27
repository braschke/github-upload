#pragma once
#ifndef stristr_w_SOURCE_INCLUDED
#define stristr_w_SOURCE_INCLUDED
/* stristr_w.c
 *
 *  string utilities (c) C.Dehning July 1990
 *
 *    ignore case strstr()
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
wchar_t *stristr_w(const wchar_t *str, const wchar_t *seek)
{
   if (!STRHASLEN(seek))
      return (wchar_t *)str; /* according to ISO 9899 */

#if IS_MSWIN

   if (STRHASLEN(str))
   {
      /*
       * Get the first seek char as UC for quick comparison
       * and let string compare start at seek+1
       */
      const int    s0 = towupper(seek[0]);
      const size_t ns = wcslen(++seek);
      if (ns)
      {
         do
         {
            if (towupper(*str) == s0 && !_wcsnicmp(str+1,seek,ns))
               return CCAST_INTO(wchar_t *,str);
         } while(*++str);
      }
      else /* Seek string is just a single char */
      {
         do
         {
            if (towupper(*str) == s0)
               return CCAST_INTO(wchar_t *,str);
         } while(*++str);
      }
   }

#else

   #if INCLUDE_STATIC
      #error The function stristr_w() is only available under MSWin
   #endif

#endif

   return NULL;
}
#endif
