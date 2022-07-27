#ifndef hlsrgbcolorconvert_SOURCE_INCLUDED
#define hlsrgbcolorconvert_SOURCE_INCLUDED
/* hlsrgbcolorconvert.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Include file for HLS->RGB->HLS color conversion.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/05: Carsten Dehning, Initial release
 *    $Id: hlsrgbcolorconvert.c 2747 2014-03-27 16:30:40Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#define VALUE01_LIMIT(_val) \
   if (_val<0.0) _val=0.0; else if(_val>1.0) _val=1.0

#define HUE360_LIMIT(_hue) \
   while (_hue>360.0) _hue-=360.0; while (_hue<0.0) _hue+=360.0;


/****************************************************************************************/
C_FUNC_PREFIX float _HLStoRGB1(float rn1, float rn2, float hue)
/****************************************************************************************/
{
   if (hue <  60.0) return rn1 + (rn2-rn1)*hue/60.0f;
   if (hue < 180.0) return rn2;
   if (hue < 240.0) return rn1 + (rn2-rn1)*(240.0f-hue)/60.0f;
                    return rn1;
}

/****************************************************************************************/
C_FUNC_PREFIX void HLStoRGB
(
   float hue, float lig, float sat,
   float *r , float *g , float *b
)
/****************************************************************************************/
{
   float rm1, rm2;

   VALUE01_LIMIT(lig);
   VALUE01_LIMIT(sat);

   rm2 = (lig <= 0.5) ?  lig + lig*sat : lig + sat - lig*sat;
   rm1 = 2.0f*lig - rm2;

   if (sat)
   {
      HUE360_LIMIT(hue);
      *r = _HLStoRGB1(rm1, rm2, hue+120.0f);
      *g = _HLStoRGB1(rm1, rm2, hue       );
      *b = _HLStoRGB1(rm1, rm2, hue-120.0f);
   }
   else
   {
      /* Black & white only */
      *r =
      *g =
      *b = lig;
   }
}

/****************************************************************************************/
C_FUNC_PREFIX void RGBtoHLS
(
   float r    , float g    , float b    ,
   float *_hue, float *_lig, float *_sat
)
/****************************************************************************************/
{
   float minval, maxval, msum, mdiff, hue,lig,sat;
   int   imax;

   VALUE01_LIMIT(r);
   VALUE01_LIMIT(g);
   VALUE01_LIMIT(b);

                   minval =                           maxval = r; imax = -1;
   if (g < minval) minval = g; else if (g > maxval) { maxval = g; imax =  0; }
   if (b < minval) minval = b; else if (b > maxval) { maxval = b; imax =  1; }

   msum = maxval + minval;
   lig  = msum*0.5f;
   VALUE01_LIMIT(lig);

   mdiff = maxval - minval;
   if (mdiff > 0.0)
   {
      sat = (lig <= 0.5f) ? mdiff/msum : mdiff/(2.0f-msum);
      VALUE01_LIMIT(sat);

      hue = (imax < 0) ? 60.0f * (       (g-b)/mdiff) : /* red   == maxval */
            (imax > 0) ? 60.0f * (4.0f + (r-g)/mdiff) : /* blue  == maxval */
                         60.0f * (2.0f + (b-r)/mdiff);  /* green == maxval */
      HUE360_LIMIT(hue);
   }
   else
   {
      hue = sat = 0.0f;
   }

   *_hue = hue;
   *_lig = lig;
   *_sat = sat;
}

/****************************************************************************************/

#undef VALUE01_LIMIT
#undef HUE360_LIMIT

#endif
