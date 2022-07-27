#pragma once
#ifndef winshm_SOURCE_INCLUDED
#define winshm_SOURCE_INCLUDED
/* winshm.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Implements the SYS5 shared memory system calls for Windows
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Jan/28: Carsten Dehning, Initial release
 *    $Id: winshm.c 5445 2017-08-03 17:49:44Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if !IS_MSWIN
   #error This shared memory wrapper can only be compiled for MSWin
#endif

typedef int key_t;

typedef struct _SHMINFO SHMINFO;

struct _SHMINFO
{
   SHMINFO *next;
   HANDLE   hMapFile;
   void    *addr;
   key_t    key;
   DWORD    procid;
   int      id;
   int      nrefs;
   int      size;
};

struct shmid_ds
{
   int dummy;
};

C_FUNC_PREFIX_EXPORT SHMINFO *shmHead = NULL;

#define FOREACH(_obj,_head)                  for(_obj=(_head); _obj; _obj=_obj->next)
#define FOREACH_NEXT(_obj,_head,_next)       for(_obj=(_head); _obj; _obj=_next)
#define FOREACH_COND(_obj,_head,_cond)       FOREACH(_obj,_head) if (_cond)

/****************************************************************************************/
static int _shminfo_delete(SHMINFO *shmdel)
/****************************************************************************************/
{
   if (shmdel == shmHead)
   {
      shmHead = shmHead->next;
   }
   else
   {
      SHMINFO *shm;

      FOREACH_COND(shm,shmHead, shm->next == shmdel)
      {
         shm->next = shmdel->next;
         goto EXIT_FREE;
      }
   }

EXIT_FREE:
   CloseHandle(shmdel->hMapFile);
   FREE(shmdel);
   return 0;
}

/****************************************************************************************/
C_FUNC_PREFIX_EXPORT void *shmat(int shmid, const void *shmaddr, int shmflg)
/****************************************************************************************/
{
   SHMINFO *shm;
   HANDLE   hMapFile;
   LPTSTR   pBuf;
   char     szName[128];


   (shmaddr);  // keep compiler happy
   (shmflg);   // keep compiler happy

   FOREACH_COND(shm,shmHead, shmid == shm->id)
   {
      goto FOUND;
   }
   errno = EINVAL;
   return NULL;


FOUND:
   shm->nrefs++;
   if (shm->procid == GetCurrentProcessId())
      return shm->addr;


   sprintf(szName,"\\Global\\shared_memory_%08x",shm->key);

   hMapFile = OpenFileMappingA
               (
                  FILE_MAP_ALL_ACCESS, // read/write access
                  FALSE,               // do not inherit the name
                  szName               // name of mapping object
               );

   if (hMapFile == NULL)
   {
      errno = ENOMEM;
      return NULL;
   }

   pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
               FILE_MAP_ALL_ACCESS,          // read/write permission
               0,
               0,
               shm->size);

   if (pBuf == NULL)
   {
      CloseHandle(hMapFile);
      errno = ENOMEM;
   }
   return pBuf;
}

/****************************************************************************************/
C_FUNC_PREFIX_EXPORT int shmdt(const void *shmaddr)
/****************************************************************************************/
{
   SHMINFO *shm;


   FOREACH_COND(shm,shmHead, shmaddr == shm->addr)
   {
      if (--(shm->nrefs) <= 0)
         _shminfo_delete(shm);
      return 0;
   }

   errno = EINVAL;
   return -1;
}

/****************************************************************************************/
C_FUNC_PREFIX_EXPORT int shmctl(int shmid, struct shmid_ds *buf)
/****************************************************************************************/
{
   SHMINFO *shm;


   (buf);   // keep compiler happy
   FOREACH_COND(shm,shmHead, shmid == shm->id)
   {
      return 0;
   }

   errno = EINVAL;
   return -1;
}

/****************************************************************************************/
C_FUNC_PREFIX_EXPORT int shmget(key_t key, int size, int shmflag)
/****************************************************************************************/
//
// Allocate shm and return a unique ID
//
{
   static   int shmid = 0;

   SHMINFO *shm;
   HANDLE   hMapFile;
   void    *pBuf;
   char     szName[128];


   (shmflag); // keep compiler happy

   FOREACH_COND(shm,shmHead, key == shm->key)
   {
      errno = EEXIST;
      return -1;
   }

   sprintf(szName,"\\Global\\shared_memory_%08x",key);

   hMapFile = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 0,                       // maximum object size (high-order DWORD)
                 size,                    // maximum object size (low-order DWORD)
                 szName);                 // name of mapping object

   if (hMapFile == NULL)
   {
      errno = ENOMEM;
      return -1;
   }

   pBuf = (LPTSTR) MapViewOfFile(hMapFile,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        size);

   if (pBuf == NULL)
   {
      CloseHandle(hMapFile);
      errno = ENOMEM;
      return -1;
   }

   shm = MALLOC(sizeof(SHMINFO));
   MEMZERO(shm,sizeof(SHMINFO));
   shm->addr   = pBuf;
   shm->id     = ++shmid;
   shm->procid = GetCurrentProcessId();
   shm->size   = size;
   shm->next   = shmHead;
   shmHead     = shm;
   return shmid;
}

/****************************************************************************************/
C_FUNC_PREFIX_EXPORT key_t ftok(const char *pathname, char proj)
/****************************************************************************************/
//
// Generate a GUID based on a pathname ... Q&D Hack
//
{
   key_t k = 0;
   int   i;


   for(i=0; pathname[i]; i++)
      k += pathname[i];

   return (k+proj);
}

/****************************************************************************************/

#endif
