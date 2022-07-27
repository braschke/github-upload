#ifndef fseriesformat_SOURCE_INCLUDED
#define fseriesformat_SOURCE_INCLUDED
/* fseriesformat.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Find the longest series of digits in the filename portion of a pathname, replace
 *    the digits by an appropriate format "%0nd" and return the format string.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2009/Jun/29: Carsten Dehning, Initial release
 *    $Id: fseriesformat.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "getbasename.c"
#endif

C_FUNC_PREFIX
int fseriesformat(const TCHAR *path, TCHAR *fmt, size_t size, long *num)
{
   TCHAR *fname;
   size_t len = STRLENP(path);
   long   curr_num, best_num;
   int    curr_pos, best_pos;
   int    curr_len, best_len;
   int    i;
   TCHAR  digfmt[64];


   fmt[0] = '\0';
   if (len < 4 || len >= size)
      return -1; /* format string buffer too small */

   fname    = getbasename(path);
   curr_num = best_num = -1;
   curr_pos = best_pos = -1;
   curr_len = best_len =  0;

   /* find the longest series of digits in the file name */
   for(i=0; fname[i]; i++)
   {
      if (ISDIGIT(fname[i]))
      {
         if (curr_len > 0)
         {
            curr_len++;
            curr_num = 10*curr_num + (long)(fname[i]-'0');
         }
         else
         {
            curr_pos = i;
            curr_len = 1;
            curr_num = (long)(fname[i]-'0');
         }
      }
      else if (curr_len > 0) /* terminating char */
      {
         /*
          * heuristic in case of ambiguous digis:
          *    take the longest - in digits - number
          *    take the last number in the filename
          *    take the lowest number
          */
         if (curr_len > best_len) /* more digits are better */
         {
            best_pos = curr_pos;
            best_len = curr_len;
            best_num = curr_num;
         }
         else if (curr_len == best_len && curr_num <= best_num)
         {
            /* means: take the last digit series if the number is less than the previous num */
            best_pos = curr_pos;
            best_len = curr_len;
            best_num = curr_num;
         }
         curr_pos = -1;
         curr_num = -1;
         curr_len =  0;
      }
   }

   if (curr_len > 0 && curr_len > best_len)
   {
      best_pos = curr_pos;
      best_len = curr_len;
      best_num = curr_num;
   }

   if (best_len < 4)
      return -1; /* no relevant digits, no format string returned */

   best_pos += CAST_INT(fname-path); /* get the best position in the full pathname */
   sprintf(digfmt,TEXT("%%0%dd"),best_len);
   len = strlen(digfmt);
   strcpy(fmt,path);
   memcpy(fmt+best_pos,digfmt,len); /* replace at least 4 digits be the "%0nd" */

   if (best_len > 4)
      strcpy(fmt+best_pos+len,fmt+best_pos+best_len);

   if (num)
      *num = best_num;

   return best_len;
}

#if 0
int main(int argc, char *argv[])
{
   TCHAR fmt[256];
   long  n;
   if (fseriesformat(argv[1],fmt,countof(fmt),&n) > 0)
      printf("fmt=<%s>, start=<%ld>\n",fmt,n);
   else
      puts("no series.");
   return 0;
}
#endif

#endif
