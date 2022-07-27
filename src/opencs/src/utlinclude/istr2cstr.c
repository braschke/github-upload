#ifndef __ISTR2CSTR_SOURCE_INCLUDED
#define __ISTR2CSTR_SOURCE_INCLUDED
/* istr2cstr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    convert a string of integer characters into a real c character string: src == dst
 *    this is use to hide strings from hackers:
 *
 *    strings are defined as array of int[] and are then once converted
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Aug/27: Carsten Dehning, Initial release
 *    $Id: istr2cstr.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *istr2cstr(int *istr)
{
   int   i;
   char *cstr = (char *)istr;

   for (i=0; istr[i]; i++) cstr[i] = CAST_CHAR(istr[i]);
   cstr[i] = '\0';

   return cstr;
}
#endif
