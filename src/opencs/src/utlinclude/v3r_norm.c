#pragma once
#ifndef v3r_norm_SOURCE_INCLUDED
#define v3r_norm_SOURCE_INCLUDED
/* v3r_norm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Normalize a vector
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3r_norm.c 5535 2017-08-25 18:00:07Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "v3d_norm.c"
#else
   #include "v3f_norm.c"
#endif
#endif
