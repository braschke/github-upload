#pragma once
#ifndef strslen_SOURCE_INCLUDED
#define strslen_SOURCE_INCLUDED
/* strslen.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    do a secure strlen() which avoids a SIG 11 if the terminating '\0' is missing.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: strslen.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
size_t strslen(TCHAR *str, size_t size)
{
   size_t len = 0;

   if (str && size)
   {
      str[size-1] = '\0';
      len = STRLEN(str);
   }
   return len;
}
#endif
