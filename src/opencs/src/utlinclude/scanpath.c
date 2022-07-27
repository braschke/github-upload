#ifndef scanpath_SOURCE_INCLUDED
#define scanpath_SOURCE_INCLUDED
/* scanpath.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    Scan the PATH and find the executable 'exeName'.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.Fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: scanpath.c 1975 2013-11-10 14:34:01Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "strsjoinl.c"
#endif

C_FUNC_PREFIX
TCHAR *scanpath(const TCHAR *exe, const TCHAR *sfx, TCHAR *exepath, size_t exesize)
{
   TCHAR *head, *tail;


   STRJUMPNOSPACE(exe);

   /* find the last dirsep == filename part */
   tail = NULL;
   for(head=(TCHAR *)exe; *head; head++)
      if (ISDIRSEP(*head)) tail = head + 1;

   /* direct check in case of a fully qualified pathname */
   if (STRHASLEN(tail))
   {
      strsjoinl(exepath,exesize,NULL,exe,sfx,NULL);
      if (FEXISTS(exepath)) return exepath;
      exe = tail;
   }

   /* now scan the PATH */
#if IS_MSWIN

   return (SearchPath(NULL,exe,sfx,(DWORD)exesize,exepath,NULL))
      ? exepath : NULL;

#else /* simply UNIX char */

   for(head=tail=(char *)getenv("PATH"); tail && *head; head=tail+1)
   {
      tail = strchr(head,':');
      if (*head != ':') /* skip empty "::" pathes */
      {
         if (tail) *tail = '\0';
            strsjoinl(exepath,exesize,NULL,head,"/",exe,sfx,NULL);
         if (tail) *tail = ':';
         if (FEXISTS(exepath)) return exepath;
      }
   }
   exepath[0] = '\0';
   return NULL; /* not found */

#endif
}
#endif
