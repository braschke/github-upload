#pragma once
#ifndef STRUNQUOTE_SOURCE_INCLUDED
#define STRUNQUOTE_SOURCE_INCLUDED
/* strunquote.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    unquote a "string" or 'string' and return pointer to the new start of the string
 *    clean multiple quoted strings, e.g. '""I am quoted""' => I am quoted
 *    A single quote " just remains as it is.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2005/Sep/24: Carsten Dehning, Initial release
 *    $Id: strunquote.c 4132 2016-04-16 04:20:23Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strunquote(TCHAR *str, TCHAR **ppEnd)
{
   TCHAR *end;

   STRJUMPNOSPACE(str);    /* eat leading whitespace */
   STRENDNOSPACE(str,end); /* eat trailing whitespace */

   while(ISQUOTE(*str) && (*end == *str) && (end > str))
   {
      str++;
     *end-- = 0;
   }

   if (ppEnd) *ppEnd = end;
   return str;
}
#endif
