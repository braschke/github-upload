#ifndef WSAStrerror_SOURCE_INCLUDED
#define WSAStrerror_SOURCE_INCLUDED
/* WSAStrerror.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    WSAStrerror() is a special version of strerror(WSAGetLastError())
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Oct/14: Carsten Dehning, Initial release
 *    $Id: WSAStrerror.c 5464 2017-08-07 18:19:48Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

C_FUNC_PREFIX const char *WSAStrerror(void)
{
   static char wsa_msg[256]; /* static message buffer should be large enought */
   DWORD       wsa_err = WSAGetLastError();


   FormatMessageA
   (
      FORMAT_MESSAGE_FROM_SYSTEM
      |FORMAT_MESSAGE_IGNORE_INSERTS,
      0,
      wsa_err,
      MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
      wsa_msg,
      (DWORD)_countof(wsa_msg),
      NULL
   );

   /* make tail UNIX strerror() compatible without ".\n" */
   if (wsa_msg[0])
   {
      char *end;

      wsa_msg[_countof(wsa_msg)-1] = '\0';
      for(end=wsa_msg+strlen(wsa_msg)-1; end>wsa_msg && isspace(*end); end--)
      {
         *end = '\0';
      }
      if (*end == TEXT('.'))
         *end = '\0';
   }

   if (wsa_msg[0])
   {
      wsa_msg[_countof(wsa_msg)-1] = '\0';
   }
   else /* error message empty: make one */
   {
      _snprintf
      (
         wsa_msg,_countof(wsa_msg),
         "Error #%u: No error message available",
         (unsigned)wsa_err
      );
   }

   return wsa_msg;
}

#endif
#endif
