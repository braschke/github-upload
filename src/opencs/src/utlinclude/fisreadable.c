#pragma once
#ifndef fisreadable_SOURCE_INCLUDED
#define fisreadable_SOURCE_INCLUDED
/* fisreadable.c
 *
 *****************************************************************************************
 *
 * Purpose: UNICODE save
 *    Check if a file exists and is readable.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fisreadable.c 4143 2016-04-16 15:04:16Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

C_FUNC_PREFIX
int fisreadable(const TCHAR *pathname)
{
   HANDLE fh = CreateFile
               (
                  pathname,
                  GENERIC_READ,
                  FILE_SHARE_READ,
                  NULL,
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,
                  NULL
               );
   if (IS_INVALID_HANDLE(fh)) return 0;
   CloseHandle(fh);
   return 1;
}
#endif
#endif
