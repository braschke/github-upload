#pragma once
#ifndef pownr_SOURCE_INCLUDED
#define pownr_SOURCE_INCLUDED
/* pownr.c
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
 *    $Id: pownr.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "pownd.c"
#else
   #include "pownf.c"
#endif
#endif
