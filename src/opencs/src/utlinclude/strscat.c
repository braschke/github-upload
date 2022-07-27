#pragma once
#ifndef strscat_SOURCE_INCLUDED
#define strscat_SOURCE_INCLUDED
/* strscat.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    do a securce strcat() - check for buffer overflow - and always make sure the
 *    last byte of target string is '\0' and the target string is properly terminated.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: strscat.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strscat(TCHAR *dst, size_t size, const TCHAR *src)
{
   if (dst && src && size)
   {
      size_t len = STRLEN(dst);
      if (len < --size)
         STRNCPY(dst+len,src,size-len);
      dst[size] = TEXT('\0');
   }
   return dst;
}
#endif
