#ifndef fxgets_SOURCE_INCLUDED
#define fxgets_SOURCE_INCLUDED
/* fxgets.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Extended fgets(line,size,fp))
 *       Deals with optional comments => skips comment lines
 *       handles \\\n continue, return a single line
 *       skips leading whitespace
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fxgets.c 5634 2017-10-14 03:59:45Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "fnextchar.c"
#endif

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
TCHAR *fxgets(TCHAR *xline, const size_t size, FILE *fp, const TCHAR *comments)
{
   TCHAR *buf   = xline;
   TCHAR *end   = xline + size;
   F_WINT lastc = 0;
   F_WINT c;


   if (comments && !comments[0])
      comments = NULL;


   /* read until we reach the first non space character or EOF */
   c = (F_WINT)fnextchar(fp,comments); /* get the next valid char */
   for(; c!=F_EOF && buf<end; c=F_GETC(fp))
   {
      /* if we have comment chars and c is a comment char terminate line and bye bye */
      if (comments && STRCHR(comments,(TCHAR)c))
      {
         *buf++ = TEXT('\n'); break;
      }

      if (c != TEXT('\n')) /* got a valid char, store it */
      {
         *buf++ = (TCHAR)(lastc = c);  continue;
      }

      if (lastc != TEXT('\\')) /* TRUE newline: terminate line and bye bye */
      {
         *buf++ = TEXT('\n'); break;
      }

      buf--; /* give up the '\\' already stored in the buffer */
      lastc = 0;
   }

   while (c != F_EOF && c != TEXT('\n'))
      c = F_GETC(fp);

   *buf = TEXT('\0');
   return (buf>xline) ? xline : NULL; /* if no char was stored return NULL pointer */
}

#undef F_GETC
#undef F_EOF
#undef F_WINT

#endif
