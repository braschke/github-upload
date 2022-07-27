#ifndef xmsg_HEADER_INCLUDED
#define xmsg_HEADER_INCLUDED
/* xmsg.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Header for the XMSG stuff
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: xmsg.h 2391 2014-01-07 16:59:16Z dehning $
 *
 *****************************************************************************************
 */

/*
 * The printf wrapper functions can easily be implemented
 * in C/C++ and Fortran(hidden stringlength is the last argument)
 *
 *  Fortran:
 *
 *    SUBROUTINE PRINT_LINE(STRING)
 *    CHARACTER*(*) STRING
 *    COMMON /IOUNITS/ IOUT,IERR
 *    WRITE(IOUT,*) STRING
 *    RETURN
 *    END
 *
 *    SUBROUTINE PRINT_ERROR(STRING)
 *    CHARACTER*(*) STRING
 *    COMMON /IOUNITS/ IOUT,IERR
 *    WRITE(IERR,*) STRING
 *    RETURN
 *    END
 *
 */
#ifndef XMSG_USE_PTR
   #define XMSG_USE_PTR    0
#endif

#ifndef XMSG_USE_CIO
   #define XMSG_USE_CIO    0
#endif

#ifndef XMSG_USE_ATEXIT
   #define XMSG_USE_ATEXIT 0
#endif

#ifndef XMSG_USE_TIME
   #define XMSG_USE_TIME   0
#endif

#ifndef XMSG_USE_PREFIX
   #define XMSG_USE_PREFIX 0
#endif

#ifndef XMSG_USE_ASSERT
   #define XMSG_USE_ASSERT 1
#endif


/* Need stdarg.h for the va_list */
#include <stdarg.h>

#if !INCLUDE_STATIC
#ifdef __cplusplus
extern "C" {
#endif

   #if XMSG_USE_PTR
   extern void XMSG_setfunct
               (
                  void (*printFullMessage)(const char *str, int len),
                  void (*printFullWarning)(const char *str, int len),
                  void (*printFullFatal  )(const char *str, int len),
                  void (*printLineMessage)(const char *str, int len),
                  void (*printLineWarning)(const char *str, int len),
                  void (*printLineFatal  )(const char *str, int len)
               );
   #endif

   #if XMSG_USE_ATEXIT
   extern void XMSG_atexit  (void (*atExitHandler)(void));
   #endif

   extern int  XMSG_vmessage(int level, const char *fmt, va_list ap);
#if defined(__GNUC__) && !defined(__ICC) /* Let gcc/g++ check the format string and arguments */
   extern int  XMSG_message (int level, const char *fmt, ...) __attribute__ ((format(printf,2,3)));
#else
   extern int  XMSG_message (int level, const char *fmt, ...);
#endif
   extern int  XMSG_setlevel(int level);
   extern int  XMSG_getlevel(void);
   #if XMSG_USE_PREFIX
   extern int  XMSG_vsetprefix(const char *fmt,va_list ap);
   extern int  XMSG_setprefix (const char *fmt,...);
   #endif

#ifdef __cplusplus
}
#endif
#endif

#define XMSG_SETFUNCT      XMSG_setfunct
#define XMSG_ATEXIT        XMSG_atexit
#define XMSG_SETLEVEL      XMSG_setlevel
#define XMSG_GETLEVEL      XMSG_getlevel
#define XMSG_VSETPREFIX    XMSG_vsetprefix
#define XMSG_SETPREFIX     XMSG_setprefix
#define XMSG_VMESSAGE      XMSG_vmessage
#define XMSG_MESSAGE       XMSG_message



#define XMSG_LEVEL_FATAL    -1
#define XMSG_LEVEL_WARNING   0
#define XMSG_LEVEL_INFO      1
#define XMSG_LEVEL_ACTION    2
#define XMSG_LEVEL_DEBUG     3

#ifndef XMSG_NO_DOMACROS
   #define DO_DEBUG   XMSG_GETLEVEL() >= XMSG_LEVEL_DEBUG
   #define DO_ACTION  XMSG_GETLEVEL() >= XMSG_LEVEL_ACTION
   #define DO_INFO    XMSG_GETLEVEL() >= XMSG_LEVEL_INFO
#endif

/*
 * Basic informational messages passed at debug level >= 1
 */
#define XMSG_INFOV(f,ap)                                       XMSG_VMESSAGE(XMSG_LEVEL_INFO,f,ap)

#define XMSG_INFO0(f)                                          XMSG_MESSAGE(XMSG_LEVEL_INFO,f)
#define XMSG_INFO1(f,a1)                                       XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1)
#define XMSG_INFO2(f,a1,a2)                                    XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2)
#define XMSG_INFO3(f,a1,a2,a3)                                 XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3)
#define XMSG_INFO4(f,a1,a2,a3,a4)                              XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4)
#define XMSG_INFO5(f,a1,a2,a3,a4,a5)                           XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5)
#define XMSG_INFO6(f,a1,a2,a3,a4,a5,a6)                        XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6)
#define XMSG_INFO7(f,a1,a2,a3,a4,a5,a6,a7)                     XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_INFO8(f,a1,a2,a3,a4,a5,a6,a7,a8)                  XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_INFO9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)               XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_INFO10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)          XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_INFO11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)      XMSG_MESSAGE(XMSG_LEVEL_INFO,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

/*
 * Informational activity messages passed at debug level >= 2
 */
#define XMSG_ACTIONV(f,ap)                                     XMSG_VMESSAGE(XMSG_LEVEL_ACTION,f,ap)

#define XMSG_ACTION0(f)                                        XMSG_MESSAGE(XMSG_LEVEL_ACTION,f)
#define XMSG_ACTION1(f,a1)                                     XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1)
#define XMSG_ACTION2(f,a1,a2)                                  XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2)
#define XMSG_ACTION3(f,a1,a2,a3)                               XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3)
#define XMSG_ACTION4(f,a1,a2,a3,a4)                            XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4)
#define XMSG_ACTION5(f,a1,a2,a3,a4,a5)                         XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5)
#define XMSG_ACTION6(f,a1,a2,a3,a4,a5,a6)                      XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6)
#define XMSG_ACTION7(f,a1,a2,a3,a4,a5,a6,a7)                   XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_ACTION8(f,a1,a2,a3,a4,a5,a6,a7,a8)                XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_ACTION9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)             XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_ACTION10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)        XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_ACTION11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)    XMSG_MESSAGE(XMSG_LEVEL_ACTION,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

/*
 * Detailed debug messages passed at debug level >= 3
 */
#define XMSG_DEBUGV(f,ap)                                      XMSG_VMESSAGE(XMSG_LEVEL_DEBUG,f,ap)

#define XMSG_DEBUG0(f)                                         XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f)
#define XMSG_DEBUG1(f,a1)                                      XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1)
#define XMSG_DEBUG2(f,a1,a2)                                   XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2)
#define XMSG_DEBUG3(f,a1,a2,a3)                                XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3)
#define XMSG_DEBUG4(f,a1,a2,a3,a4)                             XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4)
#define XMSG_DEBUG5(f,a1,a2,a3,a4,a5)                          XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5)
#define XMSG_DEBUG6(f,a1,a2,a3,a4,a5,a6)                       XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6)
#define XMSG_DEBUG7(f,a1,a2,a3,a4,a5,a6,a7)                    XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_DEBUG8(f,a1,a2,a3,a4,a5,a6,a7,a8)                 XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_DEBUG9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)              XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_DEBUG10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)         XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_DEBUG11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)     XMSG_MESSAGE(XMSG_LEVEL_DEBUG,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

/*
 * Warning messages always printed:
 *    No check on message level required
 */
#if !XMSG_USE_CIO || XMSG_USE_PTR || XMSG_USE_PREFIX

#define XMSG_WARNINGV(f,ap)                                    XMSG_VMESSAGE(XMSG_LEVEL_WARNING,f,ap)

#define XMSG_WARNING0(f)                                       XMSG_MESSAGE(XMSG_LEVEL_WARNING,f)
#define XMSG_WARNING1(f,a1)                                    XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1)
#define XMSG_WARNING2(f,a1,a2)                                 XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2)
#define XMSG_WARNING3(f,a1,a2,a3)                              XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3)
#define XMSG_WARNING4(f,a1,a2,a3,a4)                           XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4)
#define XMSG_WARNING5(f,a1,a2,a3,a4,a5)                        XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5)
#define XMSG_WARNING6(f,a1,a2,a3,a4,a5,a6)                     XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6)
#define XMSG_WARNING7(f,a1,a2,a3,a4,a5,a6,a7)                  XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_WARNING8(f,a1,a2,a3,a4,a5,a6,a7,a8)               XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_WARNING9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)            XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_WARNING10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)       XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_WARNING11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)   XMSG_MESSAGE(XMSG_LEVEL_WARNING,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

#else

#define XMSG_WARNINGV(f,ap)                                    vfprintf(stderr,f,ap)

#define XMSG_WARNING0(f)                                       fprintf(stderr,f)
#define XMSG_WARNING1(f,a1)                                    fprintf(stderr,f,a1)
#define XMSG_WARNING2(f,a1,a2)                                 fprintf(stderr,f,a1,a2)
#define XMSG_WARNING3(f,a1,a2,a3)                              fprintf(stderr,f,a1,a2,a3)
#define XMSG_WARNING4(f,a1,a2,a3,a4)                           fprintf(stderr,f,a1,a2,a3,a4)
#define XMSG_WARNING5(f,a1,a2,a3,a4,a5)                        fprintf(stderr,f,a1,a2,a3,a4,a5)
#define XMSG_WARNING6(f,a1,a2,a3,a4,a5,a6)                     fprintf(stderr,f,a1,a2,a3,a4,a5,a6)
#define XMSG_WARNING7(f,a1,a2,a3,a4,a5,a6,a7)                  fprintf(stderr,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_WARNING8(f,a1,a2,a3,a4,a5,a6,a7,a8)               fprintf(stderr,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_WARNING9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)            fprintf(stderr,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_WARNING10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)       fprintf(stderr,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_WARNING11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)   fprintf(stderr,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

#endif

/*
 * Fatal warning messages and exit
 */
#define XMSG_FATALV(f,ap)                                      XMSG_VMESSAGE(XMSG_LEVEL_FATAL,f,ap)

#define XMSG_FATAL0(f)                                         XMSG_MESSAGE(XMSG_LEVEL_FATAL,f)
#define XMSG_FATAL1(f,a1)                                      XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1)
#define XMSG_FATAL2(f,a1,a2)                                   XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2)
#define XMSG_FATAL3(f,a1,a2,a3)                                XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3)
#define XMSG_FATAL4(f,a1,a2,a3,a4)                             XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4)
#define XMSG_FATAL5(f,a1,a2,a3,a4,a5)                          XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5)
#define XMSG_FATAL6(f,a1,a2,a3,a4,a5,a6)                       XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6)
#define XMSG_FATAL7(f,a1,a2,a3,a4,a5,a6,a7)                    XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6,a7)
#define XMSG_FATAL8(f,a1,a2,a3,a4,a5,a6,a7,a8)                 XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6,a7,a8)
#define XMSG_FATAL9(f,a1,a2,a3,a4,a5,a6,a7,a8,a9)              XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6,a7,a8,a9)
#define XMSG_FATAL10(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)         XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10)
#define XMSG_FATAL11(f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)     XMSG_MESSAGE(XMSG_LEVEL_FATAL,f,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11)

#if XMSG_USE_ASSERT
   #define XMSG_ASSERT(cond)  if (!(cond)) XMSG_FATAL3("Assertion failed (%s): %s(%d)\n",#cond,__FILE__,__LINE__)
#else
   #define XMSG_ASSERT(cond)
#endif
#endif
