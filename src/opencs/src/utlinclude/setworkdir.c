#ifndef __setworkdir_SOURCE_INCLUDED
#define __setworkdir_SOURCE_INCLUDED
/* setworkdir.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    wrapper to set the CWD of the current user
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2008, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: setworkdir.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"

#if IS_MSWIN
   #include <direct.h>
#endif

C_FUNC_PREFIX
void setworkdir(const TCHAR *dirname)
{
   if (CHDIR(dirname))
      XMSG_FATAL2
      (
         "Can\'t chdir \"%s\". %s.\n",
         dirname,strerror(errno)
      );
}
#endif
