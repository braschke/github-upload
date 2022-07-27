#pragma once
#ifndef getusername_SOURCE_INCLUDED
#define getusername_SOURCE_INCLUDED
/* getusername.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Wrapper to get the login name of the current user.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/May/23: Carsten Dehning, Initial release
 *    $Id: getusername.c 5464 2017-08-07 18:19:48Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   #include <lmcons.h>

   #ifndef IS_MINGW
      #pragma comment(lib,"advapi32")
   #endif

   #define MY_UNAME_MAX  UNLEN

#else

   #include <pwd.h>
   #define MY_UNAME_MAX  64

#endif

C_FUNC_PREFIX
TCHAR *getusername(void)
{
   static TCHAR usersName[MY_UNAME_MAX+1] = TEXT("");

   if (!usersName[0])
   {
   #if IS_MSWIN

      DWORD dwSize = MY_UNAME_MAX;
      if (!GetUserName(usersName,&dwSize) || !usersName[0])
         _sntprintf(usersName,MY_UNAME_MAX,TEXT("GetUserName:E-%u"),(unsigned)GetLastError());

   #else

      struct passwd *pwent = getpwuid(getuid());
      if (pwent && STRHASLEN(pwent->pw_name))
         strncpy(usersName,pwent->pw_name,MY_UNAME_MAX);
      else
         snprintf(usersName,MY_UNAME_MAX,"getpwuid:E-%d",errno);

   #endif

      usersName[MY_UNAME_MAX] = TEXT('\0');
   }

   return usersName;

}
#undef MY_UNAME_MAX
#endif
