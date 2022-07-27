#pragma once
#ifndef getnetdeviceaddr_SOURCE_INCLUDED
#define getnetdeviceaddr_SOURCE_INCLUDED
/* getnetdeviceaddr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the IPvX socket address bound to a specific local network
 *    device/interface with name 'devname'
 *
 *    MSwin: devname is a unambiguous substring of a network adapter description, not a GUID !
 *    Unix : devname is a true device name: "eth0", "ib0", "gm0" etc.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Nov/23: Carsten Dehning, Initial release
 *    $Id: getnetdeviceaddr.c 5530 2017-08-25 11:51:53Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"

#if IS_MSWIN

#include <iphlpapi.h>
#pragma comment(lib,"iphlpapi")

#if INCLUDE_STATIC
   #include "getipadapters.c"
   #include "stristr_w.c"
#endif

C_FUNC_PREFIX int getnetdeviceaddr(const char *devname, USOCKADDR *usockaddr)
{
   const IP_ADAPTER_ADDRESSES *aa;
   unsigned nfound = 0; /* count the no. of matches to avoid any ambiguity */
   wchar_t  wdev[256];


   MEMZERO(usockaddr,sizeof(USOCKADDR));
   MultiByteToWideChar(CP_ACP,0,devname,-1,wdev,countof(wdev));
   for(aa=getipadapters(0); aa; aa=aa->Next)
   {
      XMSG_DEBUG2
      (
         "getnetdeviceaddr(\"%s\"): Device \"%S\".\n"
         ,devname
         ,aa->FriendlyName
      );

      if (aa->OperStatus != IfOperStatusUp)
      {
         XMSG_DEBUG0("getnetdeviceaddr: Device not up.\n");
         continue; /* Not operating */
      }

      switch(aa->IfType)
      {
         case IF_TYPE_ETHERNET_CSMACD:
         case IF_TYPE_IEEE80211:
         case IF_TYPE_TUNNEL:
            break;

         default: /* Not an  ethernet device */
            XMSG_DEBUG0("getnetdeviceaddr: Device not ethernet.\n");
            continue;
      }

      if (!(aa->IfIndex) && !(aa->Ipv6IfIndex))
      {
         XMSG_DEBUG0("getnetdeviceaddr: Device not IPvX.\n");
         continue; /* Not an IPvX device */
      }

      /*
       * devname must be at least a unique substring of the device description
       */
      if (!stristr_w(aa->Description,wdev) && !stristr_w(aa->FriendlyName,wdev))
      {
         XMSG_DEBUG0("getnetdeviceaddr: Device does not match.\n");
         continue;
      }

      if (aa->FirstUnicastAddress)
      {
         SOCKET_ADDRESS *saddr = &(aa->FirstUnicastAddress->Address);

         memcpy(usockaddr,saddr->lpSockaddr,saddr->iSockaddrLength);
         XMSG_INFO3
         (
            "getnetdeviceaddr(\"%s\"): Found network device \"%S\" -> \"%s\".\n"
            ,devname
            ,aa->Description
            ,"TODO-ADDRESS-CONVERSION"
         );

         if (!_wcsicmp(wdev,aa->Description) || !_wcsicmp(wdev,aa->FriendlyName))
         {
            /* Exact name match */
            nfound = 1;
            break;
         }

         nfound++; /* count the no. of matches to avoid any ambiguity */
      }
   }

   return (nfound == 1) ? 0 : (nfound > 1) ? EXDEV : ENOENT;
}

#else

#ifdef IS_SUNOS
   #include <sys/sockio.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

C_FUNC_PREFIX int getnetdeviceaddr(const char *devname, USOCKADDR *usockaddr)
{
   struct ifreq ifr;
   int    sock,ret;


   MEMZERO(usockaddr,sizeof(USOCKADDR));
   if (strlen(devname) >= IFNAMSIZ)
   {
      return ENAMETOOLONG;
   }

   /* Try IPv6 & IPv4 socket, favor IPv6 */
   if ((sock=socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP)) < 0 &&
       (sock=socket(AF_INET ,SOCK_DGRAM,IPPROTO_IP)) < 0)
   {
      XMSG_FATAL2
      (
         "getnetdeviceaddr(\"%s\"): Can\'t create socket: %s.\n"
         ,devname
         ,strerror(errno)
      );
   }

   strcpy(ifr.ifr_name,devname);
   ret = ioctl(sock,SIOCGIFADDR,&ifr);
   close(sock);
   if (ret)
      return ENOENT;

   /* Yeah, we have the device "devname" */
   memcpy(&(usockaddr->saddr),&(ifr.ifr_addr),sizeof(struct sockaddr));
   return 0;
}

#endif
#endif
