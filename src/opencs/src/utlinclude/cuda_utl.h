#pragma once
#ifndef cuda_utl_HEADER_INCLUDED
#define cuda_utl_HEADER_INCLUDED
/* cuda_utl.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Utilities to check if CUDA software + hardware is installed
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2009/Jun/11: Carsten Dehning, Initial release
 *    $Id: cuda_utl.h 5511 2017-08-21 08:02:53Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

/*
 * Optional CUDA development kit headers
 */
#if defined(CUDA_INC_PATH)
   #include "driver_types.h"
#elif IS_MSWIN
   #pragma message("CUDA_INC_PATH not defined. Compiling dummy version")
#endif


#ifndef __DRIVER_TYPES_H__

   /*
    * this is just a copy of the basics information from driver_types.h
    */
   enum cudaError { cudaSuccess = 0 };
   typedef enum cudaError cudaError_t;

   struct cudaDeviceProp
   {
      char     name[256];
      size_t   totalGlobalMem;
      size_t   sharedMemPerBlock;
      int      regsPerBlock;
      int      warpSize;
      size_t   memPitch;
      int      maxThreadsPerBlock;
      int      maxThreadsDim[3];
      int      maxGridSize[3];
      int      clockRate;
      size_t   totalConstMem;
      int      major;
      int      minor;
      size_t   textureAlignment;
      int      deviceOverlap;
      int      multiProcessorCount;
      int      kernelExecTimeoutEnabled;
      int      integrated;
      int      canMapHostMemory;
      int      computeMode;

      int      __cudaReserved[36];
   };

#endif

typedef struct _CUDA
{
   /* no of installed CUDA devices */
   int  ndev;

   /* handle to the shared library */
   #if IS_MSWIN
      HMODULE   handle;
   #else
      void     *handle;
   #endif

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
} CUDA;

#if !INCLUDE_STATIC
   extern const CUDA *CUDA_init              (const int prinfo);
   extern int         CUDA_getdevicebyname   (const CUDA *cuda, const char *devname);
   extern int         CUDA_getdevicebymemory (const CUDA *cuda);
#endif

#endif
