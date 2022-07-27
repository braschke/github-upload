#pragma once
#ifndef octbox_SOURCE_INCLUDED
#define octbox_SOURCE_INCLUDED
/* octbox.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Implements an octal-tree point container based on unsigned coordinates.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2011/Apr: Carsten Dehning, Initial release
 *    $Id: octbox.c 5825 2018-05-09 17:25:25Z dehning $
 *
 *****************************************************************************************
 */
#include "octbox.h"
#include <limits.h>
#include <math.h>

#if INCLUDE_STATIC
   #include "xmem.c"
   #include "xmsg.c"
   #include "ipool.c"
   #include "getcpuseconds.c"
#endif

#define XYZ_ORDER          0

#if XYZ_ORDER
   #define OMASK_HIGHX     0x0004 /* octant is in high x */
   #define OMASK_HIGHY     0x0002 /* octant is in high y */
   #define OMASK_HIGHZ     0x0001 /* octant is in high z */
#else /* ZYX-Order */
   #define OMASK_HIGHX     0x0001 /* octant is in high x */
   #define OMASK_HIGHY     0x0002 /* octant is in high y */
   #define OMASK_HIGHZ     0x0004 /* octant is in high z */
#endif

/* Leaf node signature is child[0] == (void *)1) */
#if IS_MSWIN && !defined(IS_MINGW) && IS_64BIT
   #define BLEAF_TRUE   ((void *)1i64) /* avoid VC 64 complaints */
#else
   #define BLEAF_TRUE   ((void *)1)
#endif


#if OCTBOX_USE_UINT64

   /* 64 bit unsigned coords */
   #ifdef PRIu64
      #define OBFMT_CU     PRIu64
   #else
      #if IS_MSWIN
         #if IS_MINGW
            #define OBFMT_CU  "llu"
         #else
            #define OBFMT_CU  "I64u"
         #endif
      #else
         #define OBFMT_CU  "lu"
      #endif
   #endif

#else

   /* 32 bit unsigned coords */
   #define OBFMT_CU  "u"

#endif

/*
 * The min. resolution of the box:
 * This must be a small even number, since initial uxmid is uxmax/2 without roundoff error.
 */
#define OBBOX_MINRES    2


/*
 * The max resolution of the box is limited, since we add uxmin and uxmax and
 * we need to avoid any unsigned addition overflow
 */
#if OCTBOX_USE_UINT64
   #if OCTBOX_USE_HIGHRES
      #define OBBOX_MAXRES                (UINT64_MAX/2-1)
      #define OBBOX_CENTER(_cmin,_cmax)   (((_cmin)>>1) + ((_cmax)>>1))
   #else
      #define OBBOX_MAXRES                (INT64_MAX/2+1)
      #define OBBOX_CENTER(_cmin,_cmax)   (((_cmin)+(_cmax))>>1)
   #endif
#else
   #if OCTBOX_USE_HIGHRES
      #define OBBOX_MAXRES                (UINT32_MAX/2-1)
      #define OBBOX_CENTER(_cmin,_cmax)   (((_cmin)>>1) + ((_cmax)>>1))
   #else
      #define OBBOX_MAXRES                (INT32_MAX/2+1)
      #define OBBOX_CENTER(_cmin,_cmax)   (((_cmin)+(_cmax))>>1)
   #endif
#endif

/*
 * Size of the hash used with the Z-Curve algorithm
 */
#define OHASH_SIZE      32768


#define OB_STARTUP(_ob)\
   const OCTBOX_C ux = OCTBOX_UX(_ob,rx);\
   const OCTBOX_C uy = OCTBOX_UY(_ob,ry);\
   const OCTBOX_C uz = OCTBOX_UZ(_ob,rz);

/*
 * Check if the ucoords are outside the domain 0...max.
 * A comparison  _ux < _box.b_xmin is not necessary, since min=0.
 */
#define OUTSIDE_BBOX_DOMAIN(_box,_ux,_uy,_uz)\
(\
   _ux > _box.b_xmax ||\
   _uy > _box.b_ymax ||\
   _uz > _box.b_zmax\
)

#define OUTSIDE_OCT_DOMAIN(_ob,_ux,_uy,_uz)\
(\
   _ux > (_ob)->root_box.b_xmax ||\
   _uy > (_ob)->root_box.b_ymax ||\
   _uz > (_ob)->root_box.b_zmax\
)

#define MOVE_INSIDE_OCT_DOMAIN(_ob,_ux,_uy,_uz)\
{\
   if (_ux > (_ob)->root_box.b_xmax) _ux = (_ob)->root_box.b_xmax;\
   if (_uy > (_ob)->root_box.b_ymax) _uy = (_ob)->root_box.b_ymax;\
   if (_uz > (_ob)->root_box.b_zmax) _uz = (_ob)->root_box.b_zmax;\
}

/*
 * Get the octant [0...7] based on the 3 bits of the coordinates.
 */
#define MORTON_GETOCTANT_1M(_octant,_ux,_uy,_uz,_mask)\
   _octant = 0;\
   if (_ux&_mask) _octant |= OMASK_HIGHX;\
   if (_uy&_mask) _octant |= OMASK_HIGHY;\
   if (_uz&_mask) _octant |= OMASK_HIGHZ


#define MORTON_GETOCTANT_3M(_octant,_ux,_uy,_uz,_maskx,_masky,_maskz)\
   _octant = 0;\
   if (_ux&_maskx) _octant |= OMASK_HIGHX;\
   if (_uy&_masky) _octant |= OMASK_HIGHY;\
   if (_uz&_maskz) _octant |= OMASK_HIGHZ


/*
 * Get the octant [0...7] based on the 3 bits of the coordinates.
 * and decrement the bit no.
 */
#define MORTON_NEWOCTANT_1M(_octant,_ux,_uy,_uz,_mask)\
   _octant = 0;\
   if (_ux&_mask) _octant |= OMASK_HIGHX;\
   if (_uy&_mask) _octant |= OMASK_HIGHY;\
   if (_uz&_mask) _octant |= OMASK_HIGHZ;\
   _mask >>= 1;


#define MORTON_NEWOCTANT_3M(_octant,_ux,_uy,_uz,_maskx,_masky,_maskz)\
   _octant = 0;\
   if (_ux&_maskx) _octant |= OMASK_HIGHX; _maskx >>= 1;\
   if (_uy&_masky) _octant |= OMASK_HIGHY; _masky >>= 1;\
   if (_uz&_maskz) _octant |= OMASK_HIGHZ; _maskz >>= 1



/*
 * Find box octant [0...7] for point xyz.
 * RULE: The midpoint itself is part of the left side.
 *    x-left: [0,xmid]   x-right: ]xmid,whatever]
 */
#define OBBOX_GETOCTANT(_box,_ux,_uy,_uz,_octant)\
   _octant = 0;\
   if (_ux > _box.b_xmid) _octant |= OMASK_HIGHX;\
   if (_uy > _box.b_ymid) _octant |= OMASK_HIGHY;\
   if (_uz > _box.b_zmid) _octant |= OMASK_HIGHZ;

/*
 * Get the octant [0...7] for the coordinates.
 * And change the box dimensions for this octant.
 * RULE: The midpoint itself is part of the lower left side.
 */
#define OBBOX_NEWOCTANT(_box,_ux,_uy,_uz,_octant)\
{\
   _octant = 0;\
\
   if (_ux > _box.b_xmid)\
   {\
      _box.b_xmin = _box.b_xmid+1;\
       _octant |= OMASK_HIGHX;\
   }\
   else\
   {\
      _box.b_xmax = _box.b_xmid;\
   }\
   _box.b_xmid=OBBOX_CENTER(_box.b_xmin,_box.b_xmax);\
\
   if (_uy > _box.b_ymid)\
   {\
      _box.b_ymin = _box.b_ymid+1;\
      _octant |= OMASK_HIGHY;\
   }\
   else\
   {\
      _box.b_ymax = _box.b_ymid;\
    }\
   _box.b_ymid=OBBOX_CENTER(_box.b_ymin,_box.b_ymax);\
\
   if (_uz > _box.b_zmid)\
   {\
      _box.b_zmin = _box.b_zmid+1;\
      _octant |= OMASK_HIGHZ;\
   }\
   else\
   {\
      _box.b_zmax = _box.b_zmid;\
   }\
   _box.b_zmid=OBBOX_CENTER(_box.b_zmin,_box.b_zmax);\
}

#define OBBOX_SETOCTANT(_box,_octant)\
{\
   if (_octant & OMASK_HIGHX)\
   {\
      _box.b_xmin = _box.b_xmid+1;\
   }\
   else\
   {\
      _box.b_xmax = _box.b_xmid;\
   }\
   _box.b_xmid=OBBOX_CENTER(_box.b_xmin,_box.b_xmax);\
\
   if (_octant & OMASK_HIGHY)\
   {\
      _box.b_ymin = _box.b_ymid+1;\
   }\
   else\
   {\
      _box.b_ymax = _box.b_ymid;\
   }\
   _box.b_ymid=OBBOX_CENTER(_box.b_ymin,_box.b_ymax);\
\
   if (_octant & OMASK_HIGHZ)\
   {\
      _box.b_zmin = _box.b_zmid+1;\
   }\
   else\
   {\
      _box.b_zmax = _box.b_zmid;\
   }\
   _box.b_zmid=OBBOX_CENTER(_box.b_zmin,_box.b_zmax);\
}


/*
 * Returns true, if the small rbox does NOT(!) overlap with the big bbox at all.
 */
#define OBBOX_OVERLAP_EQ0(_bbox,_rbox)\
(\
   _rbox.r_xmin > _bbox.b_xmax || _rbox.r_xmax < _bbox.b_xmin ||\
   _rbox.r_ymin > _bbox.b_ymax || _rbox.r_ymax < _bbox.b_ymin ||\
   _rbox.r_zmin > _bbox.b_zmax || _rbox.r_zmax < _bbox.b_zmin\
)

/*
 * Returns true, if the small rbox overlaps with more than 1 octant of the big bbox,
 * which means the center of the big box is part of the smaller range box.
 * RULE: The midpoint itself is part of the lower left side.
 */
#define OBBOX_OVERLAP_GT1(_bbox,_rbox)\
(\
   (_rbox.r_xmin <= _bbox.b_xmid && _rbox.r_xmax > _bbox.b_xmid) ||\
   (_rbox.r_ymin <= _bbox.b_ymid && _rbox.r_ymax > _bbox.b_ymid) ||\
   (_rbox.r_zmin <= _bbox.b_zmid && _rbox.r_zmax > _bbox.b_zmid)\
)

/*
 * Returns true, if the small rbox overlaps with NOT(!) more than 1 octant of the big bbox,
 * which means the center of the big box is NOT(!) part of the smaller range box.
 * RULE: The midpoint itself is part of the lower left side.
 *  OBBOX_OVERLAP_LE1() == !OBBOX_OVERLAP_GT1()
 */
#define OBBOX_OVERLAP_LE1(_bbox,_rbox)\
(\
   (_rbox.r_xmin > _bbox.b_xmid || _rbox.r_xmax <= _bbox.b_xmid) &&\
   (_rbox.r_ymin > _bbox.b_ymid || _rbox.r_ymax <= _bbox.b_ymid) &&\
   (_rbox.r_zmin > _bbox.b_zmid || _rbox.r_zmax <= _bbox.b_zmid)\
)

/*
 * Returns the max. of 3 values
 */
#define VMAX3(v1,v2,v3)\
(\
   (v1>v2) ? ((v1>v3)?v1:v3)\
           : ((v2>v3)?v2:v3)\
)


#define ONODE_IS_LEAF(_onode)        (_onode->bleaf==BLEAF_TRUE)
#define ONODE_IS_BRANCH(_onode)      (_onode->bleaf!=BLEAF_TRUE)
#define ONODE_TRUE_BRANCH(_onode)    (_onode && ONODE_IS_BRANCH(_onode))

#define OLEAF_NEW(_ipool,_oleaf,_rx,_ry,_rz,_ux,_uy,_uz,_idx,_flags)\
   IPOOL_GETITEM_ISIZE(_ipool,_oleaf,OLEAF);\
   _oleaf->l_bleaf = BLEAF_TRUE; /* child[0] == (void *)1 signals a leaf node */\
   _oleaf->l_next  = NULL;\
   _oleaf->l_rx    = _rx;\
   _oleaf->l_ry    = _ry;\
   _oleaf->l_rz    = _rz;\
   _oleaf->l_ux    = _ux;\
   _oleaf->l_uy    = _uy;\
   _oleaf->l_uz    = _uz;\
   _oleaf->l_flags = _flags;\
   _oleaf->l_idx   = _idx;

#define ONODE_DUP(_ipool,_onode_dup,_onode_src)\
   IPOOL_GETITEM_SITEM(_ipool,_onode_dup,ONODE);\
   *_onode_dup = *_onode_src /* struct copy */

#define PUSH_LEAF_LIST(_leafList,_oleaf)\
   _leafList->ll_list[1] = _leafList->ll_list[0];\
   _leafList->ll_list[0] = _oleaf

/*
 * Internal helper function, at most inline stuff
 */

/****************************************************************************************/
static unsigned ob_uinthighbit(OCTBOX_C uval)
/****************************************************************************************/
/*
 * Returns the number of the highest non 0 bit of an unsigned.
 * Returns 0...31 (for 32 bits) and 0...63 (for 64 bits)
 */
{
   unsigned highbit = 0;

   while((uval>>=1))
   {
      highbit++;
   }
   return highbit;
}

/****************************************************************************************/
static OCTBOX_C ob_nextpow2(const OCTBOX_C uval)
/****************************************************************************************/
/*
 * Returns the smallest number 2^n >= uval, n= 0...31
 */
{
   OCTBOX_C p2 = 1; /* Start with 2^0 = 0x00000001 */

   while(p2 < uval)
   {
      p2 <<= 1;
   }
   return p2;
}

/****************************************************************************************/
static const char *ob_flags2str(const unsigned flags, char descr[128])
/****************************************************************************************/
/*
 * Returns a string description of the OCTBOX_setup_x(...,flags,...)
 */
{
   switch(flags & OCTBOX_MASK_MESH)
   {
      case OCTBOX_FLAG_POINT  : strcpy(descr,"point")       ; break;
      case OCTBOX_FLAG_NODEC  : strcpy(descr,"node")        ; break;
      case OCTBOX_FLAG_ECENT  : strcpy(descr,"element")     ; break;
      case OCTBOX_FLAG_NDELC  : strcpy(descr,"NE-both")     ; break;
      default:                  strcpy(descr,"*unknown*")   ; break;
   }
   switch(flags & OCTBOX_TYPE_MASK)
   {
      case OCTBOX_TYPE_GEOM   : strcat(descr,", geometric") ; break;
      case OCTBOX_TYPE_GEOMZ  : strcat(descr,", zhash/geom"); break;
      case OCTBOX_TYPE_MORTON : strcat(descr,", morton")    ; break;
      case OCTBOX_TYPE_MORTON3: strcat(descr,", morton3")   ; break;
      case OCTBOX_TYPE_ZHASH  : strcat(descr,", zhash")     ; break;
      case OCTBOX_TYPE_ZHASH3 : strcat(descr,", zhash3")    ; break;
      case OCTBOX_TYPE_ZBINARY: strcat(descr,", zbinary")   ; break;
      case OCTBOX_TYPE_KDTREE:  strcat(descr,", kdtree")    ; break;
      default:                  strcat(descr,", *BAD*")     ; break;
   }

   if (flags & OCTBOX_FBIT_CUBE    ) strcat(descr,", cube");
   if (flags & OCTBOX_FBIT_POW2    ) strcat(descr,", pow2");
   if (flags & OCTBOX_FBIT_KEEPLIST) strcat(descr,", keeplist");
   if (flags & OCTBOX_FBIT_COMPACT ) strcat(descr,", compact");
   if (flags & OCTBOX_FBIT_SORTED  ) strcat(descr,", z-sorted");

   return descr;
}

/****************************************************************************************/
inline static unsigned ob_15_bit_zcode
(
   const OCTBOX_C ux5,
   const OCTBOX_C uy5,
   const OCTBOX_C uz5
)
/****************************************************************************************/
/*
 * Generate a Morton bit interleaving code ( 3 x 5 bits ) starting with the fifth bit out
 * of bits 0...4. All values ux5 are already >>t'ed and contain only the topmost 5 bits.
 */
{
#if XYZ_ORDER

   /* xyz xyz xyz xyz xyz */
   return
         ((ux5 & 0x0010)<< 10) |
         ((ux5 & 0x0008)<<  8) |
         ((ux5 & 0x0004)<<  6) |
         ((ux5 & 0x0002)<<  4) |
         ((ux5 & 0x0001)<<  2) |

         ((uy5 & 0x0010)<<  9) |
         ((uy5 & 0x0008)<<  7) |
         ((uy5 & 0x0004)<<  5) |
         ((uy5 & 0x0002)<<  3) |
         ((uy5 & 0x0001)<<  1) |

         ((uz5 & 0x0010)<<  8) |
         ((uz5 & 0x0008)<<  6) |
         ((uz5 & 0x0004)<<  4) |
         ((uz5 & 0x0002)<<  2) |
         ((uz5 & 0x0001)     );

#else

   /* zyx zyx zyx zyx zyx */
   return
         ((uz5 & 0x0010)<< 10) |
         ((uz5 & 0x0008)<<  8) |
         ((uz5 & 0x0004)<<  6) |
         ((uz5 & 0x0002)<<  4) |
         ((uz5 & 0x0001)<<  2) |

         ((uy5 & 0x0010)<<  9) |
         ((uy5 & 0x0008)<<  7) |
         ((uy5 & 0x0004)<<  5) |
         ((uy5 & 0x0002)<<  3) |
         ((uy5 & 0x0001)<<  1) |

         ((ux5 & 0x0010)<<  8) |
         ((ux5 & 0x0008)<<  6) |
         ((ux5 & 0x0004)<<  4) |
         ((ux5 & 0x0002)<<  2) |
         ((ux5 & 0x0001)     );

#endif
}

/***************************************************************************************/

/***************************************************************************************/
static void ob_rbox_setup
(
   ORBOX         *rbox,
   const OCTBOX_C ux,
   const OCTBOX_C uy,
   const OCTBOX_C uz,
   const unsigned srange
)
/***************************************************************************************/
/*
 * Setup a search range box around a point uxyz with a range srange
 */
{
   /* Avoid unsigned underflow */
   rbox->r_xmin = (ux > srange) ? ux-srange : 0;
   rbox->r_ymin = (uy > srange) ? uy-srange : 0;
   rbox->r_zmin = (uz > srange) ? uz-srange : 0;

   /* Overflow should not happen */
   rbox->r_xmax = (rbox->r_ux=ux) + srange;
   rbox->r_ymax = (rbox->r_uy=uy) + srange;
   rbox->r_zmax = (rbox->r_uz=uz) + srange;

   rbox->r_range  = srange;
   rbox->r_range2 = srange*srange;
#if 0
printf("RBOX-setup : %u\n",srange);
#endif
}

/***************************************************************************************/
static unsigned ob_rbox_shrink
(
   ORBOX       *rbox,
   const double dist2
)
/***************************************************************************************/
/*
 * Shrink the search range of a range box into sqrt(dist2),
 * but never increase the search range.
 */
{
   if (dist2 < rbox->r_range2)
   {
      const unsigned srange = CAST_UINT(sqrt(dist2))+1;

      if (srange < rbox->r_range)
      {
         /* Avoid unsigned underflow */
         rbox->r_xmin = (rbox->r_ux > srange) ? rbox->r_ux-srange : 0;
         rbox->r_ymin = (rbox->r_uy > srange) ? rbox->r_uy-srange : 0;
         rbox->r_zmin = (rbox->r_uz > srange) ? rbox->r_uz-srange : 0;

         rbox->r_xmax = rbox->r_ux + srange;
         rbox->r_ymax = rbox->r_uy + srange;
         rbox->r_zmax = rbox->r_uz + srange;

#if 0
printf("      RBOX-shrink: by %u to %u\n",rbox->r_range - srange,srange);
#endif

         rbox->r_range  = srange;
         rbox->r_range2 = dist2;
         return 1;
      }

#if 0
if (srange>rbox->r_range) puts("RBOX-blowup");
#endif
   }

   return 0;
}

/***************************************************************************************/

/***************************************************************************************/
inline static void ob_zbox_setup
(
   OZBOX         *zbox,
   const OCTBOX_C ux,
   const OCTBOX_C uy,
   const OCTBOX_C uz,
   const unsigned srange,
   const unsigned hbitx4,
   const unsigned hbity4,
   const unsigned hbitz4

)
/***************************************************************************************/
/*
 * Setup a search range xyz-box around a point uxyz with a range srange
 */
{
   ob_rbox_setup(&(zbox->rbox),ux,uy,uz,srange);
   zbox->z_min = ob_15_bit_zcode
                  (
                     zbox->rbox.r_xmin>>hbitx4,
                     zbox->rbox.r_ymin>>hbity4,
                     zbox->rbox.r_zmin>>hbitz4
                  );
   zbox->z_max = ob_15_bit_zcode
                  (
                     zbox->rbox.r_xmax>>hbitx4,
                     zbox->rbox.r_ymax>>hbity4,
                     zbox->rbox.r_zmax>>hbitz4
                  );
}

/***************************************************************************************/
inline static unsigned ob_zbox_shrink
(
   OZBOX         *zbox,
   const double   dist2,
   const unsigned hbitx4,
   const unsigned hbity4,
   const unsigned hbitz4
)
/***************************************************************************************/
/*
 * Shrink the search range of a range box into sqrt(dist2),
 * but never increase the search range.
 */
{
   if (ob_rbox_shrink(&(zbox->rbox),dist2))
   {
      zbox->z_min = ob_15_bit_zcode
                     (
                        zbox->rbox.r_xmin>>hbitx4,
                        zbox->rbox.r_ymin>>hbity4,
                        zbox->rbox.r_zmin>>hbitz4
                     );
      zbox->z_max = ob_15_bit_zcode
                     (
                        zbox->rbox.r_xmax>>hbitx4,
                        zbox->rbox.r_ymax>>hbity4,
                        zbox->rbox.r_zmax>>hbitz4
                     );
      return 1;
   }

   return 0;
}

/***************************************************************************************/

/***************************************************************************************/
forceinline int ob_oleaf_cmp_morton
(
   const OLEAF *p1,
   const OLEAF *p2,
   unsigned    zmask
)
/***************************************************************************************/
/*
 * Compare two OLEAF objects according to Morton Z-curve order. Returns ...
 *    +n: p1>p2
 *    -n: p1<p2
 *     0: p1=p2
 */
{
    for(; zmask; zmask>>=1)
    {
        register int bitdiff;
      #if XYZ_ORDER
        if ((bitdiff = CAST_INT(p1->l_ux&zmask)-CAST_INT(p2->l_ux&zmask)) != 0) return bitdiff;
        if ((bitdiff = CAST_INT(p1->l_uy&zmask)-CAST_INT(p2->l_uy&zmask)) != 0) return bitdiff;
        if ((bitdiff = CAST_INT(p1->l_uz&zmask)-CAST_INT(p2->l_uz&zmask)) != 0) return bitdiff;
      #else
        if ((bitdiff = CAST_INT(p1->l_uz&zmask)-CAST_INT(p2->l_uz&zmask)) != 0) return bitdiff;
        if ((bitdiff = CAST_INT(p1->l_uy&zmask)-CAST_INT(p2->l_uy&zmask)) != 0) return bitdiff;
        if ((bitdiff = CAST_INT(p1->l_ux&zmask)-CAST_INT(p2->l_ux&zmask)) != 0) return bitdiff;
      #endif
    }

    return 0;
}

/***************************************************************************************/

/****************************************************************************************/
forceinline double ob_leaf_get_udist2
(
   const OLEAF   *oleaf,
   const OCTBOX_C ux,
   const OCTBOX_C uy,
   const OCTBOX_C uz
)
/****************************************************************************************/
/*
 * Returns the distance^2 between a leaf and the point uxyz as a double
 */
{
   const int64_t dx = CAST_INT64(oleaf->l_ux) - CAST_INT64(ux);
   const int64_t dy = CAST_INT64(oleaf->l_uy) - CAST_INT64(uy);
   const int64_t dz = CAST_INT64(oleaf->l_uz) - CAST_INT64(uz);
   return CAST_DOUBLE(dx*dx) + CAST_DOUBLE(dy*dy) + CAST_DOUBLE(dz*dz);
}

/****************************************************************************************/
forceinline const OLEAF *ob_leaf_best_rdist2
(
   const OLEAF *oleaf,
   const real   rx,
   const real   ry,
   const real   rz,
   double      *bestDist2
)
/****************************************************************************************/
/*
 * Returns true/false if the distance^2 between a leaf->rxyz and the point rxyz is
 * smaller/larger than the bestDist2. Update bestDist2 in case of true.
 * Calculating distance^2 may cause an int overflow, so we do an early stop.
 */
{
   const double bd2 = *bestDist2;
   double       dd;


   const double dx = oleaf->l_rx - rx;
   if ((dd=dx*dx) < bd2)
   {
      const double dy = oleaf->l_ry - ry;
      if ((dd+=dy*dy) < bd2)
      {
         const double dz = oleaf->l_rz - rz;
         if ((dd+=dz*dz) < bd2)
         {
            *bestDist2 = dd;
            return oleaf;  /* return true = smaller dist^2 found */
         }
      }
   }
   return NULL; /* return false */
}

/****************************************************************************************/

#if 0
/****************************************************************************************/
inline static void ob_15_bit_bbox1
(
   OBBOX         *box,
   const unsigned ux,
   const unsigned uy,
   const unsigned uz,
   const unsigned hbit4
)
/****************************************************************************************/
{
   unsigned mask = 0xffffffff >> (32-hbit4);
   unsigned v;

   if ((v=ux|mask) < box->b_xmax) box->b_xmax = v;
   if ((v=uy|mask) < box->b_ymax) box->b_ymax = v;
   if ((v=uz|mask) < box->b_zmax) box->b_zmax = v;

   mask = ~mask;
   if ((v=ux&mask) > box->b_xmin) box->b_xmin = v;
   if ((v=uy&mask) > box->b_ymin) box->b_ymin = v;
   if ((v=uz&mask) > box->b_zmin) box->b_zmin = v;

   box->b_xmid = OBBOX_CENTER(box->b_xmin,box->b_xmax);
   box->b_ymid = OBBOX_CENTER(box->b_ymin,box->b_ymax);
   box->b_zmid = OBBOX_CENTER(box->b_zmin,box->b_zmax);

#if 0 /* Unit test */
   if (box->b_xmid)
   {
      printf("hbit4    = %d\n",hbit4);
      printf("mask     = 10987654321098765432109876543210\n");
      mask = (~0u);
      printf("~0       = %32s\n",ulong2bin(mask,0,NULL,0));
      mask = mask >> (32-hbit4);
      printf("mask-max = %32s\n",ulong2bin(mask,0,NULL,0));
      mask = ~mask;
      printf("mask-min = %32s\n",ulong2bin(mask,0,NULL,0));
      printf("ux       = %32s, %7u\n",ulong2bin(ux         ,0,NULL,0),ux);
      printf("ux-min   = %32s, %7u\n",ulong2bin(box->b_xmin,0,NULL,0),box->b_xmin);
      printf("ux-max   = %32s, %7u\n",ulong2bin(box->b_xmax,0,NULL,0),box->b_xmax);
      printf("uy       = %32s, %7u\n",ulong2bin(uy         ,0,NULL,0),uy);
      printf("uy-min   = %32s, %7u\n",ulong2bin(box->b_ymin,0,NULL,0),box->b_ymin);
      printf("uy-max   = %32s, %7u\n",ulong2bin(box->b_ymax,0,NULL,0),box->b_ymax);
      printf("uz       = %32s, %7u\n",ulong2bin(uz         ,0,NULL,0),uz);
      printf("uz-min   = %32s, %7u\n",ulong2bin(box->b_zmin,0,NULL,0),box->b_zmin);
      printf("uz-max   = %32s, %7u\n",ulong2bin(box->b_zmax,0,NULL,0),box->b_zmax);
      exit(1);
   }
#endif
}

/****************************************************************************************/
#endif

/****************************************************************************************/
forceinline static const OLEAF *ob_leaf_list_append
(
   IPOOL         *ipool,
   OLEAF         *head,
   const real     rx,
   const real     ry,
   const real     rz,
   const OCTBOX_C ux,
   const OCTBOX_C uy,
   const OCTBOX_C uz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
{
   OLEAF *oleaf;
   OLEAF_NEW(ipool,oleaf,rx,ry,rz,ux,uy,uz,idx,flags);

   oleaf->l_next = head->l_next;
   head->l_next  = oleaf;
   return oleaf;
}

/****************************************************************************************/
forceinline const OLEAF *ob_leaf_list_rscan2
(
   const OLEAF *oleaf,
   const real   rx,
   const real   ry,
   const real   rz,
   double      *bestDist2
)
/****************************************************************************************/
{
   const OLEAF *bestLeaf = NULL;

   for(; oleaf; oleaf=oleaf->l_next)
   {
      if (ob_leaf_best_rdist2(oleaf,rx,ry,rz,bestDist2))
         bestLeaf = oleaf;
   }

   return bestLeaf;
}

/****************************************************************************************/


/****************************************************************************************/
static const OLEAF *ob_find_from_here_nearest
(
   const ONODE *node,
   const real   rx,
   const real   ry,
   const real   rz,
   double      *bestDist2
)
/****************************************************************************************/
/*
 * Try to find a leaf node and update the bestDist2
 */
{
   const OLEAF *bestLeaf = NULL;
   const ONODE *stack_node[32];   /* 32 == No. of bits for OBBOX_MAXRES */
   unsigned     stack_octant[32];
   unsigned     octant;
   int          nstack;


   if (ONODE_IS_LEAF(node))
   {
      /* Update the distance of the leaf found */
      const OLEAF *oleaf = &(node->oleaf);
      return (oleaf->l_next)
         ? ob_leaf_list_rscan2(oleaf,rx,ry,rz,bestDist2)
         : ob_leaf_best_rdist2(oleaf,rx,ry,rz,bestDist2);
   }

   nstack = 0;

CALL_FUNCTION:
   for(octant=0; octant<8; octant++)
   {
      const ONODE *child;

      if ((child=node->child[octant]) != NULL)
      {
         if (ONODE_IS_LEAF(child))
         {
            /* Update the distance of the leaf found */
            const OLEAF *oleaf = &(child->oleaf);
            if (oleaf->l_next)
            {
               if ((oleaf=ob_leaf_list_rscan2(oleaf,rx,ry,rz,bestDist2)) != NULL)
               {
                  bestLeaf = oleaf;
               }
            }
            else if (ob_leaf_best_rdist2(oleaf,rx,ry,rz,bestDist2))
            {
               bestLeaf = oleaf;
            }
         }
         else /* branch node: start a recursion, but without recursive calls */
         {
            stack_node[nstack]   = node;
            stack_octant[nstack] = octant;
            nstack++;

            /* Restart the loop and use the branch-child as the root */
            node = child;
            goto CALL_FUNCTION;  /* recursive call(node) */

         FUNCTION_RETURN:
            if (--nstack < 0)
               return bestLeaf;

            node   = stack_node[nstack];
            octant = stack_octant[nstack];
         }
      }
   }
   goto FUNCTION_RETURN;
}

/****************************************************************************************/
static const OLEAF *ob_find_from_here_approximate
(
   const ONODE *node,
   const real   rx,
   const real   ry,
   const real   rz
)
/****************************************************************************************/
/*
 * Try to find a leaf node starting at root-node node by just walking down.
 * We'll end up in an endless loop if there is NO leaf at the bottom of the tree :))
 */
{
   /* Repeat until we reach a leaf, get killed or we have the CO2 catastrophe */
   for(;;) /* for(i=0; i<no-of-bits-of-unsigned(32); i++) */
   {
      ONODE * const *child = node->child;

      /*
       * Prefer a random search order (randowm walk along the pathes) with each step,
       * which we haven't implemented here, we're so sad :((
       */
      if (
            (node=child[4]) != NULL ||
            (node=child[3]) != NULL ||
            (node=child[5]) != NULL ||
            (node=child[2]) != NULL ||
            (node=child[6]) != NULL ||
            (node=child[1]) != NULL ||
            (node=child[7]) != NULL ||
            (node=child[0]) != NULL
         )
      {
         if (ONODE_IS_LEAF(node))
         {
            /* Return the APPROXIMATE best leaf on our path */
            const OLEAF *oleaf = &(node->oleaf);
            if (oleaf->l_next)
            {
               double bestDist2 = DBL_MAX;
               return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
            }
            return oleaf;
         }
      }
   }
}

/****************************************************************************************/

/*
 * The impossible stuff
 */
/****************************************************************************************/
static const OLEAF *ob_insert_impossible
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Dummy/error insert: Used/hooked after the IPOOL data manipulation and impossible
 *                     insertion to avoid any further point insertion.
 */
{
   if (ob||rx||ry||rz||idx||flags)
   {
      XMSG_MESSAGE
      (
         XMSG_LEVEL_FATAL,
         "   OCTBOX_insert(\"%s\"): Internal error:\n"
         "      Insertion impossible after call of OCTBOX_close().\n"
         ,ob->name
      );
   }
   return NULL; /* keeps compiler happy */
}

/****************************************************************************************/
static const OLEAF *ob_find_impossible
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Dummy/error find: Used/hooked if find is impossible
 */
{
   if (ob||rx||ry||rz||nearest)
   {
      XMSG_MESSAGE
      (
         XMSG_LEVEL_FATAL,
         "   OCTBOX_find(\"%s\"): Internal error:\n"
         "      Find impossible after call of OCTBOX_close().\n"
         ,ob->name
      );
   }
   return NULL; /* keeps compiler happy */
}

/****************************************************************************************/
static const OLEAF *ob_search_impossible
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
/*
 * Dummy/error find: Used/hooked if find is impossible
 */
{
   if (ob||rx||ry||rz||srange)
   {
      XMSG_MESSAGE
      (
         XMSG_LEVEL_FATAL,
         "   OCTBOX_search(\"%s\"): Internal error:\n"
         "      Search impossible after call of OCTBOX_close().\n"
         ,ob->name
      );
   }
   return NULL; /* keeps compiler happy */
}

/***************************************************************************************/

/*
 * Geometric stuff
 */
#define GEOM_WITH_ZHASH  0

/***************************************************************************************/
static const OLEAF *ob_insert_geom
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insert a new point with index 'idx', flags and optional object pointer into the
 * octal box-tree.
 * Returns a pointer to either an existing leaf, the new inserted leaf or NULL
 * in case of error (ux/uy/uz out of the octbox range).
 */
{
   ONODE *node = &(ob->root_node); /* Always a branch node, never leaf */
   IPOOL *pool = &(ob->node_pool);
   OBBOX  bbox = ob->root_box; /* struct copy */
   OB_STARTUP(ob)
#if OCTBOX_USE_RLOG
   unsigned depth = 0;
#endif

#if 0
XMSG_DEBUG3("OCTBOX_insert((%12u,%12u,%12u)\n",ux,uy,uz);
#endif

#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_BBOX_DOMAIN(bbox,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

#if GEOM_WITH_ZHASH

   if (ob->node_hash)
   {
      const unsigned zcode = ob_15_bit_zcode
                              (
                                 ux>>ob->max_hbit4,
                                 uy>>ob->max_hbit4,
                                 uz>>ob->max_hbit4
                              );

      if ((node=ob->node_hash[zcode]) == NULL)
      {
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         ob->node_hash[zcode] = RCAST_INTO(ONODE *,leaf);
         ob->nleaves++;
         return leaf;
      }
      ob_15_bit_bbox1(&bbox,ux,uy,uz,ob->max_hbit4);
   }

#endif

   /* Walk down until 'node' has no child. */
   for(;;)
   {
      ONODE   *child;
      unsigned octant; /* 0 ... 7 */

   #if OCTBOX_USE_RLOG
      depth++;
   #endif

      if (ONODE_IS_LEAF(node)) /* this is a leaf type node */
      {
         /*
          * The 'node' is already a leaf type node. Make a copy of
          * itself one level below. Make 'node' a branch node.
          * First ensure that old and new nodal points are not identical.
          * In this case, we would walk down forever, since we can
          * never put the old and new nodal point into different octants.
          */
         OLEAF         *leaf   = &(node->oleaf);
         const OCTBOX_C leafux = leaf->l_ux;
         const OCTBOX_C leafuy = leaf->l_uy;
         const OCTBOX_C leafuz = leaf->l_uz;

         if (leafux == ux && leafuy == uy && leafuz == uz)
         {
            if (!ob->keeplist)
               return leaf; /* return the already existing leaf */

            ob->nleaves++;
            return ob_leaf_list_append(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         }

         /* Get suboctant of current leaf point */
         OBBOX_GETOCTANT(bbox,leafux,leafuy,leafuz,octant);

         /* Duplicate the leaf node */
         ONODE_DUP(pool,child,node);

         /* Make 'node' a branch node with just one leaf child */
         node->child[7] =
         node->child[6] =
         node->child[5] =
         node->child[4] =
         node->child[3] =
         node->child[2] =
         node->child[1] =
         node->child[0] = NULL;
         node->child[octant] = child;
      }

      /*
       * Get suboctant of point and octant's child node of 'node'
       */
      OBBOX_NEWOCTANT(bbox,ux,uy,uz,octant)
      if ((child=node->child[octant]) == NULL)
      {
         /* We don't have a child yet, just append a new leaf type node */
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         node->child[octant] = RCAST_INTO(ONODE *,leaf);
      #if OCTBOX_USE_RLOG
         if (depth > ob->max_depth) ob->max_depth = depth;
      #endif
         ob->nleaves++;
         return leaf; /* return the new leaf */
      }

      /* There is already a child node. walk one level down, ... */
      node = child; /* make child the current node */
   }
}

/****************************************************************************************/
static const OLEAF *ob_find_geom
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Try to find a nearest leaf node with these coordinates.
 * Nearest match here means APPROXIMATE nearest match along the tree!
 * This is in fact a quick&dirty nearest match, but extremly fast.
 *
 * It may return a NULL pointer in case there is no leaf along the path.
 */
{
   const ONODE *node   = &(ob->root_node);
   const ONODE *parent = node;
   OBBOX        bbox   = ob->root_box; /* struct copy */
   OB_STARTUP(ob)

#if 0
XMSG_DEBUG3("ob_find_geom(%12u,%12u,%12u)\n",ux,uy,uz);
#endif

#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_BBOX_DOMAIN(bbox,ux,uy,uz))
      return NULL;
#endif

#if GEOM_WITH_ZHASH

   if (ob->node_hash)
   {
      const unsigned zcode = ob_15_bit_zcode
                              (
                                 ux>>ob->max_hbit4,
                                 uy>>ob->max_hbit4,
                                 uz>>ob->max_hbit4
                              );

      ob_15_bit_bbox1(&bbox,ux,uy,uz,ob->max_hbit4);
      node = ob_zhash_find_node(ob,zcode);
   }

#endif

   /* Walk down until we reach a leaf or there are no more childs */
   while(ONODE_TRUE_BRANCH(node))
   {
      unsigned octant;
      OBBOX_NEWOCTANT(bbox,ux,uy,uz,octant)
      parent = node;
      node = parent->child[octant];
   }

   if (node)
   {
      /* Great, got a leaf */
      const OLEAF *oleaf = &(node->oleaf);
      if (oleaf->l_next)
      {
         double bestDist2 = DBL_MAX;
         return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
      }
      return oleaf;
   }

   if (nearest)
   {
      /* Find true nearest */
      double bestDist2 = DBL_MAX;
      return ob_find_from_here_nearest(parent,rx,ry,rz,&bestDist2);
   }

   /* Find an approximate leaf */
   return ob_find_from_here_approximate(parent,rx,ry,rz);
}

/****************************************************************************************/
static const OLEAF *ob_search_geom
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
/*
 * Try to find a leaf node in the neighborhood with the smallest distance
 * within the unsigned search range.
 */
{
   const ONODE *stack_node[32]; /* 32 == No. of bits for OBBOX_MAXRES */
   OBBOX        stack_bbox[32];
   unsigned     stack_octant[32];

   const OLEAF *bestLeaf = NULL;
   const ONODE *root_node = &(ob->root_node);
   const ONODE *child;
   OBBOX        root_bbox = ob->root_box; /* struct copy */
   ORBOX        range_rbox;
   double       bestDist2 = DBL_MAX;
   OB_STARTUP(ob)
   unsigned     octant;
   int          nstack = 0;


#if OCTBOX_CHECK_DOMAIN
   if (OUTSIDE_BBOX_DOMAIN(root_bbox,ux,uy,uz))
      return NULL;
#endif

#if GEOM_WITH_ZHASH

   if (ob->node_hash)
   {
      const unsigned zcode = ob_15_bit_zcode
                              (
                                 ux>>ob->max_hbit4,
                                 uy>>ob->max_hbit4,
                                 uz>>ob->max_hbit4
                              );

      ob_15_bit_bbox1(&root_bbox,ux,uy,uz,ob->max_hbit4);
   }

#endif

   if (!srange)
   {
      if ((bestLeaf=ob_find_geom(ob,rx,ry,rz,0)) == NULL)
         return NULL;

      /* Get the distance in the unsigned space */
      bestDist2 = ob_leaf_get_udist2(bestLeaf,ux,uy,uz);

      srange = CAST_UINT(sqrt(bestDist2)) + 1;
#if 0
printf("srange  => %u\n",srange);
#endif

      bestDist2 = DBL_MAX;
      ob_leaf_best_rdist2(bestLeaf,rx,ry,rz,&bestDist2);
   }

   ob_rbox_setup(&range_rbox,ux,uy,uz,srange);

   /*
    * As long as the range_rbox is only in one octant of the root_bbox,
    * we walk down the tree.
    */
   while(OBBOX_OVERLAP_LE1(root_bbox,range_rbox))
   {
      OBBOX_GETOCTANT(root_bbox,ux,uy,uz,octant);
      if ((child=root_node->child[octant]) == NULL)
         break; /* Dead end with the point: stop here */

      if (ONODE_IS_LEAF(child)) /* Must be the nearest */
      {
         const OLEAF *oleaf = &(child->oleaf);
         return (oleaf->l_next)
                  ? ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2)
                  : oleaf;
      }

      OBBOX_SETOCTANT(root_bbox,octant)
      root_node = child;
      nstack++;
   }
#if 0
printf("    root_level: %2d\n",nstack);
#endif

   /*
    * Start from the root_node with the root_bbox
    * The max. distance^2 can be no more than: srange*srange + srange*srange + srange*srange
    */
   nstack = 0;

LABEL_CALL_FUNCTION: /* (root_node,root_bbox) */
   for(octant=0; octant<8; octant++)
   {
      if ((child=root_node->child[octant]) != NULL)
      {
         OBBOX bbox = root_bbox; /* struct copy */
         OBBOX_SETOCTANT(bbox,octant)
         if (!OBBOX_OVERLAP_EQ0(bbox,range_rbox))
         {
            if (ONODE_IS_LEAF(child))
            {
               const OLEAF *oleaf = &(child->oleaf);
               if (oleaf->l_next)
               {
                  oleaf = ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
               }
               else if (!ob_leaf_best_rdist2(oleaf,rx,ry,rz,&bestDist2))
               {
                  oleaf = NULL;
               }

               if (oleaf)
               {
                  bestLeaf = oleaf;
                  if (ob_rbox_shrink(&range_rbox,ob_leaf_get_udist2(oleaf,ux,uy,uz)))
                  {
                     if (OBBOX_OVERLAP_EQ0(root_bbox,range_rbox))
                     {
                        goto LABEL_FUNCTION_RETURN;
                     }
                  }
               }
            }
            else /* branch node: start a recursion, but without recursive calls */
            {
               stack_node[nstack]   = root_node;
               stack_bbox[nstack]   = root_bbox; /* struct copy */
               stack_octant[nstack] = octant;
               nstack++;

               /* Restart the loop and use the branch-child as the root */
               root_node = child;
               root_bbox = bbox;    /* struct copy */
               goto LABEL_CALL_FUNCTION;  /* recursive call(root_node,root_bbox) */

            LABEL_FUNCTION_RETURN:
               if (--nstack < 0)
               {
                  return bestLeaf;
               }

               root_node = stack_node[nstack];
               root_bbox = stack_bbox[nstack];
               octant    = stack_octant[nstack];
            }
         }
      }
   }

   /* End of loop: Jump into for-loop again if recursion has not finished */
   goto LABEL_FUNCTION_RETURN;
}

/****************************************************************************************/


/*
 * Morton code (Z-curve) stuff
 */
/***************************************************************************************/
static const OLEAF *ob_insert_morton
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insert a new point with index 'idx', flags and optional object pointer into the
 * octal box-tree.
 * Returns a pointer to either an existing leaf, the new inserted leaf or NULL
 * in case of error (ux/uy/uz out of the octbox range).
 */
{
   ONODE   *node = &(ob->root_node); /* Always a branch node, never leaf */
   IPOOL   *pool = &(ob->node_pool);
   OB_STARTUP(ob)
#if OCTBOX_USE_RLOG
   unsigned depth = 0;
#endif
   unsigned zmask = 1<<(ob->max_hbit);

#if 0
XMSG_DEBUG3("OCTBOX_insert((%12u,%12u,%12u)\n",ux,uy,uz);
#endif

#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   /* Walk down until 'node' has no child. */
   for(;;)
   {
      ONODE   *child;
      unsigned octant; /* 0 ... 7 */

   #if OCTBOX_USE_RLOG
      depth++;
   #endif

      /*
       * Get suboctant of point and octant's child node of 'node'
       */
      MORTON_NEWOCTANT_1M(octant,ux,uy,uz,zmask);
      if ((child=node->child[octant]) == NULL)
      {
         /* We don't have a child yet, just append a new leaf type node */
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         node->child[octant] = RCAST_INTO(ONODE *,leaf);
      #if OCTBOX_USE_RLOG
         if (depth > ob->max_depth) ob->max_depth = depth;
      #endif
         ob->nleaves++;
         return leaf; /* return the new leaf */
      }

      /* There is already a child node. walk one level down, ... */
      node = child; /* make child the current node */
      if (ONODE_IS_LEAF(node)) /* this is a leaf type node */
      {
         /*
          * The 'node' is already a leaf type node. Make a copy of
          * itself one level below. Make 'node' a branch node.
          * First ensure that old and new nodal points are not identical.
          * In this case, we would walk down forever, since we can
          * never put the old and new nodal point into different octants.
          */
         OLEAF         *leaf   = &(node->oleaf);
         const OCTBOX_C leafux = leaf->l_ux;
         const OCTBOX_C leafuy = leaf->l_uy;
         const OCTBOX_C leafuz = leaf->l_uz;

         if (leafux == ux && leafuy == uy && leafuz == uz)
         {
            if (!ob->keeplist)
               return leaf; /* return the already existing leaf */

            ob->nleaves++;
            return ob_leaf_list_append(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         }

         /* Get suboctant of current leaf point */
         MORTON_GETOCTANT_1M(octant,leafux,leafuy,leafuz,zmask);

         /* Duplicate the leaf node */
         ONODE_DUP(pool,child,node);

         /* Make 'node' a branch node with just one leaf child */
         node->child[7] =
         node->child[6] =
         node->child[5] =
         node->child[4] =
         node->child[3] =
         node->child[2] =
         node->child[1] =
         node->child[0] = NULL;
         node->child[octant] = child;
      }
   }
}

/****************************************************************************************/
static const OLEAF *ob_find_morton
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Try to find a nearest leaf node with these coordinates.
 * Nearest match here means APPROXIMATE nearest match along the tree!
 * This is in fact a quick&dirty nearest match, but extremly fast.
 *
 * It may return a NULL pointer in case there is no leaf along the path.
 */
{
   const ONODE *node   = &(ob->root_node);
   const ONODE *parent = node;
   OB_STARTUP(ob)
   unsigned     zmask  = 1<<(ob->max_hbit);

#if 0
XMSG_DEBUG3("ob_find_morton(%12u,%12u,%12u)\n",ux,uy,uz);
#endif

#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif

   /* Walk down until we reach a leaf or there are no more childs */
   while(ONODE_TRUE_BRANCH(node))
   {
      unsigned octant;
      MORTON_NEWOCTANT_1M(octant,ux,uy,uz,zmask);
      parent = node;
      node = parent->child[octant];
   }

   if (node)
   {
      /* Great, got a leaf */
      const OLEAF *oleaf = &(node->oleaf);
      if (oleaf->l_next)
      {
         double bestDist2 = DBL_MAX;
         return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
      }
      return oleaf;
   }

   if (nearest)
   {
      /* Find true nearest */
      double bestDist2 = DBL_MAX;
      return ob_find_from_here_nearest(parent,rx,ry,rz,&bestDist2);
   }

   /* Find an approximate leaf */
   return ob_find_from_here_approximate(parent,rx,ry,rz);
}

/****************************************************************************************/
static const OLEAF *ob_search_morton
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
{
   if (ob||rx||ry||rz||srange)
   {
      XMSG_FATAL0
      (
         "ob_search_morton: function not yet implemented.\n"
      );
   }
   return NULL;
}

/****************************************************************************************/


/*
 * Morton code (Z-curve) stuff with max. left aligned bits
 */
/***************************************************************************************/
static const OLEAF *ob_insert_morton3
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insert a new point with index 'idx', flags and optional object pointer into the
 * octal box-tree.
 * Returns a pointer to either an existing leaf, the new inserted leaf or NULL
 * in case of error (ux/uy/uz out of the octbox range).
 */
{
   ONODE   *node = &(ob->root_node); /* Always a branch node, never leaf */
   IPOOL   *pool = &(ob->node_pool);
   OB_STARTUP(ob)
#if OCTBOX_USE_RLOG
   unsigned depth = 0;
#endif
   unsigned maskx = 1<<(ob->hbitx);
   unsigned masky = 1<<(ob->hbity);
   unsigned maskz = 1<<(ob->hbitz);

#if 0
XMSG_DEBUG3("OCTBOX_insert((%12u,%12u,%12u)\n",ux,uy,uz);
#endif

#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   /* Walk down until 'node' has no child. */
   for(;;)
   {
      ONODE   *child;
      unsigned octant; /* 0 ... 7 */

   #if OCTBOX_USE_RLOG
      depth++;
   #endif

      /*
       * Get suboctant of point and octant's child node of 'node'
       */
      MORTON_NEWOCTANT_3M(octant,ux,uy,uz,maskx,masky,maskz);
      if ((child=node->child[octant]) == NULL)
      {
         /* We don't have a child yet, just append a new leaf type node */
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         node->child[octant] = RCAST_INTO(ONODE *,leaf);
      #if OCTBOX_USE_RLOG
         if (depth > ob->max_depth) ob->max_depth = depth;
      #endif
         ob->nleaves++;
         return leaf; /* return the new leaf */
      }

      /* There is already a child node. walk one level down, ... */
      node = child; /* make child the current node */
      if (ONODE_IS_LEAF(node)) /* this is a leaf type node */
      {
         /*
          * The 'node' is already a leaf type node. Make a copy of
          * itself one level below. Make 'node' a branch node.
          * First ensure that old and new nodal points are not identical.
          * In this case, we would walk down forever, since we can
          * never put the old and new nodal point into different octants.
          */
         OLEAF         *leaf   = &(node->oleaf);
         const OCTBOX_C leafux = leaf->l_ux;
         const OCTBOX_C leafuy = leaf->l_uy;
         const OCTBOX_C leafuz = leaf->l_uz;

         if (leafux == ux && leafuy == uy && leafuz == uz)
         {
            if (!ob->keeplist)
               return leaf; /* return the already existing leaf */

            ob->nleaves++;
            return ob_leaf_list_append(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         }

         /* Get suboctant of current leaf point */
         MORTON_GETOCTANT_3M(octant,leafux,leafuy,leafuz,maskx,masky,maskz);

         /* Duplicate the leaf node */
         ONODE_DUP(pool,child,node);

         /* Make 'node' a branch node with just one leaf child */
         node->child[7] =
         node->child[6] =
         node->child[5] =
         node->child[4] =
         node->child[3] =
         node->child[2] =
         node->child[1] =
         node->child[0] = NULL;
         node->child[octant] = child;
      }
   }
}

/****************************************************************************************/
static const OLEAF *ob_find_morton3
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Try to find a nearest leaf node with these coordinates.
 * Nearest match here means APPROXIMATE nearest match along the tree!
 * This is in fact a quick&dirty nearest match, but extremly fast.
 *
 * It may return a NULL pointer in case there is no leaf along the path.
 */
{
   const ONODE *node  = &(ob->root_node);
   const ONODE *parent = node;
   OB_STARTUP(ob)
   unsigned     maskx = 1<<(ob->hbitx);
   unsigned     masky = 1<<(ob->hbity);
   unsigned     maskz = 1<<(ob->hbitz);


#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif

   /* Walk down until we reach a leaf or there are no more childs */
   while(ONODE_TRUE_BRANCH(node))
   {
      unsigned octant;
      MORTON_NEWOCTANT_3M(octant,ux,uy,uz,maskx,masky,maskz);
      parent = node;
      node = parent->child[octant];
   }

   if (node)
   {
      /* Great, got a leaf */
      const OLEAF *oleaf = &(node->oleaf);
      if (oleaf->l_next)
      {
         double bestDist2 = DBL_MAX;
         return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
      }
      return oleaf;
   }

   if (nearest)
   {
      /* Find true nearest */
      double bestDist2 = DBL_MAX;
      return ob_find_from_here_nearest(parent,rx,ry,rz,&bestDist2);
   }

   /* Find an approximate leaf */
   return ob_find_from_here_approximate(parent,rx,ry,rz);
}

/****************************************************************************************/
static const OLEAF *ob_search_morton3
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
{
   if (ob||rx||ry||rz||srange)
   {
      XMSG_FATAL0
      (
         "ob_search_morton3: function not yet implemented.\n"
      );
   }
   return NULL;
}

/****************************************************************************************/

/*
 * The Z-hash stuff
 */
/***************************************************************************************/
forceinline const ONODE *ob_zhash_find_node
(
   const OCTBOX  *ob,
   const unsigned zcode
)
/***************************************************************************************/
{
   const ONODE *node = ob->node_hash[zcode];

   if (!node)
   {
      ONODE **nh   = ob->node_hash;
      int     zc_m = CAST_INT(zcode) + 1;
      int     zc_p = CAST_INT(zcode) - 1;
      int     i;

      for (i=0; i<OHASH_SIZE/2; i++)
      {
         if (zc_m >= 0         && (node=nh[zc_m]) != NULL) break;
         if (zc_p < OHASH_SIZE && (node=nh[zc_p]) != NULL) break;
         zc_m--;
         zc_p++;
      }
   }

   return node;
}

/***************************************************************************************/

/***************************************************************************************/
static const OLEAF *ob_insert_zhash
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insert a new point with index 'idx', flags and optional object pointer into the
 * octal box-tree.
 * Returns a pointer to either an existing leaf, the new inserted leaf or NULL
 * in case of error (ux/uy/uz out of the octbox range).
 */
{
   ONODE   *node;
   IPOOL   *pool = &(ob->node_pool);
   OB_STARTUP(ob)
#if OCTBOX_USE_RLOG
   unsigned depth = 0;
#endif
   unsigned zmask = 1<<ob->max_hbit4;
   unsigned zcode = ob_15_bit_zcode
                     (
                        ux>>ob->max_hbit4,
                        uy>>ob->max_hbit4,
                        uz>>ob->max_hbit4
                     );

#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   if ((node=ob->node_hash[zcode]) == NULL)
   {
      OLEAF *leaf;
      OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
      ob->node_hash[zcode] = RCAST_INTO(ONODE *,leaf);
      ob->nleaves++;
      return leaf;
   }

   /* Walk down until 'node' has no child. */
   for(;;)
   {
      ONODE   *child;
      unsigned octant; /* 0 ... 7 */

   #if OCTBOX_USE_RLOG
      depth++;
   #endif

      if (ONODE_IS_LEAF(node)) /* this is a leaf type node */
      {
         /*
          * The 'node' is already a leaf type node. Make a copy of
          * itself one level below. Make 'node' a branch node.
          * First ensure that old and new nodal points are not identical.
          * In this case, we would walk down forever, since we can
          * never put the old and new nodal point into different octants.
          */
         OLEAF         *leaf   = &(node->oleaf);
         const OCTBOX_C leafux = leaf->l_ux;
         const OCTBOX_C leafuy = leaf->l_uy;
         const OCTBOX_C leafuz = leaf->l_uz;

         if (leafux == ux && leafuy == uy && leafuz == uz)
         {
            if (!ob->keeplist)
               return leaf; /* return the already existing leaf */

            ob->nleaves++;
            return ob_leaf_list_append(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         }

         /* Get suboctant of current leaf point */
         MORTON_GETOCTANT_1M(octant,leafux,leafuy,leafuz,zmask);

         /* Duplicate the leaf node */
         ONODE_DUP(pool,child,node);

         /* Make 'node' a branch node with just one leaf child */
         node->child[7] =
         node->child[6] =
         node->child[5] =
         node->child[4] =
         node->child[3] =
         node->child[2] =
         node->child[1] =
         node->child[0] = NULL;
         node->child[octant] = child;
      }

      /*
       * Get suboctant of point and octant's child node of 'node'
       */
      MORTON_NEWOCTANT_1M(octant,ux,uy,uz,zmask);
      if ((child=node->child[octant]) == NULL)
      {
         /* We don't have a child yet, just append a new leaf type node */
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         node->child[octant] = RCAST_INTO(ONODE *,leaf);
      #if OCTBOX_USE_RLOG
         if (depth > ob->max_depth) ob->max_depth = depth;
      #endif
         ob->nleaves++;
         return leaf; /* return the new leaf */
      }

      /* There is already a child node. walk one level down, ... */
      node = child; /* make child the current node */
   }
}

/****************************************************************************************/
static const OLEAF *ob_find_zhash
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Try to find a nearest leaf node with these coordinates.
 * Nearest match here means APPROXIMATE nearest match along the tree!
 * This is in fact a quick&dirty nearest match, but extremly fast.
 *
 * It may return a NULL pointer in case there is no leaf along the path.
 */
{
   const ONODE *node,*parent;
   OB_STARTUP(ob)
   unsigned     zmask = 1<<ob->max_hbit4;
   unsigned     zcode = ob_15_bit_zcode
                        (
                           ux>>ob->max_hbit4,
                           uy>>ob->max_hbit4,
                           uz>>ob->max_hbit4
                        );


#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif


   if ((node=ob_zhash_find_node(ob,zcode)) == NULL)
      return NULL;

   /* Walk down until we reach a leaf or there are no more childs */
   parent = node;
   while(ONODE_TRUE_BRANCH(node))
   {
      unsigned octant;
      MORTON_NEWOCTANT_1M(octant,ux,uy,uz,zmask);
      parent = node;
      node = parent->child[octant];
   }

   if (node)
   {
      /* Great, got a leaf */
      const OLEAF *oleaf = &(node->oleaf);
      if (oleaf->l_next)
      {
         double bestDist2 = DBL_MAX;
         return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
      }
      return oleaf;
   }

   if (nearest)
   {
      /* Find true nearest */
      double bestDist2 = DBL_MAX;
      return ob_find_from_here_nearest(parent,rx,ry,rz,&bestDist2);
   }

   /* Find an approximate leaf */
   return ob_find_from_here_approximate(parent,rx,ry,rz);
}

/****************************************************************************************/
static const OLEAF *ob_search_zhash
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
/*
 * Try to find a leaf node in the neighborhood with the smallest distance
 * within the unsigned search range.
 */
{

   ONODE **node_hash = ob->node_hash;
   const OLEAF *bestLeaf = NULL;
   const ONODE *node;
   OZBOX        zbox;
   double       bestDist2 = INT_MAX;
   OB_STARTUP(ob)
   unsigned     zc;


#if OCTBOX_CHECK_DOMAIN
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif

   ob_zbox_setup
   (
      &zbox,ux,uy,uz,srange,
      ob->max_hbit4,
      ob->max_hbit4,
      ob->max_hbit4
   );

   for(zc=zbox.z_min; zc<=zbox.z_max ; zc++)
   {
      if ((node=node_hash[zc]) != NULL)
      {
         const OLEAF *oleaf;
         if ((oleaf=ob_find_from_here_nearest(node,rx,ry,rz,&bestDist2)) != NULL)
         {
            if (bestDist2 < 2)
               return oleaf;

            bestLeaf = oleaf;
         }
      }
   }
   return bestLeaf;
}

/****************************************************************************************/



/*
 * The Z-hash stuff with max. aligned bits
 */
/***************************************************************************************/
static const OLEAF *ob_insert_zhash3
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insert a new point with index 'idx', flags and optional object pointer into the
 * octal box-tree.
 * Returns a pointer to either an existing leaf, the new inserted leaf or NULL
 * in case of error (ux/uy/uz out of the octbox range).
 */
{
   ONODE   *node;
   IPOOL   *pool = &(ob->node_pool);
   OB_STARTUP(ob)
#if OCTBOX_USE_RLOG
   unsigned depth = 0;
#endif
   unsigned maskx = 1<<ob->hbitx4;
   unsigned masky = 1<<ob->hbity4;
   unsigned maskz = 1<<ob->hbitz4;
   unsigned zcode = ob_15_bit_zcode
                     (
                        ux>>ob->hbitx4,
                        uy>>ob->hbity4,
                        uz>>ob->hbitz4
                     );


#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   if ((node=ob->node_hash[zcode]) == NULL)
   {
      OLEAF *leaf;
      OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
      ob->node_hash[zcode] = RCAST_INTO(ONODE *,leaf);
      ob->nleaves++;
      return leaf;
   }

   /* Walk down until 'node' has no child. */
   for(;;)
   {
      ONODE   *child;
      unsigned octant; /* 0 ... 7 */

   #if OCTBOX_USE_RLOG
      depth++;
   #endif

      if (ONODE_IS_LEAF(node)) /* this is a leaf type node */
      {
         /*
          * The 'node' is already a leaf type node. Make a copy of
          * itself one level below. Make 'node' a branch node.
          * First ensure that old and new nodal points are not identical.
          * In this case, we would walk down forever, since we can
          * never put the old and new nodal point into different octants.
          */
         OLEAF         *leaf   = &(node->oleaf);
         const OCTBOX_C leafux = leaf->l_ux;
         const OCTBOX_C leafuy = leaf->l_uy;
         const OCTBOX_C leafuz = leaf->l_uz;

         if (leafux == ux && leafuy == uy && leafuz == uz)
         {
            if (!ob->keeplist)
               return leaf; /* return the already existing leaf */

            ob->nleaves++;
            return ob_leaf_list_append(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         }

         /* Get suboctant of current leaf point */
         MORTON_GETOCTANT_3M(octant,leafux,leafuy,leafuz,maskx,masky,maskz);

         /* Duplicate the leaf node */
         ONODE_DUP(pool,child,node);

         /* Make 'node' a branch node with just one leaf child */
         node->child[7] =
         node->child[6] =
         node->child[5] =
         node->child[4] =
         node->child[3] =
         node->child[2] =
         node->child[1] =
         node->child[0] = NULL;
         node->child[octant] = child;
      }

      /*
       * Get suboctant of point and octant's child node of 'node'
       */
      MORTON_NEWOCTANT_3M(octant,ux,uy,uz,maskx,masky,maskz);
      if ((child=node->child[octant]) == NULL)
      {
         /* We don't have a child yet, just append a new leaf type node */
         OLEAF *leaf;
         OLEAF_NEW(pool,leaf,rx,ry,rz,ux,uy,uz,idx,flags);
         node->child[octant] = RCAST_INTO(ONODE *,leaf);
      #if OCTBOX_USE_RLOG
         if (depth > ob->max_depth) ob->max_depth = depth;
      #endif
         ob->nleaves++;
         return leaf; /* return the new leaf */
      }

      /* There is already a child node. walk one level down, ... */
      node = child; /* make child the current node */
   }
}

/****************************************************************************************/
static const OLEAF *ob_find_zhash3
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Try to find a nearest leaf node with these coordinates.
 * Nearest match here means APPROXIMATE nearest match along the tree!
 * This is in fact a quick&dirty nearest match, but extremly fast.
 *
 * It may return a NULL pointer in case there is no leaf along the path.
 */
{
   const ONODE *node,*parent;
   OB_STARTUP(ob)
   unsigned     maskx = 1<<ob->hbitx4;
   unsigned     masky = 1<<ob->hbity4;
   unsigned     maskz = 1<<ob->hbitz4;
   unsigned     zcode = ob_15_bit_zcode
                        (
                           ux>>ob->hbitx4,
                           uy>>ob->hbity4,
                           uz>>ob->hbitz4
                        );


#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif

   if ((node=ob_zhash_find_node(ob,zcode)) == NULL)
      return NULL;

   /* Walk down until we reach a leaf or there are no more childs */
   parent = node;
   while(ONODE_TRUE_BRANCH(node))
   {
      unsigned octant;
      MORTON_NEWOCTANT_3M(octant,ux,uy,uz,maskx,masky,maskz);
      parent = node;
      node = parent->child[octant];
   }

   if (node)
   {
      /* Great, got a leaf */
      const OLEAF *oleaf = &(node->oleaf);
      if (oleaf->l_next)
      {
         double bestDist2 = DBL_MAX;
         return ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2);
      }
      return oleaf;
   }

   if (nearest)
   {
      /* Find true nearest */
      double bestDist2 = DBL_MAX;
      return ob_find_from_here_nearest(parent,rx,ry,rz,&bestDist2);
   }

   /* Find an approximate leaf */
   return ob_find_from_here_approximate(parent,rx,ry,rz);
}

/****************************************************************************************/
static const OLEAF *ob_search_zhash3
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
{
   const ONODE *node;
   const OLEAF *bestLeaf = NULL;
   OZBOX        zbox;
   OB_STARTUP(ob)
#if 0
   unsigned     maskx = 1<<ob->hbitx;
   unsigned     masky = 1<<ob->hbity;
   unsigned     maskz = 1<<ob->hbitz;
#endif
   unsigned     zcode = ob_15_bit_zcode
                        (
                           ux>>ob->hbitx4,
                           uy>>ob->hbity4,
                           uz>>ob->hbitz4
                        );
   unsigned     zc;
   double       bestDist2 = DBL_MAX;


#if OCTBOX_CHECK_DOMAIN
   /* Check that the point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL;
#endif

   if (!srange)
   {
      node = ob_zhash_find_node(ob,zcode);
      if (!node)
         return NULL;

   }

   ob_zbox_setup
   (
      &zbox,ux,uy,uz,srange,
      ob->hbitx4,
      ob->hbity4,
      ob->hbitz4
   );

#if 0
   printf
   (
      "Z-search range: %u ... %u ... %u\n"
      "zhash[%u] == %p\n"
      ,zbox.z_min
      ,zcode
      ,zbox.z_max
      ,zcode
      ,(void*)ob->node_hash[zcode]
   );
#endif

   /*
    * Walk down until we reach a leaf or there are no more childs
    */
#if 0
   printf
   (
      "Z-search range: %5u ... %5u ... %5u"
      ,zbox.z_min
      ,zcode
      ,zbox.z_max
   );
#endif

   for (zc=zbox.z_min; zc<=zbox.z_max; zc++)
   {
      if ((node=ob->node_hash[zc]) != NULL)
      {
         if (ONODE_IS_LEAF(node))
         {
            /* Update the distance of the leaf found */
            const OLEAF *oleaf = &(node->oleaf);
            if (oleaf->l_next)
            {
               if ((oleaf=ob_leaf_list_rscan2(oleaf,rx,ry,rz,&bestDist2)) != NULL)
                  bestLeaf = oleaf;
            }
            else if (ob_leaf_best_rdist2(oleaf,rx,ry,rz,&bestDist2))
            {
               bestLeaf = oleaf;
            }
         }
         else
         {
            const OLEAF *oleaf = ob_find_from_here_nearest(node,rx,ry,rz,&bestDist2);
            if (oleaf)
               bestLeaf = oleaf;
         }
      }
   }

   return bestLeaf;
}

/****************************************************************************************/




/***************************************************************************************/
static const OLEAF *ob_insert_plain
(
   OCTBOX        *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   const int      idx,
   const unsigned flags
)
/****************************************************************************************/
/*
 * Insertion without a check for doubled points
 */
{
   IPOOL *pool = &(ob->node_pool);
   OLEAF *oleaf;

   OB_STARTUP(ob)

#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   OLEAF_NEW(pool,oleaf,rx,ry,rz,ux,uy,uz,idx,flags);
   ob->nleaves++;
   return oleaf;
}

/****************************************************************************************/
static const OLEAF *ob_find_zbin
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Implements a binary search on the array of Z-sorted OLEAF points.
 * If the coordinates do not match 100%, return the nearest point one left or right.
 */
{
   const OLEAF   *ohead;
   const OLEAF   *olcmp;
   const OLEAF   *bestLeaf;
   double         bestDist2 = DBL_MAX;
   OLEAF          sleaf;
   OB_STARTUP(ob)
   const unsigned zmask = 1<<ob->max_hbit;
   unsigned       npoints = ob->nleaves;
   int            imin,imax,di;


#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

#if IS_MSWIN
   (void)(nearest); /* Keep compiler happy */
#else
   if (nearest) /* Keep compiler happy */
      ;
#endif

   if ((olcmp=ohead=SCAST_INTO(OLEAF *,ob->leaf_array))==NULL || npoints < 2)
      return olcmp;

   sleaf.l_ux = ux;
   sleaf.l_uy = uy;
   sleaf.l_uz = uz;

   do
   {
      const int ires = ob_oleaf_cmp_morton(&sleaf,olcmp=ohead+(npoints>>1),zmask);
      if (!ires)
         return olcmp; /* Great, 100% match */
      if (ires > 0)
      {
         /* search coords > center coords: continute search right */
         ohead = olcmp + 1;
         npoints--;
      }
      /* else continue search left */
   } while(npoints >>= 1);

   /*
    * Not 100% match, look left & right at the last point
    */
   ob_leaf_best_rdist2(olcmp,rx,ry,rz,&bestDist2);

   bestLeaf  = olcmp;
   ohead = SCAST_INTO(OLEAF *,ob->leaf_array);
   imin = (int)(ohead - olcmp);
   imax = imin + ob->nleaves;
   printf("imin=%d, imax=%d\n",imin,imax);

   for (di=1; di<128; )
   {
      if (di>=imin && di<imax && ob_leaf_best_rdist2(olcmp+di,rx,ry,rz,&bestDist2))
      {
         bestLeaf = olcmp + di;
         printf
         (
            "Loop %+d   idx=%7d, dist2=%g\n"
            ,di
            ,bestLeaf->l_idx
            ,bestDist2
         );
      }
      if ((di=-di) > 0) di++;
   }

   printf
   (
      "di=%+2d idx=%7d, dist2=%g\n"
      ,(int)(bestLeaf-olcmp)
      ,bestLeaf->l_idx
      ,bestDist2
   );
   printf("    suche   %12" OBFMT_CU " %12" OBFMT_CU " %12" OBFMT_CU "\n",ux,uy,uz);
   for (di=1; di<128; )
   {
      if (di>=imin && di<imax)
      {
         printf
         (
            "    finde%+d %12" OBFMT_CU " %12" OBFMT_CU " %12" OBFMT_CU "   idx=%d\n"
            ,di
            ,(bestLeaf+di)->l_ux
            ,(bestLeaf+di)->l_uy
            ,(bestLeaf+di)->l_uz
            ,(bestLeaf+di)->l_idx
         );
      }

      if ((di=-di) > 0) di++;
   }

   return bestLeaf;
}

/****************************************************************************************/
static const OLEAF *ob_search_zbin
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
/*
 * Implements a binary search on the array of Z-sorted OLEAF points.
 */
{
   const OLEAF   *ohead,*otail,*ocent,*ormin,*ormax,*otest;
   const OLEAF   *bestLeaf = NULL;
   double         bestDist2 = DBL_MAX;
   OLEAF          sleaf;
   ORBOX          range_rbox;
   OB_STARTUP(ob)
   const unsigned zmask = 1<<ob->max_hbit;
   unsigned       npoints;


#if OCTBOX_CHECK_DOMAIN
   /* Check that the new point is within the root domain */
   if (OUTSIDE_OCT_DOMAIN(ob,ux,uy,uz))
      return NULL; /* return outside domain error */
#endif

   if ((ohead=SCAST_INTO(OLEAF *,ob->leaf_array))==NULL || ob->nleaves < 2)
      return ohead;

#if 0
   ob_rbox_setup(&range_rbox,ux,uy,uz,srange);

   zc_rmin = ob_15_bit_zcode
               (
                  range_rbox.r_xmin>>hbit4,
                  range_rbox.r_ymin>>hbit4,
                  range_rbox.r_zmin>>hbit4
               );

   zc_rmax = ob_15_bit_zcode
               (
                  range_rbox.r_xmax>>hbit4,
                  range_rbox.r_ymax>>hbit4,
                  range_rbox.r_zmax>>hbit4
               );

   zc_upnt  = ob_15_bit_zcode
               (
                  ux>>hbit4,
                  uy>>hbit4,
                  uz>>hbit4
               );
   zc_head = ob_15_bit_zcode
               (
                  ohead->ux>>hbit4,
                  ohead->uy>>hbit4,
                  ohead->uz>>hbit4
               );
   zc_tail = ob_15_bit_zcode
               (
                  otail->ux>>hbit4,
                  otail->uy>>hbit4,
                  otail->uz>>hbit4
               );
#endif

   /*
    * Binary search for the point ux,uy,uz
    */
   sleaf.l_ux = ux;
   sleaf.l_uy = uy;
   sleaf.l_uz = uz;
   ocent = ohead = SCAST_INTO(OLEAF *,ob->leaf_array);
   for(npoints=ob->nleaves; npoints; npoints >>= 1)
   {
      const int ires = ob_oleaf_cmp_morton(&sleaf,ocent=ohead+(npoints>>1),zmask);
      if (!ires)
         return ocent; /* Great, 100% match */
      if (ires > 0)
      {
         /* search coords > center coords: continute search right */
         ohead = ocent + 1;
         npoints--;
      }
      /* else continue search left */
   }

   /*
    * Not 100% match, look left & right at the last point
    */
   ormin = ormax = ocent;
   if (ob_leaf_best_rdist2(ocent,rx,ry,rz,&bestDist2))
      bestLeaf = ocent;

   ohead = SCAST_INTO(OLEAF *,ob->leaf_array);
   if (ocent > ohead && ob_leaf_best_rdist2(ocent-1,rx,ry,rz,&bestDist2))
      ormin = bestLeaf = ocent-1;

   otail = ohead + ob->nleaves;
   if (ocent < (otail-1) && ob_leaf_best_rdist2(ocent+1,rx,ry,rz,&bestDist2))
      ormax = bestLeaf = ocent+1;

   /*
    * Create a range box
    */
   {
      const unsigned urange = CAST_UINT(sqrt(bestDist2));
      if (urange < srange)
      {
#if 0
printf("\nrbox with urange=%u\n",urange);
#endif
         ob_rbox_setup(&range_rbox,ux,uy,uz,urange);
      }
      else
      {
#if 0
printf("\nrbox with srange=%u\n",srange);
#endif
         ob_rbox_setup(&range_rbox,ux,uy,uz,srange);
      }
   }

   /*
    * Binary search for the point r_xmin,r_ymin,r_zmin
    */
   sleaf.l_ux = range_rbox.r_xmin;
   sleaf.l_uy = range_rbox.r_ymin;
   sleaf.l_uz = range_rbox.r_zmin;
   for(npoints=CAST_UINT(ormin-ohead); npoints; npoints >>= 1)
   {
      const int ires = ob_oleaf_cmp_morton(&sleaf,ormin=ohead+(npoints>>1),zmask);
      if (!ires)
         break; /* Great, 100% match */
      if (ires > 0)
      {
         /* search coords > center coords: continute search right */
         ohead = ormin + 1;
         npoints--;
      }
      /* else continue search left */
   }

   /*
    * Binary search for the point r_xmax,r_ymax,r_zmax
    */
   sleaf.l_ux = range_rbox.r_xmax;
   sleaf.l_uy = range_rbox.r_ymax;
   sleaf.l_uz = range_rbox.r_zmax;
   ohead = ormax;
   for(npoints=CAST_UINT(otail-ohead); npoints; npoints >>= 1)
   {
      const int ires = ob_oleaf_cmp_morton(&sleaf,ormax=ohead+(npoints>>1),zmask);
      if (!ires)
         break; /* Great, 100% match */
      if (ires > 0)
      {
         /* search coords > center coords: continute search right */
         ohead = ormax + 1;
         npoints--;
      }
      /* else continue search left */
   }

#if 0
printf("   z max-min dist=%u\n",(zc_rmax-zc_rmin));
#endif


#if 0
printf("   p cent-min dist=%u\n",(unsigned)(ocent-ormin));
#endif
   for(otest=ormin; otest<(ocent-1); otest++)
   {
      if (ob_leaf_best_rdist2(otest,rx,ry,rz,&bestDist2))
         bestLeaf = otest;
   }

#if 0
printf("   p max-cent dist=%u\n",(unsigned)(ormax-ocent));
#endif
   for(otest=ocent+2; otest<=ormax; otest++)
   {
      if (ob_leaf_best_rdist2(otest,rx,ry,rz,&bestDist2))
         bestLeaf = otest;
   }

   return bestLeaf;
}

/****************************************************************************************/



/*
 * qsort() compare wrapper for x,y,z
 */
typedef int (POINTCMP)(const void *, const void *);

#if 0
/****************************************************************************************/
static int ob_oleaf_cmp_xyz(const void *vl1, const void *vl2)
/***************************************************************************************/
{
   const OLEAF *ol1 = RCAST_INTO(const OLEAF*,vl1);
   const OLEAF *ol2 = RCAST_INTO(const OLEAF*,vl2);
   real d;
   d = ol1->l_rx - ol2->l_rx; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_ry - ol2->l_ry; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_rz - ol2->l_rz; if (d>0) return 1; if (d<0) return -1;
   return 0;
}

/****************************************************************************************/
static int ob_oleaf_cmp_yzx(const void *vl1, const void *vl2)
/***************************************************************************************/
{
   const OLEAF *ol1 = RCAST_INTO(const OLEAF*,vl1);
   const OLEAF *ol2 = RCAST_INTO(const OLEAF*,vl2);
   real d;
   d = ol1->l_ry - ol2->l_ry; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_rz - ol2->l_rz; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_rx - ol2->l_rx; if (d>0) return 1; if (d<0) return -1;
   return 0;
}

/****************************************************************************************/
static int ob_oleaf_cmp_zxy(const void *vl1, const void *vl2)
/***************************************************************************************/
{
   const OLEAF *ol1 = RCAST_INTO(const OLEAF*,vl1);
   const OLEAF *ol2 = RCAST_INTO(const OLEAF*,vl2);
   real d;
   d = ol1->l_rz - ol2->l_rz; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_rx - ol2->l_rx; if (d>0) return 1; if (d<0) return -1;
   d = ol1->l_ry - ol2->l_ry; if (d>0) return 1; if (d<0) return -1;
   return 0;
}

/****************************************************************************************/
static POINTCMP *ob_oleaf_get_cmp(const int splitdim)
/****************************************************************************************/
{
   static POINTCMP *ftab[3] =
   {
      ob_oleaf_cmp_xyz,
      ob_oleaf_cmp_yzx,
      ob_oleaf_cmp_zxy
   };

   POINTCMP *cmp = (splitdim<0||splitdim>2) ? NULL : ftab[splitdim];
   if (!cmp)
   {
      XMSG_FATAL1
      (
         "ob_oleaf_get_cmp(splitim=%d): failed to select compare function.\n"
         ,splitdim
      );
   }

   return cmp;
}

/****************************************************************************************/
static int ob_get_splitdim(const KDBOX *box)
/****************************************************************************************/
{
   const real ex = box->k_xmax - box->k_xmin;
   const real ey = box->k_ymax - box->k_ymin;
   const real ez = box->k_zmax - box->k_zmin;

   return (ex>ey) ? ((ex>ez) ? 0 : 2)
                  : ((ey>ez) ? 1 : 2);
}

/****************************************************************************************/
#endif



/****************************************************************************************/
static const OLEAF *ob_find_kdtree
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
{
   if (ob||rx||ry||rz||nearest)
   {
      XMSG_FATAL0
      (
         "ob_find_kdtree: function not yet implemented.\n"
      );
   }
   return NULL;
}

/****************************************************************************************/
static const OLEAF *ob_search_kdtree
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
{
   if (ob||rx||ry||rz||srange)
   {
      XMSG_FATAL0
      (
         "ob_search_kdtree: function not yet implemented.\n"
      );
   }
   return NULL;
}

/****************************************************************************************/


/****************************************************************************************/
static const OLEAF *ob_find_bforce
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz,
   const int     nearest
)
/****************************************************************************************/
/*
 * Brute force linear search on the compacted OLEAF list.
 * WORKS BUT SLOW: More for internal testing rather than for a productive use.
 */
{
   const OLEAF *head      = SCAST_INTO(OLEAF *,ob->leaf_array);
   const OLEAF *tail      = head + ob->nleaves;
   const OLEAF *bestLeaf  = NULL;
   double       bestDist2 = DBL_MAX;


#if IS_MSWIN
   (void)(nearest); /* Keep compiler happy */
#else
   if (nearest) /* Keep compiler happy */
      ;
#endif

   for(; head<tail; head++)
   {
      if (ob_leaf_best_rdist2(head,rx,ry,rz,&bestDist2))
         bestLeaf = head;
   }

   return bestLeaf;
}

/****************************************************************************************/
static const OLEAF *ob_search_bforce
(
   const OCTBOX  *ob,
   const real     rx,
   const real     ry,
   const real     rz,
   unsigned       srange
)
/****************************************************************************************/
/*
 * WORKS BUT SLOW: More for internal testing rather than for a productive use.
 */
{
   return (srange) ? ob_find_bforce(ob,rx,ry,rz,0) : NULL;
}

/****************************************************************************************/


#if OCTBOX_USE_MATCH
/****************************************************************************************/
C_FUNC_PREFIX const OLEAF *OCTBOX_match
(
   const OCTBOX *ob,
   const real    rx,
   const real    ry,
   const real    rz
)
/****************************************************************************************/
/*
 * Try to find a leaf node with EXACTLY these unsigned coordinates.
 * Exact match here means 100% match in the unsigned space!
 */
{
   OB_STARTUP(ob)

   const OLEAF *leaf = ob->ob_find(ob,rx,ry,rz,0);
   return (leaf &&
            leaf->l_ux == ux &&
            leaf->l_uy == uy &&
            leaf->l_uz == uz)
         ? leaf   /* 100% match */
         : NULL;  /* no leaf or mismatch */
}

/****************************************************************************************/
#endif


/****************************************************************************************/
C_FUNC_PREFIX OCTBOX *OCTBOX_setup_b
(
   OCTBOX        *ob,
   const char    *name,
   const int      npoints,
   const real     mindist,
   const real     maxdist,
   const unsigned flags,
   const real     xmin,
   const real     ymin,
   const real     zmin,
   const real     xmax,
   const real     ymax,
   const real     zmax
)
/****************************************************************************************/
{
   static const char module[] = "OCTBOX_setup_b";

   OBBOX box;
   real  size_x,size_y,size_z;
   int   nonodes;
   char  descr[128];



   /*
    * The shift into the unsigned range is based on mininal coords: xyzmin -> 0
    * Keep ob->minxyz and determine the size of our real box domain.
    */
   size_x = xmax - (ob->minx = xmin);
   size_y = ymax - (ob->miny = ymin);
   size_z = zmax - (ob->minz = zmin);
   if (size_x < 0 || size_y < 0 || size_z < 0)
   {
      XMSG_MESSAGE
      (
         XMSG_LEVEL_FATAL,
         "   %s(\"%s\"): Internal error:\n"
         "      Invalid min ... max range.\n"
         "      Range X : [ %12f ... %12f ], %g\n"
         "      Range Y : [ %12f ... %12f ], %g\n"
         "      Range Z : [ %12f ... %12f ], %g\n"
         ,module
         ,ob->name
         ,xmin,xmax,size_x
         ,ymin,ymax,size_y
         ,zmin,zmax,size_z
      );
   }

   box.b_xmin = box.b_ymin = box.b_zmin = 0;

   MEMZERO(&(ob->root_node),sizeof(ob->root_node));
   ob->leaf_array = NULL;
   ob->name       = (STRHASLEN(name)) ? name : "OCTBOX";;
   ob->max_depth  = 0;
   ob->min_dist   = mindist;
   ob->max_dist   = maxdist;
   ob->npoints    = npoints;
   ob->nleaves    = 0;
   ob->flags      = flags&(~(OCTBOX_FBIT_COMPACT|OCTBOX_FBIT_SORTED));
   ob->keeplist   = ((flags&OCTBOX_FBIT_KEEPLIST) != 0);

   /* Mark scale invalid */
   ob->scalex =
   ob->scaley =
   ob->scalez = -1;

   switch(ob->flags & OCTBOX_TYPE_MASK)
   {
      case OCTBOX_TYPE_GEOM:
      case OCTBOX_TYPE_GEOMZ:
         ob->ob_insert = ob_insert_geom;
         ob->ob_find   = ob_find_geom;
         ob->ob_search = ob_search_geom;
         break;

      case OCTBOX_TYPE_MORTON:
         ob->ob_insert = ob_insert_morton;
         ob->ob_find   = ob_find_morton;
         ob->ob_search = ob_search_morton;
         break;

      case OCTBOX_TYPE_MORTON3:
         ob->ob_insert = ob_insert_morton3;
         ob->ob_find   = ob_find_morton3;
         ob->ob_search = ob_search_morton3;
         break;

      case OCTBOX_TYPE_ZHASH:
         ob->ob_insert = ob_insert_zhash;
         ob->ob_find   = ob_find_zhash;
         ob->ob_search = ob_search_zhash;
         break;

      case OCTBOX_TYPE_ZHASH3:
         ob->ob_insert = ob_insert_zhash3;
         ob->ob_find   = ob_find_zhash3;
         ob->ob_search = ob_search_zhash3;
         break;

      case OCTBOX_TYPE_ZBINARY:
         ob->ob_insert = ob_insert_plain;
         ob->ob_find   = ob_find_zbin;
         ob->ob_search = ob_search_zbin;
         break;

      case OCTBOX_TYPE_KDTREE:
         ob->ob_insert = ob_insert_plain;
         ob->ob_find   = ob_find_kdtree;
         ob->ob_search = ob_search_kdtree;
         break;

      default:
         ob->ob_insert = ob_insert_impossible;
         ob->ob_find   = ob_find_impossible;
         ob->ob_search = ob_search_impossible;
         XMSG_MESSAGE
         (
            XMSG_LEVEL_FATAL,
            "   %s(\"%s\"): Internal error:\n"
            "      Invalid tree-type flags 0x%08x specified.\n"
            "      Flags: %s\n"
            ,module
            ,ob->name
            ,flags
            ,ob_flags2str(flags,descr)
         );
         break;
   }

   if (mindist > 0)
   {
      /*
       * Just one box 0...1,0...1,0...1 makes no sense.
       * we can only subdivide into 8 octants in unsigned space, if we have a
       * size >= 2. A subdivision without routine errors in unsigned space
       * makes only sense if the size is even.
       *
       */
      double dist,scale;

#if 0
      dist = 4*mindist;
      if (size_x < dist) size_x = dist;
      if (size_y < dist) size_y = dist;
      if (size_z < dist) size_z = dist;
#endif

      /*
       * If the minimal distance is a diagonal in 3D space, then the projection onto x/y/z
       * is identical, and is is mindist/sqrt(3).
       * To avoid roundoff errors we use 2*sqrt(3) ~ 3.4641
       */
      dist  = mindist/(2.0*M_SQRT3);
      scale = 1.0/dist;

      ob->scalex =
      ob->scaley =
      ob->scalez = CAST_REAL(scale);
      box.b_xmax = CAST_OBCOORD(size_x*scale);
      box.b_ymax = CAST_OBCOORD(size_y*scale);
      box.b_zmax = CAST_OBCOORD(size_z*scale);
      /*
       * Make the box size a multiple of 2.
       */
      if (box.b_xmax<OBBOX_MINRES) box.b_xmax = OBBOX_MINRES; else if (box.b_xmax&1) box.b_xmax++;
      if (box.b_ymax<OBBOX_MINRES) box.b_ymax = OBBOX_MINRES; else if (box.b_ymax&1) box.b_ymax++;
      if (box.b_zmax<OBBOX_MINRES) box.b_zmax = OBBOX_MINRES; else if (box.b_zmax&1) box.b_zmax++;
   }
   else /* No minimal distance: use max. granularity. */
   {
      const real size_max = VMAX3(size_x,size_y,size_z);
      ob->min_dist = -1;
      box.b_xmax = (size_x > 0) ? CAST_OBCOORD(OBBOX_MAXRES*(size_x/size_max)) : OBBOX_MINRES;
      box.b_ymax = (size_y > 0) ? CAST_OBCOORD(OBBOX_MAXRES*(size_y/size_max)) : OBBOX_MINRES;
      box.b_zmax = (size_z > 0) ? CAST_OBCOORD(OBBOX_MAXRES*(size_z/size_max)) : OBBOX_MINRES;
   }

   if (ob->flags & OCTBOX_FBIT_CUBE)
   {
      const OCTBOX_C size_max = VMAX3(box.b_xmax,box.b_ymax,box.b_zmax);
      box.b_xmax =
      box.b_ymax =
      box.b_zmax = size_max;

      /* The scales are variing */
      ob->scalex =
      ob->scaley =
      ob->scalez = -1;
   }

   if (ob->flags & OCTBOX_FBIT_POW2)
   {
      box.b_xmax = ob_nextpow2(box.b_xmax-1)-1;
      box.b_ymax = ob_nextpow2(box.b_ymax-1)-1;
      box.b_zmax = ob_nextpow2(box.b_zmax-1)-1;

      /* The scales are variing */
      ob->scalex =
      ob->scaley =
      ob->scalez = -1;
   }

   if (box.b_xmax > OBBOX_MAXRES || box.b_ymax > OBBOX_MAXRES || box.b_zmax > OBBOX_MAXRES)
   {
      XMSG_MESSAGE
      (
         XMSG_LEVEL_FATAL,
         "   %s(\"%s\"): Internal error:\n"
         "      The uint box resolution is (%" OBFMT_CU ", %" OBFMT_CU ", %" OBFMT_CU ").\n"
         "      This is either greater than the max. resolution %" OBFMT_CU "\n"
         "      or an unsigned overflow occurred.\n"
         "      Min. distance: %g\n"
         "      Real range X : [ %12f ... %12f ], %g\n"
         "      Real range Y : [ %12f ... %12f ], %g\n"
         "      Real range Z : [ %12f ... %12f ], %g\n"
         ,module
         ,ob->name
         ,box.b_xmax
         ,box.b_ymax
         ,box.b_zmax
         ,OBBOX_MAXRES
         ,mindist
         ,xmin,xmax,size_x
         ,ymin,ymax,size_y
         ,zmin,zmax,size_z
      );
   }

   box.b_xmid = (box.b_xmax>>1);
   box.b_ymid = (box.b_ymax>>1);
   box.b_zmid = (box.b_zmax>>1);
   ob->root_box = box; /* struct copy */

   /*
    * The scaling factor is in fact 1/(mindist/3), but we may want to have
    * different scaling factors in each direction and we need to consider
    * roundoff errors.
    * We may have planar coords and any of the xyz-size may be 0.
    * In this case we do not scale, just shift into the unsigned space.
    */
   if (ob->scalex <= 0)
   {
      ob->scalex = (size_x > 0) ? CAST_REAL(box.b_xmax/size_x) : 1;
      ob->scaley = (size_y > 0) ? CAST_REAL(box.b_ymax/size_y) : 1;
      ob->scalez = (size_z > 0) ? CAST_REAL(box.b_zmax/size_z) : 1;
   }

   /*
    * We use the highest 5 bits (at least bits 0...4) for the Morton code when using
    * a hash (**node_hash) to reduce the dept of the trees.
    */
   ob->hbitx = ob_uinthighbit(CAST_OBCOORD(ob->scalex*size_x));
   ob->hbity = ob_uinthighbit(CAST_OBCOORD(ob->scaley*size_y));
   ob->hbitz = ob_uinthighbit(CAST_OBCOORD(ob->scalez*size_z));

   /* Need at least 5 bits [0...4] */
   if (ob->hbitx < 4) ob->hbitx = 4;
   if (ob->hbity < 4) ob->hbity = 4;
   if (ob->hbitz < 4) ob->hbitz = 4;
   ob->max_hbit = VMAX3(ob->hbitx,ob->hbity,ob->hbitz);

   /* The highest bit no. after the 3x5 bit Morton code is generated */
   ob->hbitx4    = ob->hbitx    - 4;
   ob->hbity4    = ob->hbity    - 4;
   ob->hbitz4    = ob->hbitz    - 4;
   ob->max_hbit4 = ob->max_hbit - 4;

   if (npoints > 0)
   {
      switch(ob->flags & OCTBOX_TYPE_MASK)
      {
         case OCTBOX_TYPE_ZBINARY:
            /*
             * No tree, just a flat list of points
             */
            nonodes = npoints;
            break;

         default:
         {
            /*
             * Estimate the no. of prospective octal nodes inside the tree
             * nch[ilds] must start between 2 (max. inhomogeneous distribution)
             * and 8 (100% homogeneous distribution)
             */
            int nch = 2;                 /* Assume two child-leaves per branch on the deepest level */
            int nop = nonodes = npoints; /* no. of leaves == no. of points */
            while(nop > 0)
            {
               nonodes += (nop /= nch); /* Rounded no. of branch nodes on level above */
               if (++nch > 8)           /* Increment the no. of childs per branch, but ... */
                  nch = 8;              /* ... never more than 8 childs */
            }
            break;
         }
      }
   }
   else
   {
      /*
       * Real bad assumption, might fail later on with out of memory in the IPOOL
       */
      nonodes = 1024;
   }

   if (DO_INFO)
   {
      /*
       * printf splitted into multiple calls due to gcc complaints about a
       * too long format string.
       */
      printf
      (
         "\n"
         "   %s(\"%s\") information:\n"
         "      Size of ONODE : %u bytes\n"
         "      Size of OLEAF : %u bytes\n"
         ,module
         ,ob->name
         ,CAST_UINT(sizeof(ONODE))
         ,CAST_UINT(sizeof(OLEAF))
      );
      printf
      (
         "      Flags        : 0x%08x: %s\n"
         "      No. of points: %d\n"
         "      No. of onodes: %d [+ %d]\n"
         "      Highest bits : %u %u %u (%u out of resolution limit %u)\n"
         ,ob->flags,ob_flags2str(ob->flags,descr)
         ,ob->npoints
         ,nonodes,nonodes/4
         ,ob->hbitx,ob->hbity,ob->hbitz,ob->max_hbit,ob_uinthighbit(OBBOX_MAXRES)
      );
      printf
      (
         "      Real range X : [ %12f ... %12f ], %g\n"
         "      Real range Y : [ %12f ... %12f ], %g\n"
         "      Real range Z : [ %12f ... %12f ], %g\n"
         ,xmin,xmax,size_x
         ,ymin,ymax,size_y
         ,zmin,zmax,size_z
      );

      printf
      (
         "      Uint range X : [ %12" OBFMT_CU " ... %12" OBFMT_CU " ], %" OBFMT_CU "\n"
         "      Uint range Y : [ %12" OBFMT_CU " ... %12" OBFMT_CU " ], %" OBFMT_CU "\n"
         "      Uint range Z : [ %12" OBFMT_CU " ... %12" OBFMT_CU " ], %" OBFMT_CU " \n"
         ,box.b_xmin
         ,box.b_xmax
         ,ob_nextpow2(box.b_xmax)
         ,box.b_ymin
         ,box.b_ymax
         ,ob_nextpow2(box.b_ymax)
         ,box.b_zmin
         ,box.b_zmax
         ,ob_nextpow2(box.b_zmax)
      );
      printf
      (
         "      Scale        : %g, %g, %g\n"
         ,ob->scalex,ob->scaley,ob->scalez
      );

      if (mindist > 0)
      {
         size_x = ob->scalex*mindist;
         size_y = ob->scaley*mindist;
         size_z = ob->scalez*mindist;
      }
      else
      {
         size_x = 1/ob->scalex;
         size_y = 1/ob->scaley;
         size_z = 1/ob->scalez;
      }
      printf
      (
         "      Min. distance: %13g [%g/%u %g/%u %g/%u]\n"
         ,ob->min_dist
         ,size_x,CAST_UINT(size_x)
         ,size_y,CAST_UINT(size_y)
         ,size_z,CAST_UINT(size_z)
      );

      if (maxdist > 0)
      {
         size_x = ob->scalex*maxdist;
         size_y = ob->scaley*maxdist;
         size_z = ob->scalez*maxdist;

         printf
         (
            "      Max. distance: %13g [%g/%u %g/%u %g/%u]\n"
            ,maxdist
            ,size_x,CAST_UINT(size_x)
            ,size_y,CAST_UINT(size_y)
            ,size_z,CAST_UINT(size_z)
         );
      }
   }

   /* Set up the Z-code hash */
   if (ob->flags & OCTBOX_FBIT_ZHASH)
   {
      if (DO_INFO)
      {
         printf("      Z-hash size  : 0 ... %u\n",OHASH_SIZE-1);
      }

      if (!ob->node_hash)
      {
         ob->node_hash = SCAST_INTO(ONODE **,MALLOC(OHASH_SIZE*sizeof(ONODE *)));
      }
      MEMZERO(ob->node_hash,OHASH_SIZE*sizeof(ONODE *));
   }
   else if (ob->node_hash)
   {
      FREE(ob->node_hash);
      ob->node_hash = NULL;
   }

   /* Set up the node pool */
   IPOOL_setup(&(ob->node_pool),"ONODE",sizeof(ONODE),nonodes,nonodes/4,0,DO_DEBUG);
   ob->state = OCTBOX_STATE_SETUP;
   if (DO_INFO)
   {
      fflush(stdout);
   }
   return ob;
}

/****************************************************************************************/

#if OCTBOX_USE_SETUPC
/****************************************************************************************/
C_FUNC_PREFIX OCTBOX *OCTBOX_setup_c
(
   OCTBOX        *ob,
   const char    *name,
   const int      npoints,
   const real     mindist,
   const real     maxdist,
   const unsigned flags,
   const real    *coords
)
/****************************************************************************************/
{
   const real *cp;
   real        xmin,ymin,zmin, xmax,ymax,zmax;
   int         i;


   cp   = coords;
   xmin = xmax = *cp++;
   ymin = ymax = *cp++;
   zmin = zmax = *cp++;
   for(i=1; i<npoints; i++)
   {
      register real cv;
      if((cv=*cp++)<xmin) xmin=cv; else if(cv>xmax) xmax=cv;
      if((cv=*cp++)<ymin) ymin=cv; else if(cv>ymax) ymax=cv;
      if((cv=*cp++)<zmin) zmin=cv; else if(cv>zmax) zmax=cv;
   }

   OCTBOX_setup_b(ob,name,npoints,mindist,maxdist,flags,xmin,ymin,zmin,xmax,ymax,zmax);

   if (flags & OCTBOX_FBIT_INSERT)
   {
      cp = coords;
      for(i=0; i<npoints; i++,cp+=3)
      {
         OCTBOX_insert_rv3(ob,cp,i,0);
      }
      ob->flags &= ~OCTBOX_FBIT_INSERT;
   }

   return ob;
}

/****************************************************************************************/
#endif

/****************************************************************************************/
C_FUNC_PREFIX void OCTBOX_cleanup(OCTBOX *ob)
/****************************************************************************************/
{
   if (ob)
   {
      if (DO_DEBUG)
      {
         char descr[128];
         printf
         (
            "\n"
            "   OCTBOX_cleanup(\"%s\"):\n"
            "      Flags: 0x%08x: %s\n"
            ,ob->name
            ,ob->flags
            ,ob_flags2str(ob->flags,descr)
         );
      }

      if (ob->node_hash)
      {
         FREE(ob->node_hash);
      }

      IPOOL_cleanup(&(ob->node_pool));
      MEMZERO(ob,sizeof(OCTBOX));
   }
}

/****************************************************************************************/

#if OCTBOX_USE_CREATEB
/****************************************************************************************/
C_FUNC_PREFIX OCTBOX *OCTBOX_create_b
(
   const char    *name,
   const int      npoints,
   const real     mindist,
   const real     maxdist,
   const unsigned flags,
   const real     xmin,
   const real     ymin,
   const real     zmin,
   const real     xmax,
   const real     ymax,
   const real     zmax
)
/****************************************************************************************/
{
   OCTBOX *ob = SCAST_INTO(OCTBOX *,MALLOC(sizeof(OCTBOX)));

   MEMZERO(ob,sizeof(OCTBOX));
   return OCTBOX_setup_b(ob,name,npoints,mindist,maxdist,flags,xmin,ymin,zmin,xmax,ymax,zmax);
}

/****************************************************************************************/
#endif

#if OCTBOX_USE_CREATEC
/****************************************************************************************/
C_FUNC_PREFIX OCTBOX *OCTBOX_create_c
(
   const char    *name,
   const int      npoints,
   const real     mindist,
   const real     maxdist,
   const unsigned flags,
   const real    *coords
)
/****************************************************************************************/
{
   OCTBOX *ob = SCAST_INTO(OCTBOX *,MALLOC(sizeof(OCTBOX)));

   MEMZERO(ob,sizeof(OCTBOX));
   return OCTBOX_setup_c(ob,name,npoints,mindist,maxdist,flags,coords);
}

/****************************************************************************************/
#endif

#if OCTBOX_USE_CREATEC || OCTBOX_USE_CREATEB
/****************************************************************************************/
C_FUNC_PREFIX void OCTBOX_destroy(OCTBOX **ob)
/****************************************************************************************/
{
   if (ob && *ob)
   {
      OCTBOX_cleanup(*ob);
      FREE(*ob);
   }
   *ob = NULL;
}

/****************************************************************************************/
#endif


/***************************************************************************************/
static int ob_is_oleaf(const void *p)
/***************************************************************************************/
{
   return ((RCAST_INTO(const ONODE *,p))->bleaf == BLEAF_TRUE);
}

/****************************************************************************************/

/***************************************************************************************/

static OCTBOX *ob_static_qsort_ob = NULL; /* for use with the qsort() compare function */

/***************************************************************************************/
static int ob_oleaf_cmp_morton_qsort(const void *ol1, const void *ol2)
/***************************************************************************************/
/*
 * qsort() compare wrapper for ob_oleaf_cmp_morton()
 */
{
   return ob_oleaf_cmp_morton
          (
            SCAST_INTO(const OLEAF*,ol1),
            SCAST_INTO(const OLEAF*,ol2),
            1<<ob_static_qsort_ob->max_hbit
          );
}

/****************************************************************************************/
static void ob_print_usage(const OCTBOX *ob, const char *fname)
/***************************************************************************************/
{
   printf
   (
      "\n"
      "   %s(\"%s\"):\n"
      "      Expected points: %10u\n"
      "      Inserted points: %10u\n"
      ,fname
      ,ob->name
      ,ob->npoints
      ,ob->nleaves
   );

#if OCTBOX_USE_RLOG
   printf
   (
      "      Max. tree depth: %10u\n"
      ,ob->max_depth
   );
#endif
}

/***************************************************************************************/

#if 0
/***************************************************************************************/
static void ob_print_leaf_array(OCTBOX *ob)
/***************************************************************************************/
{
   OLEAF   *l = SCAST_INTO(OLEAF *,ob->leaf_array);
   unsigned i;

   for(i=0; i<ob->nleaves;i++,l++)
   {
      printf("%8u   %12" OBFMT_CU " %12" OBFMT_CU " %12" OBFMT_CU "\n",i,l->ux,l->uy,l->uz);
   }
}

/***************************************************************************************/
#endif

/***************************************************************************************/
C_FUNC_PREFIX void OCTBOX_close(OCTBOX *ob, const unsigned flags)
/****************************************************************************************/
{
   if (ob->ob_insert == ob_insert_impossible)
   {
      if (DO_DEBUG)
      {
         printf
         (
            "\n"
            "   OCTBOX_close(\"%s\"):\n"
            "      Octbox was already closed before.\n"
            ,ob->name
         );
      }
      return;
   }

   /* Cannot insert any longer: hook the insert-error routine. */
   ob->ob_insert = ob_insert_impossible;

   if (DO_DEBUG)
   {
      ob_print_usage(ob,"OCTBOX_close");
      printf
      (
         "      Close flags    : 0x%08x\n"
         ,flags
      );
   }

   if (flags & (OCTBOX_FBIT_COMPACT|OCTBOX_FBIT_SORTED))
   {
      IPOOL *ipool  = &(ob->node_pool);
      double cpusec = getcpuseconds();

      /*
       * The OLEAFs are now adressable as an array, but no longer
       * as a tree.
       */
      ob->leaf_array = IPOOL_GETBLOCKPTR0(ipool);

      if ((ob->flags & OCTBOX_FBIT_COMPACT) == 0)
      {
         size_t nblocks,nitems;

         if (DO_DEBUG)
         {
            puts("      Compacting leaves ...");
         }

         IPOOL_setlog(ipool,0);
         IPOOL_usage (ipool,&nblocks,&nitems);
         IPOOL_setlog(ipool,DO_DEBUG);
         if (nblocks && nitems)
         {
            size_t nleaves = IPOOL_compact(ipool,"OLEAF",sizeof(OLEAF),ob_is_oleaf);
            #if 0
               printf
               (
                  "ob->npoints=%u, nleaves=%u, nleavesitems=%u\n"
                  ,ob->npoints
                  ,ob->nleaves
                  ,(unsigned)nleaves
               );
            #endif

            #if 0
               ob_print_leaf_array(ob);
            #endif

            if (nleaves != ob->nleaves)
            {
               XMSG_FATAL3
               (
                  "   OCTBOX_close(\"%s\"): Internal error:\n"
                  "     The number of inserted points %u does not match with the no. of leaves %u.\n"
                  ,ob->name
                  ,CAST_UINT(ob->nleaves)
                  ,CAST_UINT(nleaves)
               );
            }

            if (flags & OCTBOX_FBIT_NODOUBLE)
            {
               const OLEAF *src;
                     OLEAF *dst = SCAST_INTO(OLEAF *,ob->leaf_array);
               const OLEAF *end = dst + nleaves;


               for(src=dst+1; src<end; src++,dst++)
               {
                  while(
                           dst->l_ux == src->l_ux &&
                           dst->l_uy == src->l_uy &&
                           dst->l_uz == src->l_uz
                           && src < end
                       )
                  {
                     src++;
                  }

                  if (src > dst)
                     *dst = *src;
               }
            }
         }
         else
         {
            ob->nleaves = 0;
         }

         ob->flags |= OCTBOX_FBIT_COMPACT;

         /*
          * The node_hash cannot be used any longer.
          * There is no tree any longer.
          */
         if (ob->node_hash)
         {
            FREE(ob->node_hash);
            ob->node_hash = NULL;
         }
         ob->max_depth = 0;
      }

      if (flags & OCTBOX_FBIT_SORTED)
      {
         if ((ob->flags & OCTBOX_FBIT_SORTED) == 0)
         {
            if (DO_DEBUG)
            {
               puts("      Z-sorting leaves ...");
            }

            if (ob->nleaves > 1)
            {
               ob_static_qsort_ob = ob; /* set ob for the cmp function context */
               qsort(ob->leaf_array,ob->nleaves,sizeof(OLEAF),ob_oleaf_cmp_morton_qsort);
            #if 0
               ob_print_leaf_array(ob);
            #endif
            }
         }

         ob->ob_find   = ob_find_zbin;
         ob->ob_search = ob_search_zbin;
         ob->flags    &= ~OCTBOX_TYPE_MASK;
         ob->flags    |= (OCTBOX_FBIT_SORTED|OCTBOX_TYPE_ZBINARY);
      }
      else
      {
         ob->ob_find   = ob_find_bforce;
         ob->ob_search = ob_search_bforce;
         ob->flags    &= ~OCTBOX_TYPE_MASK;
         ob->flags    |= OCTBOX_TYPE_GEOM;
         puts("      ****WARNING: Switched to brute force find and search.");
      }

      if (DO_DEBUG)
      {
         char descr[128];
         printf
         (
            "      Final flags    : 0x%08x: %s\n"
            "      CPU time used  : %g sec.\n"
            ,ob->flags
            ,ob_flags2str(ob->flags,descr)
            ,getcpuseconds()-cpusec
         );
      }
   }
}

/****************************************************************************************/


#if OCTBOX_USE_USAGE
/****************************************************************************************/
static void ob_print_octbox_tree_from_here
(
   const ONODE   *node,
   const unsigned idx,
   const unsigned level,
   const unsigned maxlevel
)
/****************************************************************************************/
{
   unsigned i;


   if (level > maxlevel)
      return;

   for(i=0; i<=level; i++)
   {
      fputs("   ",stdout);
   }

   if (!node)
   {
      printf
      (
         "Node %u-%u: NULL\n"
         ,level
         ,idx
      );
   }

   else if (ONODE_IS_LEAF(node))
   {
      const OLEAF *leaf = &(node->oleaf);
      printf
      (
         "Node %u-%u: %p, Leaf: idx=%d, coord=(%" OBFMT_CU " %" OBFMT_CU " %" OBFMT_CU "), flags=0x%08x\n"
         ,level
         ,idx
         ,(void *)leaf
         ,leaf->l_idx
         ,leaf->l_ux
         ,leaf->l_uy
         ,leaf->l_uz
         ,leaf->l_flags
      );
   }

   else
   {
      printf
      (
         "Node %u-%u: %p, Branch ("
         ,level
         ,idx
         ,(void *)node
      );
      for(i=0; i<8; i++)
      {
         putchar((node->child[i]) ? '1' : '0');
      }
      puts(")");

      if (level < maxlevel)
      {
         for(i=0; i<8; i++)
         {
            ob_print_octbox_tree_from_here(node->child[i],i,level+1,maxlevel);
         }
      }
   }
}

/****************************************************************************************/

#include "ulong2bin.c"

/****************************************************************************************/
C_FUNC_PREFIX void OCTBOX_usage(const OCTBOX *ob, const int maxlevel)
/****************************************************************************************/
{
   unsigned i;


   if (!ob)
      return;

   ob_print_usage(ob,"OCTBOX_usage");

   if (maxlevel >= 0 && !ob->node_hash)
   {
      /* The root node itself is never a leaf node, thus list just all childs */
      const ONODE *onode = &(ob->root_node);
      for(i=0; i<8; i++)
      {
         ob_print_octbox_tree_from_here(onode->child[i],0,0,CAST_UINT(maxlevel));
      }
   }

   if (ob->node_hash)
   {
      unsigned nused;
      unsigned nleaves   = 0;
      unsigned nbranches = 0;
      unsigned keyhigh   = 0;
      unsigned keylow    = 0;

      for(i=0; i<OHASH_SIZE; i++)
      {
         const ONODE *node = ob->node_hash[i];
         if (node)
         {
            if (ONODE_IS_LEAF(node))
               nleaves++;
            else
               nbranches++;
            keyhigh = i;
            if (!keylow) keylow = i;
         }
      }

      if ((nused=nleaves+nbranches)>0)
      {
         if (ob->node_hash[0]) keylow = 0;

         printf
         (
            "      Hash occupancy : %10u out of %u (%.2g%%)\n"
            "      No. of branches: %10u\n"
            "      No. of leaves  : %10u\n"
            ,nused,OHASH_SIZE
            ,(100.0*nused)/OHASH_SIZE
            ,nbranches
            ,nleaves
         );
         printf
         (
            "      Low hash key   : %10u, %15s\n"
            ,keylow,ulong2bin(keylow,15,NULL,0)
         );
         printf
         (
            "      High hash key  : %10u, %15s\n"
            ,keyhigh,ulong2bin(keyhigh,15,NULL,0)
         );

         if (maxlevel >= 0)
         {
            for(i=0; i<OHASH_SIZE; i++)
            {
               const ONODE *onode = ob->node_hash[i];
               if (onode)
               {
                  ob_print_octbox_tree_from_here(onode,i,0,CAST_UINT(maxlevel));
               }
            }
         }
      }
      else
      {
         puts("      Node hash not used.");
      }
   }

   IPOOL_usage(&(ob->node_pool),NULL,NULL);
}

/****************************************************************************************/
#endif

#undef XYZ_ORDER
#undef OMASK_HIGHX
#undef OMASK_HIGHY
#undef OMASK_HIGHZ
#undef BLEAF_TRUE

#undef OBBOX_MINRES
#undef OBBOX_MAXRES
#undef OBBOX_CENTER

#undef OHASH_SIZE
#undef OBFMT_CU

#undef OUTSIDE_BBOX_DOMAIN
#undef OUTSIDE_OCT_DOMAIN

#undef MOVE_INSIDE_OCT_DOMAIN

#undef OBBOX_GETOCTANT
#undef OBBOX_NEWOCTANT
#undef OBBOX_SETOCTANT

#undef MORTON_GETOCTANT_1M
#undef MORTON_GETOCTANT_3M

#undef MORTON_NEWOCTANT_1M
#undef MORTON_NEWOCTANT_3M

#undef OBBOX_OVERLAP_GT1
#undef OBBOX_OVERLAP_LE1
#undef OBBOX_OVERLAP_EQ0

#undef ONODE_IS_LEAF
#undef ONODE_IS_BRANCH
#undef ONODE_TRUE_BRANCH

#undef OLEAF_NEW
#undef ONODE_DUP
#undef VMAX3
#undef PUSH_LEAF_LIST

#endif
