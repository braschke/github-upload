#ifndef getipvxname_SOURCE_INCLUDED
#define getipvxname_SOURCE_INCLUDED
/* getipvxname.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Get the IPV6/IPv4 string address of a hostname.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getipvxname.c 3143 2014-08-22 11:07:34Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"

#if INCLUDE_STATIC
   #include "getlocalhostname.c"
   #include "usockaddr_ntop.c"
#endif

C_FUNC_PREFIX
const char *getipvxname(const char *hostname, int *af, char ipvxname[INET6_ADDRSTRLEN+1])
{
   struct addrinfo *ai = NULL;
   struct addrinfo  hints;


   if (!STRHASLEN(hostname)
         || !STRICMP_A(hostname,"localhost")
         || !STRICMP_A(hostname,"loopback")
      )
      hostname = getlocalhostname(NULL); /* take the local host name */

   /* Must be a hostname: get it's canonical name */
   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = *af;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   if (!getaddrinfo(hostname,NULL,&hints,&ai))
   {
      USOCKADDR usaddr;
      memcpy(&usaddr,ai->ai_addr,ai->ai_addrlen);
      freeaddrinfo(ai);
      if (usockaddr_ntop(&usaddr,ipvxname,INET6_ADDRSTRLEN+1))
      {
         *af = usaddr.saddr.sa_family;
         return ipvxname;
      }
   }

   *af = AF_INET;
   return strcpy(ipvxname,"127.0.0.1"); /* Return the IPv4 localhost address */
}
#endif
