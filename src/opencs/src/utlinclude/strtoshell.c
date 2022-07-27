#ifndef __STRTOSHELL_SOURCE_INCLUDED
#define __STRTOSHELL_SOURCE_INCLUDED
/* strtoshell.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    convert an MSwin filename string into a unix shell filename
 *    x\y   ->    x/y   dir separators
 *    x\ y  ->    x y   escaped whitespace
 *    x\\y  ->    x\\y  escaped backslash
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2005-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/24: Carsten Dehning, Initial release
 *    $Id: strtoshell.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strtoshell(char *str)
{
   char *cp;
   int   lastc, nextc;

   for (lastc=0, cp=str; *cp; lastc=*cp++)
   {
      if (*cp   != '\\') continue;
      if (lastc == '\\') continue;
      nextc = cp[1];
      if (nextc == ' ' ) continue; /* got a whitespace escape */
      if (nextc == '\\') continue; /* got a real '\' */
      *cp = '/';   /* assume a '\' path separator, but '/' is what the OpenSSH sh.exe needs */
   }
   return str;
}
#endif
