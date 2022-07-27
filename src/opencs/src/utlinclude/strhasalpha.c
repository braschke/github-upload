#pragma once
#ifndef strhasalpha_SOURCE_INCLUDED
#define strhasalpha_SOURCE_INCLUDED
/* strhasalpha.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    test if at least one char in a string isalpha()
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Dec/04: Carsten Dehning, Initial release
 *    $Id: strhasalpha.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int strhasalpha(const TCHAR *str)
{
   if (str)
   {
      int i;
      for(i=0; str[i]; i++)
         if (ISALPHA(str[i])) return 1;
   }
   return 0; /* no alpha char */
}
#endif
