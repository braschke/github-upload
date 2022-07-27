#ifndef __vstocbin_SOURCE_INCLUDED
#define __vstocbin_SOURCE_INCLUDED
/* vstocbin.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Convert a memory area *mem with size bytes into a char *bitstring
 *
 * Author:
 *    string utilities (c) C.Dehning  1990
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: vstocbin.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *vstocbin(const void *vp, size_t size)
{
   static char bitstr[256+8];

   const char *cp = (const char *)vp;
   unsigned usize = (size > 256/8) ? 256/8 : (unsigned)size;
   unsigned i,nc = 0;
   int      j;

   for(i=0; i<usize; i++)
      for(j=7; j>=0; j--)
         bitstr[nc++] = (cp[i]& (1<<j)) ? '1' : '0';

   bitstr[nc++] = '\0';
   return bitstr;
}
#if 0
int main(void)
{
   float a = 1.0;
   puts(vstocbin(&a,sizeof(float)));
   return 0;
}
#endif
#endif
