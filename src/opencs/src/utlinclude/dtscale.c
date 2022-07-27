#pragma once
#ifndef dtscale_SOURCE_INCLUDED
#define dtscale_SOURCE_INCLUDED
/* dtscale.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Scale a double time value (input units=seconds) into nano/micro/milli seconds.
 *    Returns a pointer to a static time unit string.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2016/Jan/15: Carsten Dehning, Initial release
 *    $Id: dtscale.c 3827 2016-01-15 18:14:56Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
const TCHAR *dtscale(double *dbltime)
{
   register const double t = *dbltime;

   if (t > 0)
   {
      if (t < 1e-8) { *dbltime *= 1e9; return TEXT("nsec"); }
      if (t < 1e-5) { *dbltime *= 1e6; return TEXT("usec"); }
      if (t < 1e-2) { *dbltime *= 1e3; return TEXT("msec"); }
   }
   return TEXT("sec");
}
#endif
