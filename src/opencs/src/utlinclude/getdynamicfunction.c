#ifndef __GETDYNAMICFUNCTION_SOURCE_INCLUDED
#define __GETDYNAMICFUNCTION_SOURCE_INCLUDED
/* getdynamicfunction.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    platform independent utilities to manage dynamic link libraries and functions
 *    return the address of the function flname = "functionname::libname"
 *       a) load the dynamic library "libname" if it is not already done.
 *       b) get the address of the function within the library
 *
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 *
 * Reviews/changes:
 *    2006/Jun/01: Carsten Dehning, Initial release
 *    $Id: getdynamicfunction.c 1019 2013-06-07 16:18:12Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include "xmem.h"

#if IS_MSWIN

   #if INCLUDE_STATIC
      #include "strxerror.c"
   #endif
   #define _DYNLIB_STRERROR()             strxerror(-1) /* use GetLastError() */
   #define _DYNLIB_LOAD(_filename)        LoadLibraryA(_filename)

#else

   #include <dlfcn.h>
   #define _DYNLIB_STRERROR()             dlerror() /* replacement for strerror() */
   #define _DYNLIB_LOAD(_filename)        dlopen(_filename, RTLD_LAZY)

#endif

#define _DYNLIB_NAME_MAX 128
#define _DYNLIB_LIBS_MAX  32


/****************************************************************************************/
 C_FUNC_PREFIX void *getdynamicfunction(const char *flname, int dieOnError)
/****************************************************************************************/
{
   /* struct containing the handle and name of all loaded libs */
   struct dlhandle
   {
      void *handle;
      char  name[_DYNLIB_NAME_MAX];
   };

          struct dlhandle *dynlib;
   static struct dlhandle *staticDYNLIBList[_DYNLIB_LIBS_MAX];
   static size_t           staticDYNLIBCount = 0;

   char     libName[_DYNLIB_NAME_MAX];
   char     fcnName[_DYNLIB_NAME_MAX];
   char    *cp;
   size_t   index;
   void    *pfunct;
   void    *handle;

#if 0 /* prepared for VISTA */
#if IS_MSWIN

   /*
    * emulate UNIX behaviour and use the LD_LIBRARY_PATH as the .ddl search PATH
    */
   static int ld_library_path_set = 0;
   if (!ld_library_path_set)
   {
      char ldlibpath[4096];

      if (GetEnvironmentVariableA(LD_LIBRARY_PATH,ldlibpath,4096) && ldlibpath[0])
      {
         char  *head, *last;
         for(head=last=(char*)ldLibPath; last && *head; head=last+1)
         {
            if ((last = strchr(head,';')) != NULL)
               *last = '\0';
            SetDllDirectory(head);
         }
      }
      ld_library_path_set = 0;
   }

#endif
#endif

   /* check argument length */
   STRJUMPNOSPACE(flname);
   XMSG_ACTION1("Loading dynamic function \"%s\".\n",flname);
   if (strlen(flname) >= (_DYNLIB_NAME_MAX-8))
      XMSG_FATAL2
      (
         "Can\'t load dynamic function \"%s\":\n"
         "   Name longer than %d characters.\n",
         flname,_DYNLIB_NAME_MAX-8
      );


   /* split into "fcnName::libName" */
   strcpy(fcnName,flname);
   STRJUMPCHARCP(fcnName,cp,':');
   if (cp[0] != ':' || cp[1] != ':' || !ISALPHA(cp[2]))
      XMSG_FATAL1
      (
         "Can\'t load dynamic function \"%s\":\n"
         "   Not \"function::libname\".\n",
         flname
      );

   cp[0] = '\0';
   strcpy(libName,cp+2);

   /* if .DLL/.so suffix is missing, append it now */
   cp = strrchr(libName,'.');
   if (!cp || STRICMP_A(cp,LD_SO_EXTENSION))
      strcat(libName,LD_SO_EXTENSION);


   /* check if the library is already loaded */
   for(index=0; index<staticDYNLIBCount; index++)
      if (!STRICMP_A(staticDYNLIBList[index]->name,libName))
      {
         dynlib = staticDYNLIBList[index];
         goto LIB_IS_LOADED;
      }


   /* load a new library and append it to the list */
   if (staticDYNLIBCount >= _DYNLIB_LIBS_MAX)
      XMSG_FATAL1
      (
         "Can\'t load dynamic function \"%s\":\n"
         "   More than %d dynamic libraries loaded.\n",
         _DYNLIB_LIBS_MAX
      );


   XMSG_ACTION1("Loading dynamic library \"%s\".\n",libName);
   handle = _DYNLIB_LOAD(libName);
   if (!handle)
   {
      if (!dieOnError) return NULL;
      XMSG_FATAL2
      (
         "Can\'t open library \"%s\":\n"
         "   %s.\n",
         libName, _DYNLIB_STRERROR()
      );
   }

   dynlib = (struct dlhandle *)MALLOC(sizeof(struct dlhandle));
   MEMZERO(dynlib,sizeof(struct dlhandle));
   strcpy(dynlib->name,libName);
   dynlib->handle = handle;

   staticDYNLIBList[staticDYNLIBCount++] = dynlib;


LIB_IS_LOADED:

   /*
    * get the function address from the library
    * cast is required to avoid compiler complaints.
    */
#if IS_MSWIN
   *((FARPROC *)(&pfunct)) = GetProcAddress(dynlib->handle,fcnName);
#else
   *((void **)(&pfunct)) = dlsym(dynlib->handle,fcnName);
#endif

   if (!pfunct)
   {
      if (!dieOnError) return NULL;
      XMSG_FATAL3
      (
         "Can\'t get dynamic function \"%s\" from library \"%s\":\n"
         "   %s.\n",
         fcnName, dynlib->name, _DYNLIB_STRERROR()
      );
   }
   return pfunct;
}

/****************************************************************************************/

#undef _DYNLIB_STRERROR
#undef _DYNLIB_LOAD
#undef _DYNLIB_NAME_MAX
#undef _DYNLIB_LIBS_MAX

#endif
