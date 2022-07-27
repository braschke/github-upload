#pragma once
#ifndef splitporthost_SOURCE_INCLUDED
#define splitporthost_SOURCE_INCLUDED
/* splitporthost.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    From the string 'porthost' extract the port no. and return the pointer to the
 *    hostname. The 'porthost' string may have the formats:
 *
 *       $ENVIRONMENT_VAR
 *       portnumber              returns the local host name
 *       @portnumber             strange but okay: returns the local host name
 *       :portnumber             strange but okay: returns the local host name
 *       hostname                returns the hostname
 *       @hostname               should be okay
 *       :hostname               should be okay
 *       portnumber@hostname     return the hostname and sets the portnumber
 *       portnumber:hostname     return the hostname and sets the portnumber
 *       hostname@portnumber     return the hostname and sets the portnumber
 *       hostname:portnumber     return the hostname and sets the portnumber
 *
 *    Validate the hostname:
 *       warn: 0=no validation, 1=warning, -1=fatal
 *
 *    Note: The returned host string is a pointer to an internal static array!
 *          This array will be overridden every time the method splitporthost is called.
 *          Therefore if you need to use the returned host later on(*), copy it e.g. into
 *          a local array like "char host[HOST_NAME_MAX+10]".
 *
 *          (*) using later on means:
 *             - calling splitporthost again - or any method which uses splitporthost, too
 *              (e.g. socket methods) - and after that carrying on using the first returned host
 *             - calling splitporthost with the previously returned host
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: splitporthost.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdsocket.h"
#include "xmsg.h"
#include <limits.h>

#define DO_SPLITPORTHOST_DEBUG  0

#ifndef HOST_NAME_MAX
   #define HOST_NAME_MAX 256 /* Sometimes not defined */
#endif

/****************************************************************************************/
static int _get_port_number(const char *str)
/****************************************************************************************/
/*
 * Convert string into an unsigned short port number [0..USHRT_MAX]
 * return -1=error or >= 0 short port number
 */
{
   char *endp;

   long port = strtol(str,&endp,10);
   return (*endp  /* Token not properly terminated */
         || port < 0
         || port > USHRT_MAX) /* Got an ushort overflow */
      ? -1  /* Signals invalid port number */
      : CAST_INT(port);
}

/****************************************************************************************/
static int _copy_clean_porthost(const char *src, char *dst, unsigned size)
/****************************************************************************************/
/*
 * Copy "port@host" ignoring any whitespace, "port @ host" -> would become "port@host"
 */
{
   unsigned n = 0;


   if (src)
   {
      size--;
      for(; *src && n<size; src++)
      {
         if (!isspace(*src))
            dst[n++] = *src;
      }
   }

   dst[n] = '\0';
   return n;
}

/****************************************************************************************/
C_FUNC_PREFIX const char *splitporthost(const char *pporthost, int *pport, int warn)
/****************************************************************************************/
{
   static const char sph_prefix[] = "splitporthost: ";
   static const char sph_advise[] = "Must be either [port@:]hostname or hostname@:port";
   static const char sph_envfmt[] = "%sEnvironment variable argument \"%s\" is %s.\n";
   static       char sph_porthost[HOST_NAME_MAX+10];

   char            *at    = NULL;
   struct addrinfo *ai    = NULL;
   struct addrinfo  hints;
   int              port,sep;
   char             ename[256]; /* Name of the optional environment variable */


   /* Remove leading whitespace */
   while (isspace(*pporthost))
      pporthost++;

#if DO_SPLITPORTHOST_DEBUG
   printf("splitporthost(input=%s)\n",pporthost);
#endif

   /*
    * Check if 'porthost' is the name of an environment variable "$VARNAME"
    */
   ename[0] = '\0'; /* No variable definition */
   if (pporthost[0] == '$')
   {
      const char *val;

      /* Keep name of variable for proper error messages */
      if (!_copy_clean_porthost(pporthost+1,ename,countof(ename)) || (!isalpha(ename[0]) && ename[0] != '_'))
      {
         XMSG_FATAL3(sph_envfmt,sph_prefix,pporthost,"bad or incomplete");
      }

      val = getenv(ename);
      if (!STRHASLEN(val))
      {
         /* Empty environment is allowed: leave *pport unchanged and return local hostname */
         if (warn)
         {
            XMSG_WARNING3(sph_envfmt,sph_prefix,ename,(val) ? "empty" : "undefined");
         }
         goto EXIT_LOCALHOST;
      }

   #if DO_SPLITPORTHOST_DEBUG
      printf("splitporthost(value=%s)\n",sph_porthost);
   #endif
      if (!_copy_clean_porthost(val,sph_porthost,countof(sph_porthost)))
         goto EXIT_LOCALHOST;
   }
   else
   {
      if (!_copy_clean_porthost(pporthost,sph_porthost,countof(sph_porthost)))
         goto EXIT_LOCALHOST;
   }


   /*
    * Have the true 'porthost' now: Find the separator between hostname and port number.
    */
   sep = 0;
   pporthost = sph_porthost;

   if ((at=strchr(sph_porthost,'@')) != NULL)
   {
      /*
       * Must be:
       *    [port]@[hostname|IPvX]
       *    [hostname|IPvX]@[port]
       */
      if (strchr(at+1,'@'))
         goto EXIT_BADNAME; /* multiple '@' */

      sep = '@';
   }
   else if ((at=strrchr(sph_porthost,':')) != NULL)
   {
      /* Must be:
       *    [port]:[hostname|IPv4]
       *    [hostname|IPv4]:[port]
       *    [IPv6::::][:port]
       */
      const char *colon = strchr(sph_porthost,':');
      if (colon && (colon < at))
      {
         /* Must be [IPv6::::]:port */
         if (sph_porthost[0] != '[' || *(at-1) != ']')
         {
            /* Looks like a plain IPv6 address string */
            goto NO_SEPARATOR; /* Is either hostname or portnumber */
         }
      }
      sep = ':';
   }
   else
   {
      goto NO_SEPARATOR; /* Is either hostname or portnumber */
   }

#if DO_SPLITPORTHOST_DEBUG
   printf("splitporthost(sep=%c)\n",sep);
#endif

   if (at[1] == '\0') /* '@:' at tail? */
   {
      /*
       * Must be:
       *    port@:
       *    hostname|IPvX@:
       */
      *at = '\0'; /* Remove trailing '@:' */
      goto NO_SEPARATOR; /* Is either hostname or portnumber */
   }

   if (at == pporthost) /* '@:' as the first char? */
   {
      /*
       * Must be:
       *    @:port
       *    @:hostname|IPvX
       */
      pporthost++; /* Skip leading '@:' */
      goto NO_SEPARATOR; /* Is either hostname or portnumber */
   }

   if (!isalnum(at[1])) /* Either hostname@junk or portnumber@junk */
      goto EXIT_BADNAME;

   *at = '\0';
#if DO_SPLITPORTHOST_DEBUG
   printf("splitporthost(1-port=%s)\n",at+1);
#endif
   if ((port=_get_port_number(at+1)) >= 0) /* try "hostname[@:]port" */
   {
      *pport = port;
      return sph_porthost; /* we have a "hostname[@:]port" */
   }

#if DO_SPLITPORTHOST_DEBUG
   printf("splitporthost(2-port=%s)\n",pporthost);
#endif
   if (sep == ':' || (port=_get_port_number(pporthost)) < 0)
   {
      /* port:hostname is invalid and this is not "port@porthost" */
      *at = (char)sep;
      goto EXIT_BADNAME;
   }

   *pport = port;
   at++;
   if (!warn)
      return at;

   /* Try to resolve the hostname */
   MEMZERO(&hints,sizeof(hints));
   hints.ai_family   = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_flags    = AI_CANONNAME;
   if (!getaddrinfo(at,NULL,&hints,&ai))
   {
      if (ai->ai_canonname)
         at = strcpy(sph_porthost,ai->ai_canonname);
      freeaddrinfo(ai);
      return at;
   }

   /* Unresolved hostname */
   warn = (warn < 0) ? XMSG_LEVEL_FATAL : XMSG_LEVEL_WARNING;
   if (ename[0])
   {
      XMSG_MESSAGE
      (
         warn,
         "%sThe value of the environment variable\n"
         "\n"
         "   \"%s=%s\"\n"
         "\n"
         "is invalid. Can\'t resolve hostname \"%s\".\n",
         sph_prefix,ename,sph_porthost,at
      );
   }
   else
   {
      XMSG_MESSAGE
      (
         warn,
         "%sInvalid argument \"%s\": Can\'t resolve hostname \"%s\".\n",
         sph_prefix,sph_porthost,at
      );
   }
   return NULL;


NO_SEPARATOR:
   /* May be single hostname or portnumber. */
   if ((port=_get_port_number(pporthost)) < 0) /* ushort conversion failed ... */
      return pporthost; /* ... then assume porthost is just a hostname */

   /* Port properly set: FALLTHROUGH and return localhostname */
   *pport = port;


EXIT_LOCALHOST:
   if (gethostname(sph_porthost,HOST_NAME_MAX) < 0)
   {
      XMSG_FATAL1
      (
         "Can\'t get the name of the local host: %s.\n",
         SOCKET_STRERROR()
      );
   }
   return sph_porthost;


EXIT_BADNAME:
   if (!warn)
      return NULL;

   warn = (warn < 0) ? XMSG_LEVEL_FATAL : XMSG_LEVEL_WARNING;
   if (ename[0])
   {
      XMSG_MESSAGE
      (
         warn,
         "%sThe value of the environment variable\n"
         "\n"
         "   \"%s=%s\"\n"
         "\n"
         "is invalid. %s.\n",
         sph_prefix,ename,sph_porthost,sph_advise
      );
   }
   else
   {
      XMSG_MESSAGE
      (
         warn,
         "%sInvalid argument \"%s\": %s.\n",
         sph_prefix,sph_porthost,sph_advise
      );
   }
   return NULL;
}

/****************************************************************************************/

#endif
