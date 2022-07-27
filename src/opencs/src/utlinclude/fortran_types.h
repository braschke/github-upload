#pragma once
#ifndef fortran_types_HEADER_INCLUDED
#define fortran_types_HEADER_INCLUDED
/* fortran_types.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Template to define the mapping of the integral types between C and Fortran.
 *
 *    This file may be copied to a local directory and modified and then
 *    #included (based on the cc -I search rules) instead of this template version.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Sep/03: Carsten Dehning, Initial release
 *    $Id: fortran_types.h 5532 2017-08-25 15:19:17Z dehning $
 *
 *****************************************************************************************
 */
#include "stdconfig.h"
#include "realtype.h"

#ifndef REAL_MAX
   #error REAL_MAX was not defined within included realtype.h
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if HAVE_LONGLONG
   typedef long long    Fint16;
#endif

/*
 * specify the size of the basic integral types
 */
typedef char            Fchar;
typedef unsigned char   Fbyte;
typedef short           Fint2;


typedef int          Fint4;
typedef int64_t      Fint8, Flong;

typedef float        Freal4;
typedef double       Freal8, Fdouble;

#ifdef FORTRAN_LOGICAL8
   typedef Fint8     Flogical;
#else
   typedef Fint4     Flogical;
#endif

#ifdef FORTRAN_INTEGER8
   typedef Fint8     Fint;
#else
   typedef Fint4     Fint;
#endif

#ifdef FORTRAN_REAL8
   typedef Freal8    Ffloat;
#else
   typedef Freal4    Ffloat;
#endif

#if REAL_IS_DOUBLE
   typedef double          Freal;
   #define FREAL_TYPENAME  "DOUBLE"
#else
   typedef float           Freal;
   #define FREAL_TYPENAME  "SINGLE"
#endif

#ifdef FORTRAN_REAL12
   typedef long double  Freal12; /* there is no standard!! */
#endif

/*
 * character strings and the hidden stringlength argument
 */
typedef char           *Fstring;
typedef int             Fstrlen;

#if !INCLUDE_STATIC
   extern char   *strscpy_f2c(char   *cStr, size_t  cSiz, const Fstring fStr, Fstrlen fLen);
   extern Fstring strscpy_c2f(Fstring fStr, Fstrlen fLen, const char   *cStr);
#endif

#ifdef __cplusplus
}
#endif

#endif
