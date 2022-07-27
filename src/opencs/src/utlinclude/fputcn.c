#ifndef fputcn_SOURCE_INCLUDED
#define fputcn_SOURCE_INCLUDED
/* fputcn.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Repeated call to fputc(c,fp).
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Apr/10: Carsten Dehning, Initial release
 *    $Id: fputcn.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int fputcn(int c, int n, FILE *fp)
{
   int res = 0;

   while(n-- > 0)
      if ((res=fputc(c,fp)) == EOF)
         break;

   return res;
}
#endif
