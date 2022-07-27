#ifndef fimage_SOURCE_INCLUDED
#define fimage_SOURCE_INCLUDED
/* fimage.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Map a file into the memory:
 *
 *       - get the file size,
 *       - malloc buffer,
 *       - load the file contents
 *       - always append the two bytes '\n' + '\0' at the tail of the loaded file image!
 *
 *    and return a pointer to allocated buffer.
 *
 *    If fimage() is used in a multi-threaded MSWin code it is important not to call
 *    any of the VC CRT function. Using the native kernel32 functions CreateFile()
 *    and ReadFile() is multi-threaded save, so we have a different code for MSWin and
 *    UNIX although we could use open(), read() and close() from VC CRT.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Dec/11: Carsten Dehning, Initial release
 *    $Id: fimage.c 3108 2014-08-18 10:00:08Z dehning $
 *
 *****************************************************************************************
 */
#if !defined(MALLOC) || !defined(FREE)
#include "xmem.h"
#endif

#define FIMAGE_ERROR_FSIZE  0
#define FIMAGE_ERROR_FSTAT  1
#define FIMAGE_ERROR_FTYPE  2
#define FIMAGE_ERROR_ALLOC  3
#define FIMAGE_ERROR_FOPEN  4
#define FIMAGE_ERROR_FREAD  5


#if INCLUDE_STATIC
   #include "fload.c"
#endif

#if !IS_MSWIN
   #include <sys/types.h>
   #include <sys/stat.h>
#endif

#define FIMAGE_FAIL(_error) { *fsize=_error; return NULL; }

C_FUNC_PREFIX
void *fimage(const TCHAR *path, size_t *fsize)
{
   char  *image = NULL;
   size_t isize;
   int    nread;

#if IS_MSWIN

   /*
    * do not use any of the CRT functions like [f]open, [f]read, malloc, free etc.
    * to be thread save for threads started with CreateThread() and not _beginthread().
    */
   WIN32_FILE_ATTRIBUTE_DATA fad;

   if (!GetFileAttributesEx(path,GetFileExInfoStandard,&fad))
      FIMAGE_FAIL(FIMAGE_ERROR_FSTAT)

   if (fad.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_DEVICE|FILE_ATTRIBUTE_OFFLINE|FILE_ATTRIBUTE_REPARSE_POINT))
      FIMAGE_FAIL(FIMAGE_ERROR_FTYPE)

   *fsize = isize = CAST_SIZE( (__int64)fad.nFileSizeHigh * 0x100000000i64 + (__int64)fad.nFileSizeLow );

#else /* UNIX, what else */

   struct stat st;

   if (stat(path,&st))
      FIMAGE_FAIL(FIMAGE_ERROR_FSTAT)

   if ((st.st_mode & S_IFMT) != S_IFREG)
      FIMAGE_FAIL(FIMAGE_ERROR_FTYPE)

   *fsize = isize = st.st_size;

#endif

   if (!isize)  /* empty file */
      FIMAGE_FAIL(FIMAGE_ERROR_FSIZE)

   image = MALLOC(isize+2); /* add space for the trailing '\n'+'\0' */
   if (!image)
      FIMAGE_FAIL(FIMAGE_ERROR_ALLOC)

   nread = fload(path,image,isize);
   if (nread == CAST_INT(isize))
   {
      /* Success: append an empty line at the tail */
      image[isize  ] = '\n';
      image[isize+1] = '\0';
      return (void *)image;
   }

   /* Failed loading the image */
   FREE(image);
   if (nread < 0)
      FIMAGE_FAIL(FIMAGE_ERROR_FOPEN)

   FIMAGE_FAIL(FIMAGE_ERROR_FREAD)
}

#undef FIMAGE_FAIL

#endif
