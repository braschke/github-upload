#pragma once
#ifndef usockaddr_islocal_SOURCE_INCLUDED
#define usockaddr_islocal_SOURCE_INCLUDED
/* usockaddr_islocal.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Check if the USOCKADDR is any local address.
 *    Dependent on the platform:
 *       a) Scan all interface devices for a matching IPvX address.
 *       b) Compare the address with all locally available addresses.
 *    Returns: -1=error, 0=false, 1=true
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Mai/18: Carsten Dehning, Initial release
 *    $Id: usockaddr_islocal.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

#define DO_USOCKADDR_ISLOCAL_DEBUG  0

#if INCLUDE_STATIC
   #include "usockaddr_isequal.c"
   #include "usockaddr_isloopback.c"
   #if DO_USOCKADDR_ISLOCAL_DEBUG
      #include "usockaddr_ntop.c"
   #endif
#endif

#if IS_MSWIN

/*
 * Scan all interfaces for a matching IPvX address.
 */
#if INCLUDE_STATIC
   #include "stristr_w.c"
   #include "getipadapters.c"
#else
   #include <iphlpapi.h>
   #pragma comment(lib,"iphlpapi")
#endif
C_FUNC_PREFIX int usockaddr_islocal(const USOCKADDR *usaddr)
{
   const IP_ADAPTER_ADDRESSES *aa;
#if DO_USOCKADDR_ISLOCAL_DEBUG
   char hostname[512];


   usockaddr_ntop(usaddr,hostname,countof(hostname));
   printf("usockaddr_islocal: TEST <%s,af=%d>\n",hostname,(int)(usaddr->saddr.sa_family));
#endif

   if (usockaddr_isloopback(usaddr))
   {
#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>TRUE\n");
#endif
      return 1; /* true */
   }

#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>FALSE\n");
#endif

   for(aa=getipadapters(0); aa; aa=aa->Next)
   {
      if (aa->OperStatus == IfOperStatusUp
            && aa->FirstUnicastAddress
            && (aa->IfType == IF_TYPE_ETHERNET_CSMACD
               || aa->IfType == IF_TYPE_IEEE80211
               || aa->IfType == IF_TYPE_TUNNEL)
         )
      {
         const IP_ADAPTER_UNICAST_ADDRESS *uca;

         for(uca=aa->FirstUnicastAddress; uca; uca=uca->Next)
         {
            const USOCKADDR *addr = (const USOCKADDR *)(uca->Address.lpSockaddr);
            if (addr->saddr.sa_family == usaddr->saddr.sa_family)
            {
            #if DO_USOCKADDR_ISLOCAL_DEBUG
               usockaddr_ntop(addr,hostname,countof(hostname));
               printf("   usockaddr_islocal: CHECK <%s,af=%d>\n",hostname,(int)(addr->saddr.sa_family));
            #endif
               if (usockaddr_isequal(usaddr,addr))
               {
               #if DO_USOCKADDR_ISLOCAL_DEBUG
                  printf("   usockaddr_islocal: =>TRUE\n");
               #endif
                  return 1;  /* true */
               }
            }
         }
      }
   }

#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: =>FALSE\n");
#endif
   return 0; /* false */
}

#elif defined(IS_LINUX) || defined(IS_MACOSX)

/*
 * Scan all interfaces for a matching IPvX address.
 * BSD style: use the getifaddrs() wrapper to the ioctl() calls.
 */
#if INCLUDE_STATIC
   #include "getipadapters.c"
#else
   #include <ifaddrs.h>
   #include <net/if.h>
#endif
C_FUNC_PREFIX int usockaddr_islocal(const USOCKADDR *usaddr)
{
   const struct ifaddrs *ifa;
#if DO_USOCKADDR_ISLOCAL_DEBUG
   char            hostname[512];


   usockaddr_ntop(usaddr,hostname,countof(hostname));
   printf("usockaddr_islocal: TEST <%s,af=%d>\n",hostname,(int)(usaddr->saddr.sa_family));
#endif

   if (usockaddr_isloopback(usaddr))
   {
#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>TRUE\n");
#endif
      return 1; /* true */
   }

#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>FALSE\n");
#endif

   for(ifa=getipadapters(0); ifa; ifa=ifa->ifa_next)
   {
      if (ifa->ifa_addr &&
         (ifa->ifa_flags&(IFF_UP|IFF_RUNNING|IFF_LOOPBACK)) == (IFF_UP|IFF_RUNNING))
      {
         USOCKADDR addr;

         addr.saddr = *(ifa->ifa_addr); /* copy struct sockaddr */
         if (addr.saddr.sa_family == usaddr->saddr.sa_family)
         {
         #if DO_USOCKADDR_ISLOCAL_DEBUG
            usockaddr_ntop(&addr,hostname,countof(hostname));
            printf("   usockaddr_islocal: CHECK <%s,af=%d>\n",hostname,(int)(addr.saddr.sa_family));
         #endif
            if (usockaddr_isequal(usaddr,&addr))
            {
            #if DO_USOCKADDR_ISLOCAL_DEBUG
               printf("   usockaddr_islocal: =>TRUE\n");
            #endif
               return 1;  /* true */
            }
         }
      }
   }

#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: =>FALSE\n");
#endif
   return 0; /* false */
}

#else /* Unix, what else :-)) */

/*
 * Compare the address with all locally available addresses.
 */
C_FUNC_PREFIX int usockaddr_islocal(const USOCKADDR *usaddr)
{
   struct addrinfo *aihead,*ai;
   struct addrinfo  hints;
   int              ierr;
   char             hostname[512];


#if DO_USOCKADDR_ISLOCAL_DEBUG
   usockaddr_ntop(usaddr,hostname,countof(hostname));
   printf("usockaddr_islocal: TEST <%s,af=%d>\n",hostname,(int)(usaddr->saddr.sa_family));
#endif

   if (usockaddr_isloopback(usaddr))
   {
#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>TRUE\n");
#endif
      return 1; /* true */
   }

#if DO_USOCKADDR_ISLOCAL_DEBUG
   printf("   usockaddr_islocal: (loopback)=>FALSE\n");
#endif

   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = usaddr->saddr.sa_family;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_PASSIVE;
   getlocalhostname(hostname);
   if ((ierr=getaddrinfo(hostname,NULL,&hints,&aihead)) != 0)
   {
      XMSG_FATAL1
      (
         "usockaddr_islocal: getaddrinfo failed: %s.\n"
         ,gai_strerror(ierr)
      );
      return -1; /* fail */
   }

   for(ai=aihead; ai; ai=ai->ai_next)
   {
      const USOCKADDR *addr = (const USOCKADDR *)(ai->ai_addr);
   #if DO_USOCKADDR_ISLOCAL_DEBUG
      usockaddr_ntop(addr,hostname,countof(hostname));
      printf("usockaddr_islocal: CHECK <%s,af=%d>\n",hostname,(int)(addr->saddr.sa_family));
   #endif
      if (usockaddr_isequal(usaddr,addr))
      {
         freeaddrinfo(aihead);
         return 1;  /* true */
      }
   }

   freeaddrinfo(aihead);
   return 0; /* false */
}

#endif

#undef DO_USOCKADDR_ISLOCAL_DEBUG

#endif
