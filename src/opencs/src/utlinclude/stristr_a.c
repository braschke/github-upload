#pragma once
#ifndef stristr_a_SOURCE_INCLUDED
#define stristr_a_SOURCE_INCLUDED
/* stristr_a.c
 *
 *  string utilities (c) C.Dehning July 1990
 *
 *    ignore case strstr()
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *stristr_a(const char *str, const char *seek)
{
   if (!STRHASLEN(seek))
      return (char *)str; /* according to ISO 9899 */

   if (STRHASLEN(str))
   {
      /*
       * Get the first seek char as UC for quick comparison
       * and let string compare start at seek+1
       */
      const int    s0 = toupper(seek[0]);
      const size_t ns = strlen(++seek);
      if (ns)
      {
         do
         {
            if (toupper(*str) == s0 && !STRNICMP_A(str+1,seek,ns))
               return CCAST_INTO(char *,str);
         } while(*++str);
      }
      else /* Seek string is just a single char */
      {
         do
         {
            if (toupper(*str) == s0)
               return CCAST_INTO(char *,str);
         } while(*++str);
      }
   }

   return NULL;
}
#endif
