#pragma once
#ifndef getendianess_SOURCE_INCLUDED
#define getendianess_SOURCE_INCLUDED
/* getendianess.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    find out the endiness of the processor     number => bytes
 *    return 'I':  little endian (intel/amd/vax)   1234 => 4321
 *    return 'M':  big endian (motorola)           1234 => 1234
 *    return 'P':  PDP-11 endianess                1234 => 3412
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getendianess.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define B_LOW  0
#define B_HIG  sizeof(unsigned int)-1

C_FUNC_PREFIX
int getendianess(void)
{
   union
   {
      unsigned int  ui;
      unsigned char uc[sizeof(unsigned int)];
   } endian_check;

   endian_check.ui = 1;
   return (endian_check.uc[B_LOW]) ? ENDIANESS_INTEL    : /* pure little endian */
          (endian_check.uc[B_HIG]) ? ENDIANESS_MOTOROLA : /* pure big endian */
                                     ENDIANESS_PDP11    ; /* old PDP-11 format */
}

#undef B_LOW
#undef B_HIG

#endif
