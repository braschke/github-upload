#ifndef fscanf_intv_SOURCE_INCLUDED
#define fscanf_intv_SOURCE_INCLUDED
/* fscanf_intv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    formatted read variable length list of integers (e.g. nodes ids)
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: fscanf_intv.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int fscanf_intv(FILE *fp, int *ibuf, int n)
{
   int i;

   for (i=0; i<n; i++)
      if (fscanf(fp,"%d",ibuf+i) != 1)
         break;
   return i; /* return nread */
}
#endif
