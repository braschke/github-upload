#pragma once
#ifndef IPOOL_HEADER_INCLUDED
#define IPOOL_HEADER_INCLUDED
/* ipool.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines & prototypes for the umanaged item memory pool IPOOL
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/Apr: Carsten Dehning, Initial release
 *    $Id: ipool.h 5545 2017-08-29 11:26:31Z dehning $
 *
 *****************************************************************************************
 */

#include "stdheader.h"

#if !defined(IPOOL_MAX_BLOCKS) || !IPOOL_MAX_BLOCKS
   #undef  IPOOL_MAX_BLOCKS
   #define IPOOL_MAX_BLOCKS   4
#endif
#ifndef IPOOL_USE_GETITEM
   #define IPOOL_USE_GETITEM  0
#endif
#ifndef IPOOL_USE_COMPACT
   #define IPOOL_USE_COMPACT  0
#endif
#ifndef IPOOL_USE_USAGE
   #define IPOOL_USE_USAGE    0
#endif

#if IPOOL_USE_COMPACT
   #undef  IPOOL_USE_USAGE
   #define IPOOL_USE_USAGE    1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IPOOL
{
   char       *head;                   /* pointer to current head */
   char       *tail;                   /* pointer to the first byte behind the current buffer */
   char       *memp[IPOOL_MAX_BLOCKS]; /* allocated memory chunks */
   const char *name;                   /* name of pool for debugging */
   size_t      isize;                  /* 4/8 byte aligned size of each item */
   size_t      alloc_0;                /* no. of bytes of the first allocated memory block */
   size_t      alloc_x;                /* no. of bytes of subsequent memory blocks */
   unsigned    block;                  /* current block number */
   unsigned    zmemory;                /* if true: zero all allocated memory */
   unsigned    prlog;                  /* print log if true */
} IPOOL;


#if !INCLUDE_STATIC
   extern void   *IPOOL_allocate(      IPOOL *ipool);
   extern void    IPOOL_setup   (      IPOOL *ipool, const char *name, const size_t isize, const size_t nitem, const size_t niadd, const int zmem, const int prlog);
   extern void    IPOOL_cleanup (      IPOOL *ipool);
   extern void   *IPOOL_getitem (      IPOOL *ipool);
   extern int     IPOOL_usage   (const IPOOL *ipool, size_t *pnblocks, size_t *pnitems);
   extern size_t  IPOOL_compact (      IPOOL *ipool, const char *name, const size_t dsize, int (*check_function)(const void *p));
#endif

/*
 * Switch information messages on/off
 */
#define IPOOL_setlog(_ipool,_onoff)   (_ipool)->prlog = _onoff

/*
 * CPP Macro implementation of IPOOL_getitem: sizeof(_itype) MUST BE <= (_ipool)->isize
 * Use (_ipool)->isize for the next available address.
 */
#define IPOOL_GETITEM_ISIZE(_ipool,_item,_itype)\
   _item = SCAST_INTO(_itype *,((_ipool)->head < (_ipool)->tail)\
                        ? (_ipool)->head\
                        : IPOOL_allocate(_ipool)\
                     );\
   (_ipool)->head += (_ipool)->isize


/*
 * CPP Macro implementation of IPOOL_getitem: sizeof(_itype) MUST BE <= (_ipool)->isize
 * Use sizeof((_itype) for the next available address.
 */
#define IPOOL_GETITEM_SITEM(_ipool,_item,_itype)\
   _item = SCAST_INTO(_itype *,((_ipool)->head < (_ipool)->tail)\
                        ? (_ipool)->head\
                        : IPOOL_allocate(_ipool)\
                     );\
   (_ipool)->head += sizeof(_itype)


#define IPOOL_GETBLOCKPTR(_ipool,_block)\
   (_block < IPOOL_MAX_BLOCKS) ? (_ipool)->memp[_block] : NULL

#define IPOOL_GETBLOCKPTR0(_ipool) (_ipool)->memp[0]

#ifdef __cplusplus
}
#endif

#endif
