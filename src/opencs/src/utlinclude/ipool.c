#pragma once
#ifndef IPOOL_SOURCE_INCLUDED
#define IPOOL_SOURCE_INCLUDED
/* ipool.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Unmanaged item pool memory for fast allocation and free of items with
 *       - each of size ipool->isize
 *       - estimated initial no. of ipool->nitem_0 items.
 *       - post allocation no. of ipool->nitem_x.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/Apr: Carsten Dehning, Initial release
 *    $Id: ipool.c 5545 2017-08-29 11:26:31Z dehning $
 *
 *****************************************************************************************
 */
#include "ipool.h"
#include "xmem.h"
#include "xmsg.h"

#if INCLUDE_STATIC
   #include "xmsg.c"
   #include "ulong2adp.c"
   #include "ulong2adm.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX void *IPOOL_allocate(IPOOL *ipool)
/****************************************************************************************/
/*
 * Get the next memory block ipool->memp[ipool->block] pointer and head and tail pointers.
 */
{
   const unsigned block = ipool->block; /* Current block number */

   /*
    * The block size for the first block is different from the subsequent blocks
    * since the initial allocation should already be a good guess.
    */
   const size_t alsize = (block>0) ? ipool->alloc_x : ipool->alloc_0;
   const size_t nitems = alsize/ipool->isize;


   if (block >= IPOOL_MAX_BLOCKS)
   {
      XMSG_FATAL2
      (
         "***** IPOOL_allocate(\"%s\"): No more memory block pointers available.\n"
         "      Recompile with IPOOL_MAX_BLOCKS > %d.\n"
         ,ipool->name
         ,IPOOL_MAX_BLOCKS
      );
   }

   if (ipool->memp[block])
   {
      if (ipool->prlog)
      {
         printf
         (
            "***** IPOOL_allocate(\"%s\"): Using old block #%u with %s items.\n"
            ,ipool->name
            ,block
            ,ulong2adp(nitems,NULL,0)
         );
      }
   }
   else
   {
      if (ipool->prlog)
      {
         char sa[32],sb[32];
         printf
         (
            "***** IPOOL_allocate(\"%s\"): New block #%u with %s items, %s.\n"
            ,ipool->name
            ,block
            ,ulong2adp(nitems,sa,sizeof(sa))
            ,ulong2adm(alsize,sb,sizeof(sb))
         );
      }

      if ((ipool->memp[block]=CAST_CHARP(MALLOC(alsize))) == NULL)
      {
         XMSG_FATAL3
         (
            "***** IPOOL_allocate(\"%s\"): Failed to allocate new memory block #%d with %s bytes.\n"
            ,ipool->name
            ,block
            ,ulong2adp(alsize,NULL,0)
         );
      }
   }

   /*
    * Assign the current head pointer.
    * ipool->tail points to the first byte behind the allocated buffer!
    */
   ipool->tail = (ipool->head=ipool->memp[block]) + nitems*ipool->isize;
   ipool->block++; /* advance to the next block */

   if (ipool->zmemory) /* clear the item memory */
   {
      MEMZERO(ipool->head,alsize);
   }

   return ipool->head;
}

/****************************************************************************************/
C_FUNC_PREFIX void IPOOL_cleanup(IPOOL *ipool)
/****************************************************************************************/
/*
 * Free allocated memory in reverse order of allocation and clear the IPOOL structure.
 */
{
   int i;

   if (ipool->prlog)
   {
      printf
      (
         "***** IPOOL_cleanup(\"%s\")\n"
         ,ipool->name
      );
   }

   for (i=IPOOL_MAX_BLOCKS-1; i>=0; i--)
   {
      if (ipool->memp[i])
         FREE(ipool->memp[i]);
   }

   MEMZERO(ipool,sizeof(IPOOL));
}

/****************************************************************************************/
C_FUNC_PREFIX void IPOOL_setup
(
   IPOOL       *ipool,
   const char  *name,
   const size_t isize, /* item size */
   const size_t nitem, /* no. of items with first allocation */
   const size_t niadd, /* additional no. of items with subsequent allocation */
   const int    zmem,  /* true: zero memory after allocation */
   const int    prlog  /* true: print debug/logging information */
)
/****************************************************************************************/
{
   const size_t isizea = (isize < sizeof(void *))
                           ? sizeof(void *)
#if IS_64BIT
                           : ((isize+7)>>3)<<3; /* 8 byte alignment */
#else
                           : ((isize+3)>>2)<<2; /* at least 4 byte alignment */
#endif
   size_t nitems_0 = (nitem < 4) ? 4 : nitem;
   size_t nitems_x = (niadd < 4) ? 4 : niadd;


   if (ipool->alloc_0 < (nitems_0*isizea) || ipool->alloc_x < (nitems_x*isizea) || !ipool->memp[0])
   {
      /* Allocation > already allocated memory: Start again */
      if (ipool->memp[0])
         IPOOL_cleanup(ipool);

      ipool->alloc_0 = nitems_0*isizea;
      ipool->alloc_x = nitems_x*isizea;
   }

   ipool->name    = (STRHASLEN(name)) ? name : "NONAME";
   ipool->isize   = isizea;
   ipool->zmemory = zmem;
   ipool->prlog   = prlog;

   if (ipool->prlog)
   {
      nitems_0 = ipool->alloc_0/isizea;
      nitems_x = ipool->alloc_x/isizea;

      printf
      (
         "***** IPOOL_setup(\"%s\"):\n"
         "      isize: %9u -> %9u\n"
         "      nitem: %9u -> %9u\n"
         "      niadd: %9u -> %9u\n"
         "      Total no. of available items: %u + %u x %u.\n"
         ,ipool->name
         ,CAST_UINT(isize),CAST_UINT(ipool->isize)
         ,CAST_UINT(nitem),CAST_UINT(nitems_0)
         ,CAST_UINT(niadd),CAST_UINT(nitems_x)
         ,CAST_UINT(nitems_0)
         ,IPOOL_MAX_BLOCKS-1
         ,CAST_UINT(nitems_x)
      );
   }

   ipool->block = 0;      /* (Re)start with memory block 0 */
   IPOOL_allocate(ipool); /* Do the first allocation */
}

/****************************************************************************************/

#if IPOOL_USE_GETITEM
/****************************************************************************************/
C_FUNC_PREFIX void *IPOOL_getitem(IPOOL *ipool)
/****************************************************************************************/
/*
 * Get the pointer to next free piece of memory, advance ipool->head.
 * ipool->tail points to the first byte behind the allocated buffer!
 */
{
   void *p = (ipool->head < ipool->tail)
                  ? ipool->head
                  : IPOOL_allocate(ipool);
   ipool->head += ipool->isize;
   return p;
}

/****************************************************************************************/
#endif

#if IPOOL_USE_USAGE
/****************************************************************************************/
C_FUNC_PREFIX int IPOOL_usage(const IPOOL *ipool, size_t *pnblocks, size_t *pnitems)
/****************************************************************************************/
/*
 * Get/print statistics about memory usage.
 */
{
   size_t   nitems  = 0; /* used no. of items */
   unsigned nblocks = 0; /* used no. of blocks */
   unsigned i;


   /*
    * If the ipool has no blocks allocated it might have been
    * cleaned up before.
    */
   if (!ipool->alloc_0 || !ipool->block)
   {
      if (pnitems ) *pnitems  = 0;
      if (pnblocks) *pnblocks = 0;
      return -1; /* Return error */
   }

   for (i=0; i<ipool->block; i++)
   {
      if (ipool->memp[i])
      {
         size_t mem = (i == ipool->block-1)
                        /* Currently used block: calculate used items */
                        ? (ipool->head-ipool->memp[i])
                        /* Previously used block: full block already used */
                        : (i) ? ipool->alloc_x : ipool->alloc_0;
         nitems += mem/ipool->isize;
         nblocks++;
      }
   }

   if (ipool->prlog)
   {
      printf
      (
         "***** IPOOL_usage(\"%s\"): %s items in %u block%s, (%.2f%%).\n"
         ,ipool->name
         ,ulong2adp(nitems,NULL,0)
         ,nblocks,plurals(nblocks)
         ,(100.0*nitems*ipool->isize)/(ipool->alloc_0 + (nblocks-1)*ipool->alloc_x)
      );
   }

   if (pnitems ) *pnitems  = nitems;
   if (pnblocks) *pnblocks = nblocks;
   return 0; /* Return OK */
}

/****************************************************************************************/
#endif

#if IPOOL_USE_COMPACT
/****************************************************************************************/
C_FUNC_PREFIX size_t IPOOL_compact
(
   IPOOL       *ipool,
   const char  *name,
   const size_t dsize,
   int (*check_function)(const void *p)
)
/****************************************************************************************/
/*
 *
 */
{
   char    *src     ,*dst;
   char    *src_tail,*dst_tail;
   size_t   ni;
   size_t   nitems = 0; /* initially used no. of items */
   size_t   nileft = 0; /* finally used no. of items */
   unsigned i = ipool->prlog;

   ipool->prlog = 0;
   IPOOL_usage(ipool,NULL,&nitems);
   ipool->prlog = i;

   if (!nitems)
   {
      /*
       * If the ipool has no blocks allocated it might have been
       * cleaned up before.
       */
      if (ipool->prlog)
      {
         printf
         (
            "***** IPOOL_compact(\"%s\"): item pool is empty.\n"
            ,ipool->name
         );
      }
      return CAST_SIZE(-1);
   }

   dst      = src      = ipool->memp[i=0];
   dst_tail = src_tail = src + (ipool->alloc_0/ipool->isize)*ipool->isize;

   for (ni=0; (ni<nitems) && (dst<dst_tail); ni++)
   {
      if (check_function(src))
      {
         if (dst < src)
            memcpy(dst,src,dsize);

         dst += dsize;
         nileft++;
      }

      if ((src+=ipool->isize) >= src_tail)
      {
         if ((src=ipool->memp[++i]) == NULL)
            break;
         src_tail = src + (ipool->alloc_x/ipool->isize)*ipool->isize;
      }
   }

   if (ni < nitems)
   {
      XMSG_FATAL2
      (
         "***** IPOOL_compact(\"%s\"): Internal error.\n"
         "      Cannot compact all items into a single buffer: Buffer too small.\n"
         "      For at least %u items a secondary buffer is required.\n"
         ,ipool->name
         ,CAST_UINT(nitems-ni)
      );
   }

   if (STRHASLEN(name))
      ipool->name = name;

   if (ipool->prlog)
   {
      char sa[32],sb[32];
      printf
      (
         "***** IPOOL_compact(\"%s\"):\n"
         ,ipool->name
      );
      printf
      (
         "      %s items out of %s left (%.2f%%).\n"
         ,ulong2adp(nileft,sa,sizeof(sa))
         ,ulong2adp(nitems,sb,sizeof(sb))
         ,(100.0*nileft)/nitems
      );
      printf
      (
         "      %s bytes out of %s bytes left (%.2f%%).\n"
         ,ulong2adp(nileft*dsize       ,sa,sizeof(sa))
         ,ulong2adp(nitems*ipool->isize,sb,sizeof(sb))
         ,(100.0*nileft*dsize)/(nitems*ipool->isize)
      );
   }

   ipool->head  = dst;
   ipool->isize = dsize;
   ipool->tail  = ipool->memp[0] + (ipool->alloc_0/ipool->isize)*ipool->isize;
   return nileft;
}

/****************************************************************************************/
#endif

#endif
