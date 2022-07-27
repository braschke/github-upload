#ifndef getdatestring_SOURCE_INCLUDED
#define getdatestring_SOURCE_INCLUDED
/* getdatestring.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns a date string of the current date.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: getdatestring.c 2655 2014-02-13 05:19:31Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <time.h>

C_FUNC_PREFIX
const char *getdatestring(void)
{
   time_t tm = time(NULL);
   return ctime(&tm);
}
#endif
