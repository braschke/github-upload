#ifndef strhash_SOURCE_INCLUDED
#define strhash_SOURCE_INCLUDED
/* strhash.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Implements a dynamically growing hash of strings and alls its functions and
 *    preprocessor macros.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/10: Carsten Dehning, Initial release
 *    $Id: strhash.c 2711 2014-02-24 10:46:09Z dehning $
 *
 *****************************************************************************************
 */
#if !defined(MALLOC) || !defined(FREE)
#include "xmem.h"
#endif

#define _SH_HASHSIZE    101
#define _SH_DEFAULTK    TEXT("__DEFAULT__") /* default key used in case of an empty key argument */
#define _SH_DEFAULTV    TEXT("")            /* default val used in case of an empty val argument */

typedef struct _string_hash_node_struct SHNODE_t;
struct _string_hash_node_struct
{
  SHNODE_t *next;
  TCHAR    *key;
  TCHAR    *val;
};

typedef SHNODE_t *SH_t;


/****************************************************************************************/
static int _sh_hash(const TCHAR *key)
/****************************************************************************************/
{
   unsigned hi;


   for(hi=0; *key; key++)
      hi = CAST_UINT(TOLOWER(*key)) + 31*hi;
   return CAST_INT(hi % _SH_HASHSIZE);
}

/****************************************************************************************/
static SHNODE_t *_sh_node(const SHNODE_t *head, const TCHAR *key)
/****************************************************************************************/
{
   SHNODE_t *np;

   for(np=(SHNODE_t *)head; np; np=np->next)
      if (!STRICMP(np->key,key))
         break;

   return np;
}

/****************************************************************************************/
C_FUNC_PREFIX SH_t *SH_new(void)
/****************************************************************************************/
{
   SH_t *sh = (SH_t *)MALLOC(sizeof(SH_t)*_SH_HASHSIZE);
   MEMZERO(sh,sizeof(SH_t)*_SH_HASHSIZE);
   return sh;
}

/****************************************************************************************/
C_FUNC_PREFIX void SH_del(SH_t *sh, const TCHAR *key)
/****************************************************************************************/
{
   SHNODE_t *np,*prev;
   int       hi;


   if (!STRHASLEN(key)) key = _SH_DEFAULTK;

   hi   = _sh_hash(key);
   prev = NULL;
   for(np=sh[hi]; np; prev=np, np=np->next)
      if (!STRICMP(np->key,key))
         break;

   if (np)
   {
      if (prev) prev->next = np->next;
      else      sh[hi]     = np->next;
      FREE(np->key);
      FREE(np->val);
      FREE(np);
   }
}

/****************************************************************************************/
C_FUNC_PREFIX const TCHAR *SH_get(SH_t *sh, const TCHAR *key)
/****************************************************************************************/
{
   SHNODE_t *np;


   if (!STRHASLEN(key)) key = _SH_DEFAULTK;
   return ((np=_sh_node(sh[_sh_hash(key)],key)) != NULL) ? np->val : NULL;
}

/****************************************************************************************/
C_FUNC_PREFIX void SH_put(SH_t *sh, const TCHAR *key, const TCHAR *val)
/****************************************************************************************/
{
   SHNODE_t *np;
   int       hi;


   if (!STRHASLEN(key)) key = _SH_DEFAULTK;
   if (!STRHASLEN(val)) val = _SH_DEFAULTV;

   np = _sh_node(sh[hi=_sh_hash(key)],key);
_tprintf(TEXT("SH_put(%s,%s), np=%p\n"),key,val,np);
   if (!np)
   {
      np       = (SHNODE_t *)MALLOC(sizeof(SHNODE_t));
      np->key  = STRDUP(key);
      np->val  = STRDUP(val);
      np->next = sh[hi];
      sh[hi]   = np;
   }
   else if (STRLEN(np->val) >= STRLEN(val))
   {
      STRCPY(np->val,val);
   }
   else
   {
      FREE(np->val);
      np->val = STRDUP(val);
   }
}

/****************************************************************************************/
C_FUNC_PREFIX void SH_reset(SH_t *sh)
/****************************************************************************************/
{
   SHNODE_t *np, *next = NULL;
   int       hi;

   for(hi=0; hi<_SH_HASHSIZE; hi++)
      for(np=sh[hi]; np; np=next)
      {
         next = np->next;
         FREE(np->key);
         FREE(np->val);
         FREE(np);
      }

   MEMZERO(sh,sizeof(SH_t)*_SH_HASHSIZE);
}

/****************************************************************************************/
C_FUNC_PREFIX void SH_free(SH_t *sh)
/****************************************************************************************/
{
   SH_reset(sh);
   FREE(sh);
}

/****************************************************************************************/
C_FUNC_PREFIX void SH_print(SH_t *sh, FILE *fp, const TCHAR *hifmt, const TCHAR *sep)
/****************************************************************************************/
{
   int   hi;

#if IS_MSWIN
   /* The ANSI codepage is required for cases with non ASCII chars in the vector. */
   UINT concp = GetConsoleOutputCP();
   SetConsoleOutputCP(GetACP());
#endif

   if (!STRHASLEN(sep  )) sep   = TEXT(" => ");
   if (!STRHASLEN(hifmt)) hifmt = NULL;

   for (hi=0; hi<_SH_HASHSIZE; hi++)
   {
      SHNODE_t *np = sh[hi];
      if (np)
      {
         do {
            if (hifmt) FPRINTF(fp,hifmt,hi);
            FPUTS(np->key,fp);
            FPUTS(sep,fp);
            FPUTS(np->val,fp);
            fputc('\n',fp);
         } while ((np=np->next) != NULL);
      }
      else if (hifmt)
      {
         FPRINTF(fp,hifmt,hi);
         fputs("(null)\n",fp);
      }
   }
#if IS_MSWIN
   SetConsoleOutputCP(concp);
#endif
}

/****************************************************************************************/

#undef _SH_HASHSIZE
#undef _SH_DEFAULTK
#undef _SH_DEFAULTV

/****************************************************************************************/

#if 0
int main(void)
{
   int i;
   SH_t *sh;
   TCHAR* keys[]={TEXT("KEY"),TEXT("address"),TEXT("Phone" ),TEXT("key101"),TEXT("key102")};
   TCHAR* vals[]={TEXT("key"),TEXT("Geber"  ),TEXT("100828"),TEXT("Value1"),TEXT("Value2")};

   sh = SH_new();
   for(i=0;i<5;i++)
      SH_put(sh,keys[i],vals[i]);
   for(i=0; i<100; i++)
   {
      TCHAR k[32],v[32];
      _stprintf(k,TEXT("Key-%03d"),i);
      _stprintf(v,TEXT("V%03d"   ),i);
      SH_put(sh,k,v);
   }

   SH_put(sh,NULL,NULL);
   SH_put(sh,TEXT("PHONE"),TEXT("123"));
   SH_put(sh,TEXT("key"),TEXT("Schlüssel"));
   SH_print(sh,stdout,TEXT("%3d: "),TEXT(" => "));
   SH_free(sh);
   return 0;
}

#endif

#endif
