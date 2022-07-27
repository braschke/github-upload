#pragma once
#ifndef vptr2ulong_SOURCE_INCLUDED
#define vptr2ulong_SOURCE_INCLUDED
/* vptr2ulong.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    convert a void * pointer into an unsigned long: see also ulong2vptr()
 *    used to avoid compiler warnings with simple cast's
 *
 *       ulong=(unsigned long)vptr;
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Nov/07: Carsten Dehning, Initial release
 *    $Id: vptr2ulong.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
unsigned long vptr2ulong(const void *ptr)
{
   union
   {
      const    void *vp;
      unsigned long  ul;
   } u;

   u.vp = ptr;
   return u.ul;
}
#endif
