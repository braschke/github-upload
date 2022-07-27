#pragma once
#ifndef strscpy_c2f_SOURCE_INCLUDED
#define strscpy_c2f_SOURCE_INCLUDED
/* strscpy_c2f.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    copy C string into a fortran string and fill with ' 's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Dec/15: Carsten Dehning, Initial release
 *    $Id: strscpy_c2f.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "fortran_types.h"

C_FUNC_PREFIX
Fstring strscpy_c2f(Fstring fStr, Fstrlen fLen, const char *cStr)
{
   if (fStr && fLen > 0)
   {
      int  i;

      /* strncpy() C-string -> F-string */
      for(i=0; i<fLen; i++)
         if ((fStr[i] = cStr[i]) == '\0')
            break;

      /* fill F-string with trailing whitespace, no '\0' at end */
      for(; i<fLen; i++)
         fStr[i] = ' ';
   }
   return fStr;
}
#endif
