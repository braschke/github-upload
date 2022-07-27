#pragma once
#ifndef pownd_SOURCE_INCLUDED
#define pownd_SOURCE_INCLUDED
/* pownd.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    power function with +-int exponent
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Sep/18: Carsten Dehning, Initial release
 *    $Id: pownd.c 5446 2017-08-03 17:51:36Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX double pownd(double dbase, const int n)
{
   double   result = 1.0;
   unsigned count  = CAST_UINT((n<0) ? -n : n);


   while (count)
   {
      if (count & 1) result *= dbase;
      count >>= 1;
      dbase *= dbase;
   }
   return (n < 0) ? 1.0/result : result;
}
#endif
