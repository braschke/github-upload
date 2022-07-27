#pragma once
#ifndef strcountchar_SOURCE_INCLUDED
#define strcountchar_SOURCE_INCLUDED
/* strcountchar.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Count the no. of occurences of seek in a string
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: strcountchar.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int strcountchar(const TCHAR *str, TCHAR seek)
{
   int count = 0;
   if (str)
   {
      while(*str) if (*str++ == seek) count++;
   }
   return count;
}
#endif
