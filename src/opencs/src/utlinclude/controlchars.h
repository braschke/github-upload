#pragma once
#ifndef controlchars_HEADER_INCLUDED
#define controlchars_HEADER_INCLUDED
/* controlchars.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    #define symbols for ASCII controls chars
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2007/Jun/26: Carsten Dehning, Initial release
 *    $Id: controlchars.h 4468 2016-05-17 09:13:52Z dehning $
 *
 *****************************************************************************************
 */
#define NUL    0x00
#define SOH    0x01
#define STX    0x02
#define ETX    0x03
#define EOT    0x04
#define ENQ    0x05
#define ACK    0x06
#define BEL    0x07  /* \a */
#define BS     0x08  /* \b */
#define HT     0x09  /* \t */
#define LF     0x0a  /* \012 NOT ALWAYS '\n' !*/
#define VT     0x0b  /* \v */
#define FF     0x0c  /* \f */
#define CR     0x0d  /* \015 NOT ALWAYS '\r' !*/
#define SO     0x0e
#define SI     0x0f
#define DLE    0x10
#define DC1    0x11
#define DC2    0x12
#define DC3    0x13
#define DC4    0x14
#define NAK    0x15
#define SYN    0x16
#define ETB    0x17
#define CAN    0x18
#define EM     0x19
#define SUB    0x1a
#define ESC    0x1b
#define FS     0x1c
#define GS     0x1d
#define RS     0x1e
#define US     0x1f
#define SP     0x20
#define DEL    0x7f


/* >= 128 */
#define PAD    0x80
#define HOP    0x81
#define BPH    0x82
#define NBH    0x83
#define IND    0x84
#define NEL    0x85
#define SSA    0x86
#define ESA    0x87
#define HTS    0x88
#define HTJ    0x89
#define VTS    0x8a
#define PLD    0x8b
#define PLU    0x8c
#define RI     0x8d
#define SS2    0x8e
#define SS3    0x8f
#define DCS    0x90
#define PU1    0x91
#define PU2    0x92
#define STS    0x93
#define CCH    0x94
#define MW     0x95
#define SPA    0x96
#define EPA    0x97
#define GA1    0x98
#define GA2    0x99
#define GA3    0x9a
#define CSI    0x9b
#define ST     0x9c
#define OSC    0x9e
#define PM     0x9e
#define APC    0x9f

#define XFF    0xff

#endif
