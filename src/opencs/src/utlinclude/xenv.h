#ifndef xenv_HEADER_INCLUDED
#define xenv_HEADER_INCLUDED
/* xenv.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Wrapper for getenv() and extended environment
 *    the std c getenv() may by redefined via pointers from external code.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/20: Carsten Dehning, Initial release
 *    $Id: xenv.h 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#ifndef XENV_USE

   /* do not use the extended environment */

   #undef  XENV_USE_PTR

   #define XENV_USE              0
   #define XENV_USE_PTR          0
   #define XENV_USE_SET          0
   #define XENV_USE_PUT          0

   #define XENV_USE_ENVC         0
   #define XENV_USE_ENVV         0
   #define XENV_USE_CLEAN        0

   #define XENV_USE_GETINT       0
   #define XENV_USE_GETLONG      0
   #define XENV_USE_GETFLOAT     0
   #define XENV_USE_GETDOUBLE    0
   #define XENV_USE_GETBOOL      0
   #define XENV_USE_DEFINED      0
   #define XENV_USE_ISTRUE       0
   #define XENV_USE_LOAD         0
   #define XENV_USE_PRINT        0


#else

   #ifndef XENV_USE_PTR
      #define XENV_USE_PTR       0
   #endif

   #ifndef XENV_USE_SET
      #define XENV_USE_SET       0
   #endif

   #ifndef XENV_USE_PUT
      #define XENV_USE_PUT       0
   #endif

   #ifndef XENV_USE_ENVC
      #define XENV_USE_ENVC      0
   #endif

   #ifndef XENV_USE_ENVV
      #define XENV_USE_ENVV      0
   #endif

   #ifndef XENV_USE_CLEAN
      #define XENV_USE_CLEAN     0
   #endif

   #ifndef XENV_USE_GETINT
      #define XENV_USE_GETINT    0
   #endif

   #ifndef XENV_USE_GETLONG
      #define XENV_USE_GETLONG   0
   #endif

   #ifndef XENV_USE_GETFLOAT
      #define XENV_USE_GETFLOAT  0
   #endif

   #ifndef XENV_USE_GETDOUBLE
      #define XENV_USE_GETDOUBLE 0
   #endif

   #ifndef XENV_USE_GETBOOL
      #define XENV_USE_GETBOOL   0
   #endif

   #ifndef XENV_USE_DEFINED
      #define XENV_USE_DEFINED   0
   #endif

   #ifndef XENV_USE_ISTRUE
      #define XENV_USE_ISTRUE    0
   #endif

   #ifndef XENV_USE_LOAD
      #define XENV_USE_LOAD      0
   #endif

   #ifndef XENV_USE_PRINT
      #define XENV_USE_PRINT     0
   #endif

#endif

#if XENV_USE

   /* use the extended environment */

   #if !INCLUDE_STATIC
   #ifdef __cplusplus
      extern "C" {
   #endif

      extern void    XENV_init     (char *(*ptrGetenv)(const char *name));

      extern char   *XENV_get      (const char *name);
      extern int     XENV_set      (const char *name, const char *value);
      extern int     XENV_put      (const char *neqv);


      extern int     XENV_getint   (const char *name, int    *pval);
      extern int     XENV_getlong  (const char *name, long   *pval);
      extern int     XENV_getfloat (const char *name, float  *pval);
      extern int     XENV_getdouble(const char *name, double *pval);
      extern int     XENV_getbool  (const char *name, int    *pval);
      extern int     XENV_defined  (const char *name);
      extern int     XENV_istrue   (const char *name);

      extern int     XENV_load     (const char *fname);
      extern void    XENV_print    (FILE *fp, int align);

      extern void    XENV_clean    (void);

      /* helper function for direct access */
      extern int           XENV_envc     (void);
      extern const char  **XENV_envv     (void);

   #ifdef __cplusplus
   }
   #endif

   #endif

   #define GETENV       XENV_get
   #define PUTENV       XENV_put

#else

   /* use std C getenv/putenv if not yet defined */
   #ifndef GETENV
      #define GETENV    _STD_C_GETENV
   #endif

   #ifndef PUTENV
      #define PUTENV    _STD_C_PUTENV
   #endif

#endif

#endif
