#pragma once
#ifndef splitenv_SOURCE_INCLUDED
#define splitenv_SOURCE_INCLUDED
/* splitenv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Get an environment variable which contains mutiple values separated by : or ;
 *    and create and return a NULL terminated vector with the string values.
 *    Used e.g. to split the $PATH or %PATHEXT% under MSWin
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Jan/06: Carsten Dehning, Initial release
 *    $Id: splitenv.c 4314 2016-05-04 13:28:33Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include "xenv.h"
#include "xmem.h"

#if INCLUDE_STATIC
   #include "strcountchar.c"
#endif

C_FUNC_PREFIX
const TCHAR **splitenv(const TCHAR *envvar, int cleanup)
{
   const TCHAR *envval;
   TCHAR *head, *end, **listv;
   char  *cp;
   int    nval;


   envval = GETENV(envvar);
   if (!STRHASLEN(envval))
      return NULL;

   /* count the number of separators in the list + some more */
   nval = strcountchar(envval,_C_LISTSEP) + 3; /* at least 1 + terminating NULL */

   /* allocate all buffers at once */
   cp    = (char   *)MALLOC(nval*sizeof(TCHAR *) + (STRLEN(envval)+1)*sizeof(TCHAR));
   listv = (TCHAR **) cp;
   head  = (TCHAR  *)(cp + nval*sizeof(TCHAR *));
   STRCPY(head,envval);

   nval = 0;
   for (end=head; end && *head; head=end+1)
   {
      STRJUMPNOSPACE(head);
      STRJUMPCHARCP(head,end,_C_LISTSEP);
      if (*end) *end = 0;
      else end = NULL; /* terminate the for loop */
      if (*head)
         listv[nval++] = head;
   }
   listv[nval] = NULL;

   if (cleanup)
   {
      /* remove doubled entries, e.g. in the PATH */
      int (*compare)(const TCHAR *p, const TCHAR *q) = (cleanup>0) ? STRCMP : STRICMP;

      int i,j;
      for(i=0; listv[i]; i++)
         for(j=i+1; listv[j]; j++)
            if (!compare(listv[i],listv[j]))
               memcpy(listv+j,listv+j+1,(nval-j)*sizeof(TCHAR *));
   }

   //_cprintf("\nsplitenv(%s)\n",envvar);{int i;for(i=0;listv[i];i++) _tcprintf(TEXT("listv[%d]: \"%s\"\n"),i,listv[i]);}
   return listv;
}
//int main(void){splitenv(TEXT("PATH"));return 0;}
#endif
