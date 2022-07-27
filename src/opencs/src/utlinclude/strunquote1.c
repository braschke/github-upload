#pragma once
#ifndef strunquote1_SOURCE_INCLUDED
#define strunquote1_SOURCE_INCLUDED
/* strunquote1.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    from a string which is a collection of space separated substrings
 *    - like a commandline - extract the first string - the commandname - itself.
 *    return pointer to start of the true string plus
 *       ppLast: optional pointer to the end of the 1st string
 *       ppNext: optional pointer to the start of the options
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2005/Sep/24: Carsten Dehning, Initial release
 *    $Id: strunquote1.c 4295 2016-05-03 16:56:51Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strunquote1(TCHAR *str, TCHAR **ppLast, TCHAR **ppNext)
{
   TCHAR *end;

   /* eat leading and trailing whitespace */
   STRJUMPNOSPACE(str);
   STRENDNOSPACE(str,end);

   if (ppLast) *ppLast = end;
   if (ppNext) *ppNext = end + 1;
   if (end <= str+1) /* only a single char on this line */
      return str;

   if (ISQUOTE(*str)) /* this is a quoted string */
   {
      TCHAR *cp    = str;
      TCHAR  lastq = *cp++;

      /* skip all other nested leading quotes */
      while(ISQUOTE(*cp))
         lastq = *cp++;

      /* find the last inner quote char again */
      for(end=cp; *end != lastq; end++)
         if (!*end)
            return str; /* only leading or unmatched quote found: return the native string */

      if (ppLast) *ppLast = end - 1; /* points to the char before the last quote */
      while(ISQUOTE(*end))
         end++;

      str = cp;
   }
   else
   {
      for(end=str+1; *end != TEXT(' '); end++)
         if (!*end)
            return str; /* only a single token found */

      if (ppLast) *ppLast = end - 1; /* points to the char before ' ' */
   }

   if (ppNext)
   {
      while (*end && *++end == ' ');
      *ppNext = end;
   }
   return str;
}
#endif
