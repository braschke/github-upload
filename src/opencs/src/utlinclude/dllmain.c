#ifndef __DLLMAIN_SOURCE_INCLUDED
#define __DLLMAIN_SOURCE_INCLUDED
#ifdef COMPILE_DLL
/* dllmain.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    include file for MSWindows to have a DLLMain in a library
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2007, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: dllmain.c 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

/* don't like warnings about unused args */
#pragma warning(disable:4100)
#ifdef __cplusplus
   extern "C"
#endif
   BOOL APIENTRY DllMain(HANDLE hMod, DWORD reason, LPVOID res) { return TRUE; }
#pragma warning(default:4100)

#else /* UNIX */
   /*
    * under UNIX the _init() and _fini() are used
    */

   /*
    *  void _init(void) {}
    *  void _fini(void) {}
    */
#endif
#endif
#endif
