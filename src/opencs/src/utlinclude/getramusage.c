#pragma once
#ifndef getramusage_SOURCE_INCLUDED
#define getramusage_SOURCE_INCLUDED
/* getramusage.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Returns the physical (virtual) memory (ram+swapped out) used by this process
 *    and all its threads.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Sep/04: Carsten Dehning, Initial release
 *    $Id: getramusage.c 5292 2017-03-02 19:34:51Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"


#if IS_MSWIN

   /*
    * Simply return the current working set size:
    *    with Windows >= XP SP2 we just return the Working Set PrivateUsage, since
    *    this is the memory displayed by the task-manager.
    *
    * ATTENTION:
    * In the Win7 SDK header files (psapi.h) we find
    *
    *    #if (PSAPI_VERSION > 1)
    *          ......
    *          #define GetProcessMemoryInfo        K32GetProcessMemoryInfo
    *          ......
    *    #endif
    *
    * and thus a >= Win7 compilation might not work on versions < Win7
    */
   #include <psapi.h>
   #if (PSAPI_VERSION < 2)
      #ifndef IS_MINGW
         #pragma message("Compiling with old PSAPI Version 1")
      #endif
      #pragma comment(lib,"psapi")
   #endif

   C_FUNC_PREFIX size_t getramusage(void)
   {
      PROCESS_MEMORY_COUNTERS_EX pmc;
      return (GetProcessMemoryInfo(GetCurrentProcess(),(PROCESS_MEMORY_COUNTERS*)(&pmc),sizeof(pmc)))
      /* ? pmc.WorkingSetSize : 0; */
         ? pmc.PrivateUsage : 0;
   }

#elif defined(IS_LINUX)

   /*
    * We simply read the "/proc/$PID/stat" from the /proc filesystem.
    */
   #include <sys/fcntl.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      int   fd;
      char  fname[32];

      sprintf(fname,"/proc/%d/statm",getpid());
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
      if ((fd=open(fname,O_RDONLY)) >= 0)
      {
         /* first integer in the statm file is the used no. of pages */
         int n = read(fd,fname,CAST_INT(sizeof(fname))-1);
         close(fd);
         if (n > 1)
         {
            fname[n] = '\0';
            return CAST_SIZE(strtoul(fname,NULL,10))*getpagesize();
         }
      }
      return 0;
   }

#elif defined(IS_SUNOS)

   /*
    * simply get the size of the address space file "/proc/$PID/as"
    */
   #include <sys/types.h>
   #include <sys/stat.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      #if IS_64BIT
         struct stat64 sb;
         char  fname[32];
         sprintf(fname,"/proc/%d/as",getpid());
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
         return (stat64(fname,&sb) < 0)  ? 0 : sb.st_size;
      #else
         struct stat sb;
         char  fname[32];
         sprintf(fname,"/proc/%d/as",getpid());
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
         return (stat(fname,&sb) < 0)  ? 0 : sb.st_size;
      #endif
   }

#elif defined(IS_AIX)

   /*
    * "/proc/$PID/psinfo" is a binary image of the struct psinfo
    */
   #include <fcntl.h>
   #include <sys/procfs.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      int   fd;
      char  fname[32];

      sprintf(fname,"/proc/%d/psinfo",getpid());
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
      if ((fd=open(fname,O_RDONLY)) >= 0)
      {
         struct psinfo psi;
         int n = read(fd,&psi,sizeof(psi));
         close(fd);
         if (n == CAST_INT(sizeof(psi)))
            return psi.pr_size*1024; /* comment says its in KB, but I am not really sure */
      }
      return 0;
   }


#elif defined(IS_HPUX11)

   /*
    * HPUX11 has a system call
    */
   #include <sys/types.h>
   #include <sys/param.h>
   #include <sys/pstat.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      struct pst_status pst;
#if 0
printf("check file \"%s\": pagsize=%d\n","pstat_getproc",getpagesize());
#endif
      return (pstat_getproc(&pst,sizeof(pst),0,getpid()) < 0)
         ? 0
         : (pst.pst_dsize+pst.pst_tsize+pst.pst_ssize+pst.pst_mmsize) * getpagesize();
   }

#elif defined(IS_OSFALPHA)

   /*
    * OSF/Alpha has an ioctl() on "/proc/$PID"
    */
   #include <fcntl.h>
   #include <sys/procfs.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      int   fd;
      char  fname[32];

      sprintf(fname,"/proc/%d",getpid());
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
      if ((fd=open(fname,O_RDONLY)) >= 0)
      {
         struct prpsinfo psi;
         int n = ioctl(fd,PIOCPSINFO,&psi);
         close(fd);
         if (n >= 0)
            return psi.pr_size * getpagesize();
      }
      return 0;
   }

#elif defined(IS_IRIX65)

   /*
    * IRIX 6.5 has an ioctl() on "/proc/$PID"
    */
   #include <fcntl.h>
   #include <sys/procfs.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      int   fd;
      char  fname[32];

      sprintf(fname,"/proc/%010d",getpid()); /* 10 digits with leading '0' */
#if 0
printf("check file \"%s\": pagsize=%d\n",fname,getpagesize());
#endif
      if ((fd=open(fname,O_RDONLY)) >= 0)
      {
         struct prpsinfo psi;
         int n = ioctl(fd,PIOCPSINFO,&psi);
         close(fd);
         if (n >= 0)
            return psi.pr_size * getpagesize();
      }
      return 0;
   }

#elif defined(IS_MACOSX)

   #include <sys/time.h>
   #include <sys/resource.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      /* return stack + heap size */
      struct rusage buf;
      getrusage(RUSAGE_SELF,&buf);
      return CAST_SIZE(buf.ru_idrss) + CAST_SIZE(buf.ru_isrss);
   }

#elif 0 /* getrusage() does not really work */

   /*
    * dirty hack if not specialized
    */
   #include <sys/time.h>
   #include <sys/resource.h>
   C_FUNC_PREFIX size_t getramusage(void)
   {
      /* return stack + heap size */
      struct rusage buf;
      getrusage(RUSAGE_SELF,&buf);
      return CAST_SIZE(buf.ru_idrss) + CAST_SIZE(buf.ru_isrss);
   }

#else

   /*
    * dirty hack - which does not really help - if not specialized above
    */
   C_FUNC_PREFIX size_t getramusage(void)
   {
      char *end = (char *)sbrk(0);
      return CAST_SIZE(end - (char *)NULL);
   }

#endif

#if 0
int main(void)
{
#define SZ (50*1024*1024 + 64)
   void *p1,*p2;

   printf("start  : %7ld Kb\n",getramusage()/1024);

   p1 = malloc(SZ);
   memset(p1,1,SZ);
   printf("malloc-1: %7ld Kb\n",getramusage()/1024);

   p2 = malloc(SZ);
   memset(p2,1,SZ);
   printf("malloc-2: %7ld Kb\n",getramusage()/1024);

   free(p1);
   printf("free  -1: %7ld Kb\n",getramusage()/1024);

   free(p2);
   printf("free  -2: %7ld Kb\n",getramusage()/1024);

   p1 = malloc(SZ);
   memset(p1,1,SZ);
   printf("malloc-1: %7ld Kb\n",getramusage()/1024);

   p2 = malloc(SZ);
   memset(p2,1,SZ);
   printf("malloc-2: %7ld Kb\n",getramusage()/1024);

   return 0;
}
#endif

#endif
