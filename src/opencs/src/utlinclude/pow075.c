#pragma once
#ifndef pow075_SOURCE_INCLUDED
#define pow075_SOURCE_INCLUDED
/* pow075.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Evaluate pow(v,0.75) == pow(v,3/4) == sqrt(sqrt(v*v*v))
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2013/Jun: Carsten Dehning, Initial release
 *    $Id: pow075.c 5446 2017-08-03 17:51:36Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX double pow075(const double d)
{
   return sqrt(sqrt(d*d*d));
}

#endif
