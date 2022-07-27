#ifndef usockaddr_isloopback_SOURCE_INCLUDED
#define usockaddr_isloopback_SOURCE_INCLUDED
/* usockaddr_isloopback.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test if the USOCKADDR's is the loopback address. Ignore port numbers!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/May/20: Carsten Dehning, Initial release
 *    $Id: usockaddr_isloopback.c 3143 2014-08-22 11:07:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

C_FUNC_PREFIX
int usockaddr_isloopback(const USOCKADDR *usaddr)
{
   switch (usaddr->saddr.sa_family)
   {
      case AF_INET:
         return (ntohl(usaddr->saddr_ipv4.sin_addr.s_addr) == INADDR_LOOPBACK);

      case AF_INET6:
         return IN6_IS_ADDR_LOOPBACK(&(usaddr->saddr_ipv6.sin6_addr));

      default:
         break;
   }

   return 0; /* false */
}
#endif
