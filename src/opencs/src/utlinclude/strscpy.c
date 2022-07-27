#ifndef strscpy_SOURCE_INCLUDED
#define strscpy_SOURCE_INCLUDED
/* strscpy.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Do a secure strcpy()  - check for buffer overflow -  and always make sure the
 *    last byte of target string is '\0' and the target string is properly terminated.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: strscpy.c 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strscpy(TCHAR *dst, size_t size, const TCHAR *src)
{
   if (dst && size)
   {
      if (src && --size)
      {
         STRNCPY(dst,src,size);
         dst[size] = TEXT('\0');
      }
      else
      {
         dst[0] = TEXT('\0');
      }
   }
   return dst;
}
#endif
