/***********************************************************************
**
**   mapdefaults.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

// Default-Home (Poltringen)
#define HOME_DEFAULT_LAT 29125200
#define HOME_DEFAULT_LON 5364500

// Scale-values
#define L_LIMIT 10
#define U_LIMIT 1500
#define SWITCH_S 290
#define WPLABEL 400
#define BORDER_1 100
#define BORDER_2 500
#define BORDER_3 1000

/*
 * Definierte Farbwerte bei (dazwischen wurde früher linear geändert):
 *
 *   <0 m :    96 / 128 / 248
 *    0 m :   174 / 208 / 129 *
 *   10 m :   201 / 230 / 178 *
 *   50 m :   231 / 255 / 231 *
 *  100 m :   221 / 245 / 183 *
 *  250 m :   240 / 240 / 168 *
 * 1000 m :   235 / 155 /  98 *
 * 4000 m :   130 /  65 /  20 *
 * 9000 m :    96 /  43 /  16 *
 */

// Default-Values for topography:
#define COLOR_LEVEL_SUB QColor(96, 128, 248)
#define COLOR_LEVEL_0 QColor(165, 214, 126)
#define COLOR_LEVEL_10 QColor(185, 220, 131)
#define COLOR_LEVEL_25 QColor(193, 225, 138)
#define COLOR_LEVEL_50 QColor(208, 234, 151)
#define COLOR_LEVEL_75 QColor(222, 243, 164)
#define COLOR_LEVEL_100 QColor(237, 252, 178)
#define COLOR_LEVEL_150 QColor(237, 248, 175)
#define COLOR_LEVEL_200 QColor(236, 241, 169)
#define COLOR_LEVEL_250 QColor(236, 235, 165)
#define COLOR_LEVEL_300 QColor(236, 230, 160)
#define COLOR_LEVEL_350 QColor(236, 225, 156)
#define COLOR_LEVEL_400 QColor(236, 219, 151)
#define COLOR_LEVEL_450 QColor(236, 214, 147)
#define COLOR_LEVEL_500 QColor(236, 208, 142)
#define COLOR_LEVEL_600 QColor(235, 198, 133)
#define COLOR_LEVEL_700 QColor(235, 187, 124)
#define COLOR_LEVEL_800 QColor(235, 176, 115)
#define COLOR_LEVEL_900 QColor(235, 165, 106)
#define COLOR_LEVEL_1000 QColor(235, 155, 98)
#define COLOR_LEVEL_1250 QColor(226, 155, 91)
#define COLOR_LEVEL_1500 QColor(217, 155, 98)
#define COLOR_LEVEL_1750 QColor(207, 149, 112)
#define COLOR_LEVEL_2000 QColor(202, 143, 113)
#define COLOR_LEVEL_2250 QColor(211, 167, 150)
#define COLOR_LEVEL_2500 QColor(221, 198, 187)
#define COLOR_LEVEL_2750 QColor(228, 220, 217)
#define COLOR_LEVEL_3000 QColor(228, 228, 234)
#define COLOR_LEVEL_3250 QColor(221, 221, 237)
#define COLOR_LEVEL_3500 QColor(217, 217, 239)
#define COLOR_LEVEL_3750 QColor(211, 211, 242)
#define COLOR_LEVEL_4000 QColor(206, 206, 245)
#define COLOR_LEVEL_4250 QColor(199, 199, 248)
#define COLOR_LEVEL_4500 QColor(192, 194, 250)
#define COLOR_LEVEL_4750 QColor(184, 184, 255)
#define COLOR_LEVEL_5000 QColor(187, 187, 255)
#define COLOR_LEVEL_5250 QColor(185, 185, 255)
#define COLOR_LEVEL_5500 QColor(182, 182, 255)
#define COLOR_LEVEL_5750 QColor(179, 179, 255)
#define COLOR_LEVEL_6000 QColor(175, 175, 255)
#define COLOR_LEVEL_6250 QColor(172, 172, 255)
#define COLOR_LEVEL_6500 QColor(169, 169, 255)
#define COLOR_LEVEL_6750 QColor(166, 166, 255)
#define COLOR_LEVEL_7000 QColor(163, 163, 255)
#define COLOR_LEVEL_7250 QColor(160, 160, 255)
#define COLOR_LEVEL_7500 QColor(157, 157, 255)
#define COLOR_LEVEL_7750 QColor(154, 154, 255)
#define COLOR_LEVEL_8000 QColor(151, 151, 255)
#define COLOR_LEVEL_8250 QColor(148, 148, 255)
#define COLOR_LEVEL_8500 QColor(145, 145, 255)
#define COLOR_LEVEL_8750 QColor(142, 142, 255)

// Default-Values for map-element:
//
// [Trail]
#define TRAIL_COLOR_1  QColor(255,100,100)
#define TRAIL_COLOR_2  QColor(255,100,100)
#define TRAIL_COLOR_3  QColor(255,100,100)
#define TRAIL_COLOR_4  QColor(255,100,100)

#define TRAIL_PEN_1 2
#define TRAIL_PEN_2 2
#define TRAIL_PEN_3 1
#define TRAIL_PEN_4 1

#define TRAIL_PEN_STYLE_1 Qt::SolidLine
#define TRAIL_PEN_STYLE_2 Qt::SolidLine
#define TRAIL_PEN_STYLE_3 Qt::SolidLine
#define TRAIL_PEN_STYLE_4 Qt::SolidLine

#define PRINT_TRAIL_COLOR_1  QColor(255,100,100)
#define PRINT_TRAIL_COLOR_2  QColor(255,100,100)

#define PRINT_TRAIL_PEN_1 2
#define PRINT_TRAIL_PEN_2 2

#define PRINT_TRAIL_PEN_STYLE_1 Qt::SolidLine
#define PRINT_TRAIL_PEN_STYLE_2 Qt::SolidLine

// [Road]
#define ROAD_COLOR_1  QColor(255,100,100)
#define ROAD_COLOR_2  QColor(255,100,100)
#define ROAD_COLOR_3  QColor(255,100,100)
#define ROAD_COLOR_4  QColor(255,100,100)

#define ROAD_PEN_1 2
#define ROAD_PEN_2 2
#define ROAD_PEN_3 1
#define ROAD_PEN_4 1

#define ROAD_PEN_STYLE_1 Qt::SolidLine
#define ROAD_PEN_STYLE_2 Qt::SolidLine
#define ROAD_PEN_STYLE_3 Qt::SolidLine
#define ROAD_PEN_STYLE_4 Qt::SolidLine

#define PRINT_ROAD_COLOR_1  QColor(255,100,100)
#define PRINT_ROAD_COLOR_2  QColor(255,100,100)

#define PRINT_ROAD_PEN_1 2
#define PRINT_ROAD_PEN_2 2

#define PRINT_ROAD_PEN_STYLE_1 Qt::SolidLine
#define PRINT_ROAD_PEN_STYLE_2 Qt::SolidLine

// [Highway]
#define HIGH_COLOR_1  QColor(255,100,100)
#define HIGH_COLOR_2  QColor(255,100,100)
#define HIGH_COLOR_3  QColor(255,100,100)
#define HIGH_COLOR_4  QColor(255,100,100)

#define HIGH_PEN_1 5
#define HIGH_PEN_2 3
#define HIGH_PEN_3 2
#define HIGH_PEN_4 1

#define HIGH_PEN_STYLE_1 Qt::SolidLine
#define HIGH_PEN_STYLE_2 Qt::SolidLine
#define HIGH_PEN_STYLE_3 Qt::SolidLine
#define HIGH_PEN_STYLE_4 Qt::SolidLine

#define PRINT_HIGH_COLOR_1  QColor(255,100,100)
#define PRINT_HIGH_COLOR_2  QColor(255,100,100)

#define PRINT_HIGH_PEN_1 5
#define PRINT_HIGH_PEN_2 3

#define PRINT_HIGH_PEN_STYLE_1 Qt::SolidLine
#define PRINT_HIGH_PEN_STYLE_2 Qt::SolidLine

// [Railway]
#define RAIL_COLOR_1  QColor(80,80,80)
#define RAIL_COLOR_2  QColor(80,80,80)
#define RAIL_COLOR_3  QColor(80,80,80)
#define RAIL_COLOR_4  QColor(80,80,80)

#define RAIL_PEN_1 2
#define RAIL_PEN_2 2
#define RAIL_PEN_3 1
#define RAIL_PEN_4 1

#define RAIL_PEN_STYLE_1 Qt::DashLine
#define RAIL_PEN_STYLE_2 Qt::DashLine
#define RAIL_PEN_STYLE_3 Qt::DashLine
#define RAIL_PEN_STYLE_4 Qt::DashLine

#define PRINT_RAIL_COLOR_1  QColor(80,80,80)
#define PRINT_RAIL_COLOR_2  QColor(80,80,80)

#define PRINT_RAIL_PEN_1 2
#define PRINT_RAIL_PEN_2 2

#define PRINT_RAIL_PEN_STYLE_1 Qt::DashLine
#define PRINT_RAIL_PEN_STYLE_2 Qt::DashLine

// [Railway_d]
#define RAIL_D_COLOR_1  QColor(80,80,80)
#define RAIL_D_COLOR_2  QColor(80,80,80)
#define RAIL_D_COLOR_3  QColor(80,80,80)
#define RAIL_D_COLOR_4  QColor(80,80,80)

#define RAIL_D_PEN_1 2
#define RAIL_D_PEN_2 2
#define RAIL_D_PEN_3 1
#define RAIL_D_PEN_4 1

#define RAIL_D_PEN_STYLE_1 Qt::DashLine
#define RAIL_D_PEN_STYLE_2 Qt::DashLine
#define RAIL_D_PEN_STYLE_3 Qt::DashLine
#define RAIL_D_PEN_STYLE_4 Qt::DashLine

#define PRINT_RAIL_D_COLOR_1  QColor(80,80,80)
#define PRINT_RAIL_D_COLOR_2  QColor(80,80,80)

#define PRINT_RAIL_D_PEN_1 2
#define PRINT_RAIL_D_PEN_2 2

#define PRINT_RAIL_D_PEN_STYLE_1 Qt::DashLine
#define PRINT_RAIL_D_PEN_STYLE_2 Qt::DashLine

// [Aerial Cable]
#define AERIAL_CABLE_COLOR_1  QColor(80,80,80)
#define AERIAL_CABLE_COLOR_2  QColor(80,80,80)
#define AERIAL_CABLE_COLOR_3  QColor(80,80,80)
#define AERIAL_CABLE_COLOR_4  QColor(80,80,80)

#define AERIAL_CABLE_PEN_1 2
#define AERIAL_CABLE_PEN_2 2
#define AERIAL_CABLE_PEN_3 1
#define AERIAL_CABLE_PEN_4 1

#define AERIAL_CABLE_PEN_STYLE_1 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_2 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_3 Qt::DashLine
#define AERIAL_CABLE_PEN_STYLE_4 Qt::DashLine

#define PRINT_AERIAL_CABLE_COLOR_1  QColor(80,80,80)
#define PRINT_AERIAL_CABLE_COLOR_2  QColor(80,80,80)

#define PRINT_AERIAL_CABLE_PEN_1 2
#define PRINT_AERIAL_CABLE_PEN_2 2

#define PRINT_AERIAL_CABLE_PEN_STYLE_1 Qt::DashLine
#define PRINT_AERIAL_CABLE_PEN_STYLE_2 Qt::DashLine

// [River]
#define RIVER_COLOR_1 QColor(70,70,195)
#define RIVER_COLOR_2 QColor(70,70,195)
#define RIVER_COLOR_3 QColor(70,70,195)
#define RIVER_COLOR_4 QColor(70,70,195)

#define RIVER_PEN_1 2
#define RIVER_PEN_2 2
#define RIVER_PEN_3 1
#define RIVER_PEN_4 1

#define RIVER_PEN_STYLE_1 Qt::SolidLine
#define RIVER_PEN_STYLE_2 Qt::SolidLine
#define RIVER_PEN_STYLE_3 Qt::SolidLine
#define RIVER_PEN_STYLE_4 Qt::SolidLine

#define PRINT_RIVER_COLOR_1 QColor(70,70,195)
#define PRINT_RIVER_COLOR_2 QColor(70,70,195)

#define PRINT_RIVER_PEN_1 2
#define PRINT_RIVER_PEN_2 2

#define PRINT_RIVER_PEN_STYLE_1 Qt::SolidLine
#define PRINT_RIVER_PEN_STYLE_2 Qt::SolidLine

// [Canal]
#define CANAL_COLOR_1 QColor(70,195,70)
#define CANAL_COLOR_2 QColor(70,195,70)
#define CANAL_COLOR_3 QColor(70,195,70)
#define CANAL_COLOR_4 QColor(70,195,70)

#define CANAL_PEN_1 2
#define CANAL_PEN_2 2
#define CANAL_PEN_3 1
#define CANAL_PEN_4 1

#define CANAL_PEN_STYLE_1 Qt::SolidLine
#define CANAL_PEN_STYLE_2 Qt::SolidLine
#define CANAL_PEN_STYLE_3 Qt::SolidLine
#define CANAL_PEN_STYLE_4 Qt::SolidLine

#define PRINT_CANAL_COLOR_1 QColor(70,195,70)
#define PRINT_CANAL_COLOR_2 QColor(70,195,70)

#define PRINT_CANAL_PEN_1 2
#define PRINT_CANAL_PEN_2 2

#define PRINT_CANAL_PEN_STYLE_1 Qt::SolidLine
#define PRINT_CANAL_PEN_STYLE_2 Qt::SolidLine

// [City]
#define CITY_BRUSH_COLOR_1 QColor(255,250,100)
#define CITY_BRUSH_COLOR_2 QColor(255,250,100)
#define CITY_BRUSH_COLOR_3 QColor(255,250,100)
#define CITY_BRUSH_COLOR_4 QColor(255,250,100)

#define CITY_BRUSH_STYLE_1 Qt::SolidPattern
#define CITY_BRUSH_STYLE_2 Qt::SolidPattern
#define CITY_BRUSH_STYLE_3 Qt::SolidPattern
#define CITY_BRUSH_STYLE_4 Qt::SolidPattern

#define CITY_COLOR_1 QColor(0,0,0)
#define CITY_COLOR_2 QColor(0,0,0)
#define CITY_COLOR_3 QColor(0,0,0)
#define CITY_COLOR_4 QColor(0,0,0)

#define CITY_PEN_1 1
#define CITY_PEN_2 1
#define CITY_PEN_3 1
#define CITY_PEN_4 1

#define CITY_BRUSH_COLOR_1 QColor(255,250,100)
#define CITY_BRUSH_COLOR_2 QColor(255,250,100)
#define CITY_BRUSH_COLOR_3 QColor(255,250,100)
#define CITY_BRUSH_COLOR_4 QColor(255,250,100)

#define PRINT_CITY_BRUSH_STYLE_1 Qt::SolidPattern
#define PRINT_CITY_BRUSH_STYLE_2 Qt::SolidPattern

#define PRINT_CITY_COLOR_1 QColor(0,0,0)
#define PRINT_CITY_COLOR_2 QColor(0,0,0)

#define PRINT_CITY_PEN_1 1
#define PRINT_CITY_PEN_2 1

#define PRINT_CITY_BRUSH_COLOR_1 QColor(255,250,100)
#define PRINT_CITY_BRUSH_COLOR_2 QColor(255,250,100)

// [Forest]
#define FRST_COLOR_1 QColor(12,86,35)
#define FRST_COLOR_2 QColor(12,86,35)
#define FRST_COLOR_3 QColor(12,86,35)
#define FRST_COLOR_4 QColor(12,86,35)

#define FRST_PEN_1 2
#define FRST_PEN_2 2
#define FRST_PEN_3 2
#define FRST_PEN_4 2

#define FRST_PEN_STYLE_1 Qt::SolidLine
#define FRST_PEN_STYLE_2 Qt::SolidLine
#define FRST_PEN_STYLE_3 Qt::SolidLine
#define FRST_PEN_STYLE_4 Qt::SolidLine

#define FRST_BRUSH_COLOR_1 QColor(12,86,35)
#define FRST_BRUSH_COLOR_2 QColor(12,86,35)
#define FRST_BRUSH_COLOR_3 QColor(12,86,35)
#define FRST_BRUSH_COLOR_4 QColor(12,86,35)

#define FRST_BRUSH_STYLE_1 Qt::Dense5Pattern
#define FRST_BRUSH_STYLE_2 Qt::Dense5Pattern
#define FRST_BRUSH_STYLE_3 Qt::Dense6Pattern
#define FRST_BRUSH_STYLE_4 Qt::Dense6Pattern

#define PRINT_FRST_COLOR_1 QColor(12,86,35)
#define PRINT_FRST_COLOR_2 QColor(12,86,35)

#define PRINT_FRST_PEN_1 2
#define PRINT_FRST_PEN_2 2

#define PRINT_FRST_PEN_STYLE_1 Qt::SolidLine
#define PRINT_FRST_PEN_STYLE_2 Qt::SolidLine

#define PRINT_FRST_BRUSH_COLOR_1 QColor(12,86,35)
#define PRINT_FRST_BRUSH_COLOR_2 QColor(12,86,35)

#define PRINT_FRST_BRUSH_STYLE_1 Qt::Dense5Pattern
#define PRINT_FRST_BRUSH_STYLE_2 Qt::Dense5Pattern

// [Glacier]
#define GLACIER_COLOR_1 QColor(255,250,250)
#define GLACIER_COLOR_2 QColor(255,250,250)
#define GLACIER_COLOR_3 QColor(255,250,250)
#define GLACIER_COLOR_4 QColor(255,250,250)

#define GLACIER_PEN_1 2
#define GLACIER_PEN_2 1
#define GLACIER_PEN_3 1
#define GLACIER_PEN_4 1

#define GLACIER_PEN_STYLE_1 Qt::SolidLine
#define GLACIER_PEN_STYLE_2 Qt::SolidLine
#define GLACIER_PEN_STYLE_3 Qt::SolidLine
#define GLACIER_PEN_STYLE_4 Qt::SolidLine

#define GLACIER_BRUSH_COLOR_1 QColor(255,250,250)
#define GLACIER_BRUSH_COLOR_2 QColor(255,250,250)
#define GLACIER_BRUSH_COLOR_3 QColor(255,250,250)
#define GLACIER_BRUSH_COLOR_4 QColor(255,250,250)

#define GLACIER_BRUSH_STYLE_1 Qt::CrossPattern
#define GLACIER_BRUSH_STYLE_2 Qt::CrossPattern
#define GLACIER_BRUSH_STYLE_3 Qt::CrossPattern
#define GLACIER_BRUSH_STYLE_4 Qt::CrossPattern

#define PRINT_GLACIER_COLOR_1 QColor(0,0,0)
#define PRINT_GLACIER_COLOR_2 QColor(0,0,0)

#define PRINT_GLACIER_PEN_1 2
#define PRINT_GLACIER_PEN_2 2

#define PRINT_GLACIER_PEN_STYLE_1 Qt::SolidLine
#define PRINT_GLACIER_PEN_STYLE_2 Qt::SolidLine

#define PRINT_GLACIER_BRUSH_COLOR_1 QColor(0,0,0)
#define PRINT_GLACIER_BRUSH_COLOR_2 QColor(0,0,0)

#define PRINT_GLACIER_BRUSH_STYLE_1 Qt::CrossPattern
#define PRINT_GLACIER_BRUSH_STYLE_2 Qt::CrossPattern

// [Pack Ice]
#define PACK_ICE_COLOR_1 QColor(127,212,255)
#define PACK_ICE_COLOR_2 QColor(127,212,255)
#define PACK_ICE_COLOR_3 QColor(127,212,255)
#define PACK_ICE_COLOR_4 QColor(127,212,255)

#define PACK_ICE_PEN_1 2
#define PACK_ICE_PEN_2 1
#define PACK_ICE_PEN_3 1
#define PACK_ICE_PEN_4 1

#define PACK_ICE_PEN_STYLE_1 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_2 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_3 Qt::SolidLine
#define PACK_ICE_PEN_STYLE_4 Qt::SolidLine

#define PACK_ICE_BRUSH_COLOR_1 QColor(127,212,255)
#define PACK_ICE_BRUSH_COLOR_2 QColor(127,212,255)
#define PACK_ICE_BRUSH_COLOR_3 QColor(127,212,255)
#define PACK_ICE_BRUSH_COLOR_4 QColor(127,212,255)

#define PACK_ICE_BRUSH_STYLE_1 Qt::CrossPattern
#define PACK_ICE_BRUSH_STYLE_2 Qt::CrossPattern
#define PACK_ICE_BRUSH_STYLE_3 Qt::CrossPattern
#define PACK_ICE_BRUSH_STYLE_4 Qt::CrossPattern

#define PRINT_PACK_ICE_COLOR_1 QColor(127,212,255)
#define PRINT_PACK_ICE_COLOR_2 QColor(127,212,255)

#define PRINT_PACK_ICE_PEN_1 2
#define PRINT_PACK_ICE_PEN_2 2

#define PRINT_PACK_ICE_PEN_STYLE_1 Qt::SolidLine
#define PRINT_PACK_ICE_PEN_STYLE_2 Qt::SolidLine

#define PRINT_PACK_ICE_BRUSH_COLOR_1 QColor(127,212,255)
#define PRINT_PACK_ICE_BRUSH_COLOR_2 QColor(127,212,255)

#define PRINT_PACK_ICE_BRUSH_STYLE_1 Qt::CrossPattern
#define PRINT_PACK_ICE_BRUSH_STYLE_2 Qt::CrossPattern

// [RIVER_t]
#define RIVER_T_COLOR_1 QColor(255,250,100)
#define RIVER_T_COLOR_2 QColor(255,250,100)
#define RIVER_T_COLOR_3 QColor(255,250,100)
#define RIVER_T_COLOR_4 QColor(255,250,100)

#define RIVER_T_PEN_1 2
#define RIVER_T_PEN_2 2
#define RIVER_T_PEN_3 2
#define RIVER_T_PEN_4 2

#define RIVER_T_PEN_STYLE_1 Qt::SolidLine
#define RIVER_T_PEN_STYLE_2 Qt::SolidLine
#define RIVER_T_PEN_STYLE_3 Qt::SolidLine
#define RIVER_T_PEN_STYLE_4 Qt::SolidLine

#define RIVER_T_BRUSH_COLOR_1 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_2 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_3 QColor(255,250,100)
#define RIVER_T_BRUSH_COLOR_4 QColor(255,250,100)

#define RIVER_T_BRUSH_STYLE_1 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_2 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_3 Qt::SolidPattern
#define RIVER_T_BRUSH_STYLE_4 Qt::SolidPattern

#define PRINT_RIVER_T_COLOR_1 QColor(255,250,100)
#define PRINT_RIVER_T_COLOR_2 QColor(255,250,100)

#define PRINT_RIVER_T_PEN_1 2
#define PRINT_RIVER_T_PEN_2 2

#define PRINT_RIVER_T_PEN_STYLE_1 Qt::SolidLine
#define PRINT_RIVER_T_PEN_STYLE_2 Qt::SolidLine

#define PRINT_RIVER_T_BRUSH_COLOR_1 QColor(255,250,100)
#define PRINT_RIVER_T_BRUSH_COLOR_2 QColor(255,250,100)

#define PRINT_RIVER_T_BRUSH_STYLE_1 Qt::SolidPattern
#define PRINT_RIVER_T_BRUSH_STYLE_2 Qt::SolidPattern

// [Airspace A]
#define AIRA_PEN_1 4
#define AIRA_PEN_2 3
#define AIRA_PEN_3 3
#define AIRA_PEN_4 2

#define AIRA_PEN_STYLE_1 Qt::SolidLine
#define AIRA_PEN_STYLE_2 Qt::SolidLine
#define AIRA_PEN_STYLE_3 Qt::SolidLine
#define AIRA_PEN_STYLE_4 Qt::SolidLine

#define AIRA_COLOR_1 QColor(0,123,0)
#define AIRA_COLOR_2 QColor(0,120,0)
#define AIRA_COLOR_3 QColor(0,120,0)
#define AIRA_COLOR_4 QColor(0,120,0)

#define AIRA_BRUSH_COLOR_1 QColor(0,120,0)
#define AIRA_BRUSH_COLOR_2 QColor(0,120,0)
#define AIRA_BRUSH_COLOR_3 QColor(0,120,0)
#define AIRA_BRUSH_COLOR_4 QColor(0,120,0)

#define AIRA_BRUSH_STYLE_1 Qt::BDiagPattern
#define AIRA_BRUSH_STYLE_2 Qt::BDiagPattern
#define AIRA_BRUSH_STYLE_3 Qt::BDiagPattern
#define AIRA_BRUSH_STYLE_4 Qt::BDiagPattern

#define PRINT_AIRA_PEN_1 4
#define PRINT_AIRA_PEN_2 3

#define PRINT_AIRA_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRA_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRA_COLOR_1 QColor(0,120,0)
#define PRINT_AIRA_COLOR_2 QColor(0,120,0)

#define PRINT_AIRA_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRA_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_AIRA_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIRA_BRUSH_STYLE_2 Qt::NoBrush

// [Airspace B]
#define AIRB_PEN_1 4
#define AIRB_PEN_2 3
#define AIRB_PEN_3 3
#define AIRB_PEN_4 2

#define AIRB_PEN_STYLE_1 Qt::SolidLine
#define AIRB_PEN_STYLE_2 Qt::SolidLine
#define AIRB_PEN_STYLE_3 Qt::SolidLine
#define AIRB_PEN_STYLE_4 Qt::SolidLine

#define AIRB_COLOR_1 QColor(0,123,0)
#define AIRB_COLOR_2 QColor(0,120,0)
#define AIRB_COLOR_3 QColor(0,120,0)
#define AIRB_COLOR_4 QColor(0,120,0)

#define AIRB_BRUSH_COLOR_1 QColor(0,120,0)
#define AIRB_BRUSH_COLOR_2 QColor(0,120,0)
#define AIRB_BRUSH_COLOR_3 QColor(0,120,0)
#define AIRB_BRUSH_COLOR_4 QColor(0,120,0)

#define AIRB_BRUSH_STYLE_1 Qt::FDiagPattern
#define AIRB_BRUSH_STYLE_2 Qt::FDiagPattern
#define AIRB_BRUSH_STYLE_3 Qt::FDiagPattern
#define AIRB_BRUSH_STYLE_4 Qt::FDiagPattern

#define PRINT_AIRB_PEN_1 4
#define PRINT_AIRB_PEN_2 3

#define PRINT_AIRB_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRB_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRB_COLOR_1 QColor(0,120,0)
#define PRINT_AIRB_COLOR_2 QColor(0,120,0)

#define PRINT_AIRB_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRB_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_AIRB_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIRB_BRUSH_STYLE_2 Qt::NoBrush

// [Airspace C]
#define AIRC_PEN_1 4
#define AIRC_PEN_2 3
#define AIRC_PEN_3 3
#define AIRC_PEN_4 2

#define AIRC_PEN_STYLE_1 Qt::SolidLine
#define AIRC_PEN_STYLE_2 Qt::SolidLine
#define AIRC_PEN_STYLE_3 Qt::SolidLine
#define AIRC_PEN_STYLE_4 Qt::SolidLine

#define AIRC_COLOR_1 QColor(0,123,0)
#define AIRC_COLOR_2 QColor(0,120,0)
#define AIRC_COLOR_3 QColor(0,120,0)
#define AIRC_COLOR_4 QColor(0,120,0)

#define AIRC_BRUSH_COLOR_1 QColor(0,120,0)
#define AIRC_BRUSH_COLOR_2 QColor(0,120,0)
#define AIRC_BRUSH_COLOR_3 QColor(0,120,0)
#define AIRC_BRUSH_COLOR_4 QColor(0,120,0)

#define AIRC_BRUSH_STYLE_1 Qt::CrossPattern
#define AIRC_BRUSH_STYLE_2 Qt::CrossPattern
#define AIRC_BRUSH_STYLE_3 Qt::CrossPattern
#define AIRC_BRUSH_STYLE_4 Qt::CrossPattern

#define PRINT_AIRC_PEN_1 4
#define PRINT_AIRC_PEN_2 3

#define PRINT_AIRC_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRC_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRC_COLOR_1 QColor(0,120,0)
#define PRINT_AIRC_COLOR_2 QColor(0,120,0)

#define PRINT_AIRC_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRC_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_AIRC_BRUSH_STYLE_1 Qt::CrossPattern
#define PRINT_AIRC_BRUSH_STYLE_2 Qt::CrossPattern

// [Airspace D]
#define AIRD_PEN_1 4
#define AIRD_PEN_2 3
#define AIRD_PEN_3 3
#define AIRD_PEN_4 2

#define AIRD_PEN_STYLE_1 Qt::SolidLine
#define AIRD_PEN_STYLE_2 Qt::SolidLine
#define AIRD_PEN_STYLE_3 Qt::SolidLine
#define AIRD_PEN_STYLE_4 Qt::SolidLine

#define AIRD_COLOR_1 QColor(0,120,0)
#define AIRD_COLOR_2 QColor(0,120,0)
#define AIRD_COLOR_3 QColor(0,120,0)
#define AIRD_COLOR_4 QColor(0,120,0)

#define AIRD_BRUSH_COLOR_1 QColor(0,120,0)
#define AIRD_BRUSH_COLOR_2 QColor(0,120,0)
#define AIRD_BRUSH_COLOR_3 QColor(0,120,0)
#define AIRD_BRUSH_COLOR_4 QColor(0,120,0)

#define AIRD_BRUSH_STYLE_1 Qt::NoBrush
#define AIRD_BRUSH_STYLE_2 Qt::NoBrush
#define AIRD_BRUSH_STYLE_3 Qt::NoBrush
#define AIRD_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_AIRD_PEN_1 4
#define PRINT_AIRD_PEN_2 3

#define PRINT_AIRD_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRD_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRD_COLOR_1 QColor(0,120,0)
#define PRINT_AIRD_COLOR_2 QColor(0,120,0)

#define PRINT_AIRD_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRD_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_AIRD_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIRD_BRUSH_STYLE_2 Qt::NoBrush

// [Airspace E low]
#define AIREL_PEN_1 4
#define AIREL_PEN_2 3
#define AIREL_PEN_3 3
#define AIREL_PEN_4 2

#define AIREL_PEN_STYLE_1 Qt::SolidLine
#define AIREL_PEN_STYLE_2 Qt::SolidLine
#define AIREL_PEN_STYLE_3 Qt::SolidLine
#define AIREL_PEN_STYLE_4 Qt::SolidLine

#define AIREL_COLOR_1 QColor(129,158,220)
#define AIREL_COLOR_2 QColor(129,158,220)
#define AIREL_COLOR_3 QColor(129,158,220)
#define AIREL_COLOR_4 QColor(129,158,220)

#define AIREL_BRUSH_COLOR_1 QColor(129,158,220)
#define AIREL_BRUSH_COLOR_2 QColor(129,158,220)
#define AIREL_BRUSH_COLOR_3 QColor(129,158,220)
#define AIREL_BRUSH_COLOR_4 QColor(129,158,220)

#define AIREL_BRUSH_STYLE_1 Qt::NoBrush
#define AIREL_BRUSH_STYLE_2 Qt::NoBrush
#define AIREL_BRUSH_STYLE_3 Qt::NoBrush
#define AIREL_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_AIREL_PEN_1 4
#define PRINT_AIREL_PEN_2 3

#define PRINT_AIREL_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIREL_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIREL_COLOR_1 QColor(129,158,220)
#define PRINT_AIREL_COLOR_2 QColor(129,158,220)

#define PRINT_AIREL_BRUSH_COLOR_1 QColor(129,158,220)
#define PRINT_AIREL_BRUSH_COLOR_2 QColor(129,158,220)

#define PRINT_AIREL_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIREL_BRUSH_STYLE_2 Qt::NoBrush

// [Airspace E high]
#define AIREH_PEN_1 4
#define AIREH_PEN_2 3
#define AIREH_PEN_3 3
#define AIREH_PEN_4 2

#define AIREH_PEN_STYLE_1 Qt::SolidLine
#define AIREH_PEN_STYLE_2 Qt::SolidLine
#define AIREH_PEN_STYLE_3 Qt::SolidLine
#define AIREH_PEN_STYLE_4 Qt::SolidLine

#define AIREH_COLOR_1 QColor(244,63,66)
#define AIREH_COLOR_2 QColor(244,63,66)
#define AIREH_COLOR_3 QColor(244,63,66)
#define AIREH_COLOR_4 QColor(244,63,66)

#define AIREH_BRUSH_COLOR_1 QColor(244,63,66)
#define AIREH_BRUSH_COLOR_2 QColor(244,63,66)
#define AIREH_BRUSH_COLOR_3 QColor(244,63,66)
#define AIREH_BRUSH_COLOR_4 QColor(244,63,66)

#define AIREH_BRUSH_STYLE_1 Qt::NoBrush
#define AIREH_BRUSH_STYLE_2 Qt::NoBrush
#define AIREH_BRUSH_STYLE_3 Qt::NoBrush
#define AIREH_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_AIREH_PEN_1 4
#define PRINT_AIREH_PEN_2 3

#define PRINT_AIREH_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIREH_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIREH_COLOR_1 QColor(244,63,66)
#define PRINT_AIREH_COLOR_2 QColor(244,63,66)

#define PRINT_AIREH_BRUSH_COLOR_1 QColor(244,63,66)
#define PRINT_AIREH_BRUSH_COLOR_2 QColor(244,63,66)

#define PRINT_AIREH_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIREH_BRUSH_STYLE_2 Qt::NoBrush

// [Airspace F]
#define AIRF_PEN_1 4
#define AIRF_PEN_2 3
#define AIRF_PEN_3 3
#define AIRF_PEN_4 2

#define AIRF_PEN_STYLE_1 Qt::SolidLine
#define AIRF_PEN_STYLE_2 Qt::SolidLine
#define AIRF_PEN_STYLE_3 Qt::SolidLine
#define AIRF_PEN_STYLE_4 Qt::SolidLine

#define AIRF_COLOR_1 QColor(0,0,128)
#define AIRF_COLOR_2 QColor(0,0,128)
#define AIRF_COLOR_3 QColor(0,0,128)
#define AIRF_COLOR_4 QColor(0,0,128)

#define AIRF_BRUSH_COLOR_1 QColor(0,0,128)
#define AIRF_BRUSH_COLOR_2 QColor(0,0,128)
#define AIRF_BRUSH_COLOR_3 QColor(0,0,128)
#define AIRF_BRUSH_COLOR_4 QColor(0,0,128)

#define AIRF_BRUSH_STYLE_1 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_2 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_3 Qt::Dense6Pattern
#define AIRF_BRUSH_STYLE_4 Qt::Dense6Pattern

#define PRINT_AIRF_PEN_1 4
#define PRINT_AIRF_PEN_2 3

#define PRINT_AIRF_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRF_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRF_COLOR_1 QColor(0,0,128)
#define PRINT_AIRF_COLOR_2 QColor(0,0,128)

#define PRINT_AIRF_BRUSH_COLOR_1 QColor(0,0,128)
#define PRINT_AIRF_BRUSH_COLOR_2 QColor(0,0,128)

#define PRINT_AIRF_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIRF_BRUSH_STYLE_2 Qt::NoBrush

// [Control C]
#define CTRC_PEN_1 4
#define CTRC_PEN_2 3
#define CTRC_PEN_3 3
#define CTRC_PEN_4 2

#define CTRC_PEN_STYLE_1 Qt::SolidLine
#define CTRC_PEN_STYLE_2 Qt::SolidLine
#define CTRC_PEN_STYLE_3 Qt::SolidLine
#define CTRC_PEN_STYLE_4 Qt::SolidLine

#define CTRC_COLOR_1 QColor(180,18,20)
#define CTRC_COLOR_2 QColor(180,18,20)
#define CTRC_COLOR_3 QColor(180,18,20)
#define CTRC_COLOR_4 QColor(180,18,20)

#define CTRC_BRUSH_COLOR_1 QColor(180,18,20)
#define CTRC_BRUSH_COLOR_2 QColor(180,18,20)
#define CTRC_BRUSH_COLOR_3 QColor(180,18,20)
#define CTRC_BRUSH_COLOR_4 QColor(180,18,20)

#define CTRC_BRUSH_STYLE_1 Qt::HorPattern
#define CTRC_BRUSH_STYLE_2 Qt::HorPattern
#define CTRC_BRUSH_STYLE_3 Qt::HorPattern
#define CTRC_BRUSH_STYLE_4 Qt::HorPattern

#define PRINT_CTRC_PEN_1 4
#define PRINT_CTRC_PEN_2 3

#define PRINT_CTRC_PEN_STYLE_1 Qt::SolidLine
#define PRINT_CTRC_PEN_STYLE_2 Qt::SolidLine

#define PRINT_CTRC_COLOR_1 QColor(180,18,20)
#define PRINT_CTRC_COLOR_2 QColor(180,18,20)

#define PRINT_CTRC_BRUSH_COLOR_1 QColor(180,18,20)
#define PRINT_CTRC_BRUSH_COLOR_2 QColor(180,18,20)

#define PRINT_CTRC_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_CTRC_BRUSH_STYLE_2 Qt::NoBrush

// [Control D]
#define CTRD_PEN_1 4
#define CTRD_PEN_2 3
#define CTRD_PEN_3 2
#define CTRD_PEN_4 1

#define CTRD_PEN_STYLE_1 Qt::SolidLine
#define CTRD_PEN_STYLE_2 Qt::SolidLine
#define CTRD_PEN_STYLE_3 Qt::SolidLine
#define CTRD_PEN_STYLE_4 Qt::SolidLine

#define CTRD_COLOR_1 QColor(180,18,20)
#define CTRD_COLOR_2 QColor(180,18,20)
#define CTRD_COLOR_3 QColor(180,18,20)
#define CTRD_COLOR_4 QColor(180,18,20)

#define CTRD_BRUSH_COLOR_1 QColor(180,18,20)
#define CTRD_BRUSH_COLOR_2 QColor(180,18,20)
#define CTRD_BRUSH_COLOR_3 QColor(180,18,20)
#define CTRD_BRUSH_COLOR_4 QColor(180,18,20)

#define CTRD_BRUSH_STYLE_1 Qt::Dense5Pattern
#define CTRD_BRUSH_STYLE_2 Qt::Dense5Pattern
#define CTRD_BRUSH_STYLE_3 Qt::Dense5Pattern
#define CTRD_BRUSH_STYLE_4 Qt::Dense5Pattern

#define PRINT_CTRD_PEN_1 4
#define PRINT_CTRD_PEN_2 3

#define PRINT_CTRD_PEN_STYLE_1 Qt::SolidLine
#define PRINT_CTRD_PEN_STYLE_2 Qt::SolidLine

#define PRINT_CTRD_COLOR_1 QColor(180,18,20)
#define PRINT_CTRD_COLOR_2 QColor(180,18,20)

#define PRINT_CTRD_BRUSH_COLOR_1 QColor(180,18,20)
#define PRINT_CTRD_BRUSH_COLOR_2 QColor(180,18,20)

#define PRINT_CTRD_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_CTRD_BRUSH_STYLE_2 Qt::NoBrush

// [Lowflight area]
#define LOWF_PEN_1 4
#define LOWF_PEN_2 3
#define LOWF_PEN_3 3
#define LOWF_PEN_4 2

#define LOWF_PEN_STYLE_1 Qt::SolidLine
#define LOWF_PEN_STYLE_2 Qt::SolidLine
#define LOWF_PEN_STYLE_3 Qt::SolidLine
#define LOWF_PEN_STYLE_4 Qt::SolidLine

#define LOWF_COLOR_1 QColor(0,120,0)
#define LOWF_COLOR_2 QColor(0,120,0)
#define LOWF_COLOR_3 QColor(0,120,0)
#define LOWF_COLOR_4 QColor(0,120,0)

#define LOWF_BRUSH_COLOR_1 QColor(0,120,0)
#define LOWF_BRUSH_COLOR_2 QColor(0,120,0)
#define LOWF_BRUSH_COLOR_3 QColor(0,120,0)
#define LOWF_BRUSH_COLOR_4 QColor(0,120,0)

#define LOWF_BRUSH_STYLE_1 Qt::NoBrush
#define LOWF_BRUSH_STYLE_2 Qt::NoBrush
#define LOWF_BRUSH_STYLE_3 Qt::NoBrush
#define LOWF_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_LOWF_PEN_1 4
#define PRINT_LOWF_PEN_2 3

#define PRINT_LOWF_PEN_STYLE_1 Qt::SolidLine
#define PRINT_LOWF_PEN_STYLE_2 Qt::SolidLine

#define PRINT_LOWF_COLOR_1 QColor(0,120,0)
#define PRINT_LOWF_COLOR_2 QColor(0,120,0)

#define PRINT_LOWF_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_LOWF_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_LOWF_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_LOWF_BRUSH_STYLE_2 Qt::NoBrush

// [Danger]
#define DNG_PEN_1 4
#define DNG_PEN_2 3
#define DNG_PEN_3 2
#define DNG_PEN_4 1

#define DNG_PEN_STYLE_1 Qt::SolidLine
#define DNG_PEN_STYLE_2 Qt::SolidLine
#define DNG_PEN_STYLE_3 Qt::SolidLine
#define DNG_PEN_STYLE_4 Qt::SolidLine

#define DNG_COLOR_1 QColor(0,0,128)
#define DNG_COLOR_2 QColor(0,0,128)
#define DNG_COLOR_3 QColor(0,0,128)
#define DNG_COLOR_4 QColor(0,0,128)

#define DNG_BRUSH_COLOR_1 QColor(0,0,128)
#define DNG_BRUSH_COLOR_2 QColor(0,0,128)
#define DNG_BRUSH_COLOR_3 QColor(0,0,128)
#define DNG_BRUSH_COLOR_4 QColor(0,0,128)

#define DNG_BRUSH_STYLE_1 Qt::HorPattern
#define DNG_BRUSH_STYLE_2 Qt::HorPattern
#define DNG_BRUSH_STYLE_3 Qt::HorPattern
#define DNG_BRUSH_STYLE_4 Qt::HorPattern

#define PRINT_DNG_PEN_1 4
#define PRINT_DNG_PEN_2 3

#define PRINT_DNG_PEN_STYLE_1 Qt::SolidLine
#define PRINT_DNG_PEN_STYLE_2 Qt::SolidLine

#define PRINT_DNG_COLOR_1 QColor(0,0,128)
#define PRINT_DNG_COLOR_2 QColor(0,0,128)

#define PRINT_DNG_BRUSH_COLOR_1 QColor(0,0,128)
#define PRINT_DNG_BRUSH_COLOR_2 QColor(0,0,128)

#define PRINT_DNG_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_DNG_BRUSH_STYLE_2 Qt::NoBrush

// [Restricted]
#define RES_PEN_1 4
#define RES_PEN_2 3
#define RES_PEN_3 2
#define RES_PEN_4 1

#define RES_PEN_STYLE_1 Qt::SolidLine
#define RES_PEN_STYLE_2 Qt::SolidLine
#define RES_PEN_STYLE_3 Qt::SolidLine
#define RES_PEN_STYLE_4 Qt::SolidLine

#define RES_COLOR_1 QColor(0,0,128)
#define RES_COLOR_2 QColor(0,0,128)
#define RES_COLOR_3 QColor(0,0,128)
#define RES_COLOR_4 QColor(0,0,128)

#define RES_BRUSH_COLOR_1 QColor(0,0,128)
#define RES_BRUSH_COLOR_2 QColor(0,0,128)
#define RES_BRUSH_COLOR_3 QColor(0,0,128)
#define RES_BRUSH_COLOR_4 QColor(0,0,128)

#define RES_BRUSH_STYLE_1 Qt::VerPattern
#define RES_BRUSH_STYLE_2 Qt::VerPattern
#define RES_BRUSH_STYLE_3 Qt::VerPattern
#define RES_BRUSH_STYLE_4 Qt::VerPattern

#define PRINT_RES_PEN_1 4
#define PRINT_RES_PEN_2 3

#define PRINT_RES_PEN_STYLE_1 Qt::SolidLine
#define PRINT_RES_PEN_STYLE_2 Qt::SolidLine

#define PRINT_RES_COLOR_1 QColor(0,0,128)
#define PRINT_RES_COLOR_2 QColor(0,0,128)

#define PRINT_RES_BRUSH_COLOR_1 QColor(0,0,128)
#define PRINT_RES_BRUSH_COLOR_2 QColor(0,0,128)

#define PRINT_RES_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_RES_BRUSH_STYLE_2 Qt::NoBrush

// [Transponder Mandatory Zone]
#define TMZ_PEN_1 4
#define TMZ_PEN_2 3
#define TMZ_PEN_3 3
#define TMZ_PEN_4 2

#define TMZ_PEN_STYLE_1 Qt::SolidLine
#define TMZ_PEN_STYLE_2 Qt::SolidLine
#define TMZ_PEN_STYLE_3 Qt::SolidLine
#define TMZ_PEN_STYLE_4 Qt::SolidLine

#define TMZ_COLOR_1 QColor(0,120,0)
#define TMZ_COLOR_2 QColor(0,120,0)
#define TMZ_COLOR_3 QColor(0,120,0)
#define TMZ_COLOR_4 QColor(0,120,0)

#define TMZ_BRUSH_COLOR_1 QColor(0,120,0)
#define TMZ_BRUSH_COLOR_2 QColor(0,120,0)
#define TMZ_BRUSH_COLOR_3 QColor(0,120,0)
#define TMZ_BRUSH_COLOR_4 QColor(0,120,0)

#define TMZ_BRUSH_STYLE_1 Qt::NoBrush
#define TMZ_BRUSH_STYLE_2 Qt::NoBrush
#define TMZ_BRUSH_STYLE_3 Qt::NoBrush
#define TMZ_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_TMZ_PEN_1 4
#define PRINT_TMZ_PEN_2 3

#define PRINT_TMZ_PEN_STYLE_1 Qt::SolidLine
#define PRINT_TMZ_PEN_STYLE_2 Qt::SolidLine

#define PRINT_TMZ_COLOR_1 QColor(0,120,0)
#define PRINT_TMZ_COLOR_2 QColor(0,120,0)

#define PRINT_TMZ_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_TMZ_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_TMZ_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_TMZ_BRUSH_STYLE_2 Qt::NoBrush

// [FAI Area < 500]
#define FAI_LOW_500_PEN_1 4
#define FAI_LOW_500_PEN_2 3
#define FAI_LOW_500_PEN_3 3
#define FAI_LOW_500_PEN_4 2

#define FAI_LOW_500_PEN_STYLE_1 Qt::SolidLine
#define FAI_LOW_500_PEN_STYLE_2 Qt::SolidLine
#define FAI_LOW_500_PEN_STYLE_3 Qt::SolidLine
#define FAI_LOW_500_PEN_STYLE_4 Qt::SolidLine

#define FAI_LOW_500_COLOR_1 QColor(255,0,0)
#define FAI_LOW_500_COLOR_2 QColor(255,0,0)
#define FAI_LOW_500_COLOR_3 QColor(255,0,0)
#define FAI_LOW_500_COLOR_4 QColor(255,0,0)

#define FAI_LOW_500_BRUSH_COLOR_1 QColor(255,0,0)
#define FAI_LOW_500_BRUSH_COLOR_2 QColor(255,0,0)
#define FAI_LOW_500_BRUSH_COLOR_3 QColor(255,0,0)
#define FAI_LOW_500_BRUSH_COLOR_4 QColor(255,0,0)

#define FAI_LOW_500_BRUSH_STYLE_1 Qt::Dense5Pattern
#define FAI_LOW_500_BRUSH_STYLE_2 Qt::Dense5Pattern
#define FAI_LOW_500_BRUSH_STYLE_3 Qt::Dense5Pattern
#define FAI_LOW_500_BRUSH_STYLE_4 Qt::Dense5Pattern

#define PRINT_FAI_LOW_500_PEN_1 4
#define PRINT_FAI_LOW_500_PEN_2 3

#define PRINT_FAI_LOW_500_PEN_STYLE_1 Qt::SolidLine
#define PRINT_FAI_LOW_500_PEN_STYLE_2 Qt::SolidLine

#define PRINT_FAI_LOW_500_COLOR_1 QColor(255,0,0)
#define PRINT_FAI_LOW_500_COLOR_2 QColor(255,0,0)

#define PRINT_FAI_LOW_500_BRUSH_COLOR_1 QColor(255,0,0)
#define PRINT_FAI_LOW_500_BRUSH_COLOR_2 QColor(255,0,0)

#define PRINT_FAI_LOW_500_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_FAI_LOW_500_BRUSH_STYLE_2 Qt::NoBrush

// [FAI Area >= 500]
#define FAI_HIGH_500_PEN_1 4
#define FAI_HIGH_500_PEN_2 3
#define FAI_HIGH_500_PEN_3 3
#define FAI_HIGH_500_PEN_4 2

#define FAI_HIGH_500_PEN_STYLE_1 Qt::SolidLine
#define FAI_HIGH_500_PEN_STYLE_2 Qt::SolidLine
#define FAI_HIGH_500_PEN_STYLE_3 Qt::SolidLine
#define FAI_HIGH_500_PEN_STYLE_4 Qt::SolidLine

#define FAI_HIGH_500_COLOR_1 QColor(0,255,0)
#define FAI_HIGH_500_COLOR_2 QColor(0,255,0)
#define FAI_HIGH_500_COLOR_3 QColor(0,255,0)
#define FAI_HIGH_500_COLOR_4 QColor(0,255,0)

#define FAI_HIGH_500_BRUSH_COLOR_1 QColor(0,255,0)
#define FAI_HIGH_500_BRUSH_COLOR_2 QColor(0,255,0)
#define FAI_HIGH_500_BRUSH_COLOR_3 QColor(0,255,0)
#define FAI_HIGH_500_BRUSH_COLOR_4 QColor(0,255,0)

#define FAI_HIGH_500_BRUSH_STYLE_1 Qt::Dense5Pattern
#define FAI_HIGH_500_BRUSH_STYLE_2 Qt::Dense5Pattern
#define FAI_HIGH_500_BRUSH_STYLE_3 Qt::Dense5Pattern
#define FAI_HIGH_500_BRUSH_STYLE_4 Qt::Dense5Pattern

#define PRINT_FAI_HIGH_500_PEN_1 4
#define PRINT_FAI_HIGH_500_PEN_2 3

#define PRINT_FAI_HIGH_500_PEN_STYLE_1 Qt::SolidLine
#define PRINT_FAI_HIGH_500_PEN_STYLE_2 Qt::SolidLine

#define PRINT_FAI_HIGH_500_COLOR_1 QColor(0,255,0)
#define PRINT_FAI_HIGH_500_COLOR_2 QColor(0,255,0)

#define PRINT_FAI_HIGH_500_BRUSH_COLOR_1 QColor(0,255,0)
#define PRINT_FAI_HIGH_500_BRUSH_COLOR_2 QColor(0,255,0)

#define PRINT_FAI_HIGH_500_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_FAI_HIGH_500_BRUSH_STYLE_2 Qt::NoBrush
