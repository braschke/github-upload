#ifndef __nancheck_double_SOURCE_INCLUDED
#define __nancheck_double_SOURCE_INCLUDED
/* nancheck_double.c
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
 *    $Id: nancheck_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int nancheck_double(const double *buf, size_t n)
{
   size_t i;
   for(i=0; i<n; i++)
      if (ISNAN(buf[i])) return 1;
   return 0;
}
#endif
