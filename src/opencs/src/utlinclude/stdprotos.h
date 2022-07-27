#pragma once
#ifndef stdprotos_HEADER_INCLUDED
#define stdprotos_HEADER_INCLUDED
/* stdprotos.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Define prototypes of all basic function which to not require special handling.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: stdprotos.h 5655 2017-10-20 10:22:27Z dehning $
 *
 *****************************************************************************************
 */

/*****************************************************************************************
 * add missing general purpose functions
 *****************************************************************************************/

#if IS_MSWIN

   #if (IS_MSWIN < 1400) /* fabsf() missing in VC version < 8.0 */
      #define fabsf(_r)     (float)fabs((double)(_r))
   #endif

   #ifndef IS_MINGW
      #if (IS_MSWIN < 1800)
         /* cube root not available under VC but with MINGW */
         #define cbrt(_val)   pow(_val ,(1.0 /3.0 ))
         #define cbrtf(_val)  powf(_val,(1.0f/3.0f))
      #endif
   #endif

   /* [v]snprintf has _ in front under MSWIN */
   #define vsnprintf    _vsnprintf
   #define snprintf     _snprintf

#elif defined(IS_SUNOS) || ( defined(IS_AIX) && !defined(_AIXVERSION_530) )

   /* float versions of sqrt(), acos(), ... are not available */
   #define fabsf(_r)     (float)fabs((double)(_r))
   #define sqrtf(_r)     (float)sqrt((double)(_r))
   #define cbrtf(_r)     (float)cbrt((double)(_r))
   #define acosf(_r)     (float)acos((double)(_r))
   #define sinf(_r)      (float)sin ((double)(_r))
   #define cosf(_r)      (float)cos ((double)(_r))
   #define atan2f(_y,_x) (float)atan2((double)(_y),(double)(_x))
   #define powf(_r,_e)   (float)pow ((double)(_r),(double)(_e))

#elif defined(IS_IRIX65)

   #define cbrtf(_r)     (float)cbrt((double)(_r))

#endif

/*
 * keys used for pattern matching witin strmatch()
 */
#define ESTR_GROUP    340
#define ESTR_ESC      (ESTR_GROUP + 0)
#define ESTR_NOT      (ESTR_GROUP + 1)
#define ESTR_RANGE    (ESTR_GROUP + 2)
#define ESTR_BRACKET  (ESTR_GROUP + 3)
#define ESTR_NOWILD   (ESTR_GROUP + 4)
#define ESTR_NOMEM    (ESTR_GROUP + 5)
#define ESTR_PATTNO   (ESTR_GROUP + 6)


/*
 * keys used by getendianess()
 */
#define ENDIANESS_ASCII    'A' /* ASCII data formated stream/file */
#define ENDIANESS_INTEL    'I' /* little endian (intel/amd/vax)   1234 => 4321 */
#define ENDIANESS_MOTOROLA 'M' /* big endian (motorola)           1234 => 1234 */
#define ENDIANESS_PDP11    'P' /* PDP-11 endianess                1234 => 3412 */


/*
 * tolerance used v3d..s3f _ngalign()
 */
#define NGALIGN_NORM_TOLERANCE    1.0e-4

/*
 * define lots of functions by macros
 */
#define real_is_float()   ( sizeof(real) == sizeof(float)  )
#define real_is_double()  ( sizeof(real) == sizeof(double) )
#define int_is_long()     ( sizeof(int)  == sizeof(long)   )
#define int_is_short()    ( sizeof(int)  == sizeof(short)  )

typedef int (LSEARCHV_COMPARE_t)(const void *v1, const void *v2);
#define strsearchv(_strv,_size,_seek)  lsearchv((const void **)_strv,_size,_seek,(LSEARCHV_COMPARE_t *)STRCMP)
#define strisearchv(_strv,_size,_seek) lsearchv((const void **)_strv,_size,_seek,(LSEARCHV_COMPARE_t *)STRICMP)

#define strtomswin(_str)   strcxchg(_str,'/','\\')
#define strhasmeta(_str)   (strclist(_str,"~()[]{}!$*?`\t\f\r\n|&><#;\"\'") != NULL)

/*
 * copy
 */
#define memcpy_char(_d,_s,_n)                CAST_CHARP  (memcpy(_d,_s, _n               ))
#define memcpy_short(_d,_s,_n)               CAST_SHORTP (memcpy(_d,_s,(_n)*sizeof(short)))
#define memcpy_int(_d,_s,_n)                 CAST_INTP   (memcpy(_d,_s,(_n)*sizeof(int  )))
#define memcpy_long(_d,_s,_n)                CAST_LONGP  (memcpy(_d,_s,(_n)*sizeof(long )))
#if HAVE_LONGLONG
#define memcpy_longlong(_d,_s,_n)            SCAST_INTO(long long *,memcpy(_d,_s,(_n)*sizeof(long long)))
#endif
#define memcpy_float(_d,_s,_n)               CAST_FLOATP (memcpy(_d,_s,(_n)*sizeof(float )))
#define memcpy_real(_d,_s,_n)                CAST_REALP  (memcpy(_d,_s,(_n)*sizeof(real  )))
#define memcpy_double(_d,_s,_n)              CAST_DOUBLEP(memcpy(_d,_s,(_n)*sizeof(double)))
#define memcpy_quad(_d,_s,_n)                SCAST_INTO(quad *,memcpy(_d,_s,(_n)*sizeof(quad)))

/*
 * utilities for conversion of endianess
 */
#define memswapb_short(_p,_n)                memswapb(_p,_n,sizeof(short))
#define memswapb_int(_p,_n)                  memswapb(_p,_n,sizeof(int  ))
#define memswapb_long(_p,_n)                 memswapb(_p,_n,sizeof(long ))
#if HAVE_LONGLONG
#define memswapb_longlong(_p,_n)             memswapb(_p,_n,sizeof(long long))
#endif
#define memswapb_float(_p,_n)                memswapb(_p,_n,sizeof(float))
#define memswapb_real(_p,_n)                 memswapb(_p,_n,sizeof(real))
#define memswapb_double(_p,_n)               memswapb(_p,_n,sizeof(double))
#define memswapb_quad(_p,_n)                 memswapb(_p,_n,sizeof(quad))


/*
 * utilities for rotating arrays
 */
#define memrot_short(_p,_n,_r)               memrot(_p,_n,sizeof(short),_r)
#define memrot_int(_p,_n,_r)                 memrot(_p,_n,sizeof(int  ),_r)
#define memrot_long(_p,_n,_r)                memrot(_p,_n,sizeof(long ),_r)
#if HAVE_LONGLONG
#define memrot_longlong(_p,_n,_r)            memrot(_p,_n,sizeof(long long),_r)
#endif
#define memrot_float(_p,_n,_r)               memrot(_p,_n,sizeof(float ),_r)
#define memrot_real(_p,_n,_r)                memrot(_p,_n,sizeof(real  ),_r)
#define memrot_double(_p,_n,_r)              memrot(_p,_n,sizeof(double),_r)
#define memrot_quad(_p,_n,_r)                memrot(_p,_n,sizeof(quad  ),_r)

#if REAL_IS_DOUBLE

   #define move_real2double   move_double2double
   #define move_real2float    move_double2float
   #define move_double2real   move_double2double
   #define move_float2real    move_float2double

   #define vsum_real          vsum_double
   #define vvsum_real         vvsum_double

   #define vsumabs_real       vsumabs_double
   #define vvsumabs_real      vvsumabs_double

   #define vscale_real        vscale_double
   #define vvscale_real       vvscale_double
   #define vvshift_real       vvshift_double

   #define vlintr_real        vlintr_double

   #define nancheck_real      nancheck_double

   #define memset_real        memset_double
   #define memsum2p_real      memsum2p_double

   #define maxabs_real        maxabs_double

   #define esolve2_real       esolve2_double
   #define esolve3_real       esolve3_double

   #define str2real           str2double

   #define s3r_angle          s3d_angle
   #define v3r_angle          v3d_angle

   #define s3r_anglew         s3d_anglew
   #define v3r_anglew         v3d_anglew

   #define s3r_sangle         s3d_sangle
   #define v3r_sangle         v3d_sangle

   #define s3r_length         s3d_length
   #define v3r_length         v3d_length

   #define s3r_norm           s3d_norm
   #define v3r_norm           v3d_norm

   #define s3r_ngalign        s3d_ngalign
   #define v3r_ngalign        v3d_ngalign

   #define s3r_spat           s3d_spat
   #define s3r_cross          s3d_cross

   #define pownr              pownd

#else

   #define move_real2double   move_float2double
   #define move_real2float    move_float2float
   #define move_double2real   move_double2float
   #define move_float2real    move_float2float

   #define vsum_real          vsum_float
   #define vvsum_real         vvsum_float

   #define vsumabs_real       vsumabs_float
   #define vvsumabs_real      vvsumabs_float

   #define vscale_real        vscale_float
   #define vvscale_real       vvscale_float
   #define vvshift_real       vvshift_float

   #define vlintr_real        vlintr_float

   #define nancheck_real      nancheck_float

   #define memset_real        memset_float
   #define memsum2p_real      memsum2p_float

   #define maxabs_real        maxabs_float

   #define esolve2_real       esolve2_float
   #define esolve3_real       esolve3_float

   #define str2real           str2float

   #define s3r_angle          s3f_angle
   #define v3r_angle          v3f_angle

   #define s3r_anglew         s3f_anglew
   #define v3r_anglew         v3f_anglew

   #define s3r_sangle         s3f_sangle
   #define v3r_sangle         v3f_sangle

   #define s3r_length         s3f_length
   #define v3r_length         v3f_length

   #define s3r_norm           s3f_norm
   #define v3r_norm           v3f_norm

   #define s3r_ngalign        s3f_ngalign
   #define v3r_ngalign        v3f_ngalign

   #define s3r_spat           s3f_spat
   #define s3r_cross          s3f_cross

   #define pownr              pownf

#endif


#if !INCLUDE_STATIC

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * strerror() wrapper for MSWin with GetLastError()
 */
#if IS_MSWIN
   extern const char  *WSAStrerror(void);
   extern const TCHAR *strxerror(int errnum);
#else
   extern const char  *strxerror(int errnum);
#endif

/*
 * little/big endian swapping
 */
extern int     getendianess      (void);
extern void   *memswapb          (void   *buf, size_t n, size_t size);
extern void   *memswapv          (void   *buf, size_t size);

/*
 * rotate/reverse memory
 */
extern void   *memrot            (void   *buf, const size_t nobj, const size_t szobj, const int nrot);
extern int    *memrev_int        (int    *buf, size_t n);
extern void  **memrev_vptr       (void  **buf, size_t n);


/*
 * move and convert.c
 */
extern double *move_float2double (double *dst, const float  *src, size_t n);
extern float  *move_double2float (float  *dst, const double *src, size_t n);
extern long   *move_int2long     (long   *dst, const int    *src, size_t n);
extern int    *move_long2int     (int    *dst, const long   *src, size_t n);


extern double *conv_float2double (void *buf, const size_t n);
extern float  *conv_double2float (void *buf, const size_t n);

extern int64_t *conv_int2long    (int     *buf, const size_t n);
extern int     *conv_long2int    (int64_t *buf, const size_t n);

/*
 * pointer <-> integer mapping
 */
extern unsigned long  vptr2ulong (const void *ptr);
extern void          *ulong2vptr (unsigned long ulong);

/*
 * set, count and index functions
 */

extern int    *memset_int        (      int    *buf, const size_t n, const int    set);
extern long   *memset_long       (      long   *buf, const size_t n, const long   set);
extern float  *memset_float      (      float  *buf, const size_t n, const float  set);
extern double *memset_double     (      double *buf, const size_t n, const double set);

extern int     memcount_char     (const void   *buf, const int    n, const int  seek);
extern int     memcount_int      (const int    *buf, const int    n, const int  seek);
extern int     memcount_long     (const long   *buf, const int    n, const long seek);

extern int     memindex_size     (const void   *buf, const size_t n, const size_t size, const void *seek);
extern int     memindex_char     (const void   *buf, const size_t n, const int   seek);
extern int     memindex_int      (const int    *buf, const size_t n, const int   seek);
extern int     memindex_long     (const long   *buf, const size_t n, const long  seek);
extern int     memindex_vptr     (const void  **buf, const size_t n, const void *seek);

extern int     memshare_int      (const int *ai, const int ni, const int *aj, const int nj);

extern long    vsum_int          (const int    *buf, const int n);
extern long    vsum_long         (const long   *buf, const int n);
extern double  vsum_float        (const float  *buf, const int n);
extern double  vsum_double       (const double *buf, const int n);

extern double  vvsum_float       (const float  *buf, const int n, double *vsum, const int vdim);
extern double  vvsum_double      (const double *buf, const int n, double *vsum, const int vdim);

extern float  *vscale_float      (      float  *buf, const int n, const float  scale);
extern double *vscale_double     (      double *buf, const int n, const double scale);

extern float  *vvscale_float     (      float  *buf, const int n, const float  *scale, const int vdim);
extern double *vvscale_double    (      double *buf, const int n, const double *scale, const int vdim);

extern float  *vlintr_float      (      float  *buf, const int n, const float  scale, const float  offs);
extern double *vlintr_double     (      double *buf, const int n, const double scale, const double offs);

extern int     nancheck_float    (const float  *buf, size_t n);
extern int     nancheck_double   (const double *buf, size_t n);

extern double  memsum2p_float    (const float  *buf1, const float  *buf2, size_t n);
extern double  memsum2p_double   (const double *buf1, const double *buf2, size_t n);

extern int     strcountchar      (const TCHAR *str,       TCHAR seek);
extern TCHAR  *strclist          (const TCHAR *str, const TCHAR *clist);
extern TCHAR  *striclist         (const TCHAR *str, const TCHAR *clist);

/*
 * string to number functions with error return
 */
extern size_t  numlen            (long value, int radix);
extern int     str2bool          (const TCHAR *str, int      *pbool);
extern int     str2ushort        (const TCHAR *str, int      *pint);
extern int     str2int           (const TCHAR *str, int      *pint);
extern int     str2uint          (const TCHAR *str, unsigned *pint);
extern int     str2long          (const TCHAR *str, long     *plong);
extern int     str2double        (const TCHAR *str, double   *pdouble);
extern int     str2float         (const TCHAR *str, float    *pfloat);
extern int     str2frac          (const TCHAR *str, long *pnom, long *pden);
extern size_t  strslen           (      TCHAR *str, size_t size);
extern char   *istr2cstr         (      int *istr);
extern TCHAR  *strscpy           (TCHAR *dst, size_t size, const TCHAR *src);
extern TCHAR  *strsjoinl         (TCHAR *dst, size_t size, const TCHAR *sep, const TCHAR*src, ...);
extern TCHAR  *strscat           (TCHAR *dst, size_t size, const TCHAR *src);
extern TCHAR  *strscatl          (TCHAR *dst, size_t size, const TCHAR *src, ...);

/*
 * number to dotted string functions
 */
extern const TCHAR *ulong2adp    (const uint64_t uval, TCHAR *s, const size_t count);
extern const TCHAR *ulong2adm    (const uint64_t uval, TCHAR *s, const size_t count);
extern const TCHAR *ulong2bin    (const uint64_t uval, const unsigned minbits, TCHAR *s, const size_t count);
extern char        *itoa10       (int ival, char    *str);
extern wchar_t     *itow10       (int ival, wchar_t *str);

/*
 * string shift operation
 */
extern char   *strlshift         (char *str, size_t n);
extern char   *strrshift         (char *str, size_t n);

/*
 * case independent extensions
 */
extern TCHAR   *strichr           (const TCHAR *str, int c);
extern char    *stristr_a         (const char  *str, const char  *seek);
#if IS_MSWIN
extern wchar_t *stristr_w         (const wchar_t *str, const wchar_t *seek);
#endif

/*
 * string cleanup
 */
#if !IS_MSWIN
extern TCHAR  *strupper          (TCHAR *str);
extern TCHAR  *strlower          (TCHAR *str);
#endif

extern char   *strtograph        (char  *str);
extern char   *strtoword         (char  *str);
extern TCHAR  *strtoprop         (TCHAR *str);
extern TCHAR  *strunquote        (TCHAR *str, TCHAR **pEnd);
extern TCHAR  *strcxchg          (TCHAR *str, const TCHAR seek, const TCHAR replace);
extern char   *strtoshell        (char  *str);
extern TCHAR  *strbtrim          (TCHAR *str);

/*
 * convert a single char into a "c\0" two character string
 */
extern const TCHAR *chartostr     (const int c);


/*
 * split string into tokens -> tokv
 */
extern int     strsplit          (TCHAR *str, size_t maxtok, const TCHAR *tokv[],
                                  const TCHAR *separators, const unsigned flags);

/*
 * linear search in a vector of pointers
 */
extern int     lsearchv          (const void **objv, int size, const void *seek,
                                  int (*compare)(const void *obje, const void *seek));


#if 0
extern char   *strcent           (const char *s, size_t n);
extern char   *strexp            (      char *s, size_t n, const char *si);
extern char   *strfixk           (      char *s);
extern char   *strfixs           (      char *s, const char *delmtr);
extern char   *strins            (      char *s, const char *si);
extern int     strmatch          (const char *s, unsigned pattno);
extern char   *strnotchr         (const char *s, int c);
extern char   *strnotgraph       (const char *s);
extern int     strpattern        (const char *s, unsigned pattno);
extern char   *strspace          (const char *s);
#endif

/*
 * filename functions
 */
extern TCHAR  *gethomedir        (void);
extern TCHAR  *getworkdir        (void);
extern TCHAR  *getpathname       (const TCHAR *pathname);
extern TCHAR  *getbasename       (const TCHAR *pathname);
extern TCHAR  *getextname        (const TCHAR *pathname);
extern TCHAR  *mkdirname         (      TCHAR *pathname);
extern TCHAR  *pathrepair        (      TCHAR *pathname);
extern TCHAR  *scanpath          (const TCHAR *exename, const TCHAR *sfx, TCHAR *exepath, size_t exesize);


/*
 * mkdir a complete directory tree and not just the last subdirectory
 */
extern int     mkdirectory       (TCHAR *pathname, unsigned mode);

/*
 * file utility functions
 */
#if IS_MSWIN
extern int     fisreadable       (const TCHAR *pathname);  /* is a macro under UNIX */
#endif
extern int     fisdevice         (const TCHAR *filename);
extern int     fisdirectory      (const TCHAR *pathname);
extern FILE   *folines           (const char  *pathname, long *pnlines);

extern int     fxdelete          (const TCHAR *pathname, const TCHAR *extension);
extern time_t  fxmodtime         (const TCHAR *pathname, const TCHAR *extension);
extern size_t  flinecount        (FILE *fp);
extern int     fnextchar         (FILE *fp, const TCHAR *comments);
extern int     fnextline         (FILE *fp);
extern TCHAR  *fxgets            (TCHAR *xline, const size_t size, FILE *fp, const TCHAR *comments);
extern void    fprintf_intv      (FILE *fp, const int *ibuf, int n, const char *trailerFmt, ...);
extern int     fscanf_intv       (FILE *fp,       int *ibuf, int n);

extern void    fprintf_dblv      (FILE *fp, const double *dbuf, int n, const char *trailerFmt, ...);

/*
 * use files as semaphores
 */
extern int     fwaitdeleted      (const TCHAR *pathname, int msec);
extern int     fwaitexists       (const TCHAR *pathname, int msec);


/*
 * platform wrapper functions
 */
extern       TCHAR  *getusername   (void);
extern const char   *getdatestring (void);
extern double        getcpuseconds (void);
extern uint64_t      getmstime     (void);
extern size_t        getramusage   (void);
#if IS_MSWIN

   #ifndef IS_MINGW
      struct timezone
      {
         int tz_minuteswest; /* minutes W of Greenwich */
         int tz_dsttime;     /* type of dst correction */
      };
   #endif

   extern int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif

/*
 * vector utilities
 */
extern double  s3d_angle   (const double ux, const double uy, const double uz, const double vx, const double vy, const double vz);
extern float   s3f_angle   (const float  ux, const float  uy, const float  uz, const float  vx, const float  vy, const float  vz);
extern double  v3d_angle   (const double u[3], const double v[3]);
extern float   v3f_angle   (const float  u[3], const float  v[3]);

extern double  s3d_anglew  (const double ux, const double uy, const double uz, const double vx, const double vy, const double vz);
extern float   s3f_anglew  (const float  ux, const float  uy, const float  uz, const float  vx, const float  vy, const float  vz);
extern double  v3d_anglew  (const double u[3], const double v[3]);
extern float   v3f_anglew  (const float  u[3], const float  v[3]);

extern double  s3d_sangle  (const double ux, const double uy, const double uz, const double vx, const double vy, const double vz);
extern float   s3f_sangle  (const float  ux, const float  uy, const float  uz, const float  vx, const float  vy, const float  vz);
extern double  v3d_sangle  (const double u[3], const double v[3]);
extern float   v3f_sangle  (const float  u[3], const float  v[3]);

extern double  s3d_length  (const double x, const double y, const double z);
extern float   s3f_length  (const float  x, const float  y, const float  z);
extern double  v3d_length  (const double v[3]);
extern float   v3f_length  (const float  v[3]);

extern int     s3d_norm    (double *x, double *y, double *z);
extern int     s3f_norm    (float  *x, float  *y, float  *z);
extern int     v3d_norm    (double v[3]);
extern int     v3f_norm    (float  v[3]);

extern int     s3d_ngalign (const double *nx, const double *ny, const double *nz);
extern int     s3f_ngalign (const float  *nx, const float  *ny, const float  *nz);
extern int     v3d_ngalign (const double nv[3]);
extern int     v3f_ngalign (const float  nv[3]);


extern real    s3r_cosine  (real ux, real uy, real uz, real vx, real vy, real vz);
extern real    s3r_cosinew (real ux, real uy, real uz, real vx, real vy, real vz);

extern void    s3d_cross   (double ux, double uy, double uz, double vx, double vy, double vz, double *xr, double *yr, double *zr);
extern void    s3f_cross   (float  ux, float  uy, float  uz, float  vx, float  vy, float  vz, float  *xr, float  *yr, float  *zr);

extern double  s3d_spat    (const double ux, const double uy, const double uz, const double vx, const double vy, const double vz, const double wx, const double wy, const double wz);
extern float   s3f_spat    (const float  ux, const float  uy, const float  uz, const float  vx, const float  vy, const float  vz, const float  wx, const float  wy, const float  wz);

#if !IS_MSWIN
extern void       millisleep     (long msec);
#endif

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_STATIC */

#endif
