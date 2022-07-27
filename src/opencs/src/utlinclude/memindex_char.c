#ifndef __memindex_char_SOURCE_INCLUDED
#define __memindex_char_SOURCE_INCLUDED
/* memindex_char.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    find seek in buf[] and return index of first element or -1
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memindex_char.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memindex_char(const void *buf, const size_t n, const int seek)
{
   const char *s = (const char *)buf;
   size_t i;
   for(i=0; i<n; i++) if (CAST_INT(s[i]) == seek) return CAST_INT(i);
   return -1;
}
#endif
