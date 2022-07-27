#pragma once
#ifndef getipadapters_SOURCE_INCLUDED
#define getipadapters_SOURCE_INCLUDED
/* getipadapters.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Wrapper for the MSWin GetAdaptersAddresses() and the Unix getifaddrs().
 *    The allocated memory remains allocated and is keept static to avoid repeated
 *    syscalls.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Jan/12: Carsten Dehning, Initial release
 *    $Id: getipadapters.c 5304 2017-03-13 15:58:38Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"

#if INCLUDE_STATIC
   #include "xmsg.c"
#endif

#if IS_MSWIN

#include "xmem.h"
#include <iphlpapi.h>
#pragma comment(lib,"iphlpapi")

C_FUNC_PREFIX
const IP_ADAPTER_ADDRESSES *getipadapters(const unsigned dofree)
{
   static IP_ADAPTER_ADDRESSES *aa = NULL;


   if (dofree)
   {
      if (aa)
      {
         FREE(aa);
         aa = NULL;
      }
   }
   else if (!aa)
   {
      ULONG    size = 15000; /* Start MALLOC() with the suggestion from MSDN examples */
      unsigned i;

      for(i=0; i<4; i++)
      {
         DWORD ret;

         aa = (IP_ADAPTER_ADDRESSES *)MALLOC(size);
         if ((ret=GetAdaptersAddresses(AF_UNSPEC,0,NULL,aa,&size)) == ERROR_SUCCESS)
            break;

         FREE(aa);
         aa = NULL;

         if (ret != ERROR_BUFFER_OVERFLOW)
         {
            WSASetLastError(ret);
            XMSG_FATAL2
            (
               "getipadapters: E-%u: %s.\n"
               ,(unsigned)ret
               ,WSAStrerror()
            );
            return NULL;
         }

         /* ERROR_BUFFER_OVERFLOW will cause a junk error message: filename too long */
         /*WSASetLastError(ERROR_INSUFFICIENT_BUFFER);*/
      }
   }

   return aa;
}

#else

/*
 * BSD style: use the getifaddrs() wrapper to the ioctl() calls
 */
#include <ifaddrs.h>
#include <net/if.h>

C_FUNC_PREFIX
const struct ifaddrs *getipadapters(const unsigned dofree)
{
   static struct ifaddrs *ifa = NULL;


   if (dofree)
   {
      if (ifa)
      {
         freeifaddrs(ifa);
         ifa = NULL;
      }
   }
   else if (!ifa)
   {
      if (getifaddrs(&ifa))
      {
         XMSG_FATAL1
         (
            "getipadapters: Can\'t get network interface info: %s.\n",
            strerror(errno)
         );
         return NULL;
      }
   }

   return ifa;
}

#endif

#endif
