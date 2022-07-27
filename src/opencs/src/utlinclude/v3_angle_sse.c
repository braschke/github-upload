#pragma once
#ifndef v3_angle_SOURCE_INCLUDED
#define v3_angle_SOURCE_INCLUDED
/* v3_angle.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    calculate the cosine(phi), where phi is the angle between 2 oriented vectors
 *
 *    cosphi =  (v1 * v2) / ( |v1| * |v2| )
 *
 *    SSE implementation of the standard C version:
 *       return V3_SPROD(v1, v2) / sqrt( V3_LENGTH2(v1) * V3_LENGTH2(v2) );
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/23: Carsten Dehning, Initial release
 *    $Id: v3_angle_sse.c 5458 2017-08-04 17:00:54Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"
#include <math.h>

#define SIMD_SHUFFLE( srch, src1, desth, dest1 ) ( ((srch)<<6) | ((src1)<<4) | ((desth)<<2) | ((dest1)) )

/****************************************************************************************/
C_FUNC_PREFIX float v3_cosphi_fsse(float v1[4], float v2[4])
/****************************************************************************************/
{
   float cosp = -10.0;

   _asm
   {
#if defined(WIN64) || defined(_WIN64)
      mov      rsi  , v1   ;
      mov      rdi  , v2   ;
      movups   xmm0, [rsi] ; /* xmm0 = v1(x1,y1,z1,0) */
      movups   xmm2, [rdi] ; /* xmm2 = v2(x2,y2,z2,0) */
#else
      mov      esi  , v1   ;
      mov      edi  , v2   ;
      movups   xmm0, [esi] ; /* xmm0 = v1(x1,y1,z1,0) */
      movups   xmm2, [edi] ; /* xmm2 = v2(x2,y2,z2,0) */
#endif
      movaps   xmm4, xmm0  ; /* xmm4 = v1 */

      mulps    xmm4, xmm2  ; /* xmm4 = x1*x2, y1*y2, z1*z2, 0 */
      mulps    xmm0, xmm0  ; /* xmm0 = x1*x1, y1*y1, z1*z1, 0 */
      mulps    xmm2, xmm2  ; /* xmm2 = x2*x2, y2*y2, z2*z2, 0 */

#if defined(WIN64) || defined(_WIN64) || USE_SSE3
      haddps   xmm0, xmm0  ; /* xmm0 = x1*x1 + y1*y1, z1*z1, x1*x1 + y1*y1, z1*z1 */
      haddps   xmm2, xmm2  ; /* xmm2 = x2*x2 + y2*y2, z2*z2, x2*x2 + y2*y2, z2*z2 */
      haddps   xmm4, xmm4  ; /* xmm4 = x1*x2 + y1*y2, z1*z2, x1*x2 + y1*y2, z1*z2 */
      haddps   xmm0, xmm0  ; /* xmm0 = x1*x1 + y1*y1 + z1*z1, ... */
      haddps   xmm2, xmm2  ; /* xmm2 = x2*x2 + y2*y2 + z2*z2, ... */
      haddps   xmm4, xmm4  ; /* xmm4 = x1*x2 + y1*y2 + z1*z2, ... */
#else /* USE_SSE1 */

#endif

      xorps    xmm6, xmm6  ; /* xmm6 = 0,0,0,0 */
      mulss    xmm0, xmm2  ; /* xmm0 = (x1*x1 + y1*y1 + z1*z1) * (x2*x2 + y2*y2 + z2*z2), ... */
      comiss   xmm0, xmm6  ; /* compare == 0.0 */
      jz       LEN_IS_NULL ;

      rsqrtss  xmm0, xmm0  ; /* xmm1 = 1.0/sqrt(xmm0) */
      mulss    xmm4, xmm0  ; /* xmm4 = (x1*x2 + y1*y2 + z1*z2)/sqrt(xmm0) */
      movss    cosp, xmm4  ;
LEN_IS_NULL:
   }
   return cosp;
}

/****************************************************************************************/
C_FUNC_PREFIX double v3_cosphi_dsse3(double v1[4], double v2[4])
/****************************************************************************************/
/*
 * double precision only makes sense with at least SSE3 and 64 bit amd64 or em64t.
 * the asm stuff below requires SSE3.
 */
{
   double cosp = -10.0;
   _asm
   {
#if defined(WIN64) || defined(_WIN64)
      mov      rsi , v1       ;
      mov      rdi , v2       ;
      movupd   xmm0, [rsi]    ;  /* xmm0/1 = v1 */
      movupd   xmm2, [rdi]    ;  /* xmm2/3 = v2 */
      movapd   xmm4, xmm0     ;  /* xmm4/5 = v1 */
      movupd   xmm1, [rsi+16] ;
      movupd   xmm3, [rdi+16] ;
      movapd   xmm5, xmm1     ;
#else
      mov      esi , v1       ;
      mov      edi , v2       ;
      movupd   xmm0, [esi]    ;  /* xmm0/1 = v1 */
      movupd   xmm2, [edi]    ;  /* xmm2/3 = v2 */
      movapd   xmm4, xmm0     ;  /* xmm4/5 = v1 */
      movupd   xmm1, [esi+16] ;
      movupd   xmm3, [edi+16] ;
      movapd   xmm5, xmm1     ;
#endif

      mulpd    xmm4, xmm2  ; /* xmm4 = x1*x2, y1*y2 */
      mulpd    xmm5, xmm3  ; /* xmm5 = z1*z2, 0     */

      mulpd    xmm0, xmm0  ; /* xmm0 = x1*x1, y1*y1 */
      mulpd    xmm1, xmm1  ; /* xmm1 = z1*z1, 0     */

      mulpd    xmm2, xmm2  ; /* xmm2 = x2*x2, y2*y2 */
      mulpd    xmm3, xmm3  ; /* xmm3 = z2*z2, 0     */

      haddpd   xmm0, xmm1  ; /* xmm0 = x1*x1 + y1*y1, z1*z1 */
      haddpd   xmm2, xmm3  ; /* xmm2 = x2*x2 + y2*y2, z2*z2 */
      haddpd   xmm4, xmm4  ; /* xmm4 = x1*x2 + y1*y2, z1*z2 */
      haddpd   xmm0, xmm0  ; /* xmm0 = x1*x1 + y1*y1 + z1*z1, ... */
      haddpd   xmm2, xmm2  ; /* xmm2 = x2*x2 + y2*y2 + z2*z2, ... */
      haddpd   xmm4, xmm4  ; /* xmm4 = x1*x2 + y1*y2 + z1*z2, ... */

      xorpd    xmm6, xmm6  ; /* xmm6 = 0,0,0,0 */
      mulsd    xmm0, xmm2  ; /* xmm0 = (x1*x1 + y1*y1 + z1*z1) * (x2*x2 + y2*y2 + z2*z2), ... */
      sqrtsd   xmm0, xmm0  ; /* xmm0 = sqrt(xmm0), double reciprocal square root not available */
      comisd   xmm0, xmm6  ; /* compare == 0.0 */
      jz       LEN_IS_NULL ;

      divsd    xmm4, xmm0  ; /* xmm4 = (x1*x2 + y1*y2 + z1*z2)/sqrt(xmm0) */
      movsd    cosp, xmm4  ;

LEN_IS_NULL:
   }
   return cosp;
}

/****************************************************************************************/

#endif


#if 0

/*
   SSE Optimized Calculation of Triangle Plane Equations
   Copyright (C) 2005 Id Software, Inc.
   Written by J.M.P. van Waveren
   This code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   This code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.
*/
#define assert_16_byte_aligned( pointer ) assert( (((UINT_PTR)(pointer))&15) == 0 );
#define ALIGN16( x ) __declspec(align(16)) x
#define ALIGN4_INIT1( X, I ) ALIGN16( static X[4] = { I, I, I, I } )
#define R_SHUFFLE_PS( x, y, z, w ) (( (w) & 3 ) << 6 | ( (z) & 3 ) << 4 | ( (y) & 3 ) << 2 | ( (x) & 3 ))
#define IEEE_SP_ZERO 0
#define IEEE_SP_SIGN ((unsigned long) ( 1 << 31 ))

ALIGN4_INIT1( unsigned long SIMD_SP_signBit, IEEE_SP_SIGN );

struct Vec4
{
   float x, y, z, w;
};

struct Vertex
{
   Vec4 position;
   Vec4 normal;
};


#define VERTEX_SIZE_SHIFT 5
#define VERTEX_SIZE (8*4) // sizeof( Vertex )
#define VERTEX_POSITION_OFFSET (0*4) // offsetof( Vertex, position )
#define VERTEX_NORMAL_OFFSET (4*4) // offsetof( Vertex, normal )
void DeriveTrianglePlanes( Plane *planes, const Vertex *verts, const int numVerts, const int *indices, const int numIndices )
{
   int d, a;
   int n = numIndexes / 3;
   ALIGN16( float x0[4] );
   ALIGN16( float x1[4] );
   ALIGN16( float x2[4] );
   __asm
   {
      push     ebx
      mov      eax, n
      shl      eax, 4
      mov      esi, verts
      mov      edi, indexes
      mov      edx, planes
      add      edx, eax
      neg      eax
      mov      d, edx
      add      eax, 4*16
      jge      done4

      loopPlane4:
      mov      a,    eax
      mov      ecx,  [edi+0*12+0]
      shl      ecx,  VERTEX_SIZE_SHIFT
      mov      ebx,  [edi+1*12+0]
      shl      ebx,  VERTEX_SIZE_SHIFT
      mov      edx,  [edi+2*12+0]
      shl      edx,  VERTEX_SIZE_SHIFT
      mov      eax,  [edi+3*12+0]
      shl      eax,  VERTEX_SIZE_SHIFT
      movlps   xmm4, [esi+ecx+VERTEX_POSITION_OFFSET+0] /* xmm4 = 0, 1, X, X */
      movss    xmm5, [esi+ecx+VERTEX_POSITION_OFFSET+8] /* xmm5 = 2, X, X, X */
      movhps   xmm4, [esi+ebx+VERTEX_POSITION_OFFSET+0] /* xmm4 = 0, 1, 4, 5 */
      movhps   xmm5, [esi+ebx+VERTEX_POSITION_OFFSET+8] /* xmm5 = 2, X, 6, X */
      movlps   xmm6, [esi+edx+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, X, X */
      movss    xmm7, [esi+edx+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, X, X */
      movhps   xmm6, [esi+eax+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, 12, 13 */
      movhps   xmm7, [esi+eax+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, 14, X */
      movaps   xmm3, xmm4 /* xmm3 = 0, 1, 4, 5 */
      shufps   xmm3, xmm6, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm3 = 0, 4, 8, 12 */
      shufps   xmm4, xmm6, R_SHUFFLE_PS( 1, 3, 1, 3 ) /* xmm4 = 1, 5, 9, 13 */
      shufps   xmm5, xmm7, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm5 = 2, 6, 10, 14 */
      mov      ecx,  [edi+0*12+4]
      shl      ecx,  VERTEX_SIZE_SHIFT
      mov      ebx,  [edi+1*12+4]
      shl      ebx,  VERTEX_SIZE_SHIFT
      mov      edx,  [edi+2*12+4]
      shl      edx,  VERTEX_SIZE_SHIFT
      mov      eax,  [edi+3*12+4]
      shl      eax,  VERTEX_SIZE_SHIFT
      movaps   x0,   xmm3
      movaps   x1,   xmm4
      movaps   x2,   xmm5
      movlps   xmm1, [esi+ecx+VERTEX_POSITION_OFFSET+0] /* xmm1 = 0, 1, X, X */
      movss    xmm2, [esi+ecx+VERTEX_POSITION_OFFSET+8] /* xmm2 = 2, X, X, X */
      movhps   xmm1, [esi+ebx+VERTEX_POSITION_OFFSET+0] /* xmm1 = 0, 1, 4, 5 */
      movhps   xmm2, [esi+ebx+VERTEX_POSITION_OFFSET+8] /* xmm2 = 2, X, 6, X */
      movlps   xmm6, [esi+edx+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, X, X */
      movss    xmm7, [esi+edx+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, X, X */
      movhps   xmm6, [esi+eax+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, 12, 13 */
      movhps   xmm7, [esi+eax+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, 14, X */
      movaps   xmm0, xmm1 /* xmm0 = 0, 1, 4, 5 */
      shufps   xmm0, xmm6, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm0 = 0, 4, 8, 12 */
      shufps   xmm1, xmm6, R_SHUFFLE_PS( 1, 3, 1, 3 ) /* xmm1 = 1, 5, 9, 13 */
      shufps   xmm2, xmm7, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm2 = 2, 6, 10, 14 */
      mov      ecx,  [edi+0*12+8]
      shl      ecx,  VERTEX_SIZE_SHIFT
      mov      ebx,  [edi+1*12+8]
      shl      ebx,  VERTEX_SIZE_SHIFT
      mov      edx,  [edi+2*12+8]
      shl      edx,  VERTEX_SIZE_SHIFT
      mov      eax,  [edi+3*12+8]
      shl      eax,  VERTEX_SIZE_SHIFT
      subps    xmm0, xmm3
      subps    xmm1, xmm4
      subps    xmm2, xmm5
      movlps   xmm4, [esi+ecx+VERTEX_POSITION_OFFSET+0] /* xmm4 = 0, 1, X, X */
      movss    xmm5, [esi+ecx+VERTEX_POSITION_OFFSET+8] /* xmm5 = 2, X, X, X */
      movhps   xmm4, [esi+ebx+VERTEX_POSITION_OFFSET+0] /* xmm4 = 0, 1, 4, 5 */
      movhps   xmm5, [esi+ebx+VERTEX_POSITION_OFFSET+8] /* xmm5 = 2, X, 6, X */
      movlps   xmm6, [esi+edx+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, X, X */
      movss    xmm7, [esi+edx+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, X, X */
      movhps   xmm6, [esi+eax+VERTEX_POSITION_OFFSET+0] /* xmm6 = 8, 9, 12, 13 */
      movhps   xmm7, [esi+eax+VERTEX_POSITION_OFFSET+8] /* xmm6 = 10, X, 14, X */
      movaps   xmm3, xmm4 /* xmm3 = 0, 1, 4, 5 */
      shufps   xmm3, xmm6, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm3 = 0, 4, 8, 12 */
      shufps   xmm4, xmm6, R_SHUFFLE_PS( 1, 3, 1, 3 ) /* xmm4 = 1, 5, 9, 13 */
      shufps   xmm5, xmm7, R_SHUFFLE_PS( 0, 2, 0, 2 ) /* xmm5 = 2, 6, 10, 14 */
      mov      eax,  a
      mov      edx,  d
      add      edi,  4*12
      subps    xmm3, x0
      subps    xmm4, x1
      subps    xmm5, x2
      movaps   xmm6, xmm4
      mulps    xmm6, xmm2
      movaps   xmm7, xmm5
      mulps    xmm7, xmm1
      subps    xmm6, xmm7
      mulps    xmm5, xmm0
      mulps    xmm2, xmm3
      subps    xmm5, xmm2
      mulps    xmm3, xmm1
      mulps    xmm4, xmm0
      subps    xmm3, xmm4
      add      eax,  4*16
      movaps   xmm0, xmm6
      mulps    xmm6, xmm6
      movaps   xmm1, xmm5
      mulps    xmm5, xmm5
      movaps   xmm2, xmm3
      mulps    xmm3, xmm3
      addps    xmm3, xmm5
      addps    xmm3, xmm6
      rsqrtps  xmm3, xmm3
      mulps    xmm0, xmm3
      mulps    xmm1, xmm3
      mulps    xmm2, xmm3
      movaps   xmm4, x0
      movaps   xmm5, x1
      movaps   xmm6, x2
      mulps    xmm4, xmm0
      mulps    xmm5, xmm1
      mulps    xmm6, xmm2
      addps    xmm4, xmm5
      addps    xmm4, xmm6
      xorps    xmm4, SIMD_SP_signBit
      // transpose xmm0, xmm1, xmm2, xmm4 to memory
      movaps   xmm7, xmm0
      movaps   xmm5, xmm2
      unpcklps xmm0, xmm1
      unpcklps xmm2, xmm4
      movlps   [edx+eax-8*16+0], xmm0
      movlps   [edx+eax-8*16+8], xmm2
      movhps   [edx+eax-7*16+0], xmm0
      movhps   [edx+eax-7*16+8], xmm2
      unpckhps xmm7, xmm1
      unpckhps xmm5, xmm4
      movlps   [edx+eax-6*16+0], xmm7
      movlps   [edx+eax-6*16+8], xmm5
      movhps   [edx+eax-5*16+0], xmm7
      movhps   [edx+eax-5*16+8], xmm5
      jle loopPlane4
      done4:
      sub      eax, 4*16
      jge      done
      loopPlane1:
      mov      ecx, [edi+0]
      shl      ecx, VERTEX_SIZE_SHIFT
      mov      ebx, [edi+4]
      shl      ebx, VERTEX_SIZE_SHIFT
      mov      edx, [edi+8]
      shl      edx, VERTEX_SIZE_SHIFT
      movss    xmm0, [esi+ebx+VERTEX_POSITION_OFFSET+0]
      subss    xmm0, [esi+ecx+VERTEX_POSITION_OFFSET+0]
      movss    xmm1, [esi+ebx+VERTEX_POSITION_OFFSET+4]
      subss    xmm1, [esi+ecx+VERTEX_POSITION_OFFSET+4]
      movss    xmm2, [esi+ebx+VERTEX_POSITION_OFFSET+8]
      subss    xmm2, [esi+ecx+VERTEX_POSITION_OFFSET+8]
      movss    xmm3, [esi+edx+VERTEX_POSITION_OFFSET+0]
      subss    xmm3, [esi+ecx+VERTEX_POSITION_OFFSET+0]
      movss    xmm4, [esi+edx+VERTEX_POSITION_OFFSET+4]
      subss    xmm4, [esi+ecx+VERTEX_POSITION_OFFSET+4]
      movss    xmm5, [esi+edx+VERTEX_POSITION_OFFSET+8]
      subss    xmm5, [esi+ecx+VERTEX_POSITION_OFFSET+8]
      movss    xmm6, xmm4
      mulss    xmm6, xmm2
      movss    xmm7, xmm5
      mulss    xmm7, xmm1
      subss    xmm6, xmm7
      add      edi, 1*12
      mulss    xmm5, xmm0
      mulss    xmm2, xmm3
      subss    xmm5, xmm2
      mulss    xmm3, xmm1
      mulss    xmm4, xmm0
      subss    xmm3, xmm4
      mov      edx, d
      movss    xmm0, xmm6
      mulss    xmm6, xmm6
      movss    xmm1, xmm5
      mulss    xmm5, xmm5
      movss    xmm2, xmm3
      mulss    xmm3, xmm3
      add      eax, 1*16
      addss    xmm3, xmm5
      addss    xmm3, xmm6
      rsqrtss  xmm3, xmm3
      mulss    xmm0, xmm3
      mulss    xmm1, xmm3
      mulss    xmm2, xmm3
      movss    [edx+eax-1*16+0], xmm0
      movss    [edx+eax-1*16+4], xmm1
      movss    [edx+eax-1*16+8], xmm2
      mulss    xmm0, [esi+ecx+VERTEX_POSITION_OFFSET+0]
      mulss    xmm1, [esi+ecx+VERTEX_POSITION_OFFSET+4]
      mulss    xmm2, [esi+ecx+VERTEX_POSITION_OFFSET+8]
      xorps    xmm0, SIMD_SP_firstSignBit
      subss    xmm0, xmm1
      subss    xmm0, xmm2
      movss    [edx+eax-1*16+12], xmm0
      jl loopPlane1
      done:
      pop ebx
   }
}
#endif