#ifndef compile_stime_SOURCE_INCLUDED
#define compile_stime_SOURCE_INCLUDED
/* compile_stime.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns information about the compiler date, time etc:
 *    We do not want to have __DATE__ and __TIME__ everywhere in the code, but instead
 *    keep this information localized to avoid a modification of this strings
 *    in case of licensing checks.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2010/Sep/27: Carsten Dehning, Initial release
 *    $Id: compile_stime.c 2745 2014-03-27 16:22:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX const char *compile_stime(void)
{
   return __TIME__;
}

#endif
