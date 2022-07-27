#pragma once
#ifndef STRUNQUOTE_SOURCE_INCLUDED
#define STRUNQUOTE_SOURCE_INCLUDED
/* strunquote.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Unquote a "string" or 'string' and return pointer to the new start of the string
 *    clean multiple quoted strings, e.g.
 *
 *       '""I am quoted""'
 *          =>    I am quoted
 *
 *       " "string with space" and 'another space' "
 *          =>    "string with space" and 'another space'
 *
 *    A single quote " just remains as it is.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2005/Sep/24: Carsten Dehning, Initial release
 *    $Id: strunquote2.c 4132 2016-04-16 04:20:23Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strunquote(TCHAR *str, TCHAR **ppEnd)
{
   TCHAR *head,*tail;
   TCHAR lastq = TEXT('\0');


   head = str;
   STRJUMPNOSPACE(head);      /* eat leading whitespace */
   STRENDNOSPACE(head,tail);  /* eat trailing whitespace */

   while(ISQUOTE(*head) && (*head == *tail) && (tail > head))
   {
      lastq = *head++;        /* remember the last quote */
      STRJUMPNOSPACE(head);   /* again eat leading and trailing whitespace */
      *tail-- = TEXT('\0');
      while(ISSPACE(*tail) && tail>head)
         *tail-- = TEXT('\0');
   #if 0
      FPRINTF(stdout,TEXT("  unquoted <%s>\n"),head);
   #endif
   }

   if (lastq) /* we removed a quote */
   {
      /* check for quoted substrings */
      for (str=head+1; *str && *str != lastq; str++)
         ;
      if (str > head+1 && str < tail && ISSPACE(str[1]))
      {
         /* found a quoted substring, restore the last quotes */
      #if 0
         puts("restoring quotes");
      #endif
         *--head =
         *++tail = lastq;
      }
   }

   if (ppEnd)
      *ppEnd = tail;
   return head;
}

#if 0
int main(int argc, char *argv[])
{
   FPRINTF(stdout,TEXT("<%s> =>\n"),argv[1]); FPRINTF(stdout,TEXT("  => <%s>\n"),strunquote(argv[1],NULL)); return 0;
}
#endif

#endif
