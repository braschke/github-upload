#ifndef __getsigname_SOURCE_INCLUDED
#define __getsigname_SOURCE_INCLUDED
/* getsigname.c
 *
 *****************************************************************************************
 *
 * Purpose: MSWin UNICODE save
 *    return the string description for a UNIX signal id. Under MSWin also the
 *
 *       CTRL_C_EVENT        0
 *       CTRL_BREAK_EVENT    1
 *       CTRL_CLOSE_EVENT    2
 *       CTRL_LOGOFF_EVENT   5
 *       CTRL_SHUTDOWN_EVENT 6
 *
 *    controls handle event are mapped - signal 3 & 4 are reserved under MSWin.
 *    see "winsig.h"
 *
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2009, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2007/Jan/30: Carsten Dehning, Initial release
 *    $Id: getsigname.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
const TCHAR *getsigname(int sig)
{
   static TCHAR        sBuff[32];
   static const TCHAR *sList[] =
   {
      /* MSWin: CTRL_C_EVENT, CTRL_BREAK_EVENT, CTRL_CLOSE_EVENT */
   #if IS_MSWIN
      TEXT("SIGINT - Control C event"),
      TEXT("SIGBREAK - Control break event"),
      TEXT("SIGCLOSE - Control close event"),
   #else
      TEXT("SIGNULL - No signal or signal 0 received"),
      TEXT("SIGHUP - Hangup of controlling process"),
      TEXT("SIGINT - Interrupt from keyboard"),
   #endif

      TEXT("SIGQUIT - Quit from keyboard"),
      TEXT("SIGILL - Illegal instruction"),

      /* MSWin: CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT */
   #if IS_MSWIN
      TEXT("SIGLOGOFF - Logoff event"),
      TEXT("SIGSHUT - Shutdown event"),
   #else
      TEXT("SIGTRAP - Trace/breakpoint trap"),
      TEXT("SIGABRT - Signal from abort()"),
   #endif

      TEXT("SIGBUS - Bus error/bad memory access"),
      TEXT("SIGFPE - Floating point exception"),
      TEXT("SIGKILL - Kill signal"),
      TEXT("SIGBUS - Bus error/bad memory access"),
      TEXT("SIGSEGV - Invalid memory reference"),
      TEXT("SIGSYS - Bad argument to routine"),
      TEXT("SIGPIPE - Broken pipe"),
      TEXT("SIGALRM - Timer signal from alarm()"),
      TEXT("SIGTERM - Termination signal"),
      TEXT("SIGSTOP - Stop process"),
      TEXT("SIGCHLD - Child stopped or terminated"),
      TEXT("SIGTSTP - Stop typed at tty")
   };

   if (sig >= 0 && sig < CAST_INT(countof(sList)))
      return sList[sig];

#if IS_MSWIN && IS_UNICODE
   swprintf(sBuff,countof(sBuff),L"SIG%03d - Unknown signal",sig);
#else
   sprintf(sBuff,"SIG%03d - Unknown signal",sig);
#endif

   return sBuff;
}
#endif
