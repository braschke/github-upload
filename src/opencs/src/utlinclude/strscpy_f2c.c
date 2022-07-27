#pragma once
#ifndef strscpy_f2c_SOURCE_INCLUDED
#define strscpy_f2c_SOURCE_INCLUDED
/* strscpy_f2c.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    copy a ' ' padded fortran string with given length into a clean C string
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Dec/15: Carsten Dehning, Initial release
 *    $Id: strscpy_f2c.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "fortran_types.h"

C_FUNC_PREFIX
char *strscpy_f2c(char *cStr, size_t cSiz, const Fstring fStr, Fstrlen fLen)
{
   if (cStr && cSiz > 0)
   {
      int  i, iLast;

      if (fLen > CAST_INT(--cSiz)) /* need space for the trailing '\0' */
          fLen = CAST_INT(cSiz);

      iLast = -1;
      for(i=0; i<fLen; i++)
         if ((cStr[i] = fStr[i]) != ' ')
            iLast = i; /* remember the index of the last non blank char */

      cStr[iLast+1] = '\0'; /* remove trailing whitespace */
   }
   return cStr;
}
#endif
