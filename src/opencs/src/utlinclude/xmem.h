#ifndef xmem_HEADER_INCLUDED
#define xmem_HEADER_INCLUDED
/* xmem.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Wrapper for malloc/free/strdup with optional malloc failure checks.
 *    The std c malloc stuff may by redefined via pointers from external code.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/20: Carsten Dehning, Initial release
 *    $Id: xmem.h 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */

/*
 * if any of the guys below is defined, completely skip the XMEM stuff
 */
#if !defined(MALLOC) && !defined(FREE) && !defined(REALLOC) && !defined(MEMDUP) && !defined(STRDUP)

#include "stdheader.h"

#ifndef XMEM_USE
   #define XMEM_USE  0
#endif

#if XMEM_USE

   /* set the default usage flags 0/1 if not yet defined */

   #ifndef XMEM_USE_PTR
      #define XMEM_USE_PTR       0
   #endif

   #ifndef XMEM_USE_TRACE
      #define XMEM_USE_TRACE     0
   #endif

   #ifndef XMEM_USE_REALLOC
      #define XMEM_USE_REALLOC   0
   #endif

   #ifndef XMEM_USE_VALLOC
      #define XMEM_USE_VALLOC    0
   #endif

   #ifndef XMEM_USE_DUP
      #define XMEM_USE_DUP       0
   #endif

   /* use the extended allocator */

   #if !INCLUDE_STATIC
   #ifdef __cplusplus
      extern "C" {
   #endif

      #if XMEM_USE_TRACE
         extern void _XMEM_trace(int onoff);
         extern void _XMEM_free (void *ptr, const char *file, int line);
      #else
         #define     _XMEM_trace(_onoff)
         extern void _XMEM_free (void *ptr);
      #endif

      extern void *_XMEM_malloc (size_t size, const char *file, int line);

      #if XMEM_USE_PTR
         /* senseless without pointers */
         extern void _XMEM_init (void *(*ptrMalloc)  (size_t size),
                                 void *(*ptrRealloc) (void *ptr, size_t size),
                                 void  (*ptrFree)    (void *ptr));
      #endif

      #if XMEM_USE_REALLOC
         extern void *_XMEM_realloc(void *buf, size_t size, const char *file, int line);
      #endif

      #if XMEM_USE_VALLOC
         extern void **_XMEM_valloc(size_t n, size_t size, const char *file, int line);
      #endif

      #if XMEM_USE_DUP
         extern void *_XMEM_memdup (const void *buf, size_t size, const char *file, int line);
      #endif

   #ifdef __cplusplus
   }
   #endif
   #endif

   #if XMEM_USE_TRACE
      #define FREE(_ptr)   _XMEM_free(_ptr,__FILE__,__LINE__)
   #else
      #define FREE         _XMEM_free
   #endif

   #define MALLOC(_size)   _XMEM_malloc (_size,__FILE__,__LINE__)

   #if XMEM_USE_DUP
      #define MEMDUP(_ptr,_size) _XMEM_memdup (_ptr,_size,__FILE__,__LINE__)
      #define STRDUP(_str)       (TCHAR *)_XMEM_memdup (_str,STRLENP(_str)+1,__FILE__,__LINE__)
   #endif

   #if XMEM_USE_REALLOC
      #define REALLOC(_ptr,_size)   _XMEM_realloc(_ptr,_size,__FILE__,__LINE__)
   #endif

   #if XMEM_USE_VALLOC
      #define VALLOC(_n,_size)   _XMEM_valloc(_n,_size,__FILE__,__LINE__)
   #endif

#else

   /* do not use the extended allocator: delete all relevant settings */

   #undef  XMEM_USE_PTR
   #undef  XMEM_USE_TRACE
   #undef  XMEM_USE_REALLOC
   #undef  XMEM_USE_VALLOC
   #undef  XMEM_USE_DUP

   #define XMEM_USE_PTR          0
   #define XMEM_USE_TRACE        0
   #define XMEM_USE_REALLOC      0
   #define XMEM_USE_VALLOC       0
   #define XMEM_USE_DUP          0

   /*
    * if the preprocessor macros are not yet defined elsewhere before we are included
    * for the first time, then use std C malloc stuff without any check
    */
   #ifndef _XMEM_trace
      #define _XMEM_trace(_onoff) /* empty */
   #endif

   #define FREE                  free
   #define MALLOC                malloc
   #define MEMDUP(_ptr,_size)    memcpy(malloc(_size),_ptr,_size)
   #define REALLOC               realloc
   #if IS_MSWIN
      #define STRDUP             _tcsdup
   #else
      #define STRDUP             strdup
   #endif

#endif

#endif
#endif
