#pragma once
#ifndef v3r_ngalign_SOURCE_INCLUDED
#define v3r_ngalign_SOURCE_INCLUDED
/* v3r_ngalign.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    globally align a normal vector with length==1.0
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3r_ngalign.c 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "v3d_ngalign.c"
#else
   #include "v3f_ngalign.c"
#endif
#endif
