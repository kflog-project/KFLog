/***********************************************************************
**
**   mapdefaults.h
**
**   This file is part of KFLog2.
**
************************************************************************
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
#define LEVEL_SUB QColor(96, 128, 248)
#define LEVEL_0 QColor(165, 214, 126)
#define LEVEL_10 QColor(185, 220, 131)
#define LEVEL_25 QColor(193, 225, 138)
#define LEVEL_50 QColor(208, 234, 151)
#define LEVEL_75 QColor(222, 243, 164)
#define LEVEL_100 QColor(237, 252, 178)
#define LEVEL_200 QColor(236, 241, 169)
#define LEVEL_300 QColor(236, 230, 160)
#define LEVEL_400 QColor(236, 219, 151)
#define LEVEL_500 QColor(236, 208, 142)
#define LEVEL_600 QColor(235, 198, 133)
#define LEVEL_700 QColor(235, 187, 124)
#define LEVEL_800 QColor(235, 176, 115)
#define LEVEL_900 QColor(235, 165, 106)
#define LEVEL_1000 QColor(235, 155, 98)
#define LEVEL_1250 QColor(226, 147, 91)
#define LEVEL_1500 QColor(217, 140, 85)
#define LEVEL_1750 QColor(208, 132, 78)
#define LEVEL_2000 QColor(200, 125, 72)
#define LEVEL_2250 QColor(191, 117, 65)
#define LEVEL_2500 QColor(182, 110, 59)
#define LEVEL_2750 QColor(173, 102, 52)
#define LEVEL_3000 QColor(165, 95, 46)
#define LEVEL_3250 QColor(156, 87, 39)
#define LEVEL_3500 QColor(147, 80, 33)
#define LEVEL_3750 QColor(138, 72, 26)
#define LEVEL_4000 QColor(130, 65, 20)
#define LEVEL_4250 QColor(128, 63, 19)
#define LEVEL_4500 QColor(126, 62, 19)
#define LEVEL_4750 QColor(124, 61, 19)
#define LEVEL_5000 QColor(123, 60, 19)
#define LEVEL_5250 QColor(121, 59, 19)
#define LEVEL_5500 QColor(119, 58, 18)
#define LEVEL_5750 QColor(118, 57, 18)
#define LEVEL_6000 QColor(116, 56, 18)
#define LEVEL_6250 QColor(114, 55, 18)
#define LEVEL_6500 QColor(113, 54, 18)
#define LEVEL_6750 QColor(111, 52, 17)
#define LEVEL_7000 QColor(109, 51, 17)
#define LEVEL_7250 QColor(107, 50, 17)
#define LEVEL_7500 QColor(106, 49, 17)
#define LEVEL_7750 QColor(104, 48, 17)
#define LEVEL_8000 QColor(102, 47, 16)
#define LEVEL_8250 QColor(101, 46, 16)
#define LEVEL_8500 QColor(99, 45, 16)
#define LEVEL_8750 QColor(97, 44, 16)

// Default-Values for map-element:
//
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

#define AIRC_BRUSH_STYLE_1 Qt::Dense5Pattern
#define AIRC_BRUSH_STYLE_2 Qt::Dense5Pattern
#define AIRC_BRUSH_STYLE_3 Qt::Dense5Pattern
#define AIRC_BRUSH_STYLE_4 Qt::Dense5Pattern

#define PRINT_AIRC_PEN_1 4
#define PRINT_AIRC_PEN_2 3

#define PRINT_AIRC_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIRC_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIRC_COLOR_1 QColor(0,120,0)
#define PRINT_AIRC_COLOR_2 QColor(0,120,0)

#define PRINT_AIRC_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRC_BRUSH_COLOR_2 QColor(0,120,0)

#define PRINT_AIRC_BRUSH_STYLE_1 Qt::NoBrush
#define PRINT_AIRC_BRUSH_STYLE_2 Qt::NoBrush

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
#define AIREL_COLOR_2 QColor(138,169,235)
#define AIREL_COLOR_3 QColor(138,169,235)
#define AIREL_COLOR_4 QColor(138,169,235)

#define AIREL_BRUSH_COLOR_1 QColor(0,120,0)
#define AIREL_BRUSH_COLOR_2 QColor(0,120,0)
#define AIREL_BRUSH_COLOR_3 QColor(0,120,0)
#define AIREL_BRUSH_COLOR_4 QColor(0,120,0)

#define AIREL_BRUSH_STYLE_1 Qt::NoBrush
#define AIREL_BRUSH_STYLE_2 Qt::NoBrush
#define AIREL_BRUSH_STYLE_3 Qt::NoBrush
#define AIREL_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_AIREL_PEN_1 4
#define PRINT_AIREL_PEN_2 3

#define PRINT_AIREL_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIREL_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIREL_COLOR_1 QColor(0,120,0)
#define PRINT_AIREL_COLOR_2 QColor(0,120,0)

#define PRINT_AIREL_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIREL_BRUSH_COLOR_2 QColor(0,120,0)

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

#define AIREH_BRUSH_COLOR_1 QColor(0,120,0)
#define AIREH_BRUSH_COLOR_2 QColor(0,120,0)
#define AIREH_BRUSH_COLOR_3 QColor(0,120,0)
#define AIREH_BRUSH_COLOR_4 QColor(0,120,0)

#define AIREH_BRUSH_STYLE_1 Qt::NoBrush
#define AIREH_BRUSH_STYLE_2 Qt::NoBrush
#define AIREH_BRUSH_STYLE_3 Qt::NoBrush
#define AIREH_BRUSH_STYLE_4 Qt::NoBrush

#define PRINT_AIREH_PEN_1 4
#define PRINT_AIREH_PEN_2 3

#define PRINT_AIREH_PEN_STYLE_1 Qt::SolidLine
#define PRINT_AIREH_PEN_STYLE_2 Qt::SolidLine

#define PRINT_AIREH_COLOR_1 QColor(0,120,0)
#define PRINT_AIREH_COLOR_2 QColor(0,120,0)

#define PRINT_AIREH_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIREH_BRUSH_COLOR_2 QColor(0,120,0)

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

#define PRINT_AIRF_COLOR_1 QColor(0,120,0)
#define PRINT_AIRF_COLOR_2 QColor(0,120,0)

#define PRINT_AIRF_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_AIRF_BRUSH_COLOR_2 QColor(0,120,0)

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

#define PRINT_CTRC_COLOR_1 QColor(0,120,0)
#define PRINT_CTRC_COLOR_2 QColor(0,120,0)

#define PRINT_CTRC_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_CTRC_BRUSH_COLOR_2 QColor(0,120,0)

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

#define PRINT_CTRD_COLOR_1 QColor(0,120,0)
#define PRINT_CTRD_COLOR_2 QColor(0,120,0)

#define PRINT_CTRD_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_CTRD_BRUSH_COLOR_2 QColor(0,120,0)

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

#define PRINT_DNG_COLOR_1 QColor(0,120,0)
#define PRINT_DNG_COLOR_2 QColor(0,120,0)

#define PRINT_DNG_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_DNG_BRUSH_COLOR_2 QColor(0,120,0)

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

#define PRINT_RES_COLOR_1 QColor(0,120,0)
#define PRINT_RES_COLOR_2 QColor(0,120,0)

#define PRINT_RES_BRUSH_COLOR_1 QColor(0,120,0)
#define PRINT_RES_BRUSH_COLOR_2 QColor(0,120,0)

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
