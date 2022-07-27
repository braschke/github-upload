#pragma once
#ifndef pownf_SOURCE_INCLUDED
#define pownf_SOURCE_INCLUDED
/* pownf.c
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
 *    $Id: pownf.c 5446 2017-08-03 17:51:36Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX float pownf(float fbase, const int n)
{
   double   result = 1.0;
   double   dbase  = CAST_DOUBLE(fbase);
   unsigned count  = CAST_UINT((n<0) ? -n : n);


   while (count)
   {
      if (count & 1) result *= dbase;
      count >>= 1;
      dbase *= dbase;
   }
   return CAST_FLOAT((n < 0) ? 1.0/result : result);
}
#endif
