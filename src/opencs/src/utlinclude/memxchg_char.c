#ifndef __memxchg_char_SOURCE_INCLUDED
#define __memxchg_char_SOURCE_INCLUDED
/* memxchg_char.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Replace all "seek" in "buf" by "replace"
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memxchg_char.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *memxchg_char(char *buf, const int n, const char seek, const char replace)
{
   if (seek != replace)
   {
      int i;
      for(i=0; i<n; i++) if (buf[i]==seek) buf[i]=replace;
   }
   return buf;
}
#endif
