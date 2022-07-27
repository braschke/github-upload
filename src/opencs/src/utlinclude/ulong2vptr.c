#pragma once
#ifndef ulong2vptr_SOURCE_INCLUDED
#define ulong2vptr_SOURCE_INCLUDED
/* ulong2vptr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    convert an unsigned long into a void * pointer: see also vptr2ulong()
 *    used to avoid compiler warnings with simple cast's
 *
 *       vptr=(void *)ulong
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Nov/07: Carsten Dehning, Initial release
 *    $Id: ulong2vptr.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
void *ulong2vptr(unsigned long ulong)
{
   union
   {
               void *vp;
      unsigned long  ul;
   } u;

   u.ul = ulong;
   return u.vp;
}
#endif
