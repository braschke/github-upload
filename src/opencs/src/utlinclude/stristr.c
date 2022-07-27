#ifndef stristr_SOURCE_INCLUDED
#define stristr_SOURCE_INCLUDED
/* stristr.c
 *
 *  string utilities (c) C.Dehning July 1990
 *
 *    ignore case strstr()
 *
 */
#include "stdheader.h"

/*
 * we have separate versions, one for ANSI char and one for UNICODE wchar_t
 */
#if IS_MSWIN && IS_UNICODE

   #include "stristr_w.c"
   #define stristr   stristr_w

#else

   #include "stristr_a.c"
   #define stristr   stristr_a

#endif
#endif
