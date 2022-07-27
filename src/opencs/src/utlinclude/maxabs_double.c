#ifndef __maxabs_double_SOURCE_INCLUDED
#define __maxabs_double_SOURCE_INCLUDED
/* maxabs_double.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the maximum of the two absolute values.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2009, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2009/Jan/28: Carsten Dehning, Initial release
 *    $Id: maxabs_double.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
double maxabs_double(double v1, double v2)
{
   double a1 = fabs(v1);
   double a2 = fabs(v2);
   return (a2>a1) ? a2 : a1;
}
#endif
