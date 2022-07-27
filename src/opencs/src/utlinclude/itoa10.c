#pragma once
#ifndef itoa10_SOURCE_INCLUDED
#define itoa10_SOURCE_INCLUDED
/* itoa10.c
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
 *    $Id: itoa10.c 5483 2017-08-11 17:26:44Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *itoa10(int ival, char *str)
{
   static char itoa10_sbuf[32]; /* target buffer in case str==NULL */
   char       *sp,*dp;
   unsigned    uval;
   char        dbuf[32]; /* buffer keeps digits in reverse order */


   uval = (unsigned)((ival<0) ? -ival : ival);
   dp = dbuf;
   do { *dp++ = '0' + (uval % 10); } while (uval /= 10);

   sp = (str) ? str : (str=itoa10_sbuf); /* last not thread save: use static buffer */
   if (ival  < 0)   *sp++ = '-';
   while(dp > dbuf) *sp++ = *--dp;
   *sp = '\0';
   return str;
}

#if 0
int main(int argc, char *argv[])
{
   printf("argc: %d => %s\n",argc   , itoa10( argc          ,NULL));
   printf("argv: %s => %s\n",argv[1], itoa10( atoi(argv[1]) ,NULL));
   return 0;
}
#endif

#endif
