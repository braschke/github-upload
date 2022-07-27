#ifndef HASH_SOURCE_INCLUDED
#define HASH_SOURCE_INCLUDED
/* hash.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    General object hash
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/Jun: Carsten Dehning, Initial release
 *    $Id: hash.c 5546 2017-08-29 11:26:49Z dehning $
 *
 *****************************************************************************************
 */
#include "hash.h"

#define FOREACH_HITEM(_hitem,_head) for(_hitem=(_head); _hitem; _hitem=_hitem->next)


/****************************************************************************************/
C_FUNC_PREFIX void HASH_cleanup(HASH *hash)
/****************************************************************************************/
/*
 * Free allocated memory.
 */
{
   IPOOL_cleanup(&(hash->ipool));
   if (hash->itemv)
   {
      FREE(hash->itemv);
      hash->itemv = NULL;
   }
   MEMZERO(hash,sizeof(HASH));
}

/****************************************************************************************/
C_FUNC_PREFIX void HASH_setup
(
   HASH          *hash,
   const char    *name,    /* name of the item pool assigned to this hash */
   HASHCMP       *objcmp,  /* object compare function */
   HASHVAL       *objval,  /* object hash value function */
   const unsigned hmod,    /* hash modulus */
   const size_t   nitem,   /* no. of item with first allocation */
   const size_t   niadd    /* additional no. of items with subsequant allocation */
)
/****************************************************************************************/
{
   unsigned modulus = hmod;

   /* make modulus a multiple of two */
   if (modulus < 32) modulus = 32;
   modulus = ((modulus+1)>>1)<<1;

   MEMZERO(hash,sizeof(HASH));
   hash->itemv  = MALLOC(modulus*sizeof(HASHITEM *));
   hash->objcmp = objcmp;
   hash->objval = objval;
   hash->hmod   = modulus;
   IPOOL_setup(&(hash->ipool),name,sizeof(HASHITEM),nitem,niadd,0,0);
}

/****************************************************************************************/
C_FUNC_PREFIX void *HASH_additem(HASH *hash, const void *obj)
/****************************************************************************************/
/*
 * Add an item from the hash based on its hash value.
 * Returns either the pointer to the new object or the existing object
 */
{
   HASHITEM      *hitem;
   HASHCMP       *objcmp = hash->objcmp;
   IPOOL         *ipool;
   const unsigned hvalue = hash->objval(obj);
   const unsigned haddr  = hvalue % hash->hmod;


   if (objcmp)
   {
      /* Quick find via hash value & final via object comparison */
      FOREACH_HITEM(hitem,hash->itemv[haddr])
      {
         if (hitem->hvalue == hvalue && !objcmp(hitem->obj,obj))
            return CCAST_INTO(void *,hitem->obj); /* return pointer to existing object */
      }
   }
   else
   {
      /* Find via hash value only */
      FOREACH_HITEM(hitem,hash->itemv[haddr])
      {
         if (hitem->hvalue == hvalue)
            return CCAST_INTO(void *,hitem->obj); /* return pointer to existing object */
      }
   }


   /* Get a new HASHITEM from the ipool */
   ipool = &(hash->ipool);
   IPOOL_GETITEM_SITEM(ipool,hitem,HASHITEM);

   /* Add a new item at the head */
   hitem->next        = hash->itemv[haddr];
   hitem->obj         = obj;
   hitem->hvalue      = hvalue;
   hash->itemv[haddr] = hitem;
   return CCAST_INTO(void *,obj); /* return pointer to the added object */
}

/****************************************************************************************/
C_FUNC_PREFIX void *HASH_gethead(HASH *hash, const unsigned istart)
/****************************************************************************************/
/*
 * Get the first object from the hash stating at hash->itemv[istart]
 */
{
   const unsigned hmod = hash->hmod;
   unsigned       i;


   for(i=istart; i<hmod; i++)
   {
      HASHITEM *hitem;
      if ((hitem=hash->itemv[i]) != NULL)
      {
         hash->i_item = hitem;
         hash->i_addr = i;
         return CCAST_INTO(void *,hitem->obj);
      }
   }

   hash->i_item = NULL;
   hash->i_addr = 0;
   return NULL;
}

/****************************************************************************************/
C_FUNC_PREFIX void *HASH_getnext(HASH *hash)
/****************************************************************************************/
/*
 * Get the next iterator object from the hash
 */
{
   HASHITEM *hitem = hash->i_item; /* last iterator item */


   if (!hitem) /* ups */
      return NULL;

   if ((hitem=hitem->next) == NULL)
      return HASH_gethead(hash,hash->i_addr+1);

   hash->i_item = hitem;
   return CCAST_INTO(void *,hitem->obj);
}

/****************************************************************************************/
C_FUNC_PREFIX int HASH_getcount(HASH *hash)
/****************************************************************************************/
/*
 * Count the no. of object in the hash
 */
{
   const unsigned hmod = hash->hmod;
   unsigned       i;
   int            count = 0;

   for(i=0; i<hmod; i++)
   {
      const HASHITEM *hitem;
      FOREACH_HITEM(hitem,hash->itemv[i])
         count++;
   }

   return count;
}

/****************************************************************************************/

#endif
