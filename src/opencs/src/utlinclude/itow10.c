#pragma once
#ifndef itow10_SOURCE_INCLUDED
#define itow10_SOURCE_INCLUDED
/* itow10.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Reverse atoi() with radix 10
 *
 * Author:
 *    string utilities (c) C.Dehning  1990
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: itow10.c 5483 2017-08-11 17:26:44Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
wchar_t *itow10(int ival, wchar_t *str)
{
   static wchar_t itow10_sbuf[32]; /* target buffer in case str==NULL */
   wchar_t       *sp,*dp;
   unsigned       uval;
   wchar_t        dbuf[32]; /* buffer keeps digits in reverse order */


   uval = (unsigned)((ival<0) ? -ival : ival);
   dp = dbuf;
   do { *dp++ = '0' + (uval % 10); } while (uval /= 10);

   sp = (str) ? str : (str=itow10_sbuf); /* last not thread save: use static buffer */
   if (ival  < 0)   *sp++ = '-';
   while(dp > dbuf) *sp++ = *--dp;
   *sp = 0;
   return str;
}

#if 0
int main(int argc, char *argv[])
{
   wprintf(L"argc: %d => %s\n",argc   , itow10( argc          ,NULL));
   wprintf(L"argv: %S => %s\n",argv[1], itow10( atoi(argv[1]) ,NULL));
   return 0;
}
#endif

#endif
