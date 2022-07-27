#ifndef getx11color_SOURCE_INCLUDED
#define getx11color_SOURCE_INCLUDED
/* getx11color.c
 *
 *****************************************************************************************
 *
 * Purpose:
 *    Get an RGB color pattern via an X11 color name.
 *
 * Author:
 *    Carsten Dehning <carsten.dehning@scai.fraunhofer.de>
 *
 * Reviews/changes:
 *    2006/May/06: Carsten Dehning, Initial release
 *    $Id: getx11color.c 4911 2016-07-12 12:00:53Z dehning $
 *
 *****************************************************************************************
 */
#include "stdheader.h"

#if INCLUDE_STATIC
   #include "hlsrgbcolorconvert.c"
   #if !IS_MSWIN
      #include "strlower.c"
   #endif
#endif

#define MAX_X11_COLORS        ( countof(staticX11ColorNamesMap) - 1 )

#define X11COLOR_LOOP_ALL(_ce) \
   for (_ce=staticX11ColorNamesMap; _ce->name; _ce++)

/*
 * return the RGB 0x00rrggbb value of a color entry
 */
#define X11CETO0RGB(_e) (\
                           ( ( CAST_UINT((_e)->r) ) <<16 )|\
                           ( ( CAST_UINT((_e)->g) ) << 8 )|\
                           (   CAST_UINT((_e)->b) )        \
                        )

#define X11CETO0BGR(_e) (\
                           ( ( CAST_UINT((_e)->b) ) <<16 )|\
                           ( ( CAST_UINT((_e)->g) ) << 8 )|\
                           (   CAST_UINT((_e)->r) )        \
                        )

#define GetX11ColorEntrys()   staticX11ColorNamesMap

#define GetX11ColorEntryI(_i) ( ((_i)<0||(_i)>=MAX_X11_COLORS) ? (X11COLOR_t*)NULL : staticX11ColorNamesMap+(_i) )
#define GetX11ColorNameI(_i)  ( ((_i)<0||(_i)>=MAX_X11_COLORS) ? (TCHAR*)NULL : staticX11ColorNamesMap[_i].name )


typedef struct _X11colors_struct
{
   const    TCHAR *name;
   unsigned char   r,g,b;
} X11COLOR_t;

static X11COLOR_t staticX11ColorNamesMap[] =
{
   { TEXT("alice")                  , 240, 248, 255 },
   { TEXT("aliceblue")              , 240, 248, 255 },
   { TEXT("antique")                , 250, 235, 215 },
   { TEXT("antiquewhite")           , 250, 235, 215 },
   { TEXT("antiquewhite1")          , 255, 239, 219 },
   { TEXT("antiquewhite2")          , 238, 223, 204 },
   { TEXT("antiquewhite3")          , 205, 192, 176 },
   { TEXT("antiquewhite4")          , 139, 131, 120 },
   { TEXT("aquamarine")             , 127, 255, 212 },
   { TEXT("aquamarine1")            , 127, 255, 212 },
   { TEXT("aquamarine2")            , 118, 238, 198 },
   { TEXT("aquamarine3")            , 102, 205, 170 },
   { TEXT("aquamarine4")            ,  69, 139, 116 },
   { TEXT("azure")                  , 240, 255, 255 },
   { TEXT("azure1")                 , 240, 255, 255 },
   { TEXT("azure2")                 , 224, 238, 238 },
   { TEXT("azure3")                 , 193, 205, 205 },
   { TEXT("azure4")                 , 131, 139, 139 },
   { TEXT("beige")                  , 245, 245, 220 },
   { TEXT("bisque")                 , 255, 228, 196 },
   { TEXT("bisque1")                , 255, 228, 196 },
   { TEXT("bisque2")                , 238, 213, 183 },
   { TEXT("bisque3")                , 205, 183, 158 },
   { TEXT("bisque4")                , 139, 125, 107 },
   { TEXT("black")                  ,   0,   0,   0 },
   { TEXT("blanched")               , 255, 235, 205 },
   { TEXT("blanchedalmond")         , 255, 235, 205 },
   { TEXT("blue")                   , 138,  43, 226 },
   { TEXT("blue1")                  ,   0,   0, 255 },
   { TEXT("blue2")                  ,   0,   0, 238 },
   { TEXT("blue3")                  ,   0,   0, 205 },
   { TEXT("blue4")                  ,   0,   0, 139 },
   { TEXT("blueviolet")             , 138,  43, 226 },
   { TEXT("brown")                  , 165,  42,  42 },
   { TEXT("brown1")                 , 255,  64,  64 },
   { TEXT("brown2")                 , 238,  59,  59 },
   { TEXT("brown3")                 , 205,  51,  51 },
   { TEXT("brown4")                 , 139,  35,  35 },
   { TEXT("burlywood")              , 222, 184, 135 },
   { TEXT("burlywood1")             , 255, 211, 155 },
   { TEXT("burlywood2")             , 238, 197, 145 },
   { TEXT("burlywood3")             , 205, 170, 125 },
   { TEXT("burlywood4")             , 139, 115,  85 },
   { TEXT("cadet")                  ,  95, 158, 160 },
   { TEXT("cadetblue")              ,  95, 158, 160 },
   { TEXT("cadetblue1")             , 152, 245, 255 },
   { TEXT("cadetblue2")             , 142, 229, 238 },
   { TEXT("cadetblue3")             , 122, 197, 205 },
   { TEXT("cadetblue4")             ,  83, 134, 139 },
   { TEXT("chartreuse")             , 127, 255,   0 },
   { TEXT("chartreuse1")            , 127, 255,   0 },
   { TEXT("chartreuse2")            , 118, 238,   0 },
   { TEXT("chartreuse3")            , 102, 205,   0 },
   { TEXT("chartreuse4")            ,  69, 139,   0 },
   { TEXT("chocolate")              , 210, 105,  30 },
   { TEXT("chocolate1")             , 255, 127,  36 },
   { TEXT("chocolate2")             , 238, 118,  33 },
   { TEXT("chocolate3")             , 205, 102,  29 },
   { TEXT("chocolate4")             , 139,  69,  19 },
   { TEXT("console")                , 255, 255, 190 },
   { TEXT("coral")                  , 255, 127,  80 },
   { TEXT("coral1")                 , 255, 114,  86 },
   { TEXT("coral2")                 , 238, 106,  80 },
   { TEXT("coral3")                 , 205,  91,  69 },
   { TEXT("coral4")                 , 139,  62,  47 },
   { TEXT("cornflower")             , 100, 149, 237 },
   { TEXT("cornflowerblue")         , 100, 149, 237 },
   { TEXT("cornsilk")               , 255, 248, 220 },
   { TEXT("cornsilk1")              , 255, 248, 220 },
   { TEXT("cornsilk2")              , 238, 232, 205 },
   { TEXT("cornsilk3")              , 205, 200, 177 },
   { TEXT("cornsilk4")              , 139, 136, 120 },
   { TEXT("cyan")                   ,   0, 255, 255 },
   { TEXT("cyan1")                  ,   0, 255, 255 },
   { TEXT("cyan2")                  ,   0, 238, 238 },
   { TEXT("cyan3")                  ,   0, 205, 205 },
   { TEXT("cyan4")                  ,   0, 139, 139 },
   { TEXT("dark")                   , 139,   0,   0 },
   { TEXT("darkblue")               ,   0,   0, 139 },
   { TEXT("darkcyan")               ,   0, 139, 139 },
   { TEXT("darkgoldenrod")          , 184, 134,  11 },
   { TEXT("darkgoldenrod1")         , 255, 185,  15 },
   { TEXT("darkgoldenrod2")         , 238, 173,  14 },
   { TEXT("darkgoldenrod3")         , 205, 149,  12 },
   { TEXT("darkgoldenrod4")         , 139, 101,   8 },
   { TEXT("darkgray")               , 169, 169, 169 },
   { TEXT("darkgreen")              ,   0, 100,   0 },
   { TEXT("darkkhaki")              , 189, 183, 107 },
   { TEXT("darkmagenta")            , 139,   0, 139 },
   { TEXT("darkolivegreen")         ,  85, 107,  47 },
   { TEXT("darkolivegreen1")        , 202, 255, 112 },
   { TEXT("darkolivegreen2")        , 188, 238, 104 },
   { TEXT("darkolivegreen3")        , 162, 205,  90 },
   { TEXT("darkolivegreen4")        , 110, 139,  61 },
   { TEXT("darkorange")             , 255, 140,   0 },
   { TEXT("darkorange1")            , 255, 127,   0 },
   { TEXT("darkorange2")            , 238, 118,   0 },
   { TEXT("darkorange3")            , 205, 102,   0 },
   { TEXT("darkorange4")            , 139,  69,   0 },
   { TEXT("darkorchid")             , 153,  50, 204 },
   { TEXT("darkorchid1")            , 191,  62, 255 },
   { TEXT("darkorchid2")            , 178,  58, 238 },
   { TEXT("darkorchid3")            , 154,  50, 205 },
   { TEXT("darkorchid4")            , 104,  34, 139 },
   { TEXT("darkred")                , 139,   0,   0 },
   { TEXT("darksalmon")             , 233, 150, 122 },
   { TEXT("darkseagreen")           , 143, 188, 143 },
   { TEXT("darkseagreen1")          , 193, 255, 193 },
   { TEXT("darkseagreen2")          , 180, 238, 180 },
   { TEXT("darkseagreen3")          , 155, 205, 155 },
   { TEXT("darkseagreen4")          , 105, 139, 105 },
   { TEXT("darkslateblue")          ,  72,  61, 139 },
   { TEXT("darkslategray")          ,  47,  79,  79 },
   { TEXT("darkslategray1")         , 151, 255, 255 },
   { TEXT("darkslategray2")         , 141, 238, 238 },
   { TEXT("darkslategray3")         , 121, 205, 205 },
   { TEXT("darkslategray4")         ,  82, 139, 139 },
   { TEXT("darkturquoise")          ,   0, 206, 209 },
   { TEXT("darkviolet")             , 148,   0, 211 },
   { TEXT("deep")                   , 255,  20, 147 },
   { TEXT("deeppink")               , 255,  20, 147 },
   { TEXT("deeppink1")              , 255,  20, 147 },
   { TEXT("deeppink2")              , 238,  18, 137 },
   { TEXT("deeppink3")              , 205,  16, 118 },
   { TEXT("deeppink4")              , 139,  10,  80 },
   { TEXT("deepskyblue")            ,   0, 191, 255 },
   { TEXT("deepskyblue1")           ,   0, 191, 255 },
   { TEXT("deepskyblue2")           ,   0, 178, 238 },
   { TEXT("deepskyblue3")           ,   0, 154, 205 },
   { TEXT("deepskyblue4")           ,   0, 104, 139 },
   { TEXT("dim")                    , 105, 105, 105 },
   { TEXT("dimgray")                , 105, 105, 105 },
   { TEXT("dodger")                 ,  30, 144, 255 },
   { TEXT("dodgerblue")             ,  30, 144, 255 },
   { TEXT("dodgerblue1")            ,  30, 144, 255 },
   { TEXT("dodgerblue2")            ,  28, 134, 238 },
   { TEXT("dodgerblue3")            ,  24, 116, 205 },
   { TEXT("dodgerblue4")            ,  16,  78, 139 },
   { TEXT("firebrick")              , 178,  34,  34 },
   { TEXT("firebrick1")             , 255,  48,  48 },
   { TEXT("firebrick2")             , 238,  44,  44 },
   { TEXT("firebrick3")             , 205,  38,  38 },
   { TEXT("firebrick4")             , 139,  26,  26 },
   { TEXT("floral")                 , 255, 250, 240 },
   { TEXT("floralwhite")            , 255, 250, 240 },
   { TEXT("forest")                 ,  34, 139,  34 },
   { TEXT("forestgreen")            ,  34, 139,  34 },
   { TEXT("gainsboro")              , 220, 220, 220 },
   { TEXT("ghost")                  , 248, 248, 255 },
   { TEXT("ghostwhite")             , 248, 248, 255 },
   { TEXT("gold")                   , 255, 215,   0 },
   { TEXT("gold1")                  , 255, 215,   0 },
   { TEXT("gold2")                  , 238, 201,   0 },
   { TEXT("gold3")                  , 205, 173,   0 },
   { TEXT("gold4")                  , 139, 117,   0 },
   { TEXT("goldenrod")              , 218, 165,  32 },
   { TEXT("goldenrod1")             , 255, 193,  37 },
   { TEXT("goldenrod2")             , 238, 180,  34 },
   { TEXT("goldenrod3")             , 205, 155,  29 },
   { TEXT("goldenrod4")             , 139, 105,  20 },
   { TEXT("gray")                   , 190, 190, 190 },
   { TEXT("gray0")                  ,   0,   0,   0 },
   { TEXT("gray1")                  ,   3,   3,   3 },
   { TEXT("gray10")                 ,  26,  26,  26 },
   { TEXT("gray100")                , 255, 255, 255 },
   { TEXT("gray11")                 ,  28,  28,  28 },
   { TEXT("gray12")                 ,  31,  31,  31 },
   { TEXT("gray13")                 ,  33,  33,  33 },
   { TEXT("gray14")                 ,  36,  36,  36 },
   { TEXT("gray15")                 ,  38,  38,  38 },
   { TEXT("gray16")                 ,  41,  41,  41 },
   { TEXT("gray17")                 ,  43,  43,  43 },
   { TEXT("gray18")                 ,  46,  46,  46 },
   { TEXT("gray19")                 ,  48,  48,  48 },
   { TEXT("gray2")                  ,   5,   5,   5 },
   { TEXT("gray20")                 ,  51,  51,  51 },
   { TEXT("gray21")                 ,  54,  54,  54 },
   { TEXT("gray22")                 ,  56,  56,  56 },
   { TEXT("gray23")                 ,  59,  59,  59 },
   { TEXT("gray24")                 ,  61,  61,  61 },
   { TEXT("gray25")                 ,  64,  64,  64 },
   { TEXT("gray26")                 ,  66,  66,  66 },
   { TEXT("gray27")                 ,  69,  69,  69 },
   { TEXT("gray28")                 ,  71,  71,  71 },
   { TEXT("gray29")                 ,  74,  74,  74 },
   { TEXT("gray3")                  ,   8,   8,   8 },
   { TEXT("gray30")                 ,  77,  77,  77 },
   { TEXT("gray31")                 ,  79,  79,  79 },
   { TEXT("gray32")                 ,  82,  82,  82 },
   { TEXT("gray33")                 ,  84,  84,  84 },
   { TEXT("gray34")                 ,  87,  87,  87 },
   { TEXT("gray35")                 ,  89,  89,  89 },
   { TEXT("gray36")                 ,  92,  92,  92 },
   { TEXT("gray37")                 ,  94,  94,  94 },
   { TEXT("gray38")                 ,  97,  97,  97 },
   { TEXT("gray39")                 ,  99,  99,  99 },
   { TEXT("gray4")                  ,  10,  10,  10 },
   { TEXT("gray40")                 , 102, 102, 102 },
   { TEXT("gray41")                 , 105, 105, 105 },
   { TEXT("gray42")                 , 107, 107, 107 },
   { TEXT("gray43")                 , 110, 110, 110 },
   { TEXT("gray44")                 , 112, 112, 112 },
   { TEXT("gray45")                 , 115, 115, 115 },
   { TEXT("gray46")                 , 117, 117, 117 },
   { TEXT("gray47")                 , 120, 120, 120 },
   { TEXT("gray48")                 , 122, 122, 122 },
   { TEXT("gray49")                 , 125, 125, 125 },
   { TEXT("gray5")                  ,  13,  13,  13 },
   { TEXT("gray50")                 , 127, 127, 127 },
   { TEXT("gray51")                 , 130, 130, 130 },
   { TEXT("gray52")                 , 133, 133, 133 },
   { TEXT("gray53")                 , 135, 135, 135 },
   { TEXT("gray54")                 , 138, 138, 138 },
   { TEXT("gray55")                 , 140, 140, 140 },
   { TEXT("gray56")                 , 143, 143, 143 },
   { TEXT("gray57")                 , 145, 145, 145 },
   { TEXT("gray58")                 , 148, 148, 148 },
   { TEXT("gray59")                 , 150, 150, 150 },
   { TEXT("gray6")                  ,  15,  15,  15 },
   { TEXT("gray60")                 , 153, 153, 153 },
   { TEXT("gray61")                 , 156, 156, 156 },
   { TEXT("gray62")                 , 158, 158, 158 },
   { TEXT("gray63")                 , 161, 161, 161 },
   { TEXT("gray64")                 , 163, 163, 163 },
   { TEXT("gray65")                 , 166, 166, 166 },
   { TEXT("gray66")                 , 168, 168, 168 },
   { TEXT("gray67")                 , 171, 171, 171 },
   { TEXT("gray68")                 , 173, 173, 173 },
   { TEXT("gray69")                 , 176, 176, 176 },
   { TEXT("gray7")                  ,  18,  18,  18 },
   { TEXT("gray70")                 , 179, 179, 179 },
   { TEXT("gray71")                 , 181, 181, 181 },
   { TEXT("gray72")                 , 184, 184, 184 },
   { TEXT("gray73")                 , 186, 186, 186 },
   { TEXT("gray74")                 , 189, 189, 189 },
   { TEXT("gray75")                 , 191, 191, 191 },
   { TEXT("gray76")                 , 194, 194, 194 },
   { TEXT("gray77")                 , 196, 196, 196 },
   { TEXT("gray78")                 , 199, 199, 199 },
   { TEXT("gray79")                 , 201, 201, 201 },
   { TEXT("gray8")                  ,  20,  20,  20 },
   { TEXT("gray80")                 , 204, 204, 204 },
   { TEXT("gray81")                 , 207, 207, 207 },
   { TEXT("gray82")                 , 209, 209, 209 },
   { TEXT("gray83")                 , 212, 212, 212 },
   { TEXT("gray84")                 , 214, 214, 214 },
   { TEXT("gray85")                 , 217, 217, 217 },
   { TEXT("gray86")                 , 219, 219, 219 },
   { TEXT("gray87")                 , 222, 222, 222 },
   { TEXT("gray88")                 , 224, 224, 224 },
   { TEXT("gray89")                 , 227, 227, 227 },
   { TEXT("gray9")                  ,  23,  23,  23 },
   { TEXT("gray90")                 , 229, 229, 229 },
   { TEXT("gray91")                 , 232, 232, 232 },
   { TEXT("gray92")                 , 235, 235, 235 },
   { TEXT("gray93")                 , 237, 237, 237 },
   { TEXT("gray94")                 , 240, 240, 240 },
   { TEXT("gray95")                 , 242, 242, 242 },
   { TEXT("gray96")                 , 245, 245, 245 },
   { TEXT("gray97")                 , 247, 247, 247 },
   { TEXT("gray98")                 , 250, 250, 250 },
   { TEXT("gray99")                 , 252, 252, 252 },
   { TEXT("green")                  , 173, 255,  47 },
   { TEXT("green1")                 ,   0, 255,   0 },
   { TEXT("green2")                 ,   0, 238,   0 },
   { TEXT("green3")                 ,   0, 205,   0 },
   { TEXT("green4")                 ,   0, 139,   0 },
   { TEXT("greenyellow")            , 173, 255,  47 },
   { TEXT("honeydew")               , 240, 255, 240 },
   { TEXT("honeydew1")              , 240, 255, 240 },
   { TEXT("honeydew2")              , 224, 238, 224 },
   { TEXT("honeydew3")              , 193, 205, 193 },
   { TEXT("honeydew4")              , 131, 139, 131 },
   { TEXT("hot")                    , 255, 105, 180 },
   { TEXT("hotpink")                , 255, 105, 180 },
   { TEXT("hotpink1")               , 255, 110, 180 },
   { TEXT("hotpink2")               , 238, 106, 167 },
   { TEXT("hotpink3")               , 205,  96, 144 },
   { TEXT("hotpink4")               , 139,  58,  98 },
   { TEXT("indian")                 , 205,  92,  92 },
   { TEXT("indianred")              , 205,  92,  92 },
   { TEXT("indianred1")             , 255, 106, 106 },
   { TEXT("indianred2")             , 238,  99,  99 },
   { TEXT("indianred3")             , 205,  85,  85 },
   { TEXT("indianred4")             , 139,  58,  58 },
   { TEXT("ivory")                  , 255, 255, 240 },
   { TEXT("ivory1")                 , 255, 255, 240 },
   { TEXT("ivory2")                 , 238, 238, 224 },
   { TEXT("ivory3")                 , 205, 205, 193 },
   { TEXT("ivory4")                 , 139, 139, 131 },
   { TEXT("khaki")                  , 240, 230, 140 },
   { TEXT("khaki1")                 , 255, 246, 143 },
   { TEXT("khaki2")                 , 238, 230, 133 },
   { TEXT("khaki3")                 , 205, 198, 115 },
   { TEXT("khaki4")                 , 139, 134,  78 },
   { TEXT("lavender")               , 255, 240, 245 },
   { TEXT("lavenderblush")          , 255, 240, 245 },
   { TEXT("lavenderblush1")         , 255, 240, 245 },
   { TEXT("lavenderblush2")         , 238, 224, 229 },
   { TEXT("lavenderblush3")         , 205, 193, 197 },
   { TEXT("lavenderblush4")         , 139, 131, 134 },
   { TEXT("lawn")                   , 124, 252,   0 },
   { TEXT("lawngreen")              , 124, 252,   0 },
   { TEXT("legendlime")             , 177, 208, 125 }, /* The famous exterior color of my 2006 Ford Mustang convertible */
   { TEXT("lemon")                  , 255, 250, 205 },
   { TEXT("lemonchiffon")           , 255, 250, 205 },
   { TEXT("lemonchiffon1")          , 255, 250, 205 },
   { TEXT("lemonchiffon2")          , 238, 233, 191 },
   { TEXT("lemonchiffon3")          , 205, 201, 165 },
   { TEXT("lemonchiffon4")          , 139, 137, 112 },
   { TEXT("light")                  , 144, 238, 144 },
   { TEXT("lightblue")              , 173, 216, 230 },
   { TEXT("lightblue1")             , 191, 239, 255 },
   { TEXT("lightblue2")             , 178, 223, 238 },
   { TEXT("lightblue3")             , 154, 192, 205 },
   { TEXT("lightblue4")             , 104, 131, 139 },
   { TEXT("lightcoral")             , 240, 128, 128 },
   { TEXT("lightcyan")              , 224, 255, 255 },
   { TEXT("lightcyan1")             , 224, 255, 255 },
   { TEXT("lightcyan2")             , 209, 238, 238 },
   { TEXT("lightcyan3")             , 180, 205, 205 },
   { TEXT("lightcyan4")             , 122, 139, 139 },
   { TEXT("lightgoldenrod")         , 238, 221, 130 },
   { TEXT("lightgoldenrod1")        , 255, 236, 139 },
   { TEXT("lightgoldenrod2")        , 238, 220, 130 },
   { TEXT("lightgoldenrod3")        , 205, 190, 112 },
   { TEXT("lightgoldenrod4")        , 139, 129,  76 },
   { TEXT("lightgoldenrodyellow")   , 250, 250, 210 },
   { TEXT("lightgray")              , 211, 211, 211 },
   { TEXT("lightgreen")             , 144, 238, 144 },
   { TEXT("lightpink")              , 255, 182, 193 },
   { TEXT("lightpink1")             , 255, 174, 185 },
   { TEXT("lightpink2")             , 238, 162, 173 },
   { TEXT("lightpink3")             , 205, 140, 149 },
   { TEXT("lightpink4")             , 139,  95, 101 },
   { TEXT("lightsalmon")            , 255, 160, 122 },
   { TEXT("lightsalmon1")           , 255, 160, 122 },
   { TEXT("lightsalmon2")           , 238, 149, 114 },
   { TEXT("lightsalmon3")           , 205, 129,  98 },
   { TEXT("lightsalmon4")           , 139,  87,  66 },
   { TEXT("lightseagreen")          ,  32, 178, 170 },
   { TEXT("lightskyblue")           , 135, 206, 250 },
   { TEXT("lightskyblue1")          , 176, 226, 255 },
   { TEXT("lightskyblue2")          , 164, 211, 238 },
   { TEXT("lightskyblue3")          , 141, 182, 205 },
   { TEXT("lightskyblue4")          ,  96, 123, 139 },
   { TEXT("lightslateblue")         , 132, 112, 255 },
   { TEXT("lightslategray")         , 119, 136, 153 },
   { TEXT("lightsteelblue")         , 176, 196, 222 },
   { TEXT("lightsteelblue1")        , 202, 225, 255 },
   { TEXT("lightsteelblue2")        , 188, 210, 238 },
   { TEXT("lightsteelblue3")        , 162, 181, 205 },
   { TEXT("lightsteelblue4")        , 110, 123, 139 },
   { TEXT("lightyellow")            , 255, 255, 224 },
   { TEXT("lightyellow1")           , 255, 255, 224 },
   { TEXT("lightyellow2")           , 238, 238, 209 },
   { TEXT("lightyellow3")           , 205, 205, 180 },
   { TEXT("lightyellow4")           , 139, 139, 122 },
   { TEXT("lime")                   ,  50, 205,  50 },
   { TEXT("limegreen")              ,  50, 205,  50 },
   { TEXT("linen")                  , 250, 240, 230 },
   { TEXT("magenta")                , 255,   0, 255 },
   { TEXT("magenta1")               , 255,   0, 255 },
   { TEXT("magenta2")               , 238,   0, 238 },
   { TEXT("magenta3")               , 205,   0, 205 },
   { TEXT("magenta4")               , 139,   0, 139 },
   { TEXT("maroon")                 , 176,  48,  96 },
   { TEXT("maroon1")                , 255,  52, 179 },
   { TEXT("maroon2")                , 238,  48, 167 },
   { TEXT("maroon3")                , 205,  41, 144 },
   { TEXT("maroon4")                , 139,  28,  98 },
   { TEXT("medium")                 , 147, 112, 219 },
   { TEXT("mediumaquamarine")       , 102, 205, 170 },
   { TEXT("mediumblue")             ,   0,   0, 205 },
   { TEXT("mediumorchid")           , 186,  85, 211 },
   { TEXT("mediumorchid1")          , 224, 102, 255 },
   { TEXT("mediumorchid2")          , 209,  95, 238 },
   { TEXT("mediumorchid3")          , 180,  82, 205 },
   { TEXT("mediumorchid4")          , 122,  55, 139 },
   { TEXT("mediumpurple")           , 147, 112, 219 },
   { TEXT("mediumpurple1")          , 171, 130, 255 },
   { TEXT("mediumpurple2")          , 159, 121, 238 },
   { TEXT("mediumpurple3")          , 137, 104, 205 },
   { TEXT("mediumpurple4")          ,  93,  71, 139 },
   { TEXT("mediumseagreen")         ,  60, 179, 113 },
   { TEXT("mediumslateblue")        , 123, 104, 238 },
   { TEXT("mediumspringgreen")      ,   0, 250, 154 },
   { TEXT("mediumturquoise")        ,  72, 209, 204 },
   { TEXT("mediumvioletred")        , 199,  21, 133 },
   { TEXT("midnight")               ,  25,  25, 112 },
   { TEXT("midnightblue")           ,  25,  25, 112 },
   { TEXT("mint")                   , 245, 255, 250 },
   { TEXT("mintcream")              , 245, 255, 250 },
   { TEXT("misty")                  , 255, 228, 225 },
   { TEXT("mistyrose")              , 255, 228, 225 },
   { TEXT("mistyrose1")             , 255, 228, 225 },
   { TEXT("mistyrose2")             , 238, 213, 210 },
   { TEXT("mistyrose3")             , 205, 183, 181 },
   { TEXT("mistyrose4")             , 139, 125, 123 },
   { TEXT("moccasin")               , 255, 228, 181 },
   { TEXT("navajo")                 , 255, 222, 173 },
   { TEXT("navajowhite")            , 255, 222, 173 },
   { TEXT("navajowhite1")           , 255, 222, 173 },
   { TEXT("navajowhite2")           , 238, 207, 161 },
   { TEXT("navajowhite3")           , 205, 179, 139 },
   { TEXT("navajowhite4")           , 139, 121,  94 },
   { TEXT("navy")                   ,   0,   0, 128 },
   { TEXT("navyblue")               ,   0,   0, 128 },
   { TEXT("old")                    , 253, 245, 230 },
   { TEXT("oldlace")                , 253, 245, 230 },
   { TEXT("olive")                  , 107, 142,  35 },
   { TEXT("olivedrab")              , 107, 142,  35 },
   { TEXT("olivedrab1")             , 192, 255,  62 },
   { TEXT("olivedrab2")             , 179, 238,  58 },
   { TEXT("olivedrab3")             , 154, 205,  50 },
   { TEXT("olivedrab4")             , 105, 139,  34 },
   { TEXT("orange")                 , 255,  69,   0 },
   { TEXT("orange1")                , 255, 165,   0 },
   { TEXT("orange2")                , 238, 154,   0 },
   { TEXT("orange3")                , 205, 133,   0 },
   { TEXT("orange4")                , 139,  90,   0 },
   { TEXT("orangered")              , 255,  69,   0 },
   { TEXT("orangered1")             , 255,  69,   0 },
   { TEXT("orangered2")             , 238,  64,   0 },
   { TEXT("orangered3")             , 205,  55,   0 },
   { TEXT("orangered4")             , 139,  37,   0 },
   { TEXT("orchid")                 , 218, 112, 214 },
   { TEXT("orchid1")                , 255, 131, 250 },
   { TEXT("orchid2")                , 238, 122, 233 },
   { TEXT("orchid3")                , 205, 105, 201 },
   { TEXT("orchid4")                , 139,  71, 137 },
   { TEXT("pale")                   , 219, 112, 147 },
   { TEXT("palegoldenrod")          , 238, 232, 170 },
   { TEXT("palegreen")              , 152, 251, 152 },
   { TEXT("palegreen1")             , 154, 255, 154 },
   { TEXT("palegreen2")             , 144, 238, 144 },
   { TEXT("palegreen3")             , 124, 205, 124 },
   { TEXT("palegreen4")             ,  84, 139,  84 },
   { TEXT("paleturquoise")          , 175, 238, 238 },
   { TEXT("paleturquoise1")         , 187, 255, 255 },
   { TEXT("paleturquoise2")         , 174, 238, 238 },
   { TEXT("paleturquoise3")         , 150, 205, 205 },
   { TEXT("paleturquoise4")         , 102, 139, 139 },
   { TEXT("palevioletred")          , 219, 112, 147 },
   { TEXT("palevioletred1")         , 255, 130, 171 },
   { TEXT("palevioletred2")         , 238, 121, 159 },
   { TEXT("palevioletred3")         , 205, 104, 137 },
   { TEXT("palevioletred4")         , 139,  71,  93 },
   { TEXT("parchment")              , 203, 169, 121 }, /* The great interior color of my 2006 Ford Mustang convertible */
   { TEXT("papaya")                 , 255, 239, 213 },
   { TEXT("papayawhip")             , 255, 239, 213 },
   { TEXT("peach")                  , 255, 218, 185 },
   { TEXT("peachpuff")              , 255, 218, 185 },
   { TEXT("peachpuff1")             , 255, 218, 185 },
   { TEXT("peachpuff2")             , 238, 203, 173 },
   { TEXT("peachpuff3")             , 205, 175, 149 },
   { TEXT("peachpuff4")             , 139, 119, 101 },
   { TEXT("peru")                   , 205, 133,  63 },
   { TEXT("pink")                   , 255, 192, 203 },
   { TEXT("pink1")                  , 255, 181, 197 },
   { TEXT("pink2")                  , 238, 169, 184 },
   { TEXT("pink3")                  , 205, 145, 158 },
   { TEXT("pink4")                  , 139,  99, 108 },
   { TEXT("plum")                   , 221, 160, 221 },
   { TEXT("plum1")                  , 255, 187, 255 },
   { TEXT("plum2")                  , 238, 174, 238 },
   { TEXT("plum3")                  , 205, 150, 205 },
   { TEXT("plum4")                  , 139, 102, 139 },
   { TEXT("powder")                 , 176, 224, 230 },
   { TEXT("powderblue")             , 176, 224, 230 },
   { TEXT("purple")                 , 160,  32, 240 },
   { TEXT("purple1")                , 155,  48, 255 },
   { TEXT("purple2")                , 145,  44, 238 },
   { TEXT("purple3")                , 125,  38, 205 },
   { TEXT("purple4")                ,  85,  26, 139 },
   { TEXT("red")                    , 255,   0,   0 },
   { TEXT("red1")                   , 255,   0,   0 },
   { TEXT("red2")                   , 238,   0,   0 },
   { TEXT("red3")                   , 205,   0,   0 },
   { TEXT("red4")                   , 139,   0,   0 },
   { TEXT("rosy")                   , 188, 143, 143 },
   { TEXT("rosybrown")              , 188, 143, 143 },
   { TEXT("rosybrown1")             , 255, 193, 193 },
   { TEXT("rosybrown2")             , 238, 180, 180 },
   { TEXT("rosybrown3")             , 205, 155, 155 },
   { TEXT("rosybrown4")             , 139, 105, 105 },
   { TEXT("royal")                  ,  65, 105, 225 },
   { TEXT("royalblue")              ,  65, 105, 225 },
   { TEXT("royalblue1")             ,  72, 118, 255 },
   { TEXT("royalblue2")             ,  67, 110, 238 },
   { TEXT("royalblue3")             ,  58,  95, 205 },
   { TEXT("royalblue4")             ,  39,  64, 139 },
   { TEXT("saddle")                 , 139,  69,  19 },
   { TEXT("saddlebrown")            , 139,  69,  19 },
   { TEXT("salmon")                 , 250, 128, 114 },
   { TEXT("salmon1")                , 255, 140, 105 },
   { TEXT("salmon2")                , 238, 130,  98 },
   { TEXT("salmon3")                , 205, 112,  84 },
   { TEXT("salmon4")                , 139,  76,  57 },
   { TEXT("sandy")                  , 244, 164,  96 },
   { TEXT("sandybrown")             , 244, 164,  96 },
   { TEXT("sea")                    ,  46, 139,  87 },
   { TEXT("seagreen")               ,  46, 139,  87 },
   { TEXT("seagreen1")              ,  84, 255, 159 },
   { TEXT("seagreen2")              ,  78, 238, 148 },
   { TEXT("seagreen3")              ,  67, 205, 128 },
   { TEXT("seagreen4")              ,  46, 139,  87 },
   { TEXT("seashell")               , 255, 245, 238 },
   { TEXT("seashell1")              , 255, 245, 238 },
   { TEXT("seashell2")              , 238, 229, 222 },
   { TEXT("seashell3")              , 205, 197, 191 },
   { TEXT("seashell4")              , 139, 134, 130 },
   { TEXT("sienna")                 , 160,  82,  45 },
   { TEXT("sienna1")                , 255, 130,  71 },
   { TEXT("sienna2")                , 238, 121,  66 },
   { TEXT("sienna3")                , 205, 104,  57 },
   { TEXT("sienna4")                , 139,  71,  38 },
   { TEXT("sky")                    , 135, 206, 235 },
   { TEXT("skyblue")                , 135, 206, 235 },
   { TEXT("skyblue1")               , 135, 206, 255 },
   { TEXT("skyblue2")               , 126, 192, 238 },
   { TEXT("skyblue3")               , 108, 166, 205 },
   { TEXT("skyblue4")               ,  74, 112, 139 },
   { TEXT("slate")                  , 106,  90, 205 },
   { TEXT("slateblue")              , 106,  90, 205 },
   { TEXT("slateblue1")             , 131, 111, 255 },
   { TEXT("slateblue2")             , 122, 103, 238 },
   { TEXT("slateblue3")             , 105,  89, 205 },
   { TEXT("slateblue4")             ,  71,  60, 139 },
   { TEXT("slategray")              , 112, 128, 144 },
   { TEXT("slategray1")             , 198, 226, 255 },
   { TEXT("slategray2")             , 185, 211, 238 },
   { TEXT("slategray3")             , 159, 182, 205 },
   { TEXT("slategray4")             , 108, 123, 139 },
   { TEXT("snow")                   , 255, 250, 250 },
   { TEXT("snow1")                  , 255, 250, 250 },
   { TEXT("snow2")                  , 238, 233, 233 },
   { TEXT("snow3")                  , 205, 201, 201 },
   { TEXT("snow4")                  , 139, 137, 137 },
   { TEXT("spring")                 ,   0, 255, 127 },
   { TEXT("springgreen")            ,   0, 255, 127 },
   { TEXT("springgreen1")           ,   0, 255, 127 },
   { TEXT("springgreen2")           ,   0, 238, 118 },
   { TEXT("springgreen3")           ,   0, 205, 102 },
   { TEXT("springgreen4")           ,   0, 139,  69 },
   { TEXT("steel")                  ,  70, 130, 180 },
   { TEXT("steelblue")              ,  70, 130, 180 },
   { TEXT("steelblue1")             ,  99, 184, 255 },
   { TEXT("steelblue2")             ,  92, 172, 238 },
   { TEXT("steelblue3")             ,  79, 148, 205 },
   { TEXT("steelblue4")             ,  54, 100, 139 },
   { TEXT("tan")                    , 210, 180, 140 },
   { TEXT("tan1")                   , 255, 165,  79 },
   { TEXT("tan2")                   , 238, 154,  73 },
   { TEXT("tan3")                   , 205, 133,  63 },
   { TEXT("tan4")                   , 139,  90,  43 },
   { TEXT("thistle")                , 216, 191, 216 },
   { TEXT("thistle1")               , 255, 225, 255 },
   { TEXT("thistle2")               , 238, 210, 238 },
   { TEXT("thistle3")               , 205, 181, 205 },
   { TEXT("thistle4")               , 139, 123, 139 },
   { TEXT("tomato")                 , 255,  99,  71 },
   { TEXT("tomato1")                , 255,  99,  71 },
   { TEXT("tomato2")                , 238,  92,  66 },
   { TEXT("tomato3")                , 205,  79,  57 },
   { TEXT("tomato4")                , 139,  54,  38 },
   { TEXT("turquoise")              ,  64, 224, 208 },
   { TEXT("turquoise1")             ,   0, 245, 255 },
   { TEXT("turquoise2")             ,   0, 229, 238 },
   { TEXT("turquoise3")             ,   0, 197, 205 },
   { TEXT("turquoise4")             ,   0, 134, 139 },
   { TEXT("violet")                 , 238, 130, 238 },
   { TEXT("violetred")              , 208,  32, 144 },
   { TEXT("violetred1")             , 255,  62, 150 },
   { TEXT("violetred2")             , 238,  58, 140 },
   { TEXT("violetred3")             , 205,  50, 120 },
   { TEXT("violetred4")             , 139,  34,  82 },
   { TEXT("wheat")                  , 245, 222, 179 },
   { TEXT("wheat1")                 , 255, 231, 186 },
   { TEXT("wheat2")                 , 238, 216, 174 },
   { TEXT("wheat3")                 , 205, 186, 150 },
   { TEXT("wheat4")                 , 139, 126, 102 },
   { TEXT("white")                  , 255, 255, 255 },
   { TEXT("whitesmoke")             , 245, 245, 245 },
   { TEXT("yellow")                 , 255, 255,   0 },
   { TEXT("yellow1")                , 255, 255,   0 },
   { TEXT("yellow2")                , 238, 238,   0 },
   { TEXT("yellow3")                , 205, 205,   0 },
   { TEXT("yellow4")                , 139, 139,   0 },
   { TEXT("yellowgreen")            , 154, 205,  50 },
   { NULL                           , 128, 128, 128 }
};

/****************************************************************************************/
static int _ColorNameCmp(const X11COLOR_t *ce, const TCHAR *colorName)
/****************************************************************************************/
/*
 * strcmp() replacement: compare LC X11 and ?C colorName
 * return no. of leading common chars or -1 in case of a full match.
 */
{
   const TCHAR *c = colorName;
   const TCHAR *x = ce->name;
   int          n = 0; /* no of matching chars */

   while(*c == *x && *x && *c) { n++; x++; c++; }
   return (*x || *c) ? n : -1; /* -1 this is a full match */
}

/****************************************************************************************/
static const X11COLOR_t *_ColorNameScan
(
   const TCHAR      *lcName,
   const X11COLOR_t **pBest
)
/****************************************************************************************/
/*
 * return pointer to the X11Color table entry with name colorName
 * can be used to check whether a colorName is valid or not.
 */
{
   const X11COLOR_t *bestColor = (const X11COLOR_t *)NULL;
   const X11COLOR_t *ce;
   int               bestMatch = 0;
   TCHAR             lc1 = lcName[0];

#if 0
_tprintf(TEXT("ColorNameScan(%s)\n"),lcName);
#endif
   /* find the full resp. best colorname in the list */
   X11COLOR_LOOP_ALL(ce)
   {
      if (lc1 == ce->name[0]) /* quick compare the first char */
      {
         int nMatch = _ColorNameCmp(ce,lcName);
         if (nMatch < 0) return ce; /* full name match */
         if (nMatch > bestMatch)
         {
            bestMatch = nMatch;
            bestColor = ce;
         }
      }
   }

#if 0
_tprintf(TEXT("ColorNameScan(best=%s)\n"),(bestColor)?bestColor->name:TEXT("NULL"));
#endif
   if (pBest) *pBest = bestColor;
   return (const X11COLOR_t *)NULL; /* NULL == partial match only */
}

/****************************************************************************************/
 C_FUNC_PREFIX const X11COLOR_t *getx11colorbyname(const TCHAR *colorName)
/****************************************************************************************/
/*
 * return pointer to the X11Color table entry with name colorName
 * can be used to check whether a colorName is valid or not.
 */
{
   static X11COLOR_t  localColor;
   const  X11COLOR_t *matchColor;
   const  X11COLOR_t *bestColor;
   TCHAR  lcName[512];
   float  fr,fg,fb, hue,lig,sat;
   int    idir,nc;


   if (!STRHASLEN(colorName)) return (const X11COLOR_t *)NULL;
   STRJUMPNOSPACE(colorName);
   if (!*colorName) return (const X11COLOR_t *)NULL;

   /* create lower case copy of colorName */
   STRNCPY(lcName,colorName,countof(lcName));
   lcName[countof(lcName)-1] = TEXT('\0');
   STRLOWER(lcName);

   /* count the no. of ':'. No. of colors == no. of ':' + 1 */
   for(nc=idir=0; lcName[idir]; idir++)
      if (lcName[idir] == TEXT(':'))
         nc++;

   if (nc > 0 || ISDIGIT(lcName[0]))
   {
      /* got r:g:b and not a color name */
      unsigned ur,ug,ub,dum;
      int      nv;

      MEMZERO(&localColor,sizeof(X11COLOR_t));
      localColor.name = colorName;

      /* append a dummy :0 to get 4 numbers and avoid an early sscanf stop */
      STRCAT(lcName,TEXT(":0")); /* terminate the color name */
      nc += 2; /* No. of colors == no. of ':' + 1 + ":0" */

      /* try decimal numbers */
      nv = SSCANF(lcName,TEXT("%u:%u:%u:%u"),&ur,&ug,&ub,&dum);
#if 0
printf("DEC_RGB(%d) = %u %u %u   (%u)\n",nv,ur,ug,ub,dum);
#endif
      if (nv != nc)
      {
         /* try hex numbers */
         nv = SSCANF(lcName,TEXT("%x:%x:%x:%u"),&ur,&ug,&ub,&dum);
#if 0
printf("HEX_RGB(%d) = %u %u %u   (%u)\n",nv,ur,ug,ub,dum);
#endif
         if (nv != nc)  /* try x-hex numbers */
         {
            nv = SSCANF(lcName,TEXT("x%x:x%x:x%x:%u"),&ur,&ug,&ub,&dum);
#if 0
printf("XHX_RGB(%d) = %u %u %u   (%u)\n",nv,ur,ug,ub,dum);
#endif
            if (nv != nc)  /* try 0x-hex numbers */
            {
               nv = SSCANF(lcName,TEXT("0x%x:0x%x:0x%x:%u"),&ur,&ug,&ub,&dum);
#if 0
printf("0xH_RGB(%d) = %u %u %u   (%u)\n",nv,ur,ug,ub,dum);
#endif
            }
         }
      }

      switch(nv-1)
      {
         case 3: localColor.b = (unsigned char)((ub > 0xff) ? 0xff : ub);
         case 2: localColor.g = (unsigned char)((ug > 0xff) ? 0xff : ug);
         case 1: localColor.r = (unsigned char)((ur > 0xff) ? 0xff : ur);
         default: break;
      }
      return &localColor;
   }

   /* replace grey with gray */
   {
      TCHAR *cp = STRSTR(lcName,TEXT("grey"));
      if (cp) cp[2] = TEXT('a');
   }

   /* find the full resp. best colorname in the list */
   matchColor = _ColorNameScan(lcName,&bestColor);
   if (matchColor) return matchColor; /* 100% colorname match */

   /* try to find any kind of not dark color name */
   idir = 0;
   if (!STRNCMP(lcName,TEXT("dark"),4) && lcName[4])
   {
      matchColor = _ColorNameScan(lcName+4,&bestColor);
      idir = -1; /* this is a dark color */
   }
   else if (!STRNCMP(lcName,TEXT("light"),5) && lcName[5])
   {
      matchColor = _ColorNameScan(lcName+5,&bestColor);
      idir = +1; /* this is a light color */
   }
   else
   {
      return bestColor; /* neither dark nor light, accept partial match */
   }

        if (matchColor) localColor = *matchColor;
   else if (bestColor ) localColor = *bestColor;
   else return (const X11COLOR_t *)NULL;

   /* color name matches, but not dark or light: return modified color */
   fr = localColor.r/255.0f;
   fg = localColor.g/255.0f;
   fb = localColor.b/255.0f;
   RGBtoHLS(fr,fg,fb,&hue,&lig,&sat);
#if 0
printf("RGB-HLS(%2d,%2d,%2d)=(%f,%f,%f)\n",localColor.r,localColor.g,localColor.b,hue,lig,sat);
#endif

   if (idir < 0) lig = 0.75f*lig;         /* reduce by 25% */
   else          lig += 0.25f*(1.0f-lig); /* add 25% */

   /* convert back HLS to RGB and return */
   HLStoRGB(hue,lig,sat,&fr,&fg,&fb);
   localColor.r = (unsigned char)(255.0f * fr);
   localColor.g = (unsigned char)(255.0f * fg);
   localColor.b = (unsigned char)(255.0f * fb);
   localColor.name = colorName;

#if 0
printf("HLS-RGB(%2d,%2d,%2d)=(%f,%f,%f)\n",localColor.r,localColor.g,localColor.b,hue,lig,sat);
#endif

   return &localColor;
}

/****************************************************************************************/

#endif
