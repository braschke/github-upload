#pragma once
#ifndef stdconfig_HEADER_INCLUDED
#define stdconfig_HEADER_INCLUDED
/* stdconfig.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Do platform dependent preparations:
 *    Must be first #included before all other included including std c headers.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: stdconfig.h 5562 2017-08-30 11:18:23Z dehning $
 *
 *****************************************************************************************
 */

/***********************************************************
 *
 * Never include any system header before!!
 * Under MSWin we need winsock2.h FIRST!!
 *
 ************************************************************/

#ifndef     INCLUDE_STATIC
   #define  INCLUDE_STATIC    1
#endif

#ifndef     DOUBLE_PRECISION
   #define  DOUBLE_PRECISION  0
#endif

#ifndef     IS_UNICODE
   #define  IS_UNICODE        0
#endif

/*
 * step 1: separate xp/vista/win7 32/64 bits and Unix(linux,hpux,irix..)
 */
#if defined(WIN64) || defined(_WIN64)

   /* native xp/vista/win7 64 bits */
   #define IS_MSWIN     1
   #define IS_64BIT     1
   #define PTR_SSIZE    "64"

#elif defined(WIN32) || defined(_WIN32) || defined(_WINNT)

   /* native xp/vista/win7 32 bits */
   #define IS_MSWIN     1
   #define IS_64BIT     0
   #define PTR_SSIZE    "32"

#else

   /* !MSWin == assume UNIX: linux, hpux, irix, sunos, osfalpha ... */
   #define IS_MSWIN     0

#endif


#if IS_MSWIN

   #if defined(__GNUC__)
      /* gcc cross compilation for MSWin (mingw etc.) */
      #define IS_MINGW     1
   #endif

   /* OLD CHECK #if defined(_WINSOCKAPI_) || defined(_WINSOCK2API_) || defined(_WINDOWS_) || defined(_INC_WINDOWS) */
   #if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
      #error Do not include any windows header file windows.h || winsock.h before stdconfig.h
   #endif

   #ifdef _WIN32_WINNT
      /* compile for at least XP(5.1) */
      #if _WIN32_WINNT < 0x0501
         #error Need to compile for at least Windows XP 5.1, but _WIN32_WINNT is defined < 5.1
      #endif
   #else
      /*#define _WIN32_WINNT 0x0501 */
      /*#pragma message("Compiling: Platform >= XP")*/

      #define _WIN32_WINNT 0x0601
      #if !defined(IS_MINGW)
         #pragma message("Compiling: Platform >= Win7")
      #endif
   #endif

   /* cleanup the unicode flags and make a unique unicode #define */
   #if IS_UNICODE || defined(UNICODE) || defined(_UNICODE)

      #ifndef UNICODE
         #define UNICODE  /* MS SDK headers use unicode */
      #endif
      #ifndef _UNICODE
         #define _UNICODE /* VC runtime lib headers use unicode */
      #endif

      #undef  IS_UNICODE
      #define IS_UNICODE 1

   #else

      /* no unicode at all */
      #undef     UNICODE /* no MS SDK unicode */
      #undef    _UNICODE /* no VC runtime lib unicode */

      #undef  IS_UNICODE
      #define IS_UNICODE 0

   #endif


   /* avoid warnings about strcpy() vs. strcpy_s() etc. with VC >= 8.0 */
   #ifndef _CRT_SECURE_NO_DEPRECATE
      #define _CRT_SECURE_NO_DEPRECATE
   #endif

   /* get #defines for M_E, M_PI, M_PI_2 etc. from <math.h> */
   #ifndef _USE_MATH_DEFINES
      #define _USE_MATH_DEFINES
   #endif

   /*
    * manage winsock 1.0 and 2.0 stuff
    *    for VC98(6.0)  (cl version 12.000.xxxx) we use winsock 1.0
    *    for VC >= 7.0  (cl version 13.000.xxxx) we use winsock 2.x
    */
   #ifdef IS_MINGW
      #undef  IS_MSWIN
      #define IS_MSWIN  1500
   #else
      #ifndef _MSC_VER
         #error The variable _MSC_VER is not defined. I am clueless about the Visual C compiler version.
      #endif
      #undef  IS_MSWIN
      #define IS_MSWIN  _MSC_VER
   #endif

   #if (IS_MSWIN <= 1200)

      #ifndef _WINDOWS_
         #include <windows.h>
      #endif
      #ifndef  _WINSOCKAPI_
         #include <winsock.h>
      #endif

   #else

      #if (_MSC_VER >= 1800)
         #pragma warning (disable : 4996)
      #endif

      /*
       * ATTTENTION:
       *    #include winsock2.h BEFORE windows.h for VC >= 7.0
       *    -> windows.h #includes <winsock.h> which is incompatible with <winsock2.h>
       *    -> winsock2.h automatically #includes <windows.h>
       */
      #ifndef _WINSOCK2API_
         #include <winsock2.h>
      #endif

   #endif

   /* we always need this for the UNICODE stuff */
   #include <tchar.h>

   #ifndef IS_MINGW
      /* we always need the kernel32.dll */
      #pragma comment(lib,"kernel32")
   #endif

   #define IS_UNIX      0

   /* windows 32 bit has no "long long" type */
   #define HAVE_LONGLONG   0

   /* name of the environment variable which holds the DLL pathlist */
   #define LD_LIBRARY_PATH       "PATH"

   /* suffix of the .dll file */
   #define LD_SO_EXTENSION       ".dll"

   /* CPUID instruction only on x86 and Itanium type */
   #if _M_IX86 || _M_AMD64 || _M_IA64
      #define HAVE_CPUID 1
      #define HAVE_RDTSC 1
   #else
      #define HAVE_CPUID 0
      #define HAVE_RDTSC 0
   #endif

   /*
    * SSE instructions only for Intel or AMD processors.
    *
    * make an array aligned by 4 bytes (double precision). This is required
    * for array[>=4] declarations used in SSE instructions
    *
    * Examples:
    *    int i;
    *    SSE_VALIGN(float  vect1[4]);
    *    SSE_VALIGN(double dv[4*NVECTS]);
    *
    */
   #if _M_IX86 || _M_AMD64

      #define USE_SSE   1
      #define SSE_ALIGN(_s)   __declspec(align(16)) #_s

   #else

      #define USE_SSE   0
      #define SSE_ALIGN(_s)   #_s

   #endif

   #define C_FUNC_PREFIX_EXPORT  __declspec(dllexport)
   #define C_FUNC_PREFIX_IMPORT  __declspec(dllimport)
   #define C_FUNC_PREFIX_EXTERN  extern __declspec(dllimport)

   #define forceinline __forceinline

#else /* assume UNIX: linux, hpux, irix, sunos, osfalpha ... */

   #include <unistd.h>

   /* make UNIX compatible to MSwin VC */
   #define TCHAR        char
   #define TEXT(_text)  _text

   #if defined(__GNUC__) || defined(__ICC) && \
      ( \
         defined(__i386__) || \
         defined(__i486__) || \
         defined(__i586__) || \
         defined(__i686__) || \
         defined(__ia64__) || \
         defined(__x86_64__) \
      )

      #define HAVE_CPUID         1
      #define HAVE_RDTSC         1
      #define USE_SSE            1
      #define SSE_ALIGN(_s)      #_s  __attribute__ ((__aligned__ (4)))

      #define IS_UNIX            1

      /* we do not need the "long long" type right now */
      #define HAVE_LONGLONG      0

      #if defined(__APPLE__)
         #define IS_MACOSX          1
         #define LD_SO_EXTENSION    ".dylib"
         #define LD_LIBRARY_PATH    "DYLD_LIBRARY_PATH"
      #else
         #define IS_LINUX           1
         #define LD_SO_EXTENSION    ".so"
         #define LD_LIBRARY_PATH    "LD_LIBRARY_PATH"
      #endif

      #if defined(__LP64__)
         #define IS_64BIT     1
         #define PTR_SSIZE    "64"
      #else
         #define IS_64BIT     0
         #define PTR_SSIZE    "32"
      #endif

      #define forceinline __attribute__((always_inline)) inline

   #else /* no Intel or AMD processor: not Linux */

      #define HAVE_CPUID      0
      #define HAVE_RDTSC      0
      #define USE_SSE         0
      #define SSE_ALIGN(_s)   #_s

      #define forceinline inline

      /*
       * defines for a specific UNIX systems
       */
      #if defined(__osf__) || defined(__alpha__)


         #define IS_UNIX            1
         #define IS_OSFALPHA        1
         #define IS_64BIT           1 /* TRUE unix is always 64 bit */
         #define PTR_SSIZE          "64"
         #define HAVE_LONGLONG      0 /* avoid compiler warnings */
         #define LD_SO_EXTENSION    ".so"
         #define LD_LIBRARY_PATH    "LD_LIBRARY_PATH"


      #elif defined(__hpux) || defined(hpux) || defined(__hppa__)


         #define IS_UNIX            1
         #define IS_HPUX11          1
         #define HAVE_LONGLONG      1
         #define LD_SO_EXTENSION    ".sl"

         #if defined(__LP64__)
            #define IS_64BIT           1
            #define PTR_SSIZE          "64"
            #define LD_LIBRARY_PATH    "LD_LIBRARY_PATH"
         #else
            #define IS_64BIT           0
            #define PTR_SSIZE          "32"
            #define LD_LIBRARY_PATH    "SHLIB_PATH" /* only on old 32 bit systems */
         #endif


      #elif defined(sgi) || defined(__sgi__) || defined(__sgi) || defined(__mips__)


         #define IS_UNIX            1
         #define IS_IRIX65          1
         #define HAVE_LONGLONG      1
         #define LD_SO_EXTENSION    ".so"

         #if (_MIPS_SIM == _MIPS_SIM_ABI64) || defined(__LP64__)
            #define IS_64BIT           1
            #define PTR_SSIZE          "64"
            #define LD_LIBRARY_PATH    "LD_LIBRARY64_PATH"
         #else
            #define IS_64BIT           0
            #define PTR_SSIZE          "32"
            #define LD_LIBRARY_PATH    "LD_LIBRARYN32_PATH"
         #endif


      #elif defined(sun) || defined(__sun) || defined(__sparc__)


         #define IS_UNIX            1
         #define IS_SUNOS           1
         #define HAVE_LONGLONG      1
         #define LD_SO_EXTENSION    ".so"
         #define LD_LIBRARY_PATH    "LD_LIBRARY_PATH"

         #if defined(__sparcv9) || defined (__sparc_v9__) || defined(__LP64__)
            #define IS_64BIT     1
            #define PTR_SSIZE    "64"
         #else
            #define IS_64BIT     0
            #define PTR_SSIZE    "32"
         #endif


      #elif defined(aix) || defined(__aix) || defined(__ppc__) || \
            defined(_AIX)            || \
            defined(_AIXVERSION_510) || \
            defined(_AIXVERSION_520) || \
            defined(_AIXVERSION_530)


         #define IS_UNIX            1
         #define IS_AIX             1
         #define HAVE_LONGLONG      1
         #define LD_SO_EXTENSION    ".so"
         #define LD_LIBRARY_PATH    "LIBPATH"

         /* required for the socket stuff: hstrerror() */
         #ifndef _USE_IRS
            #define _USE_IRS
         #endif

         #if defined(__LP64__) || defined(__64BIT__)
            #define IS_64BIT     1
            #define PTR_SSIZE    "64"
         #else
            #define IS_64BIT     0
            #define PTR_SSIZE    "32"
         #endif


      #endif


      #if !defined(IS_UNIX) || !IS_UNIX
         #error Failed to figure out the UNIX system type
      #endif

   #endif


   #define C_FUNC_PREFIX_EXPORT
   #define C_FUNC_PREFIX_IMPORT
   #define C_FUNC_PREFIX_EXTERN  extern

#endif

#if INCLUDE_STATIC
   #define C_FUNC_PREFIX static
#else
   #define C_FUNC_PREFIX
#endif

#if IS_MSWIN && !defined(IS_MINGW)

   #if defined(WIN64) || defined(_WIN64)
      #pragma message("Compiling: Code is 64 bit")
   #else
      #pragma message("Compiling: Code is 32 bit")
   #endif

   #if IS_UNICODE
      #pragma message("Compiling: TCHAR is 16 bit UNICODE")
   #else
      #pragma message("Compiling: TCHAR is 8 bit ANSI")
   #endif

   #if INCLUDE_STATIC
      #pragma message("Compiling: Functions are static")
   #else
      #pragma message("Compiling: Functions are visible")
   #endif

   #if DOUBLE_PRECISION
      #pragma message("Compiling: real type is double")
   #else
      #pragma message("Compiling: real type is float")
   #endif

   #ifdef _OPENMP
      #pragma message("Compiling: OpenMP version")
   #else
      #pragma message("Compiling: No OpenMP version")
   #endif

#endif

#endif
