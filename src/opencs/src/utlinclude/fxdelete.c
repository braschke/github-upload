#ifndef __fxdelete_SOURCE_INCLUDED
#define __fxdelete_SOURCE_INCLUDED
/* fxdelete.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    delete a file, NOT a directory like the standard remove() does
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2008, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fxdelete.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "strsjoinl.c"
#endif

C_FUNC_PREFIX
int fxdelete(const TCHAR *pathname, const TCHAR *extension)
{
   /* make filename + extension if extension is set */
   TCHAR pathExt[MAX_PATH];
   if (STRHASLEN(extension))
      pathname = strsjoinl(pathExt,countof(pathExt),NULL,pathname,extension,NULL);

#if IS_MSWIN
   return (DeleteFile(pathname)) ? 0 : -1;
#else
   return unlink(pathname);
#endif
}
#endif
