#pragma once
#ifndef getmstime_SOURCE_INCLUDED
#define getmstime_SOURCE_INCLUDED
/* getmstime.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Implements an UTC time(NULL), but in milliseconds.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Dec/14: Carsten Dehning, Initial release
 *    $Id: getmstime.c 5518 2017-08-22 17:55:02Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   C_FUNC_PREFIX uint64_t getmstime(void)
   {
      UINT64 ft;
      GetSystemTimeAsFileTime((FILETIME *)&ft);

      /* make time Unix epoch 1970/1/1 based and scale from 100 nanoseconds to milliseconds */
      return CAST_UINT64((ft-FT_EPOCH_BIAS)/FT_SCALE_MSEC);
   }

#else /* UNIX, what else */

   #include <sys/time.h>
   C_FUNC_PREFIX uint64_t getmstime(void)
   {
      struct timeval tv;
      gettimeofday(&tv,NULL);
      return CAST_UINT64(tv.tv_sec)*1000 + CAST_UINT64(tv.tv_usec)/1000;
   }

#endif
#endif
