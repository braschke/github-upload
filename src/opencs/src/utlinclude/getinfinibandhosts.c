#ifndef getinfinibandhosts_SOURCE_INCLUDED
#define getinfinibandhosts_SOURCE_INCLUDED
/* getinfinibandhosts.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Checks if an infiniband device is installed.
 *    If so it returns the list of fabric hosts as a single string with ' '
 *    separated hostnames + the IPvX socket address for an IP on IB.
 *
 *    The true function getinfinibandhosts() is located in a separate library which
 *    links to special infiniband libs, which might not be installed on this system.
 *    Therefore this lib must be dynamically loaded by this wrapper.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2014/Aug/27: Carsten Dehning, Initial release
 *    $Id: getinfinibandhosts.c 3205 2014-09-11 10:44:30Z dehning $
 *
 *****************************************************************************************
 */

#include "stdsocket.h"
#include "xmsg.h"

#if INCLUDE_STATIC
   #include "xmsg.c"
   #include "getinfinibandaddr.c"
#endif

#if IS_MSWIN

   #if INCLUDE_STATIC
      #include "strxerror.c"
   #endif
   #define SO_strerror()   strxerror(-1)

#else /* UNIX, what else */

   #include <dlfcn.h>
   #define SO_strerror()   dlerror()

#endif

C_FUNC_PREFIX char *getinfinibandhosts
(
   const int  af,
   USOCKADDR *usaddr,
   char      *hostlist,
   size_t    *phsize
)
{
#if IS_MSWIN
   typedef char *(WINAPI PGIBH)(char *hostlist, size_t *phsize);
#else
   typedef char *(       PGIBH)(char *hostlist, size_t *phsize);
#endif

   static union
   {
      PGIBH *fp;
      void  *vp; /* UNIX dlsym() requires a void pointer */
   } pgibh = {NULL};
   static char fct_name[] = "getinfinibandhosts";
   static char lib_name[] = "libibhcollect" LD_SO_EXTENSION;


   if (getinfinibandaddr(af,usaddr) < 0)
   {
      /* No infiniband device installed */
      *phsize = 0;
      return NULL;
   }

   if (!pgibh.fp)
   {
      /*
       * Try to load the infiniband dll and get a pointer
       * to the true function
       */
   #if IS_MSWIN
      HMODULE handle = LoadLibraryA(lib_name);
   #else
      void *handle = dlopen(lib_name,RTLD_LAZY|RTLD_GLOBAL);
   #endif
      if (!handle)
      {
         XMSG_WARNING2
         (
            "Failed to load the infiniband library\n\n"
            "   \"%s\"\n\n"
            "%s.\n\n"
            ,lib_name
            ,SO_strerror()
         );
         return NULL;
      }

   #if IS_MSWIN
      pgibh.fp = (PGIBH *)GetProcAddress(handle,fct_name);
   #else
      pgibh.vp = dlsym(handle,fct_name);
   #endif
      if (!pgibh.fp)
      {
         XMSG_WARNING3
         (
            "Failed to hook the infiniband library function\n\n"
            "   \"%s::%s\"\n\n"
            "%s\n\n"
            ,fct_name
            ,lib_name
            ,SO_strerror()
         );

      #if IS_MSWIN
         FreeLibrary(handle);
      #else
         dlclose(handle);
      #endif
         return NULL;
      }
   }

   /* Return the list of fabric hosts */
   return pgibh.fp(hostlist,phsize);
}
#undef SO_strerror

#endif
