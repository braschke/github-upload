#pragma once
#ifndef getinfinibandaddr_SOURCE_INCLUDED
#define getinfinibandaddr_SOURCE_INCLUDED
/* getinfinibandaddr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Checks if an infiniband device is installed.
 *    If so it returns the IPvX socket address for an IP on IB.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: getinfinibandaddr.c 4503 2016-05-18 11:19:49Z dehning $
 *
 *****************************************************************************************
 */
#define DO_GETIBADDR_TESTING 0
#if DO_GETIBADDR_TESTING
   #define XMSG_USE_CIO    1
#endif

#include "stdsocket.h"
#if INCLUDE_STATIC
   #include "getipadapters.c"
#endif


#if IS_MSWIN

#if INCLUDE_STATIC
   #include "stristr_w.c"
#endif

C_FUNC_PREFIX int getinfinibandaddr(const int af, USOCKADDR *usaddr)
{
   const IP_ADAPTER_ADDRESSES *aa;


   for(aa=getipadapters(0); aa; aa=aa->Next)
   {
      if (aa->OperStatus != IfOperStatusUp)
         continue; /* Not operating */

      switch(aa->IfType)
      {
         case IF_TYPE_ETHERNET_CSMACD:
         case IF_TYPE_IEEE80211:
         case IF_TYPE_TUNNEL:
            if ((aa->PhysicalAddress)[0] == 0)  /* IP="0.0.0.0" indicates a deactivated adapter */
               continue;

            if (!STRHASLEN(aa->Description))
               continue;

            if (!stristr_w(aa->Description,L"infiniband") && !stristr_w(aa->FriendlyName,L"infiniband"))
               continue;

            if (aa->FirstUnicastAddress)
            {
               SOCKET_ADDRESS *saddr = &(aa->FirstUnicastAddress->Address);
               memcpy(usaddr,saddr->lpSockaddr,saddr->iSockaddrLength);
               switch(usaddr->saddr.sa_family)
               {
                  case AF_INET:
                     if (af != AF_INET6)  /* AF_INET or AF_UNSPEC */
                        return 0;
                     break;

                  case AF_INET6:
                     if (af != AF_INET) /* AF_INET6 or AF_UNSPEC */
                        return 0;
                     break;

                  default:
                     break;
               }
            }
            break;

         default:
            break;
      }
   }

   MEMZERO(usaddr,sizeof(USOCKADDR));
   return -1; /* Not found */
}

/****************************************************************************************/

#elif defined(IS_LINUX) || defined(IS_MACOSX)

/*
 * BSD style: use the getifaddrs() wrapper to the ioctl() calls
 */
#include <ifaddrs.h>
#include <net/if.h>

C_FUNC_PREFIX int getinfinibandaddr(const int af, USOCKADDR *usaddr)
{
   const struct ifaddrs *ifa;


   MEMZERO(usaddr,sizeof(USOCKADDR));
   for(ifa=getipadapters(0); ifa; ifa=ifa->ifa_next)
   {
      if ( !ifa->ifa_addr
         || (ifa->ifa_flags & (IFF_UP|IFF_RUNNING|IFF_LOOPBACK)) != (IFF_UP|IFF_RUNNING)
         || !ifa->ifa_name || memcmp(ifa->ifa_name,"ib",2)
         )
         continue;

      switch(ifa->ifa_addr->sa_family)
      {
         case AF_INET:
            if (af != AF_INET6)  /* AF_INET or AF_UNSPEC */
            {
               memcpy(usaddr,ifa->ifa_addr,sizeof(struct sockaddr));
               return 0;
            }
            break;

         case AF_INET6:
            if (af != AF_INET) /* AF_INET6 or AF_UNSPEC */
            {
               memcpy(usaddr,ifa->ifa_addr,sizeof(struct sockaddr));
               if (!IN6_IS_ADDR_LINKLOCAL(&(usaddr->saddr_ipv6.sin6_addr)))
                  return 0;
               MEMZERO(usaddr,sizeof(USOCKADDR));
            }
            break;

         default:
            break;
      }
   }

   return -1; /* Not found */
}


#else

C_FUNC_PREFIX int getinfinibandaddr(const int af, USOCKADDR *usaddr)
{
   /* NOT IMPLEMENTED */
   return (af || usaddr) ? -1 : -1; /* Keep compiler happy */
}

#endif

#if DO_GETIBADDR_TESTING

#include "usockaddr_ntop.c"
int main(void)
{
   USOCKADDR a;
   if (!getinfinibandaddr(AF_INET6,&a))
   {
      char s[INET6_ADDRSTRLEN+1];
      usockaddr_ntop(&a,s,sizeof(s));
      printf("Infiniband address: AF=%d, ADDR=\'%s\'.\n",a.saddr.sa_family,s);
   }
   else
   {
      puts("No Infiniband device.");
   }
   return 0;
}
#endif

#undef DO_GETIBADDR_TESTING

#endif
