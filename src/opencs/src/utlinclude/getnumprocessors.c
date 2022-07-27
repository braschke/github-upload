#ifndef __getnumprocessors_SOURCE_INCLUDED
#define __getnumprocessors_SOURCE_INCLUDED
/* getnumprocessors.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the number of prozessors/cores on this system.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2009, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2009/Jun/28: Carsten Dehning, Initial release
 *    $Id: getnumprocessors.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX int getnumprocessors(void)
{
#if IS_MSWIN

   SYSTEM_INFO si;
   GetSystemInfo(&si);
   return CAST_INT(si.dwNumberOfProcessors);

#elif defined(IS_LINUX)||defined(IS_SUNOS)||defined(IS_AIX)||defined(IS_OSFALPHA)||defined(IS_MACOSX)

   #if 0
      printf("Processors configured : %ld\n",sysconf(_SC_NPROCESSORS_CONF));
      printf("Processors online     : %ld\n",sysconf(_SC_NPROCESSORS_ONLN));
   #endif
   return CAST_INT(sysconf(_SC_NPROCESSORS_CONF));

#elif defined(IS_HPUX11)

   return CAST_INT(sysconf(_SC_NUM_CPUS));

#elif defined(IS_IRIX65)

   #if 0
      printf("Processors configured : %ld\n",sysconf(_SC_NPROC_CONF));
      printf("Processors online     : %ld\n",sysconf(_SC_NPROC_ONLN));
   #endif
   return CAST_INT(sysconf(_SC_NPROC_CONF));

#else

   return 1;

#endif
}

#if 1
int main(void)
{
   printf("No. of CPU's: %d\n",getnumprocessors());
   return 0;
}
#endif

#endif
