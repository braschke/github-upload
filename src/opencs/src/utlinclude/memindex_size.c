#ifndef __memindex_size_SOURCE_INCLUDED
#define memindex_size_SOURCE_INCLUDED
/* memindex_size.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Find seek in buf[] and return index of first element or -1
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/24: Carsten Dehning, Initial release
 *    $Id: memindex_size.c 2700 2014-02-24 10:07:07Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memindex_size(const void *buf, const size_t n, const size_t size, const void *seek)
{
   size_t      i;
   const char *c = (const char *)buf;

   for(i=0; i<n; i++, c+=size)
      if (!memcmp(c,seek,size))
         return CAST_INT(i);
   return -1;
}
#endif
