#pragma once
#ifndef getifnamev_SOURCE_INCLUDED
#define getifnamev_SOURCE_INCLUDED
/* getifnamev.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns an (malloc'ed) vector with names of active non loopback socket interfaces
 *    char **namev = getifnamev();
 *    ..use ........
 *    free(namev);
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Mar/26: Carsten Dehning, Initial release
 *    $Id: getifnamev.c 5530 2017-08-25 11:51:53Z dehning $
 *
 *****************************************************************************************
 */
#define WITH_MAIN 0
#include "stdsocket.h"

#ifndef MALLOC
   #define MALLOC malloc
#endif

#if WITH_MAIN
   #define XMSG_USE_CIO 1
   #include "xmsg.h"
   #include "xmsg.c"
#else
   #include "xmsg.h"
#endif

/****************************************************************************************/

#if IS_MSWIN

#include <iphlpapi.h>
#pragma comment(lib,"iphlpapi")
#include "xmem.h"

#if INCLUDE_STATIC
   #include "getipadapters.c"
#endif

   C_FUNC_PREFIX char **getifnamev(void)
   {
      const IP_ADAPTER_ADDRESSES *aa;
      char   **namev;
      char    *name;
      size_t   total;
      unsigned nfound;


      /* First round: count and determine memory size */
      total = nfound = 0;
      for(aa=getipadapters(0); aa; aa=aa->Next)
      {
         //if (aa->OperStatus != IfOperStatusUp)
         //   continue; /* Not operating */

         switch(aa->IfType)
         {
            case IF_TYPE_ETHERNET_CSMACD:
            case IF_TYPE_IEEE80211:
            case IF_TYPE_TUNNEL:
               //if ((aa->PhysicalAddress)[0] == 0)  /* IP="0.0.0.0" indicates a deactivated adapter */
               //   continue;
               if (!STRHASLEN(aa->Description))
                  continue;

               total += wcslen(aa->Description) + 1;
               nfound++;
               break;

            default:
               break;
         }
      }

      if (!nfound)
      {
         return NULL;
      }

      /* allocate namev in a single block */
      name  = (char  *)MALLOC((nfound+1)*sizeof(char *) + total);
      namev = (char **)name;
      name += (nfound+1)*sizeof(char *);

      /* step 2: copy names into the namev */
      nfound = 0;
      for(aa=getipadapters(0); aa; aa=aa->Next)
      {
         const wchar_t *desc;
         //if (aa->OperStatus != IfOperStatusUp)
         //   continue; /* Not operating */

         switch(aa->IfType)
         {
            case IF_TYPE_ETHERNET_CSMACD:
            case IF_TYPE_IEEE80211:
            case IF_TYPE_TUNNEL:
               //if ((aa->PhysicalAddress)[0] == 0)  /* IP="0.0.0.0" indicates a deactivated adapter */
               //   continue;
               if (!STRHASLEN(aa->Description))
                  continue;

               desc = aa->Description;
               namev[nfound++] = name;
               while((*name++ = (char)*desc++) != 0)
                  ;
               break;

            default:
               break;
         }
      }

      namev[nfound] = NULL;
      return namev;
   }

/****************************************************************************************/

#elif defined(IS_MACOSX)

   /*
    * BSD style: use the getifaddrs() wrapper to the ioctl() calls
    */
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <net/if_dl.h>
   #include <ifaddrs.h>

   C_FUNC_PREFIX char **getifnamev(void)
   {
      struct ifaddrs *ifaphead;
      struct ifaddrs *ifap;
      char         **namev;
      char          *name;
      size_t         total,len;
      int            n;


      if (getifaddrs(&ifaphead))
      {
         XMSG_FATAL1
         (
            "getifnamev: Can\'t get network interface info: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      /* step 1: count and determine memory size */
      for(total=n=0,ifap=ifaphead; ifap; ifap=ifap->ifa_next)
      {
         if (ifap->ifa_addr
            && ifap->ifa_addr->sa_family == AF_LINK
            && (len=strlen(ifap->ifa_name)) > 0)
         {
            total += len + 1;
            n++;
         }
      }
      if (!n) return NULL;

      /* allocate namev in a single block */
      name  = (char  *)MALLOC((n+1)*sizeof(char *) + total);
      namev = (char **)name;
      name += (n+1)*sizeof(char *);

      /* step 2: copy names into the namev */
      for(n=0,ifap=ifaphead; ifap; ifap=ifap->ifa_next)
      {
         if (ifap->ifa_addr
            && ifap->ifa_addr->sa_family == AF_LINK
            && (len=strlen(ifap->ifa_name)) > 0)
         {
            namev[n++] = strcpy(name,ifap->ifa_name);
            name += len + 1;
         }
      }

      freeifaddrs(ifaphead);
      namev[n] = NULL;
      return namev;
   }

/****************************************************************************************/

#else

   #ifdef IS_SUNOS
      #include <sys/sockio.h>
   #endif
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <sys/ioctl.h>
   #include <net/if.h>
   #include <net/if_arp.h>

   #ifdef IS_AIX
      /*
       * NOTE: On AIX structures returned are no really unions but are of
       * different length and not just sizeof(struct ifreq).
       * Therefore the length needs to be checked.
       * See: http://www.ibm.com/developerworks/aix/library/au-ioctl-socket.html
       */
      #define IFR_NEXT(ifr)\
         ifr = (struct ifreq *)(((char *)ifr)\
               + sizeof(ifr->ifr_name)\
               + (\
                  (ifr->ifr_addr.sa_len<sizeof(ifr->ifr_addr))\
                     ? sizeof(ifr->ifr_addr)\
                     : ifr->ifr_addr.sa_len\
                 ))
   #else
      #define IFR_NEXT(ifr) ifr++
   #endif

   C_FUNC_PREFIX char **getifnamev(void)
   {
      char         **namev;
      char          *name;
      struct ifreq  *ifr, *ifr_tail;
      struct ifreq   ifreqs[32]; /* buffer save for 32 adapters */
      struct ifconf  ifc;
      size_t         total,len;
      int            n,fd;


      if ((fd=socket(AF_INET6,SOCK_DGRAM,IPPROTO_IP)) < 0 &&
          (fd=socket(AF_INET ,SOCK_DGRAM,IPPROTO_IP)) < 0)
      {
         XMSG_FATAL1
         (
            "getifnamev: Can\'t create socket: %s.\n",
            strerror(errno)
         );
         return NULL;
      }

      /* assign the buffer */
      ifc.ifc_buf = (char *)ifreqs;
      ifc.ifc_len = sizeof(ifreqs);
      if (ioctl(fd,SIOCGIFCONF,&ifc) == -1)
      {
         XMSG_FATAL1
         (
            "getifnamev: Can\'t get interface list: %s.\n",
            strerror(errno)
         );
         close(fd);
         return NULL;
      }

      ifr_tail = (struct ifreq *)(((char *)(ifc.ifc_req)) + ifc.ifc_len);

      /* First round: count and determine memory size */
      for(total=n=0,ifr=ifc.ifc_req; ifr<ifr_tail; IFR_NEXT(ifr))
      {
         struct ifreq ifrf;

         switch(((struct sockaddr *)&(ifr->ifr_addr))->sa_family)
         {
            case AF_INET6:
            case AF_INET:
               if (STRHASLEN(ifr->ifr_name))
               {
                  len = strlen(ifr->ifr_name);
                  break;
               }

            default: /* Skip this interface */
               continue;
         }

         /* NOTE: make/check a copy here to avoid overwriting of ifr->ifr_addr */
         memcpy(&ifrf,ifr,sizeof(struct ifreq));
         if (ioctl(fd,SIOCGIFFLAGS,&ifrf)      != -1
            && ((ifrf.ifr_flags)&IFF_UP      ) !=  0
            && ((ifrf.ifr_flags)&IFF_LOOPBACK) ==  0)
         {
            /* valid adapter: update size and count */
            total += len + 1;
            n++;

#if WITH_MAIN
   #if defined(IS_LINUX)
         {
            int ret;
            memcpy(&ifrf,ifr,sizeof(struct ifreq));
            ret = ioctl(fd,SIOCGIFHWADDR,&ifrf);
            if (ret == -1)
               perror("ioctl(SIOCGIFHWADDR)");
            else
               printf
               (
                  "device \"%s\", ret=%d, SIOCGIFHWADDR=%02x-%02x-%02x-%02x-%02x-%02x\n"
                  ,ifrf.ifr_name,ret
                  ,(unsigned char)ifrf.ifr_addr.sa_data[0]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[1]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[2]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[3]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[4]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[5]
               );
         }
   #endif
   #if  defined(IS_LINUX) || defined(IS_AIX) || defined(IS_SUNOS) || defined(IS_IRIX65) || defined(IS_HPUX11) || defined(IS_OSFALPHA)
         {
            int ret;
            memcpy(&ifrf,ifr,sizeof(struct ifreq));
            ret = ioctl(fd,SIOCGIFADDR,&ifrf);
            if (ret == -1)
               perror("ioctl(SIOCGIFADDR)");
            else
               printf
               (
                  "device \"%s\", ret=%d, SIOCGIFADDR=%02x-%02x-%02x-%02x-%02x-%02x\n"
                  ,ifrf.ifr_name,ret
                  ,(unsigned char)ifrf.ifr_addr.sa_data[0]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[1]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[2]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[3]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[4]
                  ,(unsigned char)ifrf.ifr_addr.sa_data[5]
               );
         }
   #endif
   #if defined(IS_SUNOS) || defined(IS_HPUX11)
         {
            int ret;
            struct arpreq arp;

            /*
             * HPUX11 & Linux for Itanium2:
             * Use a union to avoid compiler complaints about misalignment.
             */
            union
            {
              void               *p;
              struct sockaddr_in *s;
            } us,ud;

            MEMZERO(&arp,sizeof(arp));
            ud.p = &(arp.arp_pa   );
            us.p = &(ifrf.ifr_addr);
            (ud.s)->sin_addr.s_addr = (us.s)->sin_addr.s_addr;

            ret = ioctl(fd,SIOCGARP,&arp);
            if (ret == -1) perror("ioctl(SIOCGARP)");
            printf
            (
               "device \"%s\", ret=%d, SIOCGARP=%02x-%02x-%02x-%02x-%02x-%02x\n"
               ,ifrf.ifr_name,ret
               ,(unsigned char)arp.arp_ha.sa_data[0]
               ,(unsigned char)arp.arp_ha.sa_data[1]
               ,(unsigned char)arp.arp_ha.sa_data[2]
               ,(unsigned char)arp.arp_ha.sa_data[3]
               ,(unsigned char)arp.arp_ha.sa_data[4]
               ,(unsigned char)arp.arp_ha.sa_data[5]
            );
         }
   #endif
#endif
         }
      }
      if (!n) return NULL;

      /* allocate namev in a single block */
      name  = (char  *)MALLOC((n+1)*sizeof(char *) + total);
      namev = (char **)name;
      name += (n+1)*sizeof(char *);

      /* Second round: copy names into the namev */
      for(n=0,ifr=ifc.ifc_req; ifr<ifr_tail; IFR_NEXT(ifr))
      {
         struct ifreq ifrf;

         switch(((struct sockaddr *)&(ifr->ifr_addr))->sa_family)
         {
            case AF_INET6:
            case AF_INET:
               if (STRHASLEN(ifr->ifr_name))
               {
                  len = strlen(ifr->ifr_name);
                  break;
               }

            default: /* Skip this interface */
               continue;
         }

         memcpy(&ifrf,ifr,sizeof(struct ifreq));
         if (ioctl(fd,SIOCGIFFLAGS,&ifrf)      != -1
            && ((ifrf.ifr_flags)&IFF_UP      ) !=  0
            && ((ifrf.ifr_flags)&IFF_LOOPBACK) ==  0)
         {
            /* Valid adapter: build up the namev */
            namev[n++] = strcpy(name,ifr->ifr_name);
            name += len + 1;
         }
      }

      close(fd);
      namev[n] = NULL; /* NULL terminated namev[] */
      return namev;
   }

   #undef IFR_NEXT

/****************************************************************************************/

#endif


#if WITH_MAIN
int main(void)
{
   char **namev = getifnamev();
   if (namev)
   {
      unsigned i;
      for(i=0; namev[i]; i++)
         printf("INTERFACE: \"%s\"\n",namev[i]);
   }
   else
      puts("getifnamev: no name vector.");
   return 0;
}

#endif

#undef WITH_MAIN

#endif
