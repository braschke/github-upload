#ifndef xmem_SOURCE_INCLUDED
#define xmem_SOURCE_INCLUDED
/* xmem.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Wrapper for malloc/free/strdup with malloc failure checks.
 *    the std c malloc may by redefined via pointers from external code.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/20: Carsten Dehning, Initial release
 *    $Id: xmem.c 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */
#include "xmem.h"

/* do not compile anything if we do not use it */
#if XMEM_USE

#include "xmsg.h"

#if INCLUDE_STATIC
   #include "getbasename.c"
#endif

#if XMEM_USE_PTR || XMEM_USE_TRACE

   static struct _xmem_info_struct
   {
      void * (*malloc)  (size_t size);
      void * (*realloc) (void *ptr, size_t size);
      void   (*free)    (void *ptr);
      int      doTrace;
   } _xmem_data = { NULL,NULL,NULL, 0 };

#endif


#if XMEM_USE_TRACE

   C_FUNC_PREFIX void _XMEM_trace(int onoff) { _xmem_data.doTrace=onoff;}

#else

   #ifndef _XMEM_trace
      #define _XMEM_trace(_onoff)
   #endif

#endif

/*
 * core functions
 */

#if XMEM_USE_PTR
/****************************************************************************************/
 C_FUNC_PREFIX void _XMEM_init(void *(*ptrMalloc)  (size_t size),
                               void *(*ptrRealloc) (void *ptr, size_t size),
                               void  (*ptrFree)    (void *ptr))
/****************************************************************************************/
{
   _xmem_data.malloc  = ptrMalloc;
   _xmem_data.realloc = ptrRealloc;
   _xmem_data.free    = ptrFree;
}

/****************************************************************************************/
#endif


/****************************************************************************************/
#if XMEM_USE_TRACE
 C_FUNC_PREFIX void _XMEM_free(void *ptr, const char *file, int line)
#else
 C_FUNC_PREFIX void _XMEM_free(void *ptr)
#endif
/****************************************************************************************/
{
#if XMEM_USE_TRACE
   if ( _xmem_data.doTrace)
      XMSG_INFO3
      (
         "## _XMEM_free(*%p): %s(%d)\n",
         ptr,getbasename(file),line
      );
#endif

   #if XMEM_USE_PTR
      ( _xmem_data.free != NULL) ? _xmem_data.free(ptr) : free(ptr);
   #else
      free(ptr);
   #endif
}

/****************************************************************************************/


/****************************************************************************************/
 C_FUNC_PREFIX void *_XMEM_malloc(size_t size, const char *file, int line)
/****************************************************************************************/
{
   void *ptr;

#if XMEM_USE_TRACE
   if ( _xmem_data.doTrace)
      XMSG_INFO3
      (
         "## _XMEM_malloc(%lu): %s(%d)\n",
         CAST_ULONG(size),getbasename(file),line
      );
#endif

   if (!size) size = 1;
   #if XMEM_USE_PTR
      ptr = ( _xmem_data.malloc != NULL) ? _xmem_data.malloc(size) : malloc(size);
   #else
      ptr = malloc(size);
   #endif

   if (!ptr)
      XMSG_FATAL3
      (
         "XMEM: malloc(%lu): Failed to allocate new memory.\n"
         "File \"%s\", line(%d)\n",
         CAST_ULONG(size),getbasename(file),line
      );

   return MEMZERO(ptr,size);
}

/****************************************************************************************/



#if XMEM_USE_REALLOC
/****************************************************************************************/
 C_FUNC_PREFIX void *_XMEM_realloc(void *buf, size_t size, const char *file, int line)
/****************************************************************************************/
{
   void *ptr;

#if XMEM_USE_TRACE
   if ( _xmem_data.doTrace)
      XMSG_INFO4
      (
         "## _XMEM_realloc(*%p,%lu): %s(%d)\n",
         buf,CAST_ULONG(size),getbasename(file),line
      );
#endif

   if (!size) size = 1;
   if (!buf) return _XMEM_malloc(size,file,line);

   #if XMEM_USE_PTR
      ptr = ( _xmem_data.realloc != NULL) ? _xmem_data.realloc(buf,size) :  realloc(buf,size);
   #else
      ptr = realloc(buf,size);
   #endif

   if (!ptr)
      XMSG_FATAL3
      (
         "XMEM: realloc(%lu): Failed to reallocate new memory.\n"
         "File \"%s\", line(%d)\n",
         CAST_ULONG(size),getbasename(file),line
      );

   return ptr;
}

/****************************************************************************************/
#endif


#if XMEM_USE_VALLOC
/****************************************************************************************/
 C_FUNC_PREFIX void **_XMEM_valloc(size_t n, size_t size, const char *file, int line)
/****************************************************************************************/
/*
 * allocate char array[n][size] as:   void *array[n]  -> char buf[size];
 */
{
   void **ptr;
   char *buf;
   size_t i;

#if XMEM_USE_TRACE
   if ( _xmem_data.doTrace)
      XMSG_INFO4
      (
         "## _XMEM_valloc(%lu,%lu): %s(%d)\n",
         CAST_ULONG(n),CAST_ULONG(size),getbasename(file),line
      );
#endif

   /* allocate at least 1 row with a buffer of 8 bytes */
   if (!n) n = 1;
   size = (size) ? 8*((size+7)/8) : 8;

   /* allocate just a single block that can later be freed in once */
   buf = (char *)_XMEM_malloc(n*sizeof(void *) + n*size, file,line);
   ptr = (void **)buf; /* void *ptr[n] */
   buf += n*sizeof(void *);
   for(i=0; i<n; i++, buf+=size)
      ptr[i] = (void *)buf;

   return ptr;
}

/****************************************************************************************/
#endif


#if XMEM_USE_DUP
/****************************************************************************************/
 C_FUNC_PREFIX void *_XMEM_memdup(const void *buf, size_t size, const char *file, int line)
/****************************************************************************************/
{
   void *ptr;

#if XMEM_USE_TRACE
   if ( _xmem_data.doTrace)
      XMSG_INFO4
      (
         "## _XMEM_memdup(*%p,%lu): %s(%d)\n",
         buf,CAST_ULONG(size),getbasename(file),line
      );
#endif

   if (!size) size = 1;
   ptr = _XMEM_malloc(size,file,line);
   return (buf) ? memcpy(ptr,buf,size) : MEMZERO(ptr,size);
}

/****************************************************************************************/
#endif

#endif  /* XMEM_USE */

#endif
