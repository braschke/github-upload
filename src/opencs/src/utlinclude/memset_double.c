#ifndef __memset_double_SOURCE_INCLUDED
#define __memset_double_SOURCE_INCLUDED
/* memset_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    like memset(), but with double's
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memset_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double *memset_double(double *buf, const size_t n, const double set)
{
   size_t i;
   for(i=0; i<n; i++) buf[i] = set;
   return buf;
}
#endif
