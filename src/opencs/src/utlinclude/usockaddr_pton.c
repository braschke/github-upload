#ifndef usockaddr_pton_SOURCE_INCLUDED
#define usockaddr_pton_SOURCE_INCLUDED
/* usockaddr_pton.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Checks if a hostname/address string is a IPv4 or an IPv6 address string.
 *    In case of success the USOCKADDR is set up.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: usockaddr_pton.c 3143 2014-08-22 11:07:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

C_FUNC_PREFIX int usockaddr_pton(const char *addrstr, USOCKADDR *usaddr)
{
   USOCKADDR my_usaddr;


   if (!usaddr) /* Argument is optional */
      usaddr = &my_usaddr;

   MEMZERO(usaddr,sizeof(USOCKADDR));

   if (STRHASLEN(addrstr))
   {
      const char *ap;

      for(ap=addrstr; *ap; ap++)
      {
         switch(*ap)
         {
            case ':': /* Check for a valid IPV6 address string */
            {
            #if IS_MSWIN
               int isize = sizeof(USOCKADDR);
               if (!WSAStringToAddressA((char *)addrstr,AF_INET6,NULL,&(usaddr->saddr),&isize))
            #else
               if (inet_pton(AF_INET6,addrstr,&(usaddr->saddr_ipv6.sin6_addr)))
            #endif
               {
                  usaddr->saddr.sa_family = AF_INET6;
                  return 6;
               }
               return 0;
            }

            case '.': /* Check for a valid IPV4 address string */
            {
            #if IS_MSWIN
               int isize = sizeof(USOCKADDR);
               if (!WSAStringToAddressA((char *)addrstr,AF_INET,NULL,&(usaddr->saddr),&isize))
            #else
               if (inet_pton(AF_INET,addrstr,&(usaddr->saddr_ipv4.sin_addr.s_addr)))
            #endif
               {
                  usaddr->saddr.sa_family = AF_INET;
                  return 4;
               }
               return 0;
            }

            default:
               break;
         }
      }
   }

   return 0;
}
#endif
