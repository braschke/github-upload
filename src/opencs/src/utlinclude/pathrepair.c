#ifndef __PATHREPAIR_SOURCE_INCLUDED
#define __PATHREPAIR_SOURCE_INCLUDED
/* pathrepair.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    repair a pathname
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2011, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: pathrepair.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *pathrepair(TCHAR *pathname)
{
   TCHAR *c;

   while(ISSPACE(*pathname) || ISQUOTE(*pathname))
      pathname++; /* remove leading whitespace */

   for(c=pathname; *c; c++)
   {
#if IS_MSWIN
      if (ISUNXDIRSEP(*c)) *c = _C_WINDIRSEP;
#else
      if (ISWINDIRSEP(*c)) *c = _C_UNXDIRSEP;
#endif
   }

   while(--c >= pathname && (ISSPACE(*c) || ISQUOTE(*c)))
      *c = TEXT('\0');

   return pathname;
}
#endif
