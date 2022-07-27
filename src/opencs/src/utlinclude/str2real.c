#pragma once
#ifndef str2real_SOURCE_INCLUDED
#define str2real_SOURCE_INCLUDED
/* str2real.c , C.Dehning  July '90
 *
 *  string utilities (c) C.Dehning  1990
 *
 *  konvertiert string s in double,
 *  wenn moeglich. return ist 0, wenns geklappt hat, ansonsten +-1
 */
#include "stdheader.h"
#if REAL_IS_DOUBLE
   #include "str2double.c"
#else
   #include "str2float.c"
#endif
#endif
