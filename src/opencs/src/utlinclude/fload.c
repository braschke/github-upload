#pragma once
#ifndef fload_SOURCE_INCLUDED
#define fload_SOURCE_INCLUDED
/* fload.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Atomic (try to ...) load the first 'n' bytes of a file into a char buffer buf[n].
 *    return:   -1: file open error
 *             >=0: no. of bytes loaded
 *
 *    If fload() is used in a multi-threaded MSWin code it is important not to call
 *    any of the VC CRT function. Using the native kernel32 functions CreateFile()
 *    and ReadFile() is multi-threaded save, so we have a different code for MSWin and
 *    UNIX although we could use open(), read() and close() from VC CRT.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Jun/20: Carsten Dehning, Initial release
 *    $Id: fload.c 4143 2016-04-16 15:04:16Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if !IS_MSWIN
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
#endif

C_FUNC_PREFIX
int fload(const TCHAR *path, void *buf, size_t n)
{
   char *head = CAST_CHARP(buf);
   char *tail = head + n;

#if IS_MSWIN
   DWORD  nr;
   HANDLE fh = CreateFile
               (
                  path,
                  GENERIC_READ,
                  FILE_SHARE_READ,
                  NULL,
                  OPEN_EXISTING,
                  FILE_FLAG_SEQUENTIAL_SCAN,
                  NULL
               );
   if (IS_INVALID_HANDLE(fh)) return -1;
   for(; head<tail; head+=nr)
   {
      if (!ReadFile(fh,head,(DWORD)(tail-head),&nr,NULL) || !nr)
         break;
   }
   CloseHandle(fh);
#else
   int nr, fd = open(path,O_RDONLY);
   if (fd < 0) return -1;
   for(; head<tail; head+=nr)
   {
      if ((nr=CAST_INT(read(fd,head,CAST_INT(tail-head)))) <= 0)
         break;
   }
   close(fd);
#endif

   return CAST_INT(head-CAST_CHARP(buf));
}
#endif
