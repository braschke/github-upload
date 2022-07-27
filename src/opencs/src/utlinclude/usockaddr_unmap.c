#pragma once
#ifndef usockaddr_unmap_SOURCE_INCLUDED
#define usockaddr_unmap_SOURCE_INCLUDED
/* usockaddr_unmap.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Unmap an IPv4 mapped IPv6 USOCKADDR back into a true IPv4 USOCKADDR.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: usockaddr_unmap.c 4243 2016-04-29 05:10:15Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

#if IS_MSWIN
   #include <wininet.h>
   #include <ws2tcpip.h>
   #include <mstcpip.h>
   #pragma comment(lib,"ws2_32")
#endif

#define GET_IPV4_ADDR(ip6addr) (((const u_short *)(ip6addr))+6)

C_FUNC_PREFIX
int usockaddr_unmap(USOCKADDR *usaddr)
{
   if (usaddr->saddr.sa_family == AF_INET6 && IN6_IS_ADDR_V4MAPPED(&(usaddr->saddr_ipv6.sin6_addr)))
   {
      /* Convert usaddr back into a true IPv4 address */
      unsigned      addr;
      const u_short port = usaddr->saddr_ipv6.sin6_port;

      memcpy(&addr,GET_IPV4_ADDR(&(usaddr->saddr_ipv6.sin6_addr)),sizeof(unsigned));
      usaddr->saddr_ipv4.sin_addr.s_addr = addr;
      usaddr->saddr_ipv4.sin_port        = port;
      usaddr->saddr_ipv4.sin_family      = AF_INET;
      return 1; /* TRUE */
   }
   return 0; /* FALSE */
}

#undef GET_IPV4_ADDR

#endif
