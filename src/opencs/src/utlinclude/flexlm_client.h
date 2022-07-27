#pragma once
#ifndef flexlm_client_HEADER_INCLUDED
#define flexlm_client_HEADER_INCLUDED
/* flexlm_client.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    #include, #define and #pragma stuff for the flexlm usage as a client
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2008/Aug: Carsten Dehning, Initial release
 *    $Id: flexlm_client.h 4951 2016-07-27 13:13:12Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if IS_MSWIN

   /*
    * Automatically add all required FLEXLm libs
    */
   #pragma comment(lib,"lmgr_trl")
   #pragma comment(lib,"libsb")
   #pragma comment(lib,"libcrvs")
   #pragma comment(lib,"libnoact")
   #pragma comment(lib,"lmgr_dongle_stub")
   #pragma comment(lib,"libredir_std")

   /*
    * Automatically add all libs required by the FLEXlm stuff which we will
    * definitely forget on the link line inside the NMakefile
    */
   #pragma comment(lib,"user32")
   #pragma comment(lib,"advapi32")
   #pragma comment(lib,"netapi32")
   #pragma comment(lib,"comdlg32")
   #pragma comment(lib,"comctl32")
   #pragma comment(lib,"userenv")
   #pragma comment(lib,"shell32")
   #pragma comment(lib,"ole32")
   #pragma comment(lib,"oleaut32")
   #pragma comment(lib,"wbemuuid")
   #pragma comment(lib,"dhcpcsvc")
   #pragma comment(lib,"shlwapi")

   #pragma comment(lib,"legacy_stdio_wide_specifiers")
   #pragma comment(lib,"legacy_stdio_definitions")

#endif

#include "lmclient.h"
#include "lm_attr.h"

#endif
