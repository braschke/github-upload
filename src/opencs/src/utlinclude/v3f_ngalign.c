#pragma once
#ifndef v3f_ngalign_SOURCE_INCLUDED
#define v3f_ngalign_SOURCE_INCLUDED
/* v3f_ngalign.c
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
 *    $Id: v3f_ngalign.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int v3f_ngalign(float nv[3])
{
   int  dir = 0; /* -1=swap, 1=okay, 0=length(nv) == 0 */

        if (nv[0] < -NGALIGN_NORM_TOLERANCE) dir = -1;
   else if (nv[0] >  NGALIGN_NORM_TOLERANCE) dir =  1;
   else     nv[0] = 0.0;

        if (nv[1] < -NGALIGN_NORM_TOLERANCE) { if (!dir) dir = -1; }
   else if (nv[1] >  NGALIGN_NORM_TOLERANCE) { if (!dir) dir =  1; }
   else     nv[1] = 0.0;

        if (nv[2] < -NGALIGN_NORM_TOLERANCE) { if (!dir) dir = -1; }
   else if (nv[2] >  NGALIGN_NORM_TOLERANCE) { if (!dir) dir =  1; }
   else     nv[2] = 0.0;

   if (dir < 0) V3_NEG(nv);
   return dir;
}
#endif
