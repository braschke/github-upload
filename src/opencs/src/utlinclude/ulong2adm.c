#pragma once
#ifndef ulong2adm_SOURCE_INCLUDED
#define ulong2adm_SOURCE_INCLUDED
/* ulong2adm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Print a 64 bit unsigned (with many digits) indicating a memory size into a
 *    TCHAR string including pretty formatting and return a pointer to the first
 *    TCHAR of the buffer.
 *    The string buffer is optional and may be NULL. If not NULL, the string buffer
 *    must have a size of at least 32 TCHARS.
 *
 * Author:
 *    string utilities (c) C.Dehning  1990
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: ulong2adm.c 5498 2017-08-18 06:27:39Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
const TCHAR *ulong2adm(const uint64_t uval, TCHAR *s, const size_t count)
{
   static TCHAR sbuf[32];

   const uint64_t ONEK = 1024;
   uint64_t       v = uval;
   unsigned       exa,peta,tera,giga,mega,kilo,rest;


   if (!s || count<32)
   {
      /* Use local static buffer */
      s = sbuf;
   }

   exa  = CAST_UINT(v/(ONEK*ONEK*ONEK*ONEK*ONEK*ONEK)); v %= (ONEK*ONEK*ONEK*ONEK*ONEK*ONEK);
   peta = CAST_UINT(v/(ONEK*ONEK*ONEK*ONEK*ONEK)     ); v %= (ONEK*ONEK*ONEK*ONEK*ONEK);
   tera = CAST_UINT(v/(ONEK*ONEK*ONEK*ONEK)          ); v %= (ONEK*ONEK*ONEK*ONEK);
   if (exa)
   {
      sprintf(s,TEXT("%u EiB %u PiB %u TiB"),exa,peta,tera+(v>0));
      return s;
   }

   giga = CAST_UINT(v/(ONEK*ONEK*ONEK)); v %= (ONEK*ONEK*ONEK);
   if (peta)
   {
      sprintf(s,TEXT("%u PiB %u TiB %u GiB"),peta,tera,giga+(v>0));
      return s;
   }

   mega = CAST_UINT(v/(ONEK*ONEK)); v %= (ONEK*ONEK);
   if (tera)
   {
      sprintf(s,TEXT("%u TiB %u GiB %u MiB"),tera,giga,mega+(v>0));
      return s;
   }

   kilo = CAST_UINT(v/ONEK); v %= ONEK;
   if (giga)
   {
      sprintf(s,TEXT("%u GiB %u MiB %u KiB"),giga,mega,kilo+(v>0));
      return s;
   }

   rest = CAST_UINT(v);
   if (mega)
   {
      if (rest) sprintf(s,TEXT("%u MiB %u KiB %u B"),mega,kilo,rest);
      else      sprintf(s,TEXT("%u MiB %u KiB"     ),mega,kilo);
      return s;
   }

   if (kilo)
   {
      if (rest) sprintf(s,TEXT("%u KiB %u B"),kilo,rest);
      else      sprintf(s,TEXT("%u KiB"     ),kilo);
      return s;
   }

   sprintf(s,TEXT("%u B"),rest);
   return s;
}

#endif
