#pragma once
#ifndef strsplit_SOURCE_INCLUDED
#define strsplit_SOURCE_INCLUDED
/* strsplit.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Tokenize a string: the separators char set is an optional argument.
 *    Remove quotes from each token and fill up the *tokv[maxtok] with pointers
 *    to the tokens.
 *
 *    The source string will be modified - unquoted, and filled with '\0'.
 *    All leading and trailing isspace() chars will be removed from each token.
 *    If bKeepSep=true, the non space separator chars will remain part of the token
 *    list.
 *
 *    Separators: " \t\n"
 *       "  hallo I am   \t  space  \n  ' sep a rated '" ==
 *                   "hallo" "I"  "am"  "space"  " sep a rated "
 *
 *    Separators: " |><\t\n" and bKeepSep=true
 *       " exec|filter > \"filename with white space\"  " ==
 *                "exec" "|"  "filter"  ">"  "filename with white space"
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/24: Carsten Dehning, Initial release
 *    $Id: strsplit.c 4541 2016-05-24 04:47:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "chartostr.c"
#endif

#define STRSPLIT_FLAG_KEEPSEP       0x0001 /* Put separators into the tokenv */
#define STRSPLIT_FLAG_KEEPEMPTY     0x0002 /* Put empty strings into the tokenv */
#define STRSPLIT_FLAG_ESCAPE        0x0004 /* Allow '\sep' to be skipped */
#define STRSPLIT_FLAG_PARENTHESIS   0x0008 /* Ignore 'sep' within () */
#define STRSPLIT_FLAG_BRACKETS      0x0010 /* Ignore 'sep' within [] */
#define STRSPLIT_FLAG_BRACES        0x0020 /* Ignore 'sep' within {} */

#define IS_SEP_CHAR(_c) charIsSep[(CAST_UINT(_c))&0xff]

C_FUNC_PREFIX int strsplit
(
   TCHAR          *str,
   size_t          maxtok,
   const TCHAR    *tokv[],
   const TCHAR    *separators,
   const unsigned  flags
)
{
   unsigned       tokc, mxtok;
   const unsigned keepsep = (flags & STRSPLIT_FLAG_KEEPSEP);
   unsigned char  charIsSep[256];

   /* Terminate the token vector */
   MEMZERO((void *)tokv,maxtok*sizeof(TCHAR *));
   if (!str || !str[0] || maxtok < 3)
   {
      /*
       * maxtok < 3 is absolutely senseless:
       * one pointer + terminating NULL means no splitting possible at all.
       */
      return -1;
   }

   /*
    * Prepare the charIsSep[] list:
    *    for each TCHAR from the list of token separators set the charIsSep
    *    flag to 1. '\0' is always a terminator: NEVER FORGET
    */
   if (!separators || !separators[0])
      separators = TEXT(" \n\t"); /* default separators */

   MEMZERO(charIsSep,sizeof(charIsSep));
   for (; *separators; separators++)
   {
      IS_SEP_CHAR(*separators) = 1;
   }

   tokc  = 0;
   mxtok = CAST_INT(maxtok) - 1;
   while(tokc < mxtok)
   {
      TCHAR *end;
      TCHAR  lastc;


      STRJUMPNOSPACE(str); /* eat leading whitespace */

      if (!*str) /* reached the end of the string */
         return tokc;

      if (IS_SEP_CHAR(*str)) /* found a leading non space separator */
      {
         if (flags & STRSPLIT_FLAG_KEEPEMPTY)
         {
            tokv[tokc] = TEXT("");  /* we have an empty token between two non space separators */
            if (++tokc >= mxtok)    /* no more tokens available */
               break;               /* append the tail as one final string */
         }

         if (keepsep) /* keep the separator char as a token string */
         {
            tokv[tokc] = chartostr(*str);
            if (++tokc >= mxtok)    /* no more tokens available */
               break;               /* append the tail as one final string */
         }

         str++;   /* jump behind separator */
         continue;
      }

      if (ISQUOTE(*str)) /* assume string is quoted */
      {
         /* get the quote TCHAR and find end of quoted string */
         const TCHAR quote = *str;
         for (lastc=*str, end=++str; *end; lastc=*end++)
         {
            /* allow a single quotes inside a quoted string */
            if (*end == quote && lastc != TEXT('\\') && IS_SEP_CHAR(end[1]))
            {
               *end++ = TEXT('\0');
               break;
            }
         }
      }
      else
      {
         /* find next separator or '\0', handle whitespace escapes: "\ " remains as is */
         for (lastc=*str, end=str+1; *end; lastc=*end++)
            if (IS_SEP_CHAR(*end) && !(ISSPACE(*end) && lastc == TEXT('\\')))
             break;
      }

      tokv[tokc++] = str;
      if (!*end)
         return tokc;

      str = end + 1;

      /* add !ISSPACE() chars into the token list */
      if (keepsep && !ISSPACE(*end))
      {
         if (tokc >= mxtok)   /* no more tokens available */
            break;            /* append the tail */

         tokv[tokc++] = chartostr(*end);
      }

      *end = TEXT('\0');
   }

   /* last tokv entry may contains the rest of the string */
   if (tokc <= mxtok)
   {
      STRJUMPNOSPACE(str); /* eat leading whitespace */
      if (*str) tokv[tokc++] = str;
   }

   return tokc;

}

#undef IS_SEP_CHAR
#endif
