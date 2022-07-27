#pragma once
#ifndef stdcasts_HEADER_INCLUDED
#define stdcasts_HEADER_INCLUDED
/* stdcasts.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Defines compiler (C/C++) dependent typecast macros to avoid compiler complaints.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2010/Aug/04: Carsten Dehning, Initial release
 *    $Id: stdcasts.h 5653 2017-10-19 20:17:06Z dehning $
 *
 *****************************************************************************************
 */
#include <limits.h>

#ifdef __cplusplus
   #define SCAST_INTO(_type,_expr)     static_cast<_type>(_expr)
   #define RCAST_INTO(_type,_expr)     reinterpret_cast<_type>(_expr)
   #define CCAST_INTO(_type,_expr)     const_cast<_type>(_expr)
#else
   #define SCAST_INTO(_type,_expr)     (_type)(_expr)
   #define RCAST_INTO(_type,_expr)     (_type)(_expr)
   #define CCAST_INTO(_type,_expr)     (_type)(_expr)
#endif

#define CAST_CHAR(_expr)         SCAST_INTO(char                   ,_expr)
#define CAST_CHARP(_expr)        SCAST_INTO(char *                 ,_expr)
#define CAST_CHARPP(_expr)       SCAST_INTO(char **                ,_expr)
#define CAST_CCHARP(_expr)       SCAST_INTO(const char *           ,_expr)

#define CAST_TCHAR(_expr)        SCAST_INTO(TCHAR                  ,_expr)
#define CAST_TCHARP(_expr)       SCAST_INTO(TCHAR *                ,_expr)
#define CAST_TCHARPP(_expr)      SCAST_INTO(TCHAR **               ,_expr)
#define CAST_CTCHARP(_expr)      SCAST_INTO(const TCHAR *          ,_expr)

#define CAST_UCHAR(_expr)        SCAST_INTO(unsigned char          ,_expr)
#define CAST_UCHARP(_expr)       SCAST_INTO(unsigned char *        ,_expr)
#define CAST_CUCHARP(_expr)      SCAST_INTO(unsigned const char *  ,_expr)

#define CAST_SHORT(_expr)        SCAST_INTO(short                  ,_expr)
#define CAST_SHORTP(_expr)       SCAST_INTO(short *                ,_expr)
#define CAST_CSHORTP(_expr)      SCAST_INTO(const short *          ,_expr)

#define CAST_USHORT(_expr)       SCAST_INTO(unsigned short         ,_expr)
#define CAST_USHORTP(_expr)      SCAST_INTO(unsigned short *       ,_expr)
#define CAST_CUSHORTP(_expr)     SCAST_INTO(const unsigned short * ,_expr)

#define CAST_INT(_expr)          SCAST_INTO(int                    ,_expr)
#define CAST_INTP(_expr)         SCAST_INTO(int *                  ,_expr)
#define CAST_CINTP(_expr)        SCAST_INTO(const int *            ,_expr)

#define CAST_UINT(_expr)         SCAST_INTO(unsigned int           ,_expr)
#define CAST_UINTP(_expr)        SCAST_INTO(unsigned int *         ,_expr)
#define CAST_CUINTP(_expr)       SCAST_INTO(const unsigned int *   ,_expr)

#define CAST_LONG(_expr)         SCAST_INTO(long                   ,_expr)
#define CAST_LONGP(_expr)        SCAST_INTO(long *                 ,_expr)
#define CAST_CLONGP(_expr)       SCAST_INTO(const long *           ,_expr)

#define CAST_ULONG(_expr)        SCAST_INTO(unsigned long          ,_expr)
#define CAST_ULONGP(_expr)       SCAST_INTO(unsigned long *        ,_expr)
#define CAST_CULONGP(_expr)      SCAST_INTO(const unsigned long *  ,_expr)

#define CAST_SIZE(_expr)         SCAST_INTO(size_t                 ,_expr)
#define CAST_SIZEP(_expr)        SCAST_INTO(size_t *               ,_expr)
#define CAST_CSIZEP(_expr)       SCAST_INTO(const size_t *         ,_expr)

#define CAST_INT64(_expr)        SCAST_INTO(int64_t                ,_expr)
#define CAST_INT64P(_expr)       SCAST_INTO(int64_t *              ,_expr)
#define CAST_CINT64P(_expr)      SCAST_INTO(const int64_t *        ,_expr)

#define CAST_UINT32(_expr)       SCAST_INTO(uint32_t               ,_expr)
#define CAST_UINT32P(_expr)      SCAST_INTO(uint32_t *             ,_expr)
#define CAST_CUINT32P(_expr)     SCAST_INTO(const uint32_t *       ,_expr)

#define CAST_UINT64(_expr)       SCAST_INTO(uint64_t               ,_expr)
#define CAST_UINT64P(_expr)      SCAST_INTO(uint64_t *             ,_expr)
#define CAST_CUINT64P(_expr)     SCAST_INTO(const uint64_t *       ,_expr)

#define CAST_FLOAT(_expr)        SCAST_INTO(float                  ,_expr)
#define CAST_FLOATP(_expr)       SCAST_INTO(float *                ,_expr)
#define CAST_CFLOATP(_expr)      SCAST_INTO(const float *          ,_expr)

#define CAST_DOUBLE(_expr)       SCAST_INTO(double                 ,_expr)
#define CAST_DOUBLEP(_expr)      SCAST_INTO(double *               ,_expr)
#define CAST_CDOUBLEP(_expr)     SCAST_INTO(const double *         ,_expr)

#define CAST_REAL(_expr)         SCAST_INTO(real                   ,_expr)
#define CAST_REALP(_expr)        SCAST_INTO(real *                 ,_expr)
#define CAST_REALPP(_expr)       SCAST_INTO(real **                ,_expr)
#define CAST_CREALP(_expr)       SCAST_INTO(const real *           ,_expr)

#define CAST_VOIDP(_expr)        SCAST_INTO(void *                 ,_expr)
#define CAST_CVOIDP(_expr)       SCAST_INTO(const void *           ,_expr)


#define CAST_DOUBLE2FLOAT(_d)\
    (_d < 0)\
      ? ( (_d <-FLT_MAX) ? -FLT_MAX : (_d >-FLT_MIN) ? 0 : CAST_FLOAT(_d) )\
      : ( (_d > FLT_MAX) ?  FLT_MAX : (_d < FLT_MIN) ? 0 : CAST_FLOAT(_d) )

#endif
