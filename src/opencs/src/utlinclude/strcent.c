#ifndef __STRCENT_SOURCE_INCLUDED
#define __STRCENT_SOURCE_INCLUDED
/* strcent.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  string s zentriert in neu allokiertes *cp einbauen und cp uebergeben
 *
 */
#include "stdheader.h"
#include "xmem.h"

C_FUNC_PREFIX
char *strcent(const char *s, size_t n)
{
   static   char *cp = NULL;
   static   size_t  len = 0;
   register size_t  ns;


   if ((ns = strlen(s)) > n)
      ns = n; /* eingebauten string an ende clippen */

   if (n+1 > len)
   {
     if (cp) FREE(cp);
     if ((cp = (char *)MALLOC(len = n+1)) == NULL)
        return (char *)" (strcent) Can't allocate storage. ";
   }

   /* cp mit blanks fuellen */
   if (n > ns)
      memset(cp,' ',n);
   cp[n] = '\0';

   /* s zentriert kopieren */
   memcpy(cp+(n-ns)/2, s, ns);
   return cp;
}
#endif
