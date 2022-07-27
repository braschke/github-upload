#pragma once
#ifndef strclist_SOURCE_INCLUDED
#define strclist_SOURCE_INCLUDED
/* strclist.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *    Find TCHARs from *clist in string *str and return pointer
 *    to match in str or NULL.
 *    In fact an implementation of strpbrk()
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *strclist(const TCHAR *str, const TCHAR *clist)
{
   if (str && STRHASLEN(clist))
   {
      unsigned char isclist[256];

      /*
       * Set up flag for each char from the clist
       */
      MEMZERO(isclist,sizeof(isclist));
      for(; *clist; clist++)
      {
         isclist[CAST_UINT(*clist&0xff)] = 1;
      }

      /*
       * Does any char from str appear in the char list?
       */
      for (; *str; str++)
      {
         if (isclist[CAST_UINT(*str&0xff)])
            return CCAST_INTO(TCHAR *,str);
      }
   }
   return NULL; /* no match */
}
#endif
