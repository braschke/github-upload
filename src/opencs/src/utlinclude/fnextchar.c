#pragma once
#ifndef fnextchar_SOURCE_INCLUDED
#define fnextchar_SOURCE_INCLUDED
/* fnextchar.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Advance to the next non space character in a textfile. Take care
 *    for possible comment chars. Returns the last valid char.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fnextchar.c 4132 2016-04-16 04:20:23Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_UNICODE
   #define F_GETC       getwc
   #define F_EOF        WEOF
   #define F_WINT       wint_t
#else
   #define F_GETC       getc
   #define F_EOF        EOF
   #define F_WINT       int
#endif

C_FUNC_PREFIX
int fnextchar(FILE *fp, const TCHAR *comments)
{
   if (comments && !comments[0])
      comments = NULL;

   for(;;)
   {
      F_WINT c;

      /* eat whitespace */
      do { c = F_GETC(fp); } while(ISSPACE(c));

      if (c == F_EOF)
         return EOF;

      if (!comments || !STRCHR(comments,c))
         return CAST_INT(c); /* return if EOF or not comment char */

      /* eat comment line */
      do { c = F_GETC(fp); } while (c != TEXT('\n') && c != F_EOF);
   }
}

#undef F_GETC
#undef F_EOF
#undef F_WINT

#endif
