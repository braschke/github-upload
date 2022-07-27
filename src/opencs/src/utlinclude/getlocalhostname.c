#pragma once
#ifndef getlocalhostname_SOURCE_INCLUDED
#define getlocalhostname_SOURCE_INCLUDED
/* getlocalhostname.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Get the canonical name of the local host in lowercase letters.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getlocalhostname.c 4914 2016-07-13 09:27:10Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"

#ifndef HOST_NAME_MAX
   #define HOST_NAME_MAX 256 /* Sometimes not defined */
#endif

#if IS_MSWIN

#if INCLUDE_STATIC
   #include "usockaddr_unmap.c"
   #include "usockaddr_ntop.c"
#endif

C_FUNC_PREFIX
const char *getlocalhostname(char *lhostname)
{
   static char localHostName[HOST_NAME_MAX] = "";
   struct addrinfo *ai = NULL;
   struct addrinfo  hints;
   DWORD            dwSize;
   int              ierr;


   /* Get the hostname once */
   if (localHostName[0])
      goto EXIT_HOSTNAME;

   dwSize = _countof(localHostName);
   if (!GetComputerNameExA(ComputerNamePhysicalDnsHostname,localHostName,&dwSize))
   {
      XMSG_FATAL1
      (
         "Can\'t get the name of the local host: %s.\n"
         ,WSAStrerror()
      );
      return NULL;
   }
   printf("Netbios name  : <%s>\n",localHostName);

RETRY_NEW_ADDRINFO:
   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_CANONNAME;
   if ((ierr=getaddrinfo(localHostName,NULL,&hints,&ai)) != 0)
   {
      XMSG_FATAL1
      (
         "Can\'t get the canonical name of the local host: %s.\n"
         ,gai_strerror(ierr)
      );
      return NULL;
   }

   printf("Canonical name: <%s>\n",ai->ai_canonname);
   if (_stricmp(ai->ai_canonname,localHostName))
   {
      strcpy_s(localHostName,_countof(localHostName),ai->ai_canonname);
      freeaddrinfo(ai);
      goto RETRY_NEW_ADDRINFO;
   }

   usockaddr_ntop((USOCKADDR *)(ai->ai_addr),localHostName,_countof(localHostName));
   printf("IPv4 address  : <%s>\n",localHostName);
   if (getnameinfo
         (
             ai->ai_addr
            ,(socklen_t)(ai->ai_addrlen)
            ,localHostName
            ,_countof(localHostName)
            ,NULL
            ,0
            ,NI_NAMEREQD
         ))
   {
      XMSG_FATAL1
      (
         "getnameinfo failed: %s.\n"
         ,WSAStrerror()
      );
      return NULL;
   }

   freeaddrinfo(ai);
   strlwr(localHostName);
   printf("Resolved name : <%s>\n",localHostName);

EXIT_HOSTNAME:
   return (lhostname) ? strcpy(lhostname,localHostName) : localHostName;
}

#else

C_FUNC_PREFIX
const char *getlocalhostname(char *lhostname)
{
   static char localHostName[HOST_NAME_MAX] = "";
   int ierr;


   /* Get the hostname once */
   if (!localHostName[0])
   {
      struct addrinfo *ai = NULL;
      struct addrinfo  hints;

      if (gethostname(localHostName,sizeof(localHostName)) < 0)
      {
         XMSG_FATAL1
         (
            "Can\'t get the name of the local host: %s.\n",
            SOCKET_STRERROR()
         );
      }

      MEMZERO(&hints,sizeof(hints));
      hints.ai_family   = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_flags    = AI_CANONNAME;
      if ((ierr=getaddrinfo(localHostName,NULL,&hints,&ai)) != 0)
      {
         XMSG_FATAL1
         (
            "Can\'t get the canonical name of the local host: %s.\n"
            ,gai_strerror(ierr)
         );
         return NULL;
      }
      strcpy(localHostName,ai->ai_canonname);
      freeaddrinfo(ai);
   }

   return (lhostname) ? strcpy(lhostname,localHostName) : localHostName;
}
#endif

#endif
