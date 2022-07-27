#pragma once
#ifndef chartostr_SOURCE_INCLUDED
#define chartostr_SOURCE_INCLUDED
/* chartostr.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Convert a single character into a 2 byte string " c + \0 ".
 *    In fact just return a pointer to a static string.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/Jul/01: Carsten Dehning, Initial release
 *    $Id: chartostr.c 5446 2017-08-03 17:51:36Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

C_FUNC_PREFIX
const TCHAR *chartostr(const int c)
{
   static const TCHAR *charAsStringV[256] =
   {
      TEXT("\x00"),TEXT("\x01"),TEXT("\x02"),TEXT("\x03"),
      TEXT("\x04"),TEXT("\x05"),TEXT("\x06"),TEXT("\x07"),
      TEXT("\x08"),TEXT("\x09"),TEXT("\x0a"),TEXT("\x0b"),
      TEXT("\x0c"),TEXT("\x0d"),TEXT("\x0e"),TEXT("\x0f"),

      TEXT("\x10"),TEXT("\x11"),TEXT("\x12"),TEXT("\x13"),
      TEXT("\x14"),TEXT("\x15"),TEXT("\x16"),TEXT("\x17"),
      TEXT("\x18"),TEXT("\x19"),TEXT("\x1a"),TEXT("\x1b"),
      TEXT("\x1c"),TEXT("\x1d"),TEXT("\x1e"),TEXT("\x1f"),

      TEXT("\x20"),TEXT("\x21"),TEXT("\x22"),TEXT("\x23"),
      TEXT("\x24"),TEXT("\x25"),TEXT("\x26"),TEXT("\x27"),
      TEXT("\x28"),TEXT("\x29"),TEXT("\x2a"),TEXT("\x2b"),
      TEXT("\x2c"),TEXT("\x2d"),TEXT("\x2e"),TEXT("\x2f"),

      TEXT("\x30"),TEXT("\x31"),TEXT("\x32"),TEXT("\x33"),
      TEXT("\x34"),TEXT("\x35"),TEXT("\x36"),TEXT("\x37"),
      TEXT("\x38"),TEXT("\x39"),TEXT("\x3a"),TEXT("\x3b"),
      TEXT("\x3c"),TEXT("\x3d"),TEXT("\x3e"),TEXT("\x3f"),

      TEXT("\x40"),TEXT("\x41"),TEXT("\x42"),TEXT("\x43"),
      TEXT("\x44"),TEXT("\x45"),TEXT("\x46"),TEXT("\x47"),
      TEXT("\x48"),TEXT("\x49"),TEXT("\x4a"),TEXT("\x4b"),
      TEXT("\x4c"),TEXT("\x4d"),TEXT("\x4e"),TEXT("\x4f"),

      TEXT("\x50"),TEXT("\x51"),TEXT("\x52"),TEXT("\x53"),
      TEXT("\x54"),TEXT("\x55"),TEXT("\x56"),TEXT("\x57"),
      TEXT("\x58"),TEXT("\x59"),TEXT("\x5a"),TEXT("\x5b"),
      TEXT("\x5c"),TEXT("\x5d"),TEXT("\x5e"),TEXT("\x5f"),

      TEXT("\x60"),TEXT("\x61"),TEXT("\x62"),TEXT("\x63"),
      TEXT("\x64"),TEXT("\x65"),TEXT("\x66"),TEXT("\x67"),
      TEXT("\x68"),TEXT("\x69"),TEXT("\x6a"),TEXT("\x6b"),
      TEXT("\x6c"),TEXT("\x6d"),TEXT("\x6e"),TEXT("\x6f"),

      TEXT("\x70"),TEXT("\x71"),TEXT("\x72"),TEXT("\x73"),
      TEXT("\x74"),TEXT("\x75"),TEXT("\x76"),TEXT("\x77"),
      TEXT("\x78"),TEXT("\x79"),TEXT("\x7a"),TEXT("\x7b"),
      TEXT("\x7c"),TEXT("\x7d"),TEXT("\x7e"),TEXT("\x7f"),

      TEXT("\x80"),TEXT("\x81"),TEXT("\x82"),TEXT("\x83"),
      TEXT("\x84"),TEXT("\x85"),TEXT("\x86"),TEXT("\x87"),
      TEXT("\x88"),TEXT("\x89"),TEXT("\x8a"),TEXT("\x8b"),
      TEXT("\x8c"),TEXT("\x8d"),TEXT("\x8e"),TEXT("\x8f"),

      TEXT("\x90"),TEXT("\x91"),TEXT("\x92"),TEXT("\x93"),
      TEXT("\x94"),TEXT("\x95"),TEXT("\x96"),TEXT("\x97"),
      TEXT("\x98"),TEXT("\x99"),TEXT("\x9a"),TEXT("\x9b"),
      TEXT("\x9c"),TEXT("\x9d"),TEXT("\x9e"),TEXT("\x9f"),

      TEXT("\xa0"),TEXT("\xa1"),TEXT("\xa2"),TEXT("\xa3"),
      TEXT("\xa4"),TEXT("\xa5"),TEXT("\xa6"),TEXT("\xa7"),
      TEXT("\xa8"),TEXT("\xa9"),TEXT("\xaa"),TEXT("\xab"),
      TEXT("\xac"),TEXT("\xad"),TEXT("\xae"),TEXT("\xaf"),

      TEXT("\xb0"),TEXT("\xb1"),TEXT("\xb2"),TEXT("\xb3"),
      TEXT("\xb4"),TEXT("\xb5"),TEXT("\xb6"),TEXT("\xb7"),
      TEXT("\xb8"),TEXT("\xb9"),TEXT("\xba"),TEXT("\xbb"),
      TEXT("\xbc"),TEXT("\xbd"),TEXT("\xbe"),TEXT("\xbf"),

      TEXT("\xc0"),TEXT("\xc1"),TEXT("\xc2"),TEXT("\xc3"),
      TEXT("\xc4"),TEXT("\xc5"),TEXT("\xc6"),TEXT("\xc7"),
      TEXT("\xc8"),TEXT("\xc9"),TEXT("\xca"),TEXT("\xcb"),
      TEXT("\xcc"),TEXT("\xcd"),TEXT("\xce"),TEXT("\xcf"),

      TEXT("\xd0"),TEXT("\xd1"),TEXT("\xd2"),TEXT("\xd3"),
      TEXT("\xd4"),TEXT("\xd5"),TEXT("\xd6"),TEXT("\xd7"),
      TEXT("\xd8"),TEXT("\xd9"),TEXT("\xda"),TEXT("\xdb"),
      TEXT("\xdc"),TEXT("\xdd"),TEXT("\xde"),TEXT("\xdf"),

      TEXT("\xe0"),TEXT("\xe1"),TEXT("\xe2"),TEXT("\xe3"),
      TEXT("\xe4"),TEXT("\xe5"),TEXT("\xe6"),TEXT("\xe7"),
      TEXT("\xe8"),TEXT("\xe9"),TEXT("\xea"),TEXT("\xeb"),
      TEXT("\xec"),TEXT("\xed"),TEXT("\xee"),TEXT("\xef"),

      TEXT("\xf0"),TEXT("\xf1"),TEXT("\xf2"),TEXT("\xf3"),
      TEXT("\xf4"),TEXT("\xf5"),TEXT("\xf6"),TEXT("\xf7"),
      TEXT("\xf8"),TEXT("\xf9"),TEXT("\xfa"),TEXT("\xfb"),
      TEXT("\xfc"),TEXT("\xfd"),TEXT("\xfe"),TEXT("\xff")
   };

   /* return "00" if c is out of range */
   return charAsStringV[((c<0||c>0xff) ? 0 : c)];
}
#endif
