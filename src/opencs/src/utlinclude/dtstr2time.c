#ifndef dtstr2time_SOURCE_INCLUDED
#define dtstr2time_SOURCE_INCLUDED
/* dtstr2time.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Converts a __DATE__ & __TIME__ type chars string into a time_t
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2010/Sep/29: Carsten Dehning, Initial release
 *    $Id: dtstr2time.c 2711 2014-02-24 10:46:09Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <time.h>


C_FUNC_PREFIX
time_t dtstr2time
(
   const char *c_date, /* format must be like __DATE__ */
   const char *c_time  /* format must by like __TIME__ or NULL */
)
{
   const char *mtab[] =
   {
      "Jan","Feb","Mar","Apr","May","Jun",
      "Jul","Aug","Sep","Oct","Nov","Dec",
      NULL
   };

   struct tm tm;
   int   month;
   char  mstr[32];


   MEMZERO(&tm,sizeof(tm));


   if (!STRHASLEN(c_date) || sscanf(c_date,"%s %d %d",mstr,&(tm.tm_mday),&(tm.tm_year)) != 3)
      return SCAST_INTO(time_t,-1);

   tm.tm_year -= 1900;

   if (STRHASLEN(c_time))
   {
      if (sscanf(c_time,"%d:%d:%d",&(tm.tm_hour),&(tm.tm_min),&(tm.tm_sec)) != 3)
         return SCAST_INTO(time_t,-1);
   }

   /* find int month 0..11 */
   for (month=0; mtab[month]; month++)
   {
      if (!STRICMP(mstr,mtab[month]))
      {
         tm.tm_mon = month;
         #if 0
            printf
            (
               "dtstr2time(%s): %02d.%02d.%04d %02d:%02d:%02d\n",
               c_date,
               tm.tm_mday,
               tm.tm_mon,
               tm.tm_year + 1900,
               tm.tm_hour,
               tm.tm_min,
               tm.tm_sec
            );
         #endif

         return mktime(&tm); /* correct the broken down time structure tm */
      }
   }

   return SCAST_INTO(time_t,-1);
}

#endif
