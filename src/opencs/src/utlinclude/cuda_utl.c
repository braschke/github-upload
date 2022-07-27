#pragma once
#ifndef cuda_utl_SOURCE_INCLUDED
#define cuda_utl_SOURCE_INCLUDED
/* cuda_utl.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Test if CUDA software + hardware is installed
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2009/Jun/11: Carsten Dehning, Initial release
 *    $Id: cuda_utl.c 5511 2017-08-21 08:02:53Z dehning $
 *
 *****************************************************************************************
 */
#include "cuda_utl.h"

#if !IS_MSWIN
   #include <dlfcn.h>
#endif



#if defined(CUDA_INC_PATH) && IS_MSWIN

/****************************************************************************************/
static HMODULE _cuda_init_load_lib(const TCHAR *name)
/****************************************************************************************/
{
   HMODULE handle = LoadLibrary(name);
   if (!handle)
   {
      _ftprintf
      (
         stderr,
         TEXT("CUDA_init: no CUDA library \"%s\".\n"),
         name
      );
   }
   return handle;
}

/****************************************************************************************/

#endif

/****************************************************************************************/
C_FUNC_PREFIX const CUDA *CUDA_init(const int prinfo)
/****************************************************************************************/
{
#if defined(CUDA_INC_PATH)

   static CUDA cuda = {-1,0,0}; /* ndev<0 means: not yet initialised */

#if IS_MSWIN

   static const TCHAR *dllnames[] =
   {
   #if IS_64BIT
      TEXT("cudart64_80.dll"),
      TEXT("cudart64.dll"),
   #else
      TEXT("cudart32_80.dll"),
      TEXT("cudart32.dll"),
      TEXT("cudart.dll"),
   #endif
      NULL
   };

   HMODULE handle;
   int     i;

#else

   void   *handle;

#endif

   TCHAR  *binpath;
   TCHAR   dllpath[MAX_PATH];

   union
   {
      void         *vp;
      cudaError_t (*GetDeviceCount)      (int *count);
      cudaError_t (*GetDeviceProperties) (struct cudaDeviceProp *prop, int dev);
      cudaError_t (*SetDevice)           (int dev);
      cudaError_t (*GetLastError)        (void);
      const char *(*GetErrorString)      (cudaError_t err);
      cudaError_t (*Malloc)              (void **devPtr, size_t count);
      cudaError_t (*Free)                (void *devPtr);
      cudaError_t (*Memcpy)              (void *dst, const void *src, size_t count, int kind);
      cudaError_t (*MemGetInfo)          (size_t *free, size_t *total);
      cudaError_t (*RuntimeGetVersion)   (int *runtimeVersion);
      cudaError_t (*DriverGetVersion)    (int *driverVersion);
      /*
       * .. add more function pointers here
       */
   } fp;


   cudaError_t err;
   int ndev = 0;


   if (cuda.ndev >= 0)
   {
      /* we already (tried to) find & initialize the cuda device */
      return (cuda.ndev > 0) ? &cuda : NULL;
   }
   cuda.ndev = 0; /* assume no cuda device available */

   #define CUDA_PATH    "CUDA_PATH"

   binpath = _STD_C_GETENV(TEXT(CUDA_PATH));
   if (!STRHASLEN(binpath))
   {
   #if IS_MSWIN
      binpath = TEXT("C:\\CUDA");
   #else
      binpath = TEXT("/usr/local/cuda");
   #endif
      fprintf
      (
         stderr,
         "CUDA_init: Environment variable \"%s\" ist not defined or empty.\n"
         "           Assuming \"%s=%s\"\n",
         CUDA_PATH,
         CUDA_PATH,
         binpath
      );
   }
   #undef CUDA_PATH

#if IS_MSWIN

   #define CUDA_GETFP(_name) *((FARPROC *)(&fp._name)) = GetProcAddress(handle,"cuda" #_name);

   /* try to load any of the dll's from the list */
   handle = NULL;
   for(i=0; dllnames[i]; i++)
   {
      _snprintf(dllpath,countof(dllpath),TEXT("%s\\bin\\%s"),binpath,dllnames[i]);
      if ((handle=_cuda_init_load_lib(dllpath    )) != NULL
       || (handle=_cuda_init_load_lib(dllnames[i])) != NULL)
         break;
   }

   if (!handle)
      return NULL;

#else

   #define CUDA_GETFP(_name)  fp.vp = dlsym(handle,"cuda" #_name);

   #if IS_64BIT
      snprintf(dllpath,countof(dllpath),TEXT("%s/lib64/%s"),binpath,"libcudart.so");
   #else
      snprintf(dllpath,countof(dllpath),TEXT("%s/lib/%s"),binpath,"libcudart.so");
   #endif

      handle = dlopen(dllpath,RTLD_LAZY|RTLD_GLOBAL);
      if (!handle)
      {
         fprintf
         (
            stderr,
            "CUDA_init: no CUDA library \"%s\".\n",
            dllpath
         );
         return NULL;
      }

#endif

   if (prinfo)
   {
      printf
      (
         "CUDA_init: loaded library \"%s\".\n",
         dllpath
      );
   }

   #define CUDA_LINKFP(_name) \
      CUDA_GETFP(_name)\
      if (!fp._name)\
      {\
         fputs("CUDA_init: function cuda" #_name "() not available.\n",stderr);\
         return NULL;\
      }

   #define CUDA_HOOK(_name) \
      CUDA_LINKFP(_name)\
      cuda._name = fp._name

   CUDA_HOOK(GetDeviceProperties);
   CUDA_HOOK(SetDevice);
   CUDA_HOOK(GetLastError);
   CUDA_HOOK(GetErrorString);
   CUDA_HOOK(Malloc);
   CUDA_HOOK(Free);
   CUDA_HOOK(Memcpy);
   CUDA_HOOK(MemGetInfo);
   CUDA_HOOK(RuntimeGetVersion);
   CUDA_HOOK(DriverGetVersion);
   /*
    * .. add more function pointers here
    */

   CUDA_LINKFP(GetDeviceCount)

   #undef CUDA_LINKFP
   #undef CUDA_HOOK
   #undef CUDA_GETFP

   err = fp.GetDeviceCount(&ndev);
   if (err)
   {
      fprintf
      (
         stderr,
         "cudaGetDeviceCount: error #%d (%s).\n",
         err,cuda.GetErrorString(err)
      );
      return NULL;
   }

   if (ndev <= 0)  /* no CUDA hardware installed */
   {
      fprintf
      (
         stderr,
         "CUDA_init: no CUDA hardware (ndev=%d).\n",
         ndev
      );
      return NULL;
   }

   if (ndev == 1)  /* only one CUDA device adavilable */
   {
      struct cudaDeviceProp prop;
      cudaError_t err = cuda.GetDeviceProperties(&prop,0);
      if (err)
      {
         fprintf
         (
            stderr,
            "CUDA_init: cudaGetDeviceProperties: error #%d: %s.\n",
            err,cuda.GetErrorString(err)
         );
         return NULL;
      }
   }

   /* success */
   cuda.handle = handle;
   cuda.ndev   = ndev;
   return &cuda;

#else

   fprintf(stderr,"CUDA_init: This is a dummy version without CUDA.\n");
   return NULL;

#endif
}

/****************************************************************************************/
C_FUNC_PREFIX int CUDA_getdevicebyname(const CUDA *cuda, const char *devname)
/****************************************************************************************/
{
   if (!cuda)
      return -1;

   if (cuda->ndev <= 0) /* no device, no scan required */
      return -1;

   if (cuda->ndev == 1) /* only one device, no scan required */
      return 0;

   if (!STRHASLEN(devname))  /* NULL or empty filename: read environment CUDA_DEVICE */
   {
      devname = _STD_C_GETENV("CUDA_DEVICE");
   }
   else if (devname[0] == '$') /* $ENVNAME: read environment ENVNAME */
   {
      devname = _STD_C_GETENV(devname+1);
   }

   if (STRHASLEN(devname))
   {
      int i;

      for(i=0; i<cuda->ndev; i++)
      {
         struct cudaDeviceProp prop;

         if (!cuda->GetDeviceProperties(&prop,i) && strstr(prop.name,devname))
            return i;
      }
   }
   return -1;
}

/****************************************************************************************/
C_FUNC_PREFIX int CUDA_getdevicebymemory(const CUDA *cuda)
/****************************************************************************************/
{
   size_t maxm = 0;
   int   i,idev = -1;


   if (!cuda)
      return -1;

   if (cuda->ndev <= 0) /* no device, no scan required */
      return -1;

   if (cuda->ndev == 1) /* only one device, no scan required */
      return 0;

   for(i=0; i<cuda->ndev; i++)
   {
      struct cudaDeviceProp prop;

      if (!cuda->GetDeviceProperties(&prop,i) && prop.totalGlobalMem > maxm)
      {
         idev = i;
         maxm = prop.totalGlobalMem;
      }
   }

   return idev;
}

/****************************************************************************************/


#if 1

#include "ulong2adp.c"

void print_error(const CUDA *cuda, const char *func, cudaError_t err)
{
   printf("%s: error #%d (%s).\n",func,err,cuda->GetErrorString(err));
}

int main(void)
{
   const CUDA *cuda  = CUDA_init(1);
   size_t      freemem,totmem;
   cudaError_t err;
   int         i;


   if (!cuda)
   {
      puts("CUDA is not available.");
      return 1;
   }

   printf("CUDA hardware is available: %d device%s.\n",cuda->ndev,plurals(cuda->ndev));
   i = 0;
   cuda->RuntimeGetVersion(&i);
   printf("Runtime version: %d\n",i);
   i = 0;
   cuda->DriverGetVersion(&i);
   printf("Driver version : %d\n",i);

   err = cuda->MemGetInfo(&freemem,&totmem);
   if (err)
   {
      print_error(cuda,"cudaMemGetInfo",err);
   }
   else
   {
      printf
      (
         "Total memory   : %14s Bytes\n"
         ,ulong2adp(totmem,NULL,0)
      );
      printf
      (
         "Free memory    : %14s Bytes (%.2f%%)\n"
         ,ulong2adp(freemem,NULL,0)
         ,(100.0*freemem)/totmem
      );
   }

   for(i=0; i<cuda->ndev; i++)
   {
      struct cudaDeviceProp prop;

      err = cuda->GetDeviceProperties(&prop,i);
      if (err)
      {
         print_error(cuda,"cudaGetDeviceProperties",err);
      }
      else
      {
         printf
         (
            "\nDevice %d: \"%s\"\n"
            "   Total global memory (MB)     : %u\n"
            "   Shared memory per block (KB) : %u\n"
            "   Registers per block          : %d\n"
            "   Warp size                    : %d\n"
            "   Max. memory pitch            : %s\n"
            "   Max. threads per block       : %d\n"
            "   Max. threads dimension       : %d x %d x %d\n"
            "   Max. grid size               : %d x %d x %d\n"
            "   Clock rate (MHz)             : %d\n"
            "   Total constant memory (KB)   : %u\n"
            "   Compute capability           : %d.%d\n"
            "   Texture alignment            : %u\n"
            "   Device overlap               : %d\n"
            "   Multiprocessors on device    : %d\n"
            "   Kernel exec timeout enabled  : %d\n"
            "   Device is integrated         : %d\n"
            "   Can map host memory          : %d\n"
            "   Compute mode                 : %d\n"
            , i
            , prop.name
            , (unsigned)(prop.totalGlobalMem/(1024*1024))
            , (unsigned)(prop.sharedMemPerBlock/1024)
            , prop.regsPerBlock
            , prop.warpSize
            , ulong2adp(prop.memPitch,NULL,0)
            , prop.maxThreadsPerBlock
            , prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2]
            , prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]
            , prop.clockRate/1000
            , (unsigned)prop.totalConstMem
            , prop.major, prop.minor
            , (unsigned)prop.textureAlignment
            , prop.deviceOverlap
            , prop.multiProcessorCount
            , prop.kernelExecTimeoutEnabled
            , prop.integrated
            , prop.canMapHostMemory
            , prop.computeMode
         );
      }
   }

   putchar('\n');

   printf("Best devicebymemory: %d.\n",CUDA_getdevicebymemory(cuda));
   printf("Best devicebyname  : %d.\n",CUDA_getdevicebyname(cuda,NULL));
   return 0;
}
#endif

#endif
