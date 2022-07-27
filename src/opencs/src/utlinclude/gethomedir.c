#ifndef __gethomedir_SOURCE_INCLUDED
#define __gethomedir_SOURCE_INCLUDED
/* gethomedir.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Wrapper to get the home path of the current user.
 *    Under MSWin SetHomeEnv() should be called before for proper settings!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: gethomedir.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmem.h"

#if IS_MSWIN

   #pragma comment(lib,"advapi32")
   #pragma comment(lib,"userenv")

   #include <userenv.h>

   C_FUNC_PREFIX TCHAR *gethomedir(void)
   {
      static TCHAR *pHome = NULL;

      if (!pHome)
      {
         /*
          * use SetHomeEnv() before because is repairs "USERPROFILE" and redefines it via
          * SetEnvironmentVariable("USERPROFILE",...), so a simple getenv("USERPROFILE")
          * may return the wrong path!
          * Try to avoid the access to "USERPROFILE": environment may be tainted!
          */
         HANDLE hToken;
         TCHAR  home[MAX_PATH+1];

         home[0] = 0;
         if (OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hToken))
         {
            DWORD dwSize = countof(home);
            if (!GetUserProfileDirectory(hToken,home,&dwSize))
               home[0] = 0;
            CloseHandle(hToken);
         }

         if (!home[0] && !GetEnvironmentVariable(TEXT("USERPROFILE"),home,countof(home)))
            STRCPY(home,TEXT("C:\\"));
         pHome = STRDUP(home);
      }
      return pHome;
   }

#else /* UNIX, what else */

   C_FUNC_PREFIX char *gethomedir(void)
   {
      static char *pHome = NULL;

      if (!pHome)
      {
         /* Try to avoid the access to $HOME: environment may be tainted! */
         struct passwd *pwent = getpwuid(getuid());
         if (pwent && STRHASLEN(pwent->pw_dir))
         {
            pHome = STRDUP(pwent->pw_dir);
         }
         else
         {
            pHome = getenv("HOME");
            pHome = (STRHASLEN(pHome)) ? STRDUP(pHome) : "/";
         }
      }
      return pHome;
   }

#endif
#endif
