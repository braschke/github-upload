#ifndef islocalhost_SOURCE_INCLUDED
#define islocalhost_SOURCE_INCLUDED
/* islocalhost.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test whether a hostname is in fact the local host.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/09: Carsten Dehning, Initial release
 *    $Id: islocalhost.c 3042 2014-08-05 15:36:11Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"

#if INCLUDE_STATIC
   #include "usockaddr_islocal.c"
#endif

C_FUNC_PREFIX
int islocalhost(const char *hostname)
{
   struct addrinfo *aihead,*ai;
   struct addrinfo  hints;
   int              ret;


   if (!STRHASLEN(hostname)
         || !STRICMP_A(hostname,"localhost")
         || !STRICMP_A(hostname,"loopback")
      )
      return 1; /* return TRUE */

   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_CANONNAME;
   if ((ret=getaddrinfo(hostname,NULL,&hints,&aihead)) != 0)
   {
      XMSG_WARNING2
      (
         "Can\'t resolve hostname \"%s\": %s.\n"
         ,hostname
         ,gai_strerror(ret)
      );
      return -1; /* return ERROR */
   }

   ret = 0;
   for(ai=aihead; ai; ai=ai->ai_next)
   {
      USOCKADDR addr;

      addr.saddr = *(ai->ai_addr); /* copy struct sockaddr */
      if (usockaddr_islocal(&addr) > 0)
      {
         ret = 1;
         break;
      }
   }

   freeaddrinfo(aihead);
   return ret;
}
#endif
