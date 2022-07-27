#pragma once
#ifndef fisdevice_SOURCE_INCLUDED
#define fisdevice_SOURCE_INCLUDED
/* fisdevice.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    check if a filename is a special devicename:
 *    return   0=false
 *             1=unix device
 *             2=windows device name
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Jun/07: Carsten Dehning, Initial release
 *    $Id: fisdevice.c 5457 2017-08-04 16:49:05Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


C_FUNC_PREFIX
int fisdevice(const TCHAR *fname)
{
   size_t len = STRLENP(fname)*sizeof(TCHAR);

   switch(len)
   {
      case 3:
         if (!MEMICMP(fname,TEXT("NUL"),len)) return 2;
         if (!MEMICMP(fname,TEXT("AUX"),len)) return 2;
         if (!MEMICMP(fname,TEXT("PRN"),len)) return 2;
         if (!MEMICMP(fname,TEXT("CON"),len)) return 2;
         if (!MEMICMP(fname,TEXT("COM"),len)) return 2;
         if (!MEMICMP(fname,TEXT("LPT"),len)) return 2;
         break;

      case 5:
         if (!memcmp(fname,TEXT("/dev/"),len)) return 1; /* unix */
         break;

      case 6:
         if (!MEMICMP(fname,TEXT("CONIN$"),len)) return 2;
         break;

      case 7:
         if (!MEMICMP(fname,TEXT("CONOUT$"),len)) return 2;
         break;

      default:
         break;
   }

   return 0;
}
#endif
