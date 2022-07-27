#ifndef __nancheck_float_SOURCE_INCLUDED
#define __nancheck_float_SOURCE_INCLUDED
/* nancheck_float.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test a foat/double array for nan's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2011-2011, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2011/Feb/07: Carsten Dehning, Initial release
 *    $Id: nancheck_float.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int nancheck_float(const float *buf, size_t n)
{
   size_t i;
   for(i=0; i<n; i++)
      if (ISNAN(buf[i])) return 1;
   return 0;
}
#endif
