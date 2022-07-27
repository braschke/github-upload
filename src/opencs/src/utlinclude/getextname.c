#ifndef getextname_SOURCE_INCLUDED
#define getextname_SOURCE_INCLUDED
/* getextname.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    find the '.' of the extension and return a pointer to it or NULL
 *    DO NOT just strrchr(pathName,'.') because we may have ".\path\to\subdir"
 *    and we may have a ".ext.gz" file.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getextname.c 2711 2014-02-24 10:46:09Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
TCHAR *getextname(const TCHAR *pathname)
{
   /* search backwards and always ignore the first char */
   if (STRHASLEN(pathname))
   {
      const TCHAR *end;

      for(end=STRLASTCP(pathname); end>pathname && !ISDIRSEP(*end); end--)
      {
         if (*end != TEXT('.') || !STRICMP(end,TEXT(".gz"))) continue;
         if (ISDIRSEP(*(end-1))) break;
         return (TCHAR *)end;
      }
   }
   return NULL;
}
#endif
