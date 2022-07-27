#pragma once
#ifndef wallticks_HEADER_INCLUDED
#define wallticks_HEADER_INCLUDED
/* wallticks.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines a unitless and platform independent Wall-Clock-Ticks with a resolution
 *    far less than 1 millisecond, plus utilities.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2015/Dec/10: Carsten Dehning, Initial release
 *    $Id: wallticks.h 4346 2016-05-07 10:03:33Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Returns wta/wtb as double
 */
#define WALLTICKS_RATIO(wta,wtb)  ((wtb>0) ? CAST_DOUBLE(wta)/CAST_DOUBLE(wtb) : -1.0)

#if !INCLUDE_STATIC

   extern uint64_t WALLTICKS_freq (unsigned *pvari);
   extern uint64_t WALLTICKS_get  (void);
   extern void     WALLTICKS_print(const uint64_t wt, const char *stotal, const char *scount, const unsigned count, FILE *fp);

#endif

#ifdef __cplusplus
}
#endif
#endif
