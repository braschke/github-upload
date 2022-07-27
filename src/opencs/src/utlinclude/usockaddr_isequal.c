#ifndef usockaddr_isequal_SOURCE_INCLUDED
#define usockaddr_isequal_SOURCE_INCLUDED
/* usockaddr_isequal.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test if two USOCKADDR's are equal. Ignore port numbers!
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: usockaddr_isequal.c 3222 2014-09-15 14:41:13Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

#define DO_USOCKADDR_ISEQUAL_DEBUG  0

#if INCLUDE_STATIC && DO_USOCKADDR_ISEQUAL_DEBUG
   #include "usockaddr_ntop.c"
#endif

C_FUNC_PREFIX
int usockaddr_isequal(const USOCKADDR *usaddr1, const USOCKADDR *usaddr2)
{
#if DO_USOCKADDR_ISEQUAL_DEBUG
   char inastr[INET6_ADDRSTRLEN+1];


   usockaddr_ntop(usaddr1,inastr,countof(inastr));
   printf("   usockaddr_isequal-1: <%s,af=%d>\n",inastr,(int)(usaddr1->saddr.sa_family));
   usockaddr_ntop(usaddr2,inastr,countof(inastr));
   printf("   usockaddr_isequal-2: <%s,af=%d>\n",inastr,(int)(usaddr2->saddr.sa_family));
#endif

   if (usaddr1->saddr.sa_family == usaddr2->saddr.sa_family)
   {
      switch (usaddr1->saddr.sa_family)
      {
         case AF_INET: /* IN4_ADDR_EQUAL() */
#if DO_USOCKADDR_ISEQUAL_DEBUG
            printf("   usockaddr_isequal  : 0x%08x == 0x%08x\n",usaddr1->saddr_ipv4.sin_addr.s_addr,usaddr2->saddr_ipv4.sin_addr.s_addr);
#endif
            return (usaddr1->saddr_ipv4.sin_addr.s_addr == usaddr2->saddr_ipv4.sin_addr.s_addr);

         case AF_INET6: /* IN6_ARE_ADDR_EQUAL() */
            return IN6_ARE_ADDR_EQUAL(&(usaddr1->saddr_ipv6.sin6_addr),&(usaddr2->saddr_ipv6.sin6_addr));

         default:
            break;
      }
   }

   return 0; /* FALSE */
}

#undef DO_USOCKADDR_ISEQUAL_DEBUG

#endif
