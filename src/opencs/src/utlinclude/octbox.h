#pragma once
#ifndef octbox_HEADER_INCLUDED
#define octbox_HEADER_INCLUDED
/* octbox.h
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Define structures and function prototypes for the octal box-tree point container.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2012/Apr: Carsten Dehning, Initial release
 *    $Id: octbox.h 5823 2018-05-08 12:50:24Z dehning $
 *
 *****************************************************************************************
 */
#ifndef OCTBOX_USE_SETUPC
   #define OCTBOX_USE_SETUPC     0
#endif
#ifndef OCTBOX_USE_CREATEB
   #define OCTBOX_USE_CREATEB    0
#endif
#ifndef OCTBOX_USE_CREATEC
   #define OCTBOX_USE_CREATEC    0
#endif
#ifndef OCTBOX_USE_MATCH
   #define OCTBOX_USE_MATCH      0
#endif
#ifndef OCTBOX_USE_USAGE
   #define OCTBOX_USE_USAGE      0
#endif
#ifndef OCTBOX_USE_RLOG
   #define OCTBOX_USE_RLOG       0
#endif
#ifndef OCTBOX_USE_HIGHRES
   #define OCTBOX_USE_HIGHRES    0
#endif
#ifndef OCTBOX_USE_UINT64
   #define OCTBOX_USE_UINT64     0
#endif


#ifndef OCTBOX_CHECK_DOMAIN
   #define OCTBOX_CHECK_DOMAIN   1
#endif

#if OCTBOX_USE_USAGE
   #define IPOOL_USE_USAGE       1
#endif

#define IPOOL_USE_COMPACT        1


#include "ipool.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OCTBOX_TYPE_MASK      0x00000fff /* insert mask (0,1,2) */
#define OCTBOX_FBIT_ZHASH     0x00000100 /* bit set when a z-hash is used on top */

#define OCTBOX_TYPE_GEOM      0x00000000 /* insert based on geometry */
#define OCTBOX_TYPE_GEOMZ     (OCTBOX_TYPE_GEOM|OCTBOX_FBIT_ZHASH)
#define OCTBOX_TYPE_MORTON    0x00000001 /* insert based on z-curve (morton code) */
#define OCTBOX_TYPE_MORTON3   0x00000003 /* insert based on bit left aligned morton code (no search possible) */
#define OCTBOX_TYPE_ZHASH    (OCTBOX_TYPE_MORTON |OCTBOX_FBIT_ZHASH)
#define OCTBOX_TYPE_ZHASH3   (OCTBOX_TYPE_MORTON3|OCTBOX_FBIT_ZHASH)
#define OCTBOX_TYPE_ZBINARY   0x00000004
#define OCTBOX_TYPE_KDTREE    0x00000008

#define OCTBOX_MASK_MESH      0x0000f000 /*  */
#define OCTBOX_FLAG_POINT     0x00000000 /*  */
#define OCTBOX_FLAG_NODEC     0x00001000 /*  */
#define OCTBOX_FLAG_ECENT     0x00002000 /*  */
#define OCTBOX_FLAG_NDELC     0x00003000 /*  */

#define OCTBOX_FBIT_POW2      0x00010000 /* Make the uint box size a power of 2 */
#define OCTBOX_FBIT_CUBE      0x00020000 /* Make the uint box a cube (scale-x-y-z are different) */


#define OCTBOX_FBIT_COMPACT   0x00100000 /* internal: ob_oleaf_compact() has been called */
#define OCTBOX_FBIT_SORTED    0x00200000 /* internal: ob_oleaf_zsort() has been called */
#define OCTBOX_FBIT_KEEPLIST  0x00400000 /* put doubly defined points into a list */
#define OCTBOX_FBIT_NODOUBLE  0x00800000 /* remove doubly defined points from the list */
#define OCTBOX_FBIT_INSERT    0x01000000 /* if true, insert all nodes with setup_c/create_c */

/*
 * Convert reals into (unsigned) coordinates
 */
#if OCTBOX_USE_UINT64
   typedef uint64_t        OCTBOX_C;
   #define CAST_OBCOORD    CAST_UINT64
#else
   typedef unsigned        OCTBOX_C;
   #define CAST_OBCOORD    CAST_UINT
#endif

#define OCTBOX_UX(_ob,_rx)    ( CAST_OBCOORD(_ob->scalex*( (_rx) - _ob->minx )) )
#define OCTBOX_UY(_ob,_ry)    ( CAST_OBCOORD(_ob->scaley*( (_ry) - _ob->miny )) )
#define OCTBOX_UZ(_ob,_rz)    ( CAST_OBCOORD(_ob->scalez*( (_rz) - _ob->minz )) )

/*
 * Reverse conversion: This may not be the real result due to a previous real->uint roundoff
 */
#define OCTBOX_RX(_ob,_ux)    ( CAST_REAL((_ux)/_ob->scalex + _ob->minx) )
#define OCTBOX_RY(_ob,_uy)    ( CAST_REAL((_uy)/_ob->scaley + _ob->miny) )
#define OCTBOX_RZ(_ob,_uz)    ( CAST_REAL((_uz)/_ob->scalez + _ob->minz) )

#define OCTBOX_UXMAX(_ob)     _ob->root_box.b_xmax
#define OCTBOX_UYMAX(_ob)     _ob->root_box.b_ymax
#define OCTBOX_UZMAX(_ob)     _ob->root_box.b_zmax

/*
 * Bounding box of an octal node, includes a center point b_mid = (min+max)/2
 * which is permanently shifted while walking down along the tree.
 */
typedef struct _OBBOX   OBBOX;
struct _OBBOX
{
   OCTBOX_C b_xmin,b_xmax,b_xmid;
   OCTBOX_C b_ymin,b_ymax,b_ymid;
   OCTBOX_C b_zmin,b_zmax,b_zmid;
};

/*
 * Range bounding box of a point, includes a r_range = (max-min)/2
 * build around a fixed point r_ux.
 */
typedef struct _ORBOX   ORBOX;
struct _ORBOX
{
   double   r_range2;    /* == r_range*r_range */
   OCTBOX_C r_ux,r_xmin,r_xmax;
   OCTBOX_C r_uy,r_ymin,r_ymax;
   OCTBOX_C r_uz,r_zmin,r_zmax;
   OCTBOX_C r_range;
};

/*
 * Range bounding box of a point, includes a z_range = (max-min)/2
 * build around a fixed point z_ux, using Morton code.
 */
typedef struct _OZBOX   OZBOX;
struct _OZBOX
{
   ORBOX    rbox;
   unsigned z_min,z_max;
};


/*
 * Special kind of BOX used for a KD-Tree with the point data.
 */
typedef struct _KDBOX   KDBOX;
struct _KDBOX
{
   real k_xmin,k_xmax;
   real k_ymin,k_ymax;
   real k_zmin,k_zmax;
};


/*
 * Special kind of ONODE (leaf node) with the leaf point data.
 */
typedef struct _OLEAF   OLEAF;
struct _OLEAF
{
   void    *l_bleaf; /* boolean: (void *)1 == OLEAF, otherwise false == child[0] */
   OLEAF   *l_next;  /* pointer to next OLEAF in the (optional) OLEAF list */
   real     l_rx;    /* real space coordinates */
   real     l_ry;
   real     l_rz;
   OCTBOX_C l_ux;    /* translated unsigned coordinates */
   OCTBOX_C l_uy;
   OCTBOX_C l_uz;
   unsigned l_flags; /* optional flags */
   int      l_idx;   /* index of this leaf node */
};

typedef struct _OLEAFLIST  OLEAFLIST;
struct _OLEAFLIST
{
   const OLEAF *ll_list[2];
   double       ll_dist2;
};

/*
 * The general node in the octal tree. Rule: sizeof(ONODE) >= sizeof(OLEAF)
 */
typedef union _ONODE ONODE;
union _ONODE
{
   void  *bleaf;     /* boolean: (void *)1 == OLEAF, otherwise false == child[0] */
   ONODE *child[8];  /* 8 childs in case of a branch node */
   OLEAF  oleaf;     /* bleaf==(void*)1: contains the leaf node information */
};

/*
 * Special kind of node used for a KD-Tree with the point data.
 */
typedef struct _KDNODE KDNODE;
struct _KDNODE
{
   real **k_leafhead; /* NULL=intermediate node, else pointer to the leaf start */
   real   k_splitval; /* split coordinate in case of an intermediate node */
   union
   {
      int      splitdim;
      unsigned npoints;     /* no. of points below this node */
   } k;
};


typedef struct _OCTBOX  OCTBOX;

typedef const OLEAF *(OCTBOX_INSERT_FUNCTION)(      OCTBOX *ob, const real rx, const real ry, const real rz, const int idx, const unsigned flags);
typedef const OLEAF *(OCTBOX_FIND_FUNCTION)  (const OCTBOX *ob, const real rx, const real ry, const real rz, const int nearest);
typedef const OLEAF *(OCTBOX_SEARCH_FUNCTION)(const OCTBOX *ob, const real rx, const real ry, const real rz, unsigned srange);

struct _OCTBOX
{
   OCTBOX_INSERT_FUNCTION *ob_insert;  /* pointer to the insert function */
   OCTBOX_SEARCH_FUNCTION *ob_search;  /* pointer to the search function */
   OCTBOX_FIND_FUNCTION   *ob_find;    /* pointer to the fast find function */

   const char *name;                   /* pointer to optional name/information string */
   ONODE  **node_hash;                 /* optional malloc'ed ONODE *node_hash[32768] used with dual trees */
   void    *leaf_array;                /* optional pointer to IPOOL-array of OLEAFSs used with binary search and sorted Z-curve */
   ONODE    root_node;                 /* root node with geometric boxes */
   IPOOL    node_pool;                 /* item memory pool for ONODE's */
   OBBOX    root_box;                  /* overall geometric box */
   real     scalex,scaley,scalez;      /* real->unsigned scale factor */
   real     minx  ,miny  ,minz;        /* offsets for real min -> 0,0,0 */
   real     min_dist;                  /* real minimal distance inside the grid used during setup */
   real     max_dist;                  /* real maximal distance inside the grid used during setup */
   unsigned npoints;                   /* expected no. of points == leaf nodes in the tree during setup */
   unsigned nleaves;                   /* finally inserted no. of points */
   unsigned hbitx ,hbity ,hbitz;       /* the highest bit for ux/uy/uz used for Morton (Z-Curve) hash */
   unsigned hbitx4,hbity4,hbitz4;      /* hbit - 4 */
   unsigned max_hbit;                  /* max(hbitx,hbity,hbitz) */
   unsigned max_hbit4;                 /* max_hbit-4 */
   unsigned max_depth;                 /* max. tree depth at runtime after all intertions */
   unsigned flags;                     /* copy of flags used during setup */
   unsigned keeplist;                  /* true if the flags&OCTBOX_FBIT_KEEPLIST bit is set */
   unsigned state;                     /* OCTBOX_STATE_xxxxx */
};


enum
{
   OCTBOX_STATE_NULL  =  0, /* must be 0 */
   OCTBOX_STATE_SETUP =  1
};

#if !INCLUDE_STATIC

   extern OCTBOX      *OCTBOX_setup_b   (OCTBOX  *ob, const char *name, const int npoints, const real mindist, const real maxdist, const unsigned flags, const real xmin, const real ymin, const real zmin, const real xmax, const real ymax, const real zmax);
   extern OCTBOX      *OCTBOX_setup_c   (OCTBOX  *ob, const char *name, const int npoints, const real mindist, const real maxdist, const unsigned flags, const real *coords);
   extern void         OCTBOX_cleanup   (OCTBOX  *ob);
   extern void         OCTBOX_close     (OCTBOX  *ob, const unsigned flags);
   extern void         OCTBOX_destroy   (OCTBOX **ob);

   extern OCTBOX      *OCTBOX_create_b  (const char *name, const int npoints, const real mindist, const real maxdist, const unsigned flags, const real xmin, const real ymin, const real zmin, const real xmax, const real ymax, const real zmax);
   extern OCTBOX      *OCTBOX_create_c  (const char *name, const int npoints, const real mindist, const real maxdist, const unsigned flags, const real *coords);

   extern void         OCTBOX_usage     (const OCTBOX  *ob, const int maxlevel);
   extern const OLEAF *OCTBOX_match     (const OCTBOX  *ob, const real rx, const real ry, const real rz);

#endif

/*
 * General purpose functions independent of the setup
 */
#define OCTBOX_match_rv3(_ob,_c)                     OCTBOX_match(_ob,_c[0],_c[1],_c[2])
#define OCTBOX_match_rv2(_ob,_c)                     OCTBOX_match(_ob,_c[0],_c[1],    0)
#define OCTBOX_match_rv1(_ob,_c)                     OCTBOX_match(_ob,_c[0],    0,    0)

/*
 * General purpose wrappers based on the setup, called via pointers
 */
#define OCTBOX_insert(_ob,_rx,_ry,_rz,_idx,_flags) _ob->ob_insert(_ob,  _rx,  _ry,  _rz,_idx,_flags)
#define OCTBOX_insert_rv3(_ob,_c,_idx,_flags)      _ob->ob_insert(_ob,_c[0],_c[1],_c[2],_idx,_flags)
#define OCTBOX_insert_rv2(_ob,_c,_idx,_flags)      _ob->ob_insert(_ob,_c[0],_c[1],    0,_idx,_flags)
#define OCTBOX_insert_rv1(_ob,_c,_idx,_flags)      _ob->ob_insert(_ob,_c[0],    0,    0,_idx,_flags)

#define OCTBOX_find(_ob,_rx,_ry,_rz,_n)            _ob->ob_find  (_ob,  _rx,  _ry,  _rz,_n)
#define OCTBOX_find_rv3(_ob,_c,_n)                 _ob->ob_find  (_ob,_c[0],_c[1],_c[2],_n)
#define OCTBOX_find_rv2(_ob,_c,_n)                 _ob->ob_find  (_ob,_c[0],_c[1],    0,_n)
#define OCTBOX_find_rv1(_ob,_c,_n)                 _ob->ob_find  (_ob,_c[0],    0,    0,_n)

#define OCTBOX_search(_ob,_rx,_ry,_rz,_r)          _ob->ob_search(_ob,  _rx,  _ry,  _rz,_r)
#define OCTBOX_search_rv3(_ob,_c,_r)               _ob->ob_search(_ob,_c[0],_c[1],_c[2],_r)
#define OCTBOX_search_rv2(_ob,_c,_r)               _ob->ob_search(_ob,_c[0],_c[1],    0,_r)
#define OCTBOX_search_rv1(_ob,_c,_r)               _ob->ob_search(_ob,_c[0],    0,    0,_r)

#ifdef __cplusplus
}
#endif

#endif
