#pragma once
#ifndef memrot_SOURCE_INCLUDED
#define memrot_SOURCE_INCLUDED
/* memrot.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Cyclic rotate an array char buf[szobj*nobj] containing 'nobj' objects of size 'szobj' by
 *    (nrot * szobj) bytes.
 *
 *    Rotation can be left (nrot < 0) or right (nrot > 0)
 *
 *    We have optimal read/store/move performance if
 *
 *       |nrot*szobj| <= MEMROT_NMAX
 *
 *    the amount of copy/store is always  (n + |nrot|) * szobj.
 *
 *    For best performance on large rotations find out whether we rotate left or right,
 *    Just take the one with the smallest value.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Oct/23: Carsten Dehning, Initial release
 *    $Id: memrot.c 5442 2017-08-02 13:16:10Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


#define MEMROT_NMAX   (8*1024) /* size of temp. copy buffer == 8 Kb */

C_FUNC_PREFIX
void *memrot(void *buf, const size_t nobj, const size_t szobj, const int nrot)
{
   char        *cbuf;
   const size_t szbuf = nobj*szobj; /* get the size in bytes */
   size_t       nl,nr;


   if (!szbuf)
      return buf;

   /*
    * Get no. of the left and right rotate operations and use
    * the operation with the smallest value.
    */
   if (nrot < 0)
   {
      nl = szobj*CAST_SIZE((-nrot) % CAST_INT(nobj));
      nr = szbuf - nl;
   }
   else if (nrot > 0)
   {
      nr = szobj*CAST_SIZE( nrot % CAST_INT(nobj));
      nl = szbuf - nr;
   }
   else /* nrot == 0 */
   {
      return buf;
   }

   cbuf = (char *)buf;
   if (nr < nl) /* Right rotate best */
   {
      while(nr)
      {
         /* Save tail, shift buffer right, store tail at head of buffer */
         const size_t n = (nr > MEMROT_NMAX) ? MEMROT_NMAX : nr;
         char         ctmp[MEMROT_NMAX];
         memcpy (ctmp  ,cbuf+szbuf-n,      n);
         memmove(cbuf+n,cbuf        ,szbuf-n);
         memcpy (cbuf  ,ctmp        ,      n);
         nr -= n;
      }
   }
   else /* Left rotate best */
   {
      while(nl)
      {
         /* Save head, shift buffer left, store head at end of buffer */
         const size_t n = (nl > MEMROT_NMAX) ? MEMROT_NMAX : nl;
         char         ctmp[MEMROT_NMAX];
         memcpy (ctmp        ,cbuf  ,      n);
         memmove(cbuf        ,cbuf+n,szbuf-n);
         memcpy (cbuf+szbuf-n,ctmp  ,      n);
         nl -= n;
      }
   }

   return buf;
}

#undef MEMROT_NMAX

#endif
