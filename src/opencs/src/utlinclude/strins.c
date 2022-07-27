#ifndef __strins_SOURCE_INCLUDED
#define __strins_SOURCE_INCLUDED
/* strins.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 * fuege vor den string s den string si ein
 *
 */
#include "stdheader.h"

C_FUNC_PREFIX
char *strins(char *s, const char *si)
{
   size_t ni = strlen(si);
   char  *c;


   if (!ni) /* nicht einzufuegen */
      return s;

   /* string + '\0' nach rechts schieben */
   for (c = s+strlen(s); c >= s; c--)
      *(c+ni) = *c;

   /* si vorne einbauen */
   return memcpy(s,si,ni);
}
#endif
