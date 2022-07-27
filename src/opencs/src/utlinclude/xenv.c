#pragma once
#ifndef xenv_SOURCE_INCLUDED
#define xenv_SOURCE_INCLUDED
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
 *    $Id: xenv.c 5647 2017-10-19 15:33:30Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xmsg.h"
#include "xmem.h"


#define _XENV_STRMAX    4096 /* max. TCHARs in a "name=value" string */
#define _XENV_GROWSZ     512 /* table grows in 1/2K bunches */


#if XENV_USE_PTR || XENV_USE_SET

   static struct _xenv_data_struct
   {
   #if XENV_USE_PTR
      /*
       * optional pointer to getenv() wrapper function may be required in case of
       * parallel jobs where the environment variable is only defined on the master
       * process and needs to be transported - somehow - from the master to
       * all other parallel processes.
       */
      TCHAR * (*getenv)(const TCHAR *name);
   #endif

      TCHAR  **envv;  /* TCHAR *envv[n*_XENV_GROWSZ] */
      unsigned count; /* no. of entries in the envv */
      unsigned alloc; /* no. of entries mallocated */
   } _xenv_data =
   {
   #if XENV_USE_PTR
      NULL,
   #endif
      NULL,
      0,
      0
   };

#endif

/****************************************************************************************/



#if XENV_USE_SET

/****************************************************************************************/
static int _xenv_do_search(const TCHAR *name)
/****************************************************************************************/
{
   TCHAR        **envv = _xenv_data.envv;
   const unsigned envc = _xenv_data.count;


   if (envc > 0)
   {
      unsigned i,j;
      int      ucname[128];

      /* make a "NAME=" string */
      for(i=0; name[i] && i<124; i++)
         ucname[i] = TOUPPER(name[i]);
      ucname[i  ] = TEXT('=');
      ucname[i+1] = TEXT('\0');


      for(i=0; i<envc; i++)
      {
         const TCHAR *env = envv[i];
         for(j=0; ucname[j]; j++)
            if (TOUPPER(env[j]) != ucname[j])
               goto NEXT_VAR;

         return CAST_INT(i);
   NEXT_VAR:;
      }
   }
   return -1;
}

/****************************************************************************************/
C_FUNC_PREFIX int XENV_set(const TCHAR *name, const TCHAR *value)
/****************************************************************************************/
{
   TCHAR *cp;
   size_t len = STRLENP(name);
   int    pos;


   if (!len) return -1;
   len += STRLENP(value) + 2;
   if ((pos=_xenv_do_search(name)) >= 0)
   {
      /* overwrite the existing entry, reallocate only if new size > old size */
      if (STRLEN(_xenv_data.envv[pos]) >= len-1)
         goto DO_ASSIGN; /* use existing buffer */
      FREE(_xenv_data.envv[pos]);
   }
   else
   {
      /* make a new entry in the envv */
      if (_xenv_data.count+1 > _xenv_data.alloc)
      {
         _xenv_data.alloc += _XENV_GROWSZ;
         _xenv_data.envv = (_xenv_data.envv == NULL)
            ? CAST_TCHARPP(MALLOC (                _xenv_data.alloc*sizeof(TCHAR *)))
            : CAST_TCHARPP(REALLOC(_xenv_data.envv,_xenv_data.alloc*sizeof(TCHAR *)));
      }
      pos = CAST_INT(_xenv_data.count++);
   }

   _xenv_data.envv[pos] = CAST_TCHARP(MALLOC(len));

DO_ASSIGN:
   cp = _xenv_data.envv[pos];
   STRCPY(cp,name);
   STRCAT(cp,TEXT("="));
   if (STRHASLEN(value))
      STRCAT(cp,value);
   return pos;
}

/****************************************************************************************/

#endif

/*
 * public functions
 */
/****************************************************************************************/
C_FUNC_PREFIX TCHAR *XENV_get(const TCHAR *name)
/****************************************************************************************/
/*
 * get the string value of the extended environment
 */
{
   /* true environment has highest priority */
#if XENV_USE_PTR
   TCHAR *val = (_xenv_data.getenv) ? _xenv_data.getenv(name) : _STD_C_GETENV(name);
#else
   TCHAR *val = _STD_C_GETENV(name);
#endif

#if XENV_USE_SET
   if (!val && _xenv_data.count)
   {
      /*  no environment, try to find it in our envv */
      int pos = _xenv_do_search(name);
      if (pos >= 0)
      {
         val = _xenv_data.envv[pos];
         STRJUMPCHAR(val,TEXT('='));
         if (*val) ++val;
      }
   }
#endif

   return val;
}

/****************************************************************************************/


#if XENV_USE_PUT
/****************************************************************************************/
C_FUNC_PREFIX int XENV_put(const TCHAR *neqv)
/****************************************************************************************/
/*
 * append/overwrite full string neqv "name=value"
 */
{
   TCHAR *cp;
   size_t len;
   TCHAR  name[128];


   if (STRHASLEN(neqv)
      && (cp=STRCHR(neqv,TEXT('='))) != NULL
      && (len=CAST_SIZE(cp-neqv)) < 128)
   {
      memcpy(name,neqv,len*sizeof(TCHAR));
      name[len] = TEXT('\0');
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
C_FUNC_PREFIX void XENV_init(TCHAR *(*ptrGetenv)(const TCHAR *name))
/****************************************************************************************/
{
   _xenv_data.getenv  = ptrGetenv;
}
/****************************************************************************************/
#endif



#if XENV_USE_ENVC
/****************************************************************************************/
C_FUNC_PREFIX int XENV_envc(void)
/****************************************************************************************/
{
   return CAST_INT(_xenv_data.count);
}
/****************************************************************************************/
#endif


#if XENV_USE_ENVV
/****************************************************************************************/
C_FUNC_PREFIX const TCHAR **XENV_envv(void)
/****************************************************************************************/
{
   return CCAST_INTO(const TCHAR **,_xenv_data.envv);
}
/****************************************************************************************/
#endif

#if XENV_USE_CLEAN
/****************************************************************************************/
C_FUNC_PREFIX void XENV_clean(void)
/****************************************************************************************/
{
   if (_xenv_data.envv)
   {
      const unsigned n = _xenv_data.count;
      unsigned       i;
      for (i=0; i<n; i++)
      {
         FREE(_xenv_data.envv[i]);
      }
      FREE(_xenv_data.envv);
   }

   _xenv_data.envv  = NULL;
   _xenv_data.count =
   _xenv_data.alloc = 0;
}

/****************************************************************************************/
#endif

#if XENV_USE_GETINT
#if INCLUDE_STATIC
   #include "str2int.c"
#endif
/****************************************************************************************/
C_FUNC_PREFIX int XENV_getint(const TCHAR *name, int *pval)
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
C_FUNC_PREFIX int XENV_getlong(const TCHAR *name, long *pval)
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
C_FUNC_PREFIX int XENV_getfloat(const TCHAR *name, float *pval)
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
C_FUNC_PREFIX int XENV_getdouble(const TCHAR *name, double *pval)
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
C_FUNC_PREFIX int XENV_getbool(const TCHAR *name, int *pval)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && !str2bool(name,pval));
}
/****************************************************************************************/
#endif



#if XENV_USE_DEFINED
/****************************************************************************************/
C_FUNC_PREFIX int XENV_defined(const TCHAR *name)
/****************************************************************************************/
{
   return (GETENV(name) != NULL);
}
/****************************************************************************************/
#endif



#if XENV_USE_ISTRUE
/****************************************************************************************/
C_FUNC_PREFIX int XENV_istrue(const TCHAR *name)
/****************************************************************************************/
{
   return (((name=GETENV(name)) != NULL) && *name && STRCMP(name,TEXT("0")));
}
/****************************************************************************************/
#endif



#if XENV_USE_SCANS && !IS_MSWIN
/****************************************************************************************/
C_FUNC_PREFIX int XENV_scans(const TCHAR *name, const TCHAR *fmt, ...)
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

#if INCLUDE_STATIC
   #include "strunquote.c"
   #include "strtoprop.c"
   #include "fxgets.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX int XENV_load(const TCHAR *fname)
/****************************************************************************************/
/*
 * load a file and append/overwrite the variable envv
 */
{
   FILE  *fp;
   TCHAR *name, *value;
   int    nadded = 0;
   TCHAR  xline[_XENV_STRMAX];


#if IS_MSWIN
   fp = _tfopen(fname,TEXT("r"));
#else
   fp = fopen(fname,"r");
#endif

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
   while ((name=fxgets(xline,countof(xline),fp,TEXT("#"))) != NULL)
   {
      /* find '=' or ' ' */
      if ((value = STRCHR(name,TEXT('='))) == NULL)
      {
         value = name; STRJUMPSPACE(value);
      }
      if (*value)
      {
         *value = TEXT('\0');
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

/****************************************************************************************/
 static int _xenv_compare_qsort(const void *p1, const void *p2)
/****************************************************************************************/
/*
 * compare the syntactical relevant name of a variable name=value for qsort
 */
{
   return STRICMP(*((const TCHAR **)p1),*((const TCHAR **)p2));
}

/****************************************************************************************/
C_FUNC_PREFIX void XENV_print(FILE *fp, int align)
/****************************************************************************************/
{
   TCHAR          *equ;
   TCHAR         **envv = _xenv_data.envv;
   const unsigned  envc = _xenv_data.count;
   unsigned        i;
   unsigned        maxlen = 0;
   char            fmt[16];


   if (fp)
   {
      strcpy(fmt,"%s\n");
   }
   else
   {
      align = -1;
      strcpy(fmt,"   %s\n");
   }


   /* sort before printout */
   if (envc > 0)
   {
      qsort(envv,envc,sizeof(TCHAR *),_xenv_compare_qsort);

      if (!align) /* center alignment: get the max. length of a variable name */
      {
         for(i=0; i<envc; i++)
         {
            unsigned len;
            equ = STRCHR(envv[i],TEXT('='));
            if (equ && (len=CAST_UINT(equ-envv[i])) > maxlen)
               maxlen = len;
         }
         if (maxlen)
            sprintf(fmt,"%%%us = %%s\n",maxlen);
      }
      else if (align > 0) /* right bounded alignment: get the max length of "var=value" */
      {
         for(i=0; i<envc; i++)
         {
            const unsigned len = CAST_UINT(STRLEN(envv[i]));
            if (len > maxlen)
               maxlen = len;
         }
         if (maxlen)
            sprintf(fmt,"%%%us\n",maxlen);
      }
      /* else align < 0: left bounded fmt="%s\n" */
   }

   if (fp)
   {
      /* save into file */
      fprintf
      (
         fp,
         "#\n"
         "# Extended environment: %u of %u\n"
         "#\n",
         envc,_xenv_data.alloc
      );

      if (align) /* left or right bounded */
      {
         for(i=0; i<envc; i++) fprintf(fp,fmt,envv[i]);
      }
      else /* centered: split at '=' and print var = value */
      {
         for(i=0; i<envc; i++)
         {
            if ((equ=STRCHR(envv[i],TEXT('='))) != NULL)
            {
               *equ = TEXT('\0'); fprintf(fp,fmt,envv[i],equ+1); *equ = TEXT('=');
            }
            else
            {
               fprintf(fp,fmt,envv[i],"");
            }
         }
      }
      fputc('\n',fp);
   }
   else if (XMSG_GETLEVEL() >= XMSG_LEVEL_INFO)
   {
      /* save to mapped stdout */
      XMSG_INFO2
      (
         "Extended environment: %u of %u\n",
         envc,_xenv_data.alloc
      );

      for(i=0; i<envc; i++)
         XMSG_INFO1(fmt,envv[i]);
   }
}

/****************************************************************************************/

#endif

/****************************************************************************************/

#undef _XENV_STRMAX
#undef _XENV_GROWSZ

#endif
