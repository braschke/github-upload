#pragma once
#ifndef vptr2int_SOURCE_INCLUDED
#define vptr2int_SOURCE_INCLUDED
/* vptr2int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    convert a void * pointer into an int: see also ulong2vptr()
 *    used to avoid compiler warnings with simple cast's
 *
 *       ival=(int)vptr;
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Nov/07: Carsten Dehning, Initial release
 *    $Id: vptr2int.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
unsigned int vptr2int(const void *ptr)
{
   union
   {
      const    void *vp;
      unsigned long  ul;
   } u;

   u.vp = ptr;
   return CAST_INT(u.ul & 0x7fffffff);
}
#endif
