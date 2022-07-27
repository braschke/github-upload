#ifndef usockaddr_ntop_SOURCE_INCLUDED
#define usockaddr_ntop_SOURCE_INCLUDED
/* usockaddr_ntop.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Multi-platform inet_ntop() wrapper with USOCKADDR.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: usockaddr_ntop.c 3143 2014-08-22 11:07:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

#if INCLUDE_STATIC
   #include "usockaddr_unmap.c"
#endif

C_FUNC_PREFIX
const char *usockaddr_ntop(const USOCKADDR *usaddr, char *dst, const size_t size)
{
   USOCKADDR us;

#if IS_MSWIN
   DWORD len = (DWORD)size;
#endif

   if (!dst || !usaddr)
      return NULL;

   us = *usaddr; /* make a local struct copy*/
   usockaddr_unmap(&us);

#if IS_MSWIN
   return (WSAAddressToStringA(&(us.saddr),sizeof(USOCKADDR),NULL,dst,&len))
#else
   return (!inet_ntop
            (
               us.saddr.sa_family,
               (us.saddr.sa_family == AF_INET6)
                  ? (void *)&(us.saddr_ipv6.sin6_addr)
                  : (void *)&(us.saddr_ipv4.sin_addr.s_addr),
               dst,
               size
            )
          )
#endif
      ? NULL : dst;
}
#endif
