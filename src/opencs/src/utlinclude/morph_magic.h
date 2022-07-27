#ifndef __morph_magic_HEADER_INCLUDED
#define __morph_magic_HEADER_INCLUDED
/* morph_magic.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    contains the magic numbers for the morpher communication.
 *    used by client and morpher
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fhg.de>
 *    Copyright (c) 2006-2011, Fraunhofer Institute SCAI
 *
 * Reviews/changes:
 *    2006/Sep/10: Carsten Dehning, Initial release
 *    $Id: morph_magic.h 12 2012-12-19 18:27:58Z dehning $
 *
 *****************************************************************************************
 */

/*
 * magic strings used to sync the communication and to
 * indicate the vertex types (float/double) coordinates to the receiver
 * #3D/?  3D vertices
 * #2D/?  2D vertices foreseen for the future ...
 */
#define _GMD_MAGIC_SIZE    6        /* no. of chars for magic token incl. '\0'*/
#define _GMD_MAGIC_DISP    "#3D/D"  /* vertex displacement */
#define _GMD_MAGIC_COOR    "#3D/V"  /* vertex coordinate */
#define _GMD_MAGIC_GRID    "#3D/G"  /* grid vertex coordinate */
#define _GMD_MAGIC_EXIT    "#EXIT"  /* communication exit */
#define _GMD_MAGIC_EXEC    "#EXEC"  /* magic trailer after last vertex */

/*
 * vertex index used to indicate the terminatation of the vertex transfer
 * vertex indices MUST be >= 0
 */
#define _GMD_VTERM_IDX           -54321 /* just < 0 */

/*
 * magic and hello control strings used for the socket based communication
 * via SSOCK_accept() and SSOCK_connect()
 *
 * the HELLO format string contains the no. of processes (client)
 * and the output level (server)
 */
#define _GMD_HELLO_FMT_CLIENT    "C:%u:%d" /* sizeof(coord): no. of parallel processes */
#define _GMD_HELLO_FMT_SERVER    "S:%u:%d" /* sizeof(coord): printout level */
#define _GMD_HELLO_MAGIC         "MpCCI/Morpher 1.4"


#endif
