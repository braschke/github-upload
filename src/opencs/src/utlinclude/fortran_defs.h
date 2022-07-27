#ifndef fortran_defs_HEADER_INCLUDED
#define fortran_defs_HEADER_INCLUDED
/* fortran_defs.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Template to define macros for specific FORTRAN compiler implementations
 *
 *    This file may be copied to a local directory and modified and then
 *    #included (base on the cc -I search rules) instead of this template version.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Sep/03: Carsten Dehning, Initial release
 *    $Id: fortran_defs.h 2651 2014-02-11 17:20:56Z dehning $
 *
 *****************************************************************************************
 */
#include "fortran_types.h"


/*****************************************************************************************
 * not sure, FALSE should be 0, TRUE should be !FALSE
 *****************************************************************************************/

#ifndef    FORTRAN_TRUE
   #define FORTRAN_TRUE     1
#endif

#ifndef    FORTRAN_FALSE
   #define FORTRAN_FALSE    0
#endif

/*
 * for savety these macros should be used with logical comparisons
 */
#define FLOGICAL_IS_FALSE(_lvar)    ( (_lvar) == FORTRAN_FALSE )
#define FLOGICAL_IS_TRUE(_lvar)     ( (_lvar) != FORTRAN_FALSE )


/*****************************************************************************************
 * handling of character strings and hidden stringlength argument
 *****************************************************************************************/

#ifdef FORTRAN_STRING_CCLL

   /*
    * hidden length follows at end of argument list:
    * SUB(...,C,...,C,...,C,..., L,L,L)
    */

   #if defined(FSTRING_TYP) || defined(FSTRLEN_TYP) || defined(FSTRING_ARG) || defined(FSTRLEN_ARG)
      #error The preprocessor variable FORTRAN_STRING_[CCLL|CLCL] is defined multiple times
      #error Definition of the FORTRAN CHARACTER*(*) convention is not unique
   #endif

   #define FSTRING_TYP(_str)     Fstring _str
   #define FSTRLEN_TYP(_str)   , Fstrlen _str##Len

   #define FSTRING_ARG(_str)     _str
   #define FSTRLEN_ARG(_str)   , _str##Len

#endif


#ifdef FORTRAN_STRING_CLCL

   /*
    * hidden length follows the string:
    * SUB(...,C,L,..,C,L,..,C,L,..)
    */

   #if defined(FSTRING_TYP) || defined(FSTRLEN_TYP) || defined(FSTRING_ARG) || defined(FSTRLEN_ARG)
      #error The preprocessor variable FORTRAN_STRING_[CCLL|CLCL] is defined multiple times
      #error Definition of the FORTRAN CHARACTER*(*) convention is not unique
   #endif

   #define FSTRING_TYP(_str)     Fstring _str, Fstrlen _str##Len
   #define FSTRLEN_TYP(_str)

   #define FSTRING_ARG(_str)     _str, _str##Len
   #define FSTRLEN_ARG(_str)

#endif

#ifndef FSTRING_TYP
   #error The preprocessor variable FORTRAN_STRING_... is not defined
   #error Definition of the FORTRAN CHARACTER*(*) calling convention is unclear
#endif


/*****************************************************************************************
 * handling of SUBROUTINE and FUNCTION names: upper/lowercase and underscores
 *****************************************************************************************/

#ifdef FORTRAN_NAMING_UULC
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  __##_lcname
#endif

#ifdef FORTRAN_NAMING_LCUU
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _lcname##__
#endif

#ifdef FORTRAN_NAMING_LCU
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _lcname##_
#endif

#ifdef FORTRAN_NAMING_LC
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _lcname
#endif

#ifdef FORTRAN_NAMING_UCUU
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _ucname##__
#endif

#ifdef FORTRAN_NAMING_UCU
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _ucname##_
#endif

#ifdef FORTRAN_NAMING_UC
   #ifdef FORTRAN_SUBNAME
      #error The preprocessor variable FORTRAN_NAMING_... is defined multiple times
      #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is not unique
   #endif
   #define FORTRAN_SUBNAME(_lcname,_ucname)  _ucname
#endif

#ifndef FORTRAN_SUBNAME
   #error The preprocessor variable FORTRAN_NAMING_... is not defined
   #error Type of FORTRAN SUBROUTINE/FUNCTION naming convention is unclear
#endif


/*****************************************************************************************
 * handling of COMMON BLOCK names: upper/lowercase and underscores
 *****************************************************************************************/

#ifdef FORTRAN_COMMON_ABSOFT
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _C##_lcname
#endif

#ifdef FORTRAN_COMMON_LCUU
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _lcname##__
#endif

#ifdef FORTRAN_COMMON_LCU
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _lcname##_
#endif

#ifdef FORTRAN_COMMON_LC
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _lcname
#endif

#ifdef FORTRAN_COMMON_UCUU
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _ucname##__
#endif

#ifdef FORTRAN_COMMON_UCU
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _ucname##_
#endif

#ifdef FORTRAN_COMMON_UC
   #ifdef FORTRAN_COMNAME
      #error The preprocessor variable FORTRAN_COMMON_... is defined multiple times
      #error Type of FORTRAN COMMON BLOCK naming convention is not unique
   #endif
   #define FORTRAN_COMNAME(_lcname,_ucname)  _ucname
#endif

#ifndef FORTRAN_COMNAME
   #error The preprocessor variable FORTRAN_COMMON_... is not defined
   #error Type of FORTRAN COMMON BLOCK naming convention is not defined
#endif

/*****************************************************************************************
 * derived macros
 *****************************************************************************************/

#if IS_MSWIN

   #ifdef FORTRAN_DLLIMPORT
      /* make DLLIMPORT the default import type */
      #undef  FORTRAN_DLLIMPORT
      #define FORTRAN_IMPORT  extern __declspec(dllimport)
   #else
      /* no DLLIMPORT is the standard */
      #define FORTRAN_IMPORT  extern
   #endif

   /* additionally (re)define the DLLIMPORT type */
   #define FORTRAN_DLLIMPORT  extern __declspec(dllimport)



   #ifdef FORTRAN_DLLEXPORT
      /* make DLLEXPORT the default export type */
      #undef  FORTRAN_DLLEXPORT
      #define FORTRAN_EXPORT  __declspec(dllexport)
   #else
      /* no DLLEXPORT is the standard */
      #define FORTRAN_EXPORT
   #endif

   /* additionally (re)define the DLLEXPORT type */
   #define FORTRAN_DLLEXPORT  __declspec(dllexport)



   #ifdef FORTRAN_STDCALL
      #ifdef FORTRAN_CALLING
         #error Either define the preprocessor variable FORTRAN_STDCALL or FORTRAN_CDECL but not both
         #error Type of FORTRAN COMMON BLOCK naming convention is not unique
      #endif
      #define FORTRAN_CALLING    __stdcall   /* microsoft VC/F77 */
   #endif

   #ifdef FORTRAN_CDECL
      #ifdef FORTRAN_CALLING
         #error Either define the preprocessor variable FORTRAN_STDCALL or FORTRAN_CDECL but not both
         #error Type of FORTRAN COMMON BLOCK naming convention is not unique
      #endif
      #define FORTRAN_CALLING    __cdecl     /* microsoft VC/F77 */
   #endif

   #ifndef FORTRAN_CALLING
      #define FORTRAN_CALLING                /* absoft/pgi/intel */
   #endif


#else /* UNIX */


   #define FORTRAN_DLLIMPORT  extern
   #define FORTRAN_IMPORT     extern

   #define FORTRAN_DLLEXPORT
   #define FORTRAN_EXPORT

   #define FORTRAN_CALLING

#endif


/*
 * declare an external FORTRAN function to the C world
 */
#define EXTERNAL_SUBROUTINE(_lc,_uc) FORTRAN_IMPORT void     FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define EXTERNAL_FC_LOGICAL(_lc,_uc) FORTRAN_IMPORT Flogical FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define EXTERNAL_FC_INTEGER(_lc,_uc) FORTRAN_IMPORT Fint     FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define EXTERNAL_FC_FLOAT(_lc,_uc)   FORTRAN_IMPORT Ffloat   FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define EXTERNAL_FC_REAL(_lc,_uc)    FORTRAN_IMPORT Freal    FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define EXTERNAL_FC_DOUBLE(_lc,_uc)  FORTRAN_IMPORT Fdouble  FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)

/*
 * make a C function look like FORTRAN
 */
#define FORTRAN_SUBROUTINE(_lc,_uc)  FORTRAN_EXPORT void     FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define FORTRAN_FC_LOGICAL(_lc,_uc)  FORTRAN_EXPORT Flogical FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define FORTRAN_FC_INTEGER(_lc,_uc)  FORTRAN_EXPORT Fint     FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define FORTRAN_FC_FLOAT(_lc,_uc)    FORTRAN_EXPORT Ffloat   FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define FORTRAN_FC_REAL(_lc,_uc)     FORTRAN_EXPORT Freal    FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)
#define FORTRAN_FC_DOUBLE(_lc,_uc)   FORTRAN_EXPORT Fdouble  FORTRAN_CALLING FORTRAN_SUBNAME(_lc,_uc)

/*
 * call a FORTRAN subroutine or function in the C world
 */
#define FORTRAN_CALL(_lc,_uc)        FORTRAN_SUBNAME(_lc,_uc)


/*
 * declare an external FORTRAN COMMON BLOCK to the C world
 */
#define EXTERNAL_COMMON(_lc,_uc)     FORTRAN_IMPORT FORTRAN_COMNAME(_lc,_uc)

#endif
