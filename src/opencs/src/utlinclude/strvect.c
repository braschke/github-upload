#ifndef strvect_SOURCE_INCLUDED
#define strvect_SOURCE_INCLUDED
/* strvect.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Implements a dynamically growing stringv[] and all its functions and ccp macros.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Jan/10: Carsten Dehning, Initial release
 *    $Id: strvect.c 5530 2017-08-25 11:51:53Z dehning $
 *
 *****************************************************************************************
 */
#if !defined(MALLOC) || !defined(FREE) || !defined(REALLOC)
#include "xmem.h"
#endif

#if INCLUDE_STATIC
   #include "fimage.c"
#endif

#ifndef SV_USE_SEND
   #define SV_USE_SEND 0
#endif
#ifndef SV_USE_FIND
   #define SV_USE_FIND 0
#endif
#ifndef SV_USE_PRINT
   #define SV_USE_PRINT 0
#endif

typedef struct _string_vector_struct
{
   TCHAR  **list;    /* dynamic list[0...alloc-1] */
   TCHAR   *fname;   /* pathname of the history file */
   size_t   count;   /* no. of entries in the list */
   size_t   alloc;   /* allocated size of the *list[alloc] */
   size_t   grow;    /* realloc() grow size of the vector */
   size_t   limit;   /* max. size of the vector */
   size_t   vpos;    /* current position */
   int      iscirc;  /* if true sv behaves like a circular buffer when changing vpos */
   int      dupok;   /* dups allowed if true, else do not push new string if identical with last */
} SV_t;


#define SV_getcount(_sv)            (  (_sv) ? CAST_INT((_sv)->count) : -1 )
#define SV_getalloc(_sv)            (  (_sv) ? CAST_INT((_sv)->alloc) : -1 )
#define SV_getgrow(_sv)             (  (_sv) ? CAST_INT((_sv)->grow)  : -1 )
#define SV_getlimit(_sv)            (  (_sv) ? CAST_INT((_sv)->limit) : -1 )
#define SV_getpos(_sv)              (  (_sv) ? CAST_INT((_sv)->vpos)  : -1 )
#define SV_getlasti(_sv)            ( ((_sv) && (_sv)->count) ? CAST_INT((_sv)->count)-1 : -1 )

#define SV_isokay(_sv,_i)           (  (_i)>=0 && (_i)<SV_getcount(_sv) )
#define SV_getsat(_sv,_i)           ( SV_isokay(_sv,_i) ? (_sv)->list[_i]          : NULL )
#define SV_getspos(_sv)             (    (_sv)          ? (_sv)->list[(_sv)->vpos] : NULL )

#define SV_setcirc(_sv,_circ)       ( (sv) ? sv->iscirc=(_circ) : -1 )
#define SV_setdup(_sv,_dup)         ( (sv) ? sv->dupok=(_dup)   : -1 )

#define SV_posabs(_sv,_pos)         SV_setpos(_sv,    _pos        ,0)
#define SV_posrel(_sv,_off)         SV_setpos(_sv,    _off        ,1)
#define SV_posfirst(_sv)            SV_setpos(_sv,     0          ,0)
#define SV_poslast(_sv)             SV_setpos(_sv,SV_getlasti(_sv),0)
#define SV_postail(_sv)             SV_setpos(_sv,SV_getcount(_sv),0)
#define SV_posincr(_sv)             SV_setpos(_sv,       +1       ,1)
#define SV_posdecr(_sv)             SV_setpos(_sv,       -1       ,1)


#define _SV_SIZE_LIMIT(_size)    ( (_size<128) ? 128 : (_size>32768) ? 32768 : _size )

#if SV_USE_FIND

/****************************************************************************************/
C_FUNC_PREFIX int SV_find(const SV_t *sv, const TCHAR *key)
/****************************************************************************************/
{
   if (sv && sv->count)
   {
      const int count = (int)sv->count;
      int       i;


      for(i=0; i<count; i++)
      {
         if (sv->list[i] && !STRCMP(key,sv->list[i]))
            return i;
      }
   }
   return -1; /* not found */
}

/****************************************************************************************/

#endif

/****************************************************************************************/
C_FUNC_PREFIX size_t SV_setlimit(SV_t *sv, size_t limit_size)
/****************************************************************************************/
/*
 * set max. length limit of the vector
 */
{
   if (!sv)
      return 0;

   sv->limit = _SV_SIZE_LIMIT(limit_size);

   /* limit can't be less than allocated size! */
   if (sv->limit < sv->alloc)
       sv->limit = sv->alloc;
   return sv->limit;
}

/****************************************************************************************/
C_FUNC_PREFIX SV_t *SV_new(size_t init_size, size_t grow_size, size_t limit_size)
/****************************************************************************************/
/*
 * constructor resp. operator new()
 */
{
   SV_t *sv = MALLOC(sizeof(SV_t));

   if (sv)
   {
      size_t size;

      sv->fname   = NULL;
      sv->iscirc  = 0;
      sv->dupok   = 0;
      sv->vpos    = 0;
      sv->count   = 0;
      sv->alloc   = _SV_SIZE_LIMIT(init_size);
      sv->grow    = _SV_SIZE_LIMIT(grow_size);

      size        = sv->alloc*sizeof(TCHAR *);
      sv->list    = (TCHAR **)MALLOC(size);
      if (sv->list)
      {
         MEMZERO(sv->list,size);
         SV_setlimit(sv,limit_size);
      }
      else
      {
         FREE(sv);
         sv = NULL; /* signal malloc failure */
      }
   }
   return sv;
}

/****************************************************************************************/
C_FUNC_PREFIX int SV_setpos(SV_t *sv, int pos, int relative)
/****************************************************************************************/
/*
 * set the current position sv->vpos, either absolute or relative
 */
{
   if (!sv)
      return -1;


   if (relative)
      pos += CAST_INT(sv->vpos);

   if (sv->iscirc) /* circulate pos */
   {
      pos %= CAST_INT(sv->count);
      if (pos < 0)
         pos += CAST_INT(sv->count);
      sv->vpos = CAST_SIZE(pos);
   }
   else
   {
      if (pos < 0)
      {
         sv->vpos = 0;
         pos = -1; /* return out of range signal */
      }
      else if (pos > CAST_INT(sv->count))
      {
         sv->vpos = sv->count;
         pos = -1; /* return out of range signal */
      }
      else
      {
         sv->vpos = CAST_SIZE(pos);
      }
   }

   return pos;
}

/****************************************************************************************/
C_FUNC_PREFIX TCHAR *SV_unshift(SV_t *sv, int dokeep)
/****************************************************************************************/
/*
 * remove string from the top of list
 */
{
   TCHAR *cp = NULL;

   if (sv)
   {
      if (sv->count)
      {
         if (dokeep)
            cp = sv->list[0];
         else if (sv->list[0])
            FREE(sv->list[0]);

         if (--sv->count)
            memcpy(&(sv->list[0]),&(sv->list[1]),sv->count*sizeof(TCHAR *));
      }
      sv->list[sv->count] = NULL;
   }
   return cp;
}

/****************************************************************************************/
C_FUNC_PREFIX size_t SV_push(SV_t *sv, const TCHAR *str, int nodup)
/****************************************************************************************/
/*
 * append new string at the tail of list
 */
{
   if (!sv || !STRHASLEN(str))
      return 0;

   if (!sv->dupok && sv->count)
   {
      /* avoid duplicates */
      TCHAR *cp = sv->list[sv->count-1];
      if (cp && !STRCMP(cp,str))
         return sv->count;
   }

   if (sv->count+1 > sv->limit)
   {
      /* limit reached: first unshift */
      SV_unshift(sv,0);
   }
   else if (sv->count+1 >= sv->alloc) /* >=  we need a NULL in the last sv->list */
   {
      TCHAR **cpp;
      size_t  nalloc = sv->alloc + sv->grow;

      cpp = (TCHAR **)REALLOC(sv->list,nalloc*sizeof(TCHAR *));
      if (!cpp) /* relalloc failed, use old vector */
         return sv->count;

      sv->list  = cpp;
      sv->alloc = nalloc;
   }

   sv->list[sv->count++] = (nodup) ? (TCHAR *)str : STRDUP(str);
   sv->list[sv->count]   = NULL;
   return sv->count;
}

/****************************************************************************************/
C_FUNC_PREFIX TCHAR *SV_pop(SV_t *sv, int dokeep)
/****************************************************************************************/
/*
 * remove string from the tail of list
 */
{
   TCHAR *cp = NULL;

   if (sv && sv->count)
   {
      sv->count--;
      if (dokeep)
         cp = sv->list[sv->count];
      else if (sv->list[sv->count])
         FREE(sv->list[sv->count]);
      sv->list[sv->count] = NULL;
   }
   return cp;
}

/****************************************************************************************/
C_FUNC_PREFIX TCHAR *SV_set(SV_t *sv, int index, const TCHAR *s, int dokeep)
/****************************************************************************************/
{
   TCHAR *cp = NULL;


   if (SV_isokay(sv,index))
   {
      if (dokeep)
         cp = sv->list[index];
      else
         FREE(sv->list[index]);
      sv->list[index] = STRDUP(s);
   }
   return cp;
}

/****************************************************************************************/
C_FUNC_PREFIX SV_t *SV_newf(const TCHAR *fname, size_t limit)
/****************************************************************************************/
/*
 * create a new SV from a file: read line by line and put each line in the vector.
 * the vector is in fact a linewise memory image of the file.
 */
{
   char  *image, *head, *tail, *next;
   SV_t  *sv;
   size_t size;


   /* try to load the complete file into memory */
   if (!STRHASLEN(fname) || (image=(char *)fimage(fname,&size)) == NULL)
   {
      /* do not care for errors, just return empty vect with default attributes */
      sv = SV_new(1024,0,limit);
      if (sv && STRHASLEN(fname))
         sv->fname = STRDUP(fname);
      return sv;
   }


   /*
    * a) count the lines in the file image and determine the initial allocation
    *    size of string vector.
    * b) clean up all dirty control chars
    */
   size = 0;
   for(head=image; *head; head++)
   {
      int uc = (int)*head;
      if (uc == '\n')
         size++;
      else if ((uc < ' ' && !ISSPACE(uc)) || uc == 0x7f || uc == 0xff)
         *head = '?';
   }

   /*
    * fix initial size, grow size and limit:
    *    limit == 0, then limit == size
    */
   size = _SV_SIZE_LIMIT(size);
   limit = (!limit) ? size : _SV_SIZE_LIMIT(limit);

   size *= 2; /* initial allocation == 2 x no. of lines */
   sv = (size > limit) ? SV_new(limit,0,limit) : SV_new(size,size/4,limit);
   if (!sv) goto EXIT_FREE;
   sv->fname = STRDUP(fname);

   /*
    * loop over all lines in the image, remove leading and trailing whitespace
    * and SV_push() all non empty lines.
    */
   next = image;
   while(next)
   {
      /*
       * to be UNICODE save we cannot use the preprocessor macros from "stdmacros.h"
       */
      for(head=next; isspace(*head); head++);                  /* skip leading whitespace */
      if (!*head) break;                                       /* EOF reached */
      for(next=head+1; *next && *next != '\n'; next++);        /* find start of next line */
      for(tail=next-1; tail>head && isspace(*tail); tail--);   /* find tail of current line */
      if (*next) next++;                                       /* goto start of next list ... */
      else       next = NULL;                                  /* ... or EOF reached */
      tail[1] = 0;                                             /* terminate current line */
      if (tail >= head)
      {
      #if IS_UNICODE

         wchar_t *wstr;

         size = (tail-head) + 2;
         wstr = (wchar_t *)MALLOC(size*sizeof(wchar_t));
         MultiByteToWideChar(CP_ACP,0,head,-1,wstr,CAST_INT(size));
         SV_push(sv,wstr,1); /* do not dup the string */

      #else

         SV_push(sv,head,0);

      #endif
      }
   }

EXIT_FREE:
   FREE(image);
   return sv;
}

/****************************************************************************************/
C_FUNC_PREFIX size_t SV_maxstrlen(SV_t *sv)
/****************************************************************************************/
/*
 * return the max. length for all strings.
 */
{
   size_t maxlen = 0;


   if (sv)
   {
      size_t i;
      for(i=0; i<sv->count; i++)
      {
         size_t len = STRLENP(sv->list[i]);
         if (len > maxlen) maxlen = len;
      }
   }
   return maxlen;
}

/****************************************************************************************/
C_FUNC_PREFIX int SV_save(SV_t *sv, const TCHAR *fname, int istart)
/****************************************************************************************/
/*
 * save sv in a file:
 *    Under MSWin we cannot use the VC CRT functions fopen()/fprintf()/creat()/write()
 *    to be multithreaded save.
 */
{
   char  *tbuf;
   size_t i,ipos,maxlen;


#if IS_MSWIN
   HANDLE h;
#else
   int fd;
#endif


   if (!sv || !sv->count)
      return 0;

   if (!STRHASLEN(fname))
   {
      fname = sv->fname;
      if (!STRHASLEN(fname))
         return -1;
   }

   /* find the longest string in the vector and MALLOC a transfer buffer */
   maxlen = SV_maxstrlen(sv);
   if (!maxlen)
      return -1; /* nothing to save */

#if IS_MSWIN
   h = CreateFile
       (
         fname,
         GENERIC_WRITE,
         0,
         NULL,
         CREATE_ALWAYS,
         FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
         NULL
       );
   if (IS_INVALID_HANDLE(h)) return -1;
#else
   fd = creat(fname,0666));
   if (fd < 0) return -1;
#endif

   maxlen = 2*maxlen + 2; /* in case we get MB chars string due to UNICODE conversion */
   tbuf = MALLOC(maxlen);
   ipos = (istart<0) ? 0 : (istart>=CAST_INT(sv->count)) ? sv->count-1 : CAST_SIZE(istart);
   for(i=0; i<sv->count; i++)
   {
      TCHAR *str = sv->list[ipos];
      size_t len = STRLENP(str);
      if (len)
      {
      #if IS_UNICODE
         len = WideCharToMultiByte(CP_ACP,0,str,-1,tbuf,CAST_INT(maxlen),NULL,NULL);
         if (len) len--; /* len includes the '\0' */
      #else
         strcpy(tbuf,str);
      #endif
         /* change terminating '\0' into '\n' and write including the '\n' */
         tbuf[len++] = '\n';
      #if IS_MSWIN
         WriteFile(h,tbuf,(DWORD)len,(DWORD *)(&len),NULL);
      #else
         write(fd,tbuf,len);
      #endif
      }
      ipos = ++ipos % sv->count;
   }
   FREE(tbuf);


#if IS_MSWIN
   CloseHandle(h);
#else
   close(fd);
#endif

   return 0;
}

/****************************************************************************************/
C_FUNC_PREFIX SV_t *SV_free(SV_t *sv, int save)
/****************************************************************************************/
/*
 * destructor resp. operator delete()
 */
{
   if (sv)
   {
      size_t i;

      if (save)
         SV_save(sv,NULL,0);

      for(i=0; i<sv->count; i++)
         if (sv->list[i]) FREE(sv->list[i]);

      FREE(sv->list);
      if (sv->fname) FREE(sv->fname);
      FREE(sv);
   }
   return NULL;
}

/****************************************************************************************/

#if SV_USE_PRINT

/****************************************************************************************/
C_FUNC_PREFIX void SV_print(SV_t *sv, FILE *fp, int withid)
/****************************************************************************************/
{
   if (sv && sv->count)
   {
      size_t i;

   #if IS_MSWIN
      /* The ANSI codepage is required for cases with non ASCII chars in the vector. */
      UINT concp = GetConsoleOutputCP();
      SetConsoleOutputCP(GetACP());
   #endif

      if (withid)
      {
         for(i=0; i<sv->count; i++)
            if (sv->list[i]) FPRINTF(fp,TEXT("%5u  %s\n"),(unsigned)i,sv->list[i]);
      }
      else
      {
         for(i=0; i<sv->count; i++)
            if (sv->list[i]) FPRINTF(fp,TEXT("%s\n"),sv->list[i]);
      }

   #if IS_MSWIN
      SetConsoleOutputCP(concp);
   #endif
   }
}

/****************************************************************************************/

#endif

#if SV_USE_SEND && IS_MSWIN

#include "utf8.h"

#if INCLUDE_STATIC
   #include "SockSendUTF8.c"
#endif

/****************************************************************************************/
C_FUNC_PREFIX void SV_send(SV_t *sv, SOCKET sock, TCHAR *cbuf, size_t size)
/****************************************************************************************/
{
   if (sv && sv->count && buf && size)
   {
      size_t i;
      UINT   bestcp = CP_ACP;

      for(i=0; i<sv->count; i++)
      {
         if (sv->list[i])
         {
            _sntprintf(cbuf,size,TEXT("%5d  %s\r\n"),i,sv->list[i]);
            cbuf[size-1] = '\0';
         #if IS_UNICODE
            if (SockSend(sock,(const char *)cbuf,WideStringToUTF8String(cbuf)) <= 0) break;
         #else
            if (!SockSendUTF8(sock,cbuf,STRLEN(cbuf),NULL,&bestcp,FALSE)) break;
         #endif
         }
      }
   }
}

/****************************************************************************************/

#endif

#if 0
int main(void)
{
   SV_t *sv=SV_newf(TEXT("C:\\users\\dehning\\.rsh\\rlogin.his"),1024);
   SV_print(sv,stdout,1);
   SV_save(sv,TEXT("sv_file"),CAST_INT(SV_getlasti(sv)));
   SV_free(sv,0);
   return 0;
}
#endif

#endif
