#ifndef HASH_HEADER_INCLUDED
#define HASH_HEADER_INCLUDED
/* hash.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines & prototypes for the general object HASH
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *
 * Reviews/changes:
 *    2012/Jun: Carsten Dehning, Initial release
 *    $Id: hash.h 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */

#include "ipool.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * return the hash values for an object
 * compare two objects
 */
typedef unsigned (HASHVAL)(const void *o);
typedef int      (HASHCMP)(const void *o1, const void *o2);

typedef struct _HASH       HASH;
typedef struct _HASHITEM   HASHITEM;


/* chained items of hashed objects */
struct _HASHITEM
{
   HASHITEM   *next;    /* pointer to the next hash item */
   const void *obj;     /* pointer to the stored object  */
   unsigned    hvalue;  /* hash value for this object */
};

struct _HASH
{
   HASHITEM **itemv;    /* pointers to current head of HASHITEM chain */
   HASHVAL   *objval;   /* pointer to required hash value function */
   HASHCMP   *objcmp;   /* pointer to optional object compare function */
   HASHITEM  *i_item;   /* last iterator (HASH_getfirst/next) hash item */
   unsigned   i_addr;   /* last iterator (HASH_getfirst/next) hash address */
   unsigned   hmod;     /* hash modulus */
   IPOOL      ipool;    /* pool of HASHITEM objects */
};


#if !INCLUDE_STATIC
   extern void  HASH_cleanup (HASH *hash);
   extern void  HASH_setup   (HASH *hash, const char *name, HASHCMP *objcmp, HASHVAL *objval, const size_t modulus, const size_t nitem, const size_t niadd);
   extern void *HASH_additem (HASH *hash, const void *obj);
   extern void *HASH_gethead (HASH *hash, const unsigned istart);
   extern void *HASH_getnext (HASH *hash);
   extern int   HASH_getcount(HASH *hash);
#endif

/* Get the first object from the hash */
#define HASH_getfirst(_hash)  HASH_gethead(_hash,0)

#ifdef __cplusplus
}
#endif

#endif
