#ifndef __dbl2flt_SOURCE_INCLUDED
#define __dbl2flt_SOURCE_INCLUDED
/* dbl2flt.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Proper conversion of a double into a float
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2012-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2012/Jan: Carsten Dehning, Initial release
 *    $Id: dbl2flt.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX float dbl2flt(double d)
{
   return CAST_DOUBLE2FLOAT(d);
}
#endif