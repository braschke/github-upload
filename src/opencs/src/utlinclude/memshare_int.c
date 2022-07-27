#ifndef __memshare_int_SOURCE_INCLUDED
#define __memshare_int_SOURCE_INCLUDED
/* memshare_int.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    count how many integer two arrays have in common
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2012, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: memshare_int.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int memshare_int(const int *ai, const int ni, const int *aj, const int nj)
{
   int i,j;
   int count = 0;


   for(i=0; i<ni; i++)
   {
      const int iseek = ai[i];
      for(j=0; j<nj; j++)
      {
         if (aj[j] == iseek) count++;
      }
   }
   return count;
}
#endif
