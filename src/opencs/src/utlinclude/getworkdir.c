#pragma once
#ifndef getworkdir_SOURCE_INCLUDED
#define getworkdir_SOURCE_INCLUDED
/* getworkdir.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    wrapper to get the CWD of the current user
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getworkdir.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include "xmem.h"

C_FUNC_PREFIX
TCHAR *getworkdir(void)
{
   static TCHAR *pCwd = NULL;
   TCHAR   cwd[2*MAX_PATH];

#if IS_MSWIN

   if (!GetFullPathName(TEXT("."),countof(cwd),cwd,NULL))
      XMSG_FATAL1
      (
         "Can\'t get current working directory. %s.\n",
         strxerror(-1)
      );

   /* convert MSDOS 8.3 names into full pathnames */
   GetLongPathName(cwd,cwd,countof(cwd)); /* same buffer is allowed */

#else

   if (!getcwd(cwd,countof(cwd)))
      XMSG_FATAL1
      (
         "Can\'t get current working directory. %s.\n",
         strerror(errno)
      );

#endif

   if (pCwd)
   {
      if (STRLEN(pCwd) >= STRLEN(cwd))
         return STRCPY(pCwd,cwd);
      FREE(pCwd);
   }
   pCwd = STRDUP(cwd);
   return pCwd;
}
#endif
