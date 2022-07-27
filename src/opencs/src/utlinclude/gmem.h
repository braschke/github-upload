#ifndef GMEM_HEADER_INCLUDED
#define GMEM_HEADER_INCLUDED
/* gmem.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Definitions and prototype for the global memory allocator
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Aug/28: Carsten Dehning, Initial release
 *    $Id: gmem.h 2403 2014-01-09 12:47:36Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifdef __cplusplus
   extern "C" {
#endif

enum
{
   GMEM_CTRL_MALLOC = 0,
   GMEM_CTRL_LOCKED = 1,
   GMEM_CTRL_UNLOCK = 2,
   GMEM_CTRL_FREE   = 3
};

#if !INCLUDE_STATIC
   extern void *GMEM_ctrl     (const int ctrl, const size_t size);
   extern void *GMEM_getlist  (void **ptr1, ...);
#endif

#define GMEM_getall(_size)    GMEM_ctrl(GMEM_CTRL_MALLOC,_size)
#define GMEM_locked()        (GMEM_ctrl(GMEM_CTRL_LOCKED,0)!=NULL)
#define GMEM_unlock()         GMEM_ctrl(GMEM_CTRL_UNLOCK,0)
#define GMEM_free()           GMEM_ctrl(GMEM_CTRL_FREE,0)

#ifdef __cplusplus
   }
#endif

#endif
