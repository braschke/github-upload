#pragma once
#ifndef stdmacros_HEADER_INCLUDED
#define stdmacros_HEADER_INCLUDED
/* stdmacros.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Define lots of wrapper macros and extensions.
 *    Wrap standard c library functions: e.g. MSWIN requires _tchdir.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: stdmacros.h 4913 2016-07-12 12:38:37Z dehning $
 *
 *****************************************************************************************
 */

/*
 * stuff below is system dependent
 */
#define _C_WINDIRSEP       '\\'
#define _C_UNXDIRSEP       '/'
#define _C_WINLISTSEP      ';'
#define _C_UNXLISTSEP      ':'

/*
 * stuff below depends on the locale settings - not simple mapping
 */
#if IS_MSWIN

   #define TOUPPER(_c)        _totupper (CAST_TCHAR(_c))
   #define TOLOWER(_c)        _totlower (CAST_TCHAR(_c))

   #define ISLOWER(_c)        _istlower (CAST_TCHAR(_c))
   #define ISUPPER(_c)        _istupper (CAST_TCHAR(_c))

   #define ISALPHA(_c)        _istalpha (CAST_TCHAR(_c)) /* ISOFTYPE(c,"abcde..ABCDE..") */
   #define ISSPACE(_c)        _istspace (CAST_TCHAR(_c)) /* ISOFTYPE(c," \t\n\v\f\r") */
   #define ISPUNCT(_c)        _istpunct (CAST_TCHAR(_c)) /* ISOFTYPE(c,".:;!?") */
   #define ISGRAPH(_c)        _istgraph (CAST_TCHAR(_c))
   #define ISALNUM(_c)        _istalnum (CAST_TCHAR(_c))
   #define ISXDIGIT(_c)       _istxdigit(CAST_TCHAR(_c))   /* ISOFTYPE(c,"0123456789abcdefABCDEF") */
   #define ISPRINT(_c)        _istprint (CAST_TCHAR(_c))

#else

   #define TOUPPER(_c)        toupper (CAST_TCHAR(_c))
   #define TOLOWER(_c)        tolower (CAST_TCHAR(_c))

   #define ISLOWER(_c)        islower (CAST_TCHAR(_c))
   #define ISUPPER(_c)        isupper (CAST_TCHAR(_c))

   #define ISALPHA(_c)        isalpha (CAST_TCHAR(_c)) /* ISOFTYPE(c,"abcde..ABCDE..") */
   #define ISSPACE(_c)        isspace (CAST_TCHAR(_c)) /* ISOFTYPE(c," \t\n\v\f\r") */
   #define ISPUNCT(_c)        ispunct (CAST_TCHAR(_c)) /* ISOFTYPE(c,".:;!?") */
   #define ISGRAPH(_c)        isgraph (CAST_TCHAR(_c))
   #define ISALNUM(_c)        isalnum (CAST_TCHAR(_c))
   #define ISXDIGIT(_c)       isxdigit(CAST_TCHAR(_c))   /* ISOFTYPE(c,"0123456789abcdefABCDEF") */
   #define ISPRINT(_c)        isprint (CAST_TCHAR(_c))

#endif


/*
 * isxxxx based on a list of chars specified
 */
#define ISOFTYPE(_c,_list) ( STRCHR(_list,_c) != NULL )

/*
 * bit patterns used by isshmeta.c - is shell meta character
 */
enum
{
   ISSHMETA_NONE  = 0x00,
   ISSHMETA_ANY   = 0x01,
   ISSHMETA_TERM  = 0x02,
   ISSHMETA_QUOTE = 0x04,
   ISSHMETA_WILD  = 0x08,
   ISSHMETA_COMT  = 0x0f,
   ISSHMETA_PAREN = 0x1f
};

#define ISSHMETA(_c)       isshmeta(_c)   /* ISOFTYPE(c,"#|<>$~!?*&;()[]{}\"\'\`\´\\") */

/*
 * locale not required
 */
#define ISDIGIT(_c)        ( (_c)>='0' && (_c)<='9'  )
#define ISBLANK(_c)        ( (_c)==' ' || (_c)=='\t' )
#define ISDOT(_c)          ( (_c)=='.' )

/*
 * US alpha charset only without locale
 */
#define ISAZLOWER(_c)      ( (_c)>='a' && (_c)<='z' )
#define ISAZUPPER(_c)      ( (_c)>='A' && (_c)<='Z' )
#define ISAZALPHA(_c)      ( ISAZLOWER(_c)||ISAZUPPER(_c) )
#define ISWORD(_c)         ( ISAZALPHA(_c)||ISDIGIT(_c)||((_c)=='_') )

/*
 * pathname separators
 */
#define ISWINDIRSEP(_c)    ( (_c)==_C_WINDIRSEP )
#define ISUNXDIRSEP(_c)    ( (_c)==_C_UNXDIRSEP )
#define ISANYDIRSEP(_c)    ( ISWINDIRSEP(_c)||ISUNXDIRSEP(_c) )

/*
 * list ($PATH,%INCLUDE%,etc) separators
 */
#define ISWINLISTSEP(_c)   ( (_c)==_C_WINLISTSEP )
#define ISUNXLISTSEP(_c)   ( (_c)==_C_UNXLISTSEP )

/*
 * single and double quotes
 */
#define ISQUOTE(_c)        ( (_c)=='"' || (_c)=='\''||(_c)=='`')

/*
 * is pathname an UNC pathname: minimum string is "//x/y"
 */
#define PATH_ISWINUNC(_p)   ( (ISWINDIRSEP((_p)[0]) && ((_p)[1]==(_p)[0])) ? STRCHR((_p)+2,(_p)[0]) : NULL )
#define PATH_ISUNXUNC(_p)   ( (ISUNXDIRSEP((_p)[0]) && ((_p)[1]==(_p)[0])) ? STRCHR((_p)+2,(_p)[0]) : NULL )
#define PATH_ISANYUNC(_p)   ( (ISANYDIRSEP((_p)[0]) && ((_p)[1]==(_p)[0])) ? STRCHR((_p)+2,(_p)[0]) : NULL )


#if IS_MSWIN

   /*
    * specials for MSWin only
    */
   #define REGKEY_MAXLEN      4096 /* max. length of a registry key name */
   #define REGVAL_MAXLEN      4096 /* max. length of a registry value string */
   #define MSWIN_CMDLEN_MAX   8191 /* max. length of a CMD.EXE commandline under MSWin */

   /*
    * time offset between 1/1/1601 (the MSWin FILETIME 0) and 1/1/1970 in 100 nanosec units
    * (epoch bias) used to translate MSWin FILETIME into UNIX epoch time
    */
   #ifdef IS_MINGW
      /* MinGW gcc has no i64 modifier */
      #define FT_EPOCH_BIAS   116444736000000000L
      #define FT_SCALE_SEC              10000000L
      #define FT_SCALE_MSEC                10000L
      #define FT_SCALE_USEC                   10L
   #else
      #define FT_EPOCH_BIAS   116444736000000000i64
      #define FT_SCALE_SEC              10000000i64
      #define FT_SCALE_MSEC                10000i64
      #define FT_SCALE_USEC                   10i64
   #endif

   /*
    * Convert a FILETIME structure (as UINT64) into UNIX UTC time_t
    */
   #define FILETIMETOTIME_T(_fileTime)\
      ( (time_t)(((_fileTime)-FT_EPOCH_BIAS) / FT_SCALE_SEC) )


   /*
    * ctype.h extensions
    */
   #define _C_DIRSEP             _C_WINDIRSEP
   #define _C_LISTSEP            _C_WINLISTSEP

   #define ISDIRSEP              ISANYDIRSEP
   #define ISLISTSEP             ISWINLISTSEP

   #define PATH_ISUNC            PATH_ISANYUNC
   #define PATH_ISDRIVE(_p)      ( ISAZALPHA((_p)[0]) && (_p)[1]==':' && (ISDIRSEP((_p)[2])  || (_p)[2]==0) )
   #define PATH_ISABS(_p)        ((ISAZALPHA((_p)[0]) && (_p)[1]==':' &&  ISDIRSEP((_p)[2])) || ( ISANYDIRSEP((_p)[0]) && ((_p)[1]==(_p)[0]) ))
   #define PATH_ISREL(_p)        !PATH_ISABS(_p)


   /*
    * string.h
    */
   #define STRCPY                _tcscpy
   #define STRNCPY               _tcsncpy
   #define STRCAT                _tcscat
   #define STRNCAT               _tcsncat

   #define STRLEN                _tcslen

   #define STRLOWER              _tcslwr
   #define STRUPPER              _tcsupr

   #define STRCHR                _tcschr
   #define STRRCHR               _tcsrchr

   #define STRSTR                _tcsstr

   #define STRCMP                _tcscmp
   #define STRICMP               _tcsicmp
   #define STRICMP_A             _stricmp

   #define STRNCMP               _tcsncmp
   #define STRNICMP              _tcsnicmp
   #define STRNICMP_A            _strnicmp

   #define MEMICMP               _memicmp

   #define STRTOD                _tcstod

   /*
    * stdio.h
    */
   #define SSCANF                _stscanf
   #define FPRINTF               _ftprintf
   #define FPUTS                 _fputts
   #define SNPRINTF              _sntprintf

   #define ISNAN                 _isnan

   /*
    * unistd.h
    */
   #ifndef INVALID_FILE_ATTRIBUTES
      /* first defined with VC 7.0, _MCS_VER >= 1300) */
      #define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
   #endif

   #define GETCWD                _tgetcwd
   #define CHDIR                 _tchdir
   #define OPEN                  _topen
   #define CLOSE                 _close
   #define FEXISTS(_path)        (GetFileAttributes(_path) != INVALID_FILE_ATTRIBUTES)
   #define FTOUCH(_path)         CloseHandle(CreateFile(_path,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_FLAG_NO_BUFFERING,NULL))
   #define FISWRITEABLE(_path)   (GetFileAttributes(_path) != INVALID_FILE_ATTRIBUTES)
   #define FISREADABLE           fisreadable /* c code */
   #define MILLISLEEP            Sleep
/* #define FOPEN                 _tfopen */
   #define POPEN                 _tpopen
   #define PCLOSE                _pclose
   #define _STD_C_GETENV         _tgetenv
   #define _STD_C_PUTENV         _tputenv


   /*
    * code piece to handle interrupted system calls and retry in case of interrupts
    */
   #define SYSCALL_DO_AGAIN               (\
                                             errno==EINTR || \
                                             errno==EAGAIN   \
                                          )

   /*
    * A valid HANDLE must neither be 0 nor INVALID_HANDLE_VALUE
    */
   #define IS_VALID_HANDLE(_h)            (  _h && _h != INVALID_HANDLE_VALUE )
   #define IS_INVALID_HANDLE(_h)          ( !_h || _h==INVALID_HANDLE_VALUE )

#else /* UNIX, what else */

   /*
    * MAX_PATH is MSWin specific and internally we use MAX_PATH, since
    * there are lots of #define's for BSD, POSIX, LINUX etc. under Unix
    */
   #if !defined(MAX_PATH)
      #if defined(PATH_MAX)
         #define MAX_PATH  PATH_MAX

      #elif defined(MAXPATHLEN)
         #define MAX_PATH  MAXPATHLEN

      #elif defined(_MAX_PATH)
         #define MAX_PATH  _MAX_PATH

      #else
         #define MAX_PATH  2048

      #endif
   #endif

   /*
    * ctype.h extensions
    */
   #define _C_DIRSEP             _C_UNXDIRSEP
   #define _C_LISTSEP            _C_UNXLISTSEP

   #define ISDIRSEP              ISUNXDIRSEP
   #define ISLISTSEP             ISUNXLISTSEP

   #define PATH_ISUNC            PATH_ISUNXUNC
   #define PATH_ISDRIVE(_p)      ((_p)[0]==_C_UNXDIRSEP)
   #define PATH_ISABS(_p)        ((_p)[0]==_C_UNXDIRSEP)
   #define PATH_ISREL(_p)        ((_p)[0] != _C_UNXDIRSEP)


   /*
    * string.h
    */
   #define STRCPY                strcpy
   #define STRNCPY               strncpy
   #define STRCAT                strcat
   #define STRNCAT               strncat

   #define STRLEN                strlen

   #define STRLOWER              strlower
   #define STRUPPER              strupper

   #define STRCHR                strchr
   #define STRRCHR               strrchr

   #define STRSTR                strstr

   #define STRCMP                strcmp
   #define STRICMP               strcasecmp
   #define STRICMP_A             strcasecmp

   #define STRNCMP               strncmp
   #define STRNICMP              strncasecmp
   #define STRNICMP_A            strncasecmp

   #define MEMICMP               memcasecmp

   #define STRTOD                strtod

   /*
    * stdio.h
    */
   #define SSCANF                sscanf
   #define FPRINTF               fprintf
   #define FPUTS                 fputs
   #define SNPRINTF              snprintf

   #define ISNAN                 isnan

   /*
    * unistd.h
    */
   #define GETCWD                getcwd
   #define CHDIR                 chdir
   #define OPEN                  open
   #define CLOSE                 close
   #define FEXISTS(_path)        (access(_path,F_OK)==0)
   #define FTOUCH(_path)         close(creat(_path,0666))
   #define FISWRITEABLE(_path)   (access(_path,W_OK)==0)
   #define FISREADABLE(_path)    (access(_path,R_OK)==0)
   #define MILLISLEEP            millisleep /* c code */
/* #define FOPEN                 fopen    Macro conflicts under AIX */
   #define POPEN                 popen
   #define PCLOSE                pclose
   #define _STD_C_GETENV         getenv
   #define _STD_C_PUTENV         putenv


   /*
    * code piece to handle interrupted system calls and retry in case of interrupts
    */
   #define SYSCALL_DO_AGAIN               (\
                                             errno==EINTR       || \
                                             errno==EAGAIN      || \
                                             errno==EWOULDBLOCK    \
                                          )


   #define INVALID_HANDLE_VALUE           -1
   #define IS_VALID_HANDLE(_h)            (  _h>=0 )
   #define IS_INVALID_HANDLE(_h)          (  _h <  0 )

#endif


#define MEMZERO(_p,_n)        memset(_p,0,_n)

/*
 * do no use sleep(1) under Unix because of the alarm() - implementation
 */
#define SLEEPDOUBLE(_dsec)    MILLISLEEP((unsigned long)(1000.0*(_dsec)))
#define SLEEPSECOND(_ssec)    MILLISLEEP(1000*(_ssec))


#define plurals(_n)           ( ((_n)==1) ? TEXT("") : TEXT("s") )
#define plures(_n,_s)         (_n), ( ((_n)==1) ? _s : _s TEXT("s") )
#define countof(_arr)         ( sizeof(_arr)/sizeof(_arr[0]) ) /* no. of elements in an array */
#define ucountof(_arr)        (CAST_UINT(countof(_arr)))       /* used to have 32/64 bit independent size */
#define icountof(_arr)        (CAST_INT(countof(_arr)))        /* used to have 32/64 bit independent size */
#define lastof(_arr)          (countof(_arr)-1)                /* index of the last array element */

#define usizeof(_type)        (CAST_UINT(sizeof(_type)))       /* used to have 32/64 bit independent size */
#define isizeof(_type)        (CAST_INT(sizeof(_type)))        /* used to have 32/64 bit independent size */

/*
 * string macros extending string.h
 */

/*
 * size_t strlen() wrappers with check to string pointer
 */
#define STRHASLEN(_str)         ( (_str) && (_str)[0] )

#define ISTRLEN(_str)           CAST_INT(STRLEN(_str))

#define  STRLENP(_str)          ( (_str) ?  STRLEN(_str) : 0  )
#define ISTRLENP(_str)          ( (_str) ? ISTRLEN(_str) : 0  )


/*
 * get pointer to the last char in a string
 */
#define STRLASTCP(_pstr)         (_pstr+STRLEN(_pstr)-1)

/*
 * eat leading whitespace of a stringpointer
 */
#define STRJUMPNOSPACE(_pstr)          while(ISSPACE(*_pstr)) _pstr++
#define STRRJUMPNOSPACE(_head,_tail)   while(_tail>_head && ISSPACE(*_tail)) _tail--

#define STRJUMPNOBLANK(_pstr)          while(ISBLANK(*_pstr)) _pstr++
#define STRRJUMPNOBLANK(_head,_tail)   while(_tail>_head && ISBLANK(*_tail)) _tail--

/*
 * jump to the next space character
 */
#define STRJUMPSPACE(_pstr)            while(*_pstr && !ISSPACE(*_pstr)) _pstr++
#define STRJUMPBLANK(_pstr)            while(*_pstr && !ISBLANK(*_pstr)) _pstr++

/*
 * eat trailing whitespace of a stringpointer
 */
#define STRENDNOSPACE(_pstr,_pend) \
   for(_pend=STRLASTCP(_pstr); _pend>=_pstr && ISSPACE(*_pend); _pend--) *_pend = 0

/*
 * advance a string pointer until we reach a seek character
 */
#define STRJUMPCHAR(_pstr,_seek)          while(*_pstr && *_pstr != _seek) _pstr++
#define STRJUMPCHARCP(_pstr,_pend,_seek)  for(_pend=_pstr; *_pend && *_pend != _seek; _pend++)


/*
 * code pieces to handle interrupted system calls and retry in case of interrupts
 */
#define ATOMAR_SYSCALL_LOOP \
{\
   if ( !SYSCALL_DO_AGAIN ) break;\
   MILLISLEEP(1);\
}


/*
 * mathematical stuff used everywhere
 */
#ifndef M_SQRT3
   #define M_SQRT3      1.73205080756887729352 /* 1.73205080756887729352744634150587236694280525381038062805580 */
   #define M_SQRT1_3    (1.0/M_SQRT3)
#endif

#ifndef M_SQRT5
   #define M_SQRT5      2.23606797749978969641 /* 2.23606797749978969640917366873127623544061835961152572427089 */
   #define M_SQRT1_5    (1.0/M_SQRT5)
#endif

#ifndef M_SQRT6
   #define M_SQRT6      2.44948974278317809819 /* 2.44948974278317809819728407470589139196594748065667012843269 */
   #define M_SQRT1_6    (1.0/M_SQRT6)
#endif

#ifndef M_SQRT10
   #define M_SQRT10     3.16227766016837933199 /* 3.16227766016837933199889354443271853371955513932521682685750 */
   #define M_SQRT1_10   (1.0/M_SQRT10)
#endif

#define MAXABS(_A,_B)     (  fabsr(_A) > fabsr(_B) ? fabsr(_A) : fabsr(_B)  )

/*
 * angles degree <-> radiant conversion
 */
#define DEG2RAD(_deg_)        ( (_deg_)*(M_PI/180.0) )
#define RAD2DEG(_rad_)        ( (_rad_)*(180.0/M_PI) )


/*
 * some 2D scalar component operations
 */
#define S2_SPROD(_x1,_y1, _x2,_y2)              ( (_x1)*(_x2) + (_y1)*(_y2) )
#define S2_LENGTH2(_x,_y)                       S2_SPROD(_x,_y,_x,_y)
#define S2_LENGTHF(_x,_y)                       sqrtf(S2_LENGTH2(_x,_y))
#define S2_LENGTHD(_x,_y)                       sqrt (S2_LENGTH2(_x,_y))
#define S2_LENGTHR(_x,_y)                       sqrtr(S2_LENGTH2(_x,_y))
#define S2_ZERO(_x,_y)                          { _x = 0; _y = 0; }
#define S2_NEG(_x,_y)                           { _x = -_x; _y = -_y; }

#define S2_SUB(_x1,_y1, _x2,_y2, _xr,_yr) \
{\
   _xr = (_x1) - (_x2);\
   _yr = (_y1) - (_y2);\
}

/*
 * some 2D vector[2] operations
 */
#define V2_SPROD(_v,_u)       S2_SPROD  (_v[0],_v[1], _u[0],_u[1])
#define V2_LENGTH2(_v)        S2_LENGTH2(_v[0],_v[1])
#define V2_LENGTHF(_v)        S2_LENGTHF(_v[0],_v[1])
#define V2_LENGTHD(_v)        S2_LENGTHD(_v[0],_v[1])
#define V2_LENGTHR(_v)        S2_LENGTHR(_v[0],_v[1])
#define V2_ZERO(_v)           S2_ZERO   (_v[0],_v[1])
#define V2_NEG(_v)            S2_NEG    (_v[0],_v[1])
#define V2_SUB(_v,_u,_r)      S2_SUB    (_v[0],_v[1], _u[0],_u[1], _r[0],_r[1])

#define V2_VOP(_left, _op, _right)\
{\
   _left[0] _op _right[0];\
   _left[1] _op _right[1];\
}

#define V2_SOP(_left, _op, _scal)\
{\
   _left[0] _op _scal;\
   _left[1] _op _scal;\
}

#define V2_VVOP(_left, _op1, _varg1, _op2, _varg2)\
{\
   _left[0] _op1 _varg1[0] _op2 _varg2[0];\
   _left[1] _op1 _varg1[1] _op2 _varg2[1];\
}

#define V2_VSOP(_left, _op1, _varg1, _op2, _scal)\
{\
   _left[0] _op1 _varg1[0] _op2 _scal;\
   _left[1] _op1 _varg1[1] _op2 _scal;\
}

#define V2_ADDSCALE(_dst, _src, _fac)     V2_VSOP(_dst, +=, _src, *, _fac)
#define V2_SETSCALE(_dst, _src, _fac)     V2_VSOP(_dst, =, _src, *, _fac)
#define V2_SCALE(_vec, _fac)              V2_SOP(_vec, *=, _fac)
#define V2_VASSIGN(_vec, _val)            V2_VOP(_vec, =, _val)
#define V2_SASSIGN(_vec, _val)            V2_SOP(_vec, =, _val)


/*
 * some 3D scalar component operations
 */
#define S3_SPROD(_x1,_y1,_z1, _x2,_y2,_z2)      ( (_x1)*(_x2) + (_y1)*(_y2) + (_z1)*(_z2) )
#define S3_LENGTH2(_x,_y,_z)                    S3_SPROD(_x,_y,_z,_x,_y,_z)
#define S3_LENGTHF(_x,_y,_z)                    sqrtf(S3_LENGTH2(_x,_y,_z))
#define S3_LENGTHD(_x,_y,_z)                    sqrt (S3_LENGTH2(_x,_y,_z))
#define S3_LENGTHR(_x,_y,_z)                    sqrtr(S3_LENGTH2(_x,_y,_z))
#define S3_ZERO(_x,_y,_z)                       { _x = 0; _y = 0; _z = 0; }
#define S3_NEG(_x,_y,_z)                        { _x = -_x; _y = -_y; _z = -_z; }

#define S3_SUB(_x1,_y1,_z1, _x2,_y2,_z2, _xr,_yr,_zr) \
{\
   _xr = (_x1) - (_x2);\
   _yr = (_y1) - (_y2);\
   _zr = (_z1) - (_z2);\
}

#define S3_CROSS(_x1,_y1,_z1, _x2,_y2,_z2, _xr,_yr,_zr) \
{\
   _xr = (_y1)*(_z2) - (_z1)*(_y2);\
   _yr = (_z1)*(_x2) - (_x1)*(_z2);\
   _zr = (_x1)*(_y2) - (_y1)*(_x2);\
}

#define S3_SPAT(_x1,_y1,_z1, _x2,_y2,_z2, _x3,_y3,_z3, _result) \
{\
   double _xr,_yr,_zr;\
   S3_CROSS(_x1,_y1,_z1, _x2,_y2,_z2, _xr,_yr,_zr);\
   _result = S3_SPROD(_xr,_yr,_zr, _x3,_y3,_z3);\
}


/*
 * some 3D vector[3] operations
 */
#define V3_SPROD(_v,_u)        S3_SPROD  (_v[0],_v[1],_v[2], _u[0],_u[1],_u[2])
#define V3_LENGTH2(_v)         S3_LENGTH2(_v[0],_v[1],_v[2])
#define V3_LENGTHF(_v)         S3_LENGTHF(_v[0],_v[1],_v[2])
#define V3_LENGTHD(_v)         S3_LENGTHD(_v[0],_v[1],_v[2])
#define V3_LENGTHR(_v)         S3_LENGTHR(_v[0],_v[1],_v[2])
#define V3_ZERO(_v)            S3_ZERO   (_v[0],_v[1],_v[2])
#define V3_NEG(_v)             S3_NEG    (_v[0],_v[1],_v[2])
#define V3_SUB(_v, _u, _r)     S3_SUB    (_v[0],_v[1],_v[2], _u[0],_u[1],_u[2], _r[0],_r[1],_r[2])
#define V3_CROSS(_v, _u, _r)   S3_CROSS  (_v[0],_v[1],_v[2], _u[0],_u[1],_u[2], _r[0],_r[1],_r[2])

#define V3_VOP(_left, _op, _right)\
{\
   _left[0] _op _right[0];\
   _left[1] _op _right[1];\
   _left[2] _op _right[2];\
}

#define V3_SOP(_left, _op, _scal)\
{\
   _left[0] _op _scal;\
   _left[1] _op _scal;\
   _left[2] _op _scal;\
}

#define V3_VVOP(_left, _op1, _varg1, _op2, _varg2)\
{\
   _left[0] _op1 _varg1[0] _op2 _varg2[0];\
   _left[1] _op1 _varg1[1] _op2 _varg2[1];\
   _left[2] _op1 _varg1[2] _op2 _varg2[2];\
}

#define V3_VSOP(_left, _op1, _varg1, _op2, _scal)\
{\
   _left[0] _op1 _varg1[0] _op2 _scal;\
   _left[1] _op1 _varg1[1] _op2 _scal;\
   _left[2] _op1 _varg1[2] _op2 _scal;\
}

#define V3_ADDSCALE(_dst, _src, _fac)     V3_VSOP(_dst, +=, _src, *, _fac)
#define V3_SETSCALE(_dst, _src, _fac)     V3_VSOP(_dst, =, _src, *, _fac)
#define V3_SCALE(_vec, _fac)              V3_SOP(_vec, *=, _fac)
#define V3_VASSIGN(_vec, _val)            V3_VOP(_vec, =, _val)
#define V3_SASSIGN(_vec, _val)            V3_SOP(_vec, =, _val)

/*
 * recursive definition of matrix determinate for scalar matrix values
 */

#define S_DET2(a11,a12,\
               a21,a22)\
(\
    a11*a22\
  - a12*a21\
)

#define S_DET3(a11,a12,a13,\
               a21,a22,a23,\
               a31,a32,a33)\
(\
    a11*S_DET2(    a22,a23,\
                   a32,a33)\
  - a12*S_DET2(a21,    a23,\
               a31,    a33)\
  + a13*S_DET2(a21,a22    ,\
               a31,a32    )\
)

#define S_DET4(a11,a12,a13,a14,\
               a21,a22,a23,a24,\
               a31,a32,a33,a34,\
               a41,a42,a43,a44)\
(\
    a11*S_DET3(    a22,a23,a24,\
                   a32,a33,a34,\
                   a42,a43,a44)\
  - a12*S_DET3(a21,    a23,a24,\
               a31,    a33,a34,\
               a41,    a43,a44)\
  + a13*S_DET3(a21,a22,    a24,\
               a31,a32,    a34,\
               a41,a42,    a44)\
  - a14*S_DET3(a21,a22,a23,    \
               a31,a32,a33,    \
               a41,a42,a43    )\
)


#define S_DET5(a11,a12,a13,a14,a15,\
               a21,a22,a23,a24,a25,\
               a31,a32,a33,a34,a35,\
               a41,a42,a43,a44,a45,\
               a51,a52,a53,a54,a55)\
(\
    a11*S_DET4(    a22,a23,a24,a25,\
                   a32,a33,a34,a35,\
                   a42,a43,a44,a45,\
                   a52,a53,a54,a55)\
  - a12*S_DET4(a21,    a23,a24,a25,\
               a31,    a33,a34,a35,\
               a41,    a43,a44,a45,\
               a51,    a53,a54,a55)\
  + a13*S_DET4(a21,a22,    a24,a25,\
               a31,a32,    a34,a35,\
               a41,a42,    a44,a45,\
               a51,a52,    a54,a55)\
  - a14*S_DET4(a21,a22,a23,    a25,\
               a31,a32,a33,    a35,\
               a41,a42,a43,    a45,\
               a51,a52,a53,    a55)\
  + a15*S_DET4(a21,a22,a23,a24    ,\
               a31,a32,a33,a34    ,\
               a41,a42,a43,a44    ,\
               a51,a52,a53,a54    )\
)

#endif
