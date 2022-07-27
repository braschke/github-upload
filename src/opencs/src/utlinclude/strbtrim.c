#pragma once
#ifndef strbtrim_SOURCE_INCLUDED
#define strbtrim_SOURCE_INCLUDED
/* strbtrim.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    String boundary trim: remove leading/trailing whitespace from a string
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/May/23: Carsten Dehning, Initial release
 *    $Id: strbtrim.c 4132 2016-04-16 04:20:23Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strbtrim(TCHAR *str)
{
   if (str) /* avoid NULL pointer access */
   {
      STRJUMPNOSPACE(str);
      if (*str)
      {
         TCHAR *end;
         STRENDNOSPACE(str,end);
      }
   }
   return str;
}
#endif
