#ifndef __maxabs_float_SOURCE_INCLUDED
#define __maxabs_float_SOURCE_INCLUDED
/* maxabs_float.c
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
 *    $Id: maxabs_float.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
float maxabs_float(float v1, float v2)
{
   float a1 = fabsf(v1);
   float a2 = fabsf(v2);
   return (a2>a1) ? a2 : a1;
}
#endif
