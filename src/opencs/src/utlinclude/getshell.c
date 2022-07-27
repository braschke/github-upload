#ifndef __getshell_SOURCE_INCLUDED
#define __getshell_SOURCE_INCLUDED
/* getshell.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    wrapper to get the path of basic shell (/bin/sh || cmd.exe)
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getshell.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include "xmem.h"


#if IS_MSWIN
   #if INCLUDE_STATIC
      #include "GetCmdExe.c"
   #endif
#endif

C_FUNC_PREFIX
TCHAR *getshell(void)
{
   static TCHAR *pShell = NULL;

   if (!pShell)
   {
#if IS_MSWIN

      TCHAR path[MAX_PATH+1];
      if (!GetCmdExe(path,countof(path)))
         XMSG_FATAL0("Fatal: Can\'t find CMD.EXE in the PATH and COMSPEC is invalid or not defined.\n");
      SetEnvironmentVariable(TEXT("SHELL"),path);
      pShell = STRDUP(path);

#else

      pShell = getenv("SHELL");
      if (!pShell) pShell = STRDUP("/bin/sh");

#endif
   }
   return pShell;
}
#endif
