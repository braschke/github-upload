#ifndef __lsearchv_SOURCE_INCLUDED
#define __lsearchv_SOURCE_INCLUDED
/* lsearchv.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Do a linear search in a vector of POINTERS TO OBJECTS *objv[] for an existing object
 *    and return its index or -1 if the object cannot be found.
 *
 *    Do not confuse lsearchv() with a search in an array of objects of a certain size
 *    like it is done in qsort() or bsearch() or lsearch()!!!
 *
 *    compare() returns 0 if the objects are identical, !0 otherwise.
 *    so compare() may either be just strcmp() or stricmp() or strcasecmp() ...
 *
 * Example:
 *    if (lsearchv(argv,argc,"-help",strcmp)) print_usage_and_exit();
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2011, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/24: Carsten Dehning, Initial release
 *    $Id: lsearchv.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
int lsearchv(const void **objv, int size, const void *seek,
                  int (*compare)(const void *obje, const void *seek))
{
   if (objv && seek)
   {
      int i;
      if (size < 0) /* objv[] is NULL terminated like argv[] */
      {
         for(i=0; objv[i]; i++) if (!compare(objv[i],seek)) return i;
      }
      else /* size is known and objv[] may contain NULL pointers */
      {
         for(i=0; i<size; i++) if (objv[i] && !compare(objv[i],seek)) return i;
      }
   }
   return -1; /* not found */
}
#endif
