#ifndef GMEM_SOURCE_INCLUDED
#define GMEM_SOURCE_INCLUDED
/* gmem.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Umnanaged permanent memory which is (re)allocated once and may be growing, however
 *    it should never be free'd, since it is used as a permanent available memory region.
 *
 *    The GMEM avoids repeated calls of malloc()/free() pairs for temporary memory.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Aug/28: Carsten Dehning, Initial release
 *    $Id: gmem.c 2403 2014-01-09 12:47:36Z dehning $
 *
 *****************************************************************************************
 */
#include "gmem.h"
#include <stdarg.h>

#include "xmem.h"
#include "xmsg.h"

#ifndef GMEM_USE_GETLIST
   #define GMEM_USE_GETLIST   0
#endif

#define GMEM_WORD_SIZE   sizeof(double) /* wordsize for buffer alignment */
#define GMEM_PAGE_SIZE   4096           /* pagesize under most Unix'es and MSWin */

/****************************************************************************************/
C_FUNC_PREFIX void *GMEM_ctrl(const int ctrl, const size_t size)
/****************************************************************************************/
/*
 * Core control function which does the buffer management:
 */
{
   static struct _GMEM
   {
      void   *buffer; /* pointer to the allocated buffer */
      size_t  nalloc; /* currently allocated size of the buffer */
      int     locked; /* boolean buffer lock flag */
   } _gmem = {NULL,0,0};

   const char *errmsg, *errcmd;


   switch(ctrl)
   {
      case GMEM_CTRL_MALLOC: /* allocate and lock */
         if (!_gmem.locked)
         {
            /* Round up the requested size to a multiple of GMEM_PAGE_SIZE */
            const size_t rsize = GMEM_PAGE_SIZE*( (size+GMEM_PAGE_SIZE-1)/GMEM_PAGE_SIZE );
            if (rsize > _gmem.nalloc) /* throw away the too small buffer and make a new one */
            {
               if (_gmem.buffer)
                  FREE(_gmem.buffer);
               _gmem.nalloc = rsize;
               if ((_gmem.buffer=MALLOC(rsize)) == NULL)
               {
                  errmsg = "Failed to allocate buffer";
                  errcmd = "MALLOC";
                  break;
               }
            }
            _gmem.locked = 1;
            return _gmem.buffer;
         }
         errmsg = "Buffer already in use and locked";
         errcmd = "MALLOC";
         break;

      case GMEM_CTRL_UNLOCK: /* just unlock */
         if (_gmem.locked)
         {
            _gmem.locked = 0;
            return NULL;
         }
         errmsg = "Buffer unlocked multiple times";
         errcmd = "UNLOCK";
         break;

      case GMEM_CTRL_FREE: /* free and unlock */
         if (_gmem.buffer)
            FREE(_gmem.buffer);
         _gmem.buffer = NULL;
         _gmem.nalloc = 0;
         _gmem.locked = 0;
         return NULL;

      case GMEM_CTRL_LOCKED: /* query lock */
         return (_gmem.locked) ? _gmem.buffer : NULL;

      default:
         errmsg = "Invalid cmd";
         errcmd = "BAD";
         break;
   }

   XMSG_FATAL5
   (
      "GMEM_ctrl(ctrl=%s(%d),size=%u): Internal error(lock=%d): %s.\n"
      ,errcmd
      ,ctrl
      ,CAST_UINT(size)
      ,_gmem.locked
      ,errmsg
   );

   return NULL; /* keep compiler happy */
}

/****************************************************************************************/

#if GMEM_USE_GETLIST

/****************************************************************************************/
C_FUNC_PREFIX void *GMEM_getlist(void **ptr1, ...)
/****************************************************************************************/
/*
 * Return pointers to a once (re)allocated memory in arbitrary portions specified via
 * the ... arglist.
 * args ... are always groups of 3 args
 *
 *    GMEM_getlist(void **ptr, size_t size, int nval,
 *                 void **ptr, size_t size, int nval,
 *                 void **ptr, size_t size, int nval,
 *                 ......
 *                 NULL);
 *
 * the NULL pointer terminates the argument list.
 *
 * Example:
 *    GMEM_getlist(  &pInt   , sizeof(int)    , 100,
 *                   &pDouble, sizeof(double) , 20,
 *                   &pChar  , sizeof(char)   , 128+1,
 *                   ... to be continued....
 *                   NULL);
 */
{
   char    *memp;
   va_list  ap;
   size_t   args;
   size_t   total;
   int      argc;


   /* determine the total size of the requested buffers */
   total = 0;
   va_start(ap,ptr1);
   {
      do
      {
         args = va_arg(ap,size_t);  if (!args ) args = 1;
         argc = va_arg(ap,int);     if (argc<1) argc = 1;

         /* get true size rounded up to multiples of GMEM_WORD_SIZE */
         total += GMEM_WORD_SIZE * ((args*CAST_SIZE(argc) +(GMEM_WORD_SIZE-1))/GMEM_WORD_SIZE);
      } while(va_arg(ap,void **) != NULL);
   }
   va_end(ap);


   /* Allocate the memory */
   memp = (char *)GMEM_getall(total);

   /* Split the buffer and assign all argp pointers */
   total = 0;
   va_start(ap,ptr1);
   {
      void **argp; /* argument: pointer */
      for (argp=ptr1; argp; argp=va_arg(ap,void **))
      {
        *argp = (void*)(memp + total);  /* assign the arg pointer */
         args = va_arg(ap,size_t);  if (!args ) args = 1;
         argc = va_arg(ap,int);     if (argc<1) argc = 1;

         /* get true size rounded up to multiples of GMEM_WORD_SIZE */
         total += GMEM_WORD_SIZE * ((args*CAST_SIZE(argc) +(GMEM_WORD_SIZE-1))/GMEM_WORD_SIZE);
      }
   }
   va_end(ap);

   return memp;
}

/****************************************************************************************/

#endif

#undef GMEM_WORD_SIZE
#undef GMEM_PAGE_SIZE

#endif
