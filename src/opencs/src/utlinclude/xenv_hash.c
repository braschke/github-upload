#pragma once
#ifndef xenv_hash_SOURCE_INCLUDED
#define xenv_hash_SOURCE_INCLUDED
/* xenv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    - extended environment manager using environment files
 *    - getenv wrapper for parallel jobs.
 *      The environment may only be defined on
 *      one of the parallel processes
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: xenv_hash.c 4428 2016-05-14 08:57:21Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#include "xmsg.h"
#include "xmem.h"


#define _XENV_STRMAX    4096 /* max. chars in a "name=value" string */

#include "strhash.c"

static SH_t *_xenv_hash = NULL;

#if XENV_USE_PTR
   /*
    * optional pointer to getenv() wrapper function may be required in case of
    * parallel jobs where the environment variable is only defined on the master
    * process and needs to be transported - somehow - from the master to
    * all other parallel processes.
    */
   static char * (*_xenv_getenv)(const char *name) = NULL;
#endif

/****************************************************************************************/



#if XENV_USE_SET

/****************************************************************************************/
C_FUNC_PREFIX int XENV_set(const char *name, const char *value)
/****************************************************************************************/
{
   if (!_xenv_hash) _xenv_hash = SH_new();
   SH_set(_xenv_hash,name,value);
}

/****************************************************************************************/

#endif

/*
 * public functions
 */
/****************************************************************************************/
C_FUNC_PREFIX char *XENV_get(const char *name)
/****************************************************************************************/
/*
 * get the string value of the extended environment
 */
{
   /* true environment has highest priority */
#if XENV_USE_PTR
   char *val = (_xenv_getenv) ? _xenv_getenv(name) : _STD_C_GETENV(name);
#else
   char *val = _STD_C_GETENV(name);
#endif

#if XENV_USE_SET
   if (!val && _xenv_hash)
      val = SH_get(_xenv_hash,name);
#endif

   return val;
}

/****************************************************************************************/


#if XENV_USE_PUT
/****************************************************************************************/
C_FUNC_PREFIX int XENV_put(const char *neqv)
/****************************************************************************************/
/*
 * append/overwrite full string neqv "name=value"
 */
{
   char  *cp;
   size_t len;
   char   name[128];


   if (STRHASLEN(neqv)
      && (cp=strchr(neqv,'=')) != NULL
      && (len=CAST_SIZE(cp-neqv)) < 128)
   {
      memcpy(name,neqv,len);
      name[len] = '\0';
      cp++;
      STRJUMPNOSPACE(cp); /* skip leading whitespace */
      return XENV_set(name,cp);
   }
   return -1;
}

/****************************************************************************************/
#endif


#if XENV_USE_PTR
/****************************************************************************************/
C_FUNC_PREFIX void XENV_init(char *(*ptrGetenv)(const char *name))
/****************************************************************************************/
{
   _xenv_getenv  = ptrGetenv;
}
/****************************************************************************************/
#endif



#if XENV_USE_ENVC
/****************************************************************************************/
C_FUNC_PREFIX size_t XENV_envc(void)
/****************************************************************************************/
{
   return (_xenv_hash) ? SH_count(_xenv_hash) : 0;
}
/****************************************************************************************/
#endif


#if XENV_USE_ENVV
/****************************************************************************************/
C_FUNC_PREFIX const char **XENV_envv(void)
/****************************************************************************************/
{
   return _xenv_hash;
}
/****************************************************************************************/
#endif



#if XENV_USE_GETINT
#if INCLUDE_STATIC
   #include "str2int.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getint(const char *name, int *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2int(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_GETLONG
#if INCLUDE_STATIC
   #include "str2long.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getlong(const char *name, long *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2long(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_GETFLOAT
#if INCLUDE_STATIC
   #include "str2float.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getfloat(const char *name, float *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2float(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_GETDOUBLE
#if INCLUDE_STATIC
   #include "str2double.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getdouble(const char *name, double *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2double(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_GETBOOL
#if INCLUDE_STATIC
   #include "str2bool.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getbool(const char *name, int *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2bool(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_DEFINED
/****************************************************************************************/
C_FUNC_PREFIX int XENV_defined(const char *name)
/****************************************************************************************/
{
   return (GETENV(name) != NULL);
}
/****************************************************************************************/
#endif



#if XENV_USE_ISTRUE
/****************************************************************************************/
C_FUNC_PREFIX int XENV_istrue(const char *name)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && *name && STRCMP(name,"0"));
}
/****************************************************************************************/
#endif



#if XENV_USE_SCANS && !IS_MSWIN
/****************************************************************************************/
C_FUNC_PREFIX int XENV_scans(const char *name, const char *fmt, ...)
/****************************************************************************************/
{
   int n = 0;

   if (((name=GETENV(name)) != NULL) && *name)
   {
      va_list ap;
      va_start(ap,fmt);
      n = vsnscanf(name,fmt,ap);
      va_end(ap);
   }
   return n;
}
/****************************************************************************************/
#endif



#if XENV_USE_LOAD

#include "xmsg.h"

#if INCLUDE_STATIC
   #include "strunquote.c"
   #include "strtoprop.c"
   #include "fxgets.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX int XENV_load(const char *fname)
/****************************************************************************************/
/*
 * load a file and append/overwrite the variable envv
 */
{
   char  *name, *value;
   char   line[_XENV_STRMAX];
   int    nadded = 0;
   FILE  *fp = fopen(fname,"r");


   if (!fp)
   {
      XMSG_DEBUG2
      (
         "XENV_load(%s): %s.\n",
         fname, strerror(errno)
      );
      return -1;
   }

   /* search for "name=value" or "name  value"  */
   while ((name=fxgets(line,sizeof(line),fp,"#")) != NULL)
   {
      /* find '=' or ' ' */
      if ((value = strchr(name,'=')) == NULL)
      {
         value = name; STRJUMPSPACE(value);
      }
      if (*value)
      {
         *value = '\0';
          value = strunquote(value+1,NULL);
      }

      name = strtoprop(strunquote(name,NULL));
      if (!*name)
      {
         XMSG_WARNING3
         (
            "XENV_load(%s): Bad entry \"%s=%s\" ignored.\n",
            fname,name,value
         );
         continue;
      }

      XENV_set(name,value);
      nadded++;
   }

   fclose(fp);
   return nadded;
}

/****************************************************************************************/

#endif

/****************************************************************************************/

#if XENV_USE_PRINT

#include "xmsg.h"

/****************************************************************************************/
 static int _xenv_compare_qsort(const void *p1, const void *p2)
/****************************************************************************************/
/*
 * compare the syntactical relevant name of a variable name=value for qsort
 */
{
   return STRICMP(*((const char **)p1),*((const char **)p2));
}

/****************************************************************************************/
C_FUNC_PREFIX void XENV_print(FILE *fp)
/****************************************************************************************/
{
   char  **envv = _xenv_data.envv;
   size_t  envc = _xenv_data.count;
   size_t  i;

   /* sort before printout */
   if (envc > 0)
      qsort(envv,envc,sizeof(char *),_xenv_compare_qsort);

   if (fp)
   {
      /* save into file */
      fprintf
      (
         fp,
         "#\n"
         "# Extended environment: %lu of %lu\n"
         "#\n",
         (unsigned long)envc,(unsigned long)_xenv_data.alloc
      );

      for(i=0; i<envc; i++)
      {
         fputs(envv[i],fp);
         fputc('\n',fp);
      }
      fputc('\n',fp);
   }
   else if (XMSG_GETLEVEL() >= XMSG_LEVEL_INFO)
   {
      /* save to mapped stdout */
      XMSG_INFO2
      (
         "Extended environment: %lu of %lu\n",
         (unsigned long)envc,(unsigned long)_xenv_data.alloc
      );

      for(i=0; i<envc; i++)
         XMSG_INFO1("   %s\n",envv[i]);
   }
}

/****************************************************************************************/

#endif

/****************************************************************************************/

#undef _XENV_STRMAX

#endif
