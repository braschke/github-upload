#ifndef memindex_int_SOURCE_INCLUDED
#define memindex_int_SOURCE_INCLUDED
/* memindex_int.c
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
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memindex_int.c 3844 2016-01-19 15:46:00Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memindex_int(const int *buf, const size_t n, const int seek)
{
   size_t i;
   for(i=0; i<n; i++) if (buf[i] == seek) return CAST_INT(i);
   return -1;
}
#endif
