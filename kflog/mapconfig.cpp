/***********************************************************************
**
**   mapconfig.cpp
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

#include "mapconfig.h"

#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>

#include <basemapelement.h>
#include <flight.h>
#include <mapdefaults.h>

#define READ_TOPO(a,b) \
  topographyColorList.append(new QColor(config->readColorEntry(a, new b)));

#define READ_BORDER(a) \
    a[0] = config->readBoolEntry("Border 1", true); \
    a[1] = config->readBoolEntry("Border 2", true); \
    a[2] = config->readBoolEntry("Border 3", true); \
    a[3] = config->readBoolEntry("Border 4", true); \
    a[4] = config->readBoolEntry("Print Border 1", true); \
    a[5] = config->readBoolEntry("Print Border 2", true);

#define READ_PEN(G, A, B, C1, C2, C3, C4, C5, C6, P1, P2, P3, P4, P5, P6, \
    S1, S2, S3, S4, S5, S6) \
  config->setGroup(G); \
  READ_BORDER(B) \
  A.append(new QPen(config->readColorEntry("Color 1", new C1), \
        config->readNumEntry("Pen Size 1", P1), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", S1))); \
  A.append(new QPen(config->readColorEntry("Color 2", new C2), \
        config->readNumEntry("Pen Size 2", P2), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 2", S2))); \
  A.append(new QPen(config->readColorEntry("Color 3", new C3), \
        config->readNumEntry("Pen Size 3", P3), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 3", S3))); \
  A.append(new QPen(config->readColorEntry("Color 4", new C4), \
        config->readNumEntry("Pen Size 4", P4), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 4", S4))); \
  A.append(new QPen(config->readColorEntry("Print Color 1", new C5), \
        config->readNumEntry("Print Pen Size 1", P5), \
        (Qt::PenStyle)config->readNumEntry("Print Pen Style 1", S5))); \
  A.append(new QPen(config->readColorEntry("Print Color 2", new C6), \
        config->readNumEntry("Print Pen Size 2", P6), \
        (Qt::PenStyle)config->readNumEntry("Print Pen Style 2", S6)));

#define READ_PEN_BRUSH(G, a, B, A, C1, C2, C3, C4, C5, C6, P1, P2, P3, P4, P5, P6, \
    S1, S2, S3, S4, S5, S6, C7, C8, C9, C10, C11, C12, S7, S8, S9, S10, S11, S12) \
  READ_PEN(G, a, B, C1, C2, C3, C4, C5, C6, P1, P2, P3, P4, P5, P6, \
        S1, S2, S3, S4, S5, S6) \
  A.append(new QBrush(config->readColorEntry("Brush Color 1", new C7), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 1", S7))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 2", new C8), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 2", S8))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 3", new C9), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 3", S9))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 4", new C10), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 4", S10))); \
  A.append(new QBrush(config->readColorEntry("Print Brush Color 1", new C11), \
        (Qt::BrushStyle)config->readNumEntry("Print Brush Style 1", S11))); \
  A.append(new QBrush(config->readColorEntry("Print Brush Color 2", new C12), \
        (Qt::BrushStyle)config->readNumEntry("Print Brush Style 2", S12)));

MapConfig::MapConfig()
  : scaleIndex(0), printScaleIndex(0), isSwitch(false),
    drawFType(MapConfig::Vario)
{
  airCBorder = new bool[6];
  airDBorder = new bool[6];
  airElBorder = new bool[6];
  airEhBorder = new bool[6];
  airFBorder = new bool[6];
  ctrCBorder = new bool[6];
  ctrDBorder = new bool[6];
  dangerBorder = new bool[6];
  lowFBorder = new bool[6];
  restrBorder = new bool[6];
  tmzBorder = new bool[6];

  roadBorder = new bool[6];
  highwayBorder = new bool[6];
  railBorder = new bool[6];
  riverBorder = new bool[6];
  cityBorder = new bool[6];
}

MapConfig::~MapConfig()
{

}

void MapConfig::slotReadConfig()
{
  KConfig* config = KGlobal::config();
  /* Unfortunately, a ~QList() doesn't work, so we must remove all
   * item manualy ;-(
   */
  while(topographyColorList.remove());

  while(airCPenList.remove());
  while(airCBrushList.remove());
  while(airDPenList.remove());
  while(airDBrushList.remove());
  while(airElPenList.remove());
  while(airElBrushList.remove());
  while(airEhPenList.remove());
  while(airEhBrushList.remove());
  while(airFPenList.remove());
  while(airFBrushList.remove());
  while(ctrCPenList.remove());
  while(ctrCBrushList.remove());
  while(ctrDPenList.remove());
  while(ctrDBrushList.remove());
  while(dangerPenList.remove());
  while(dangerBrushList.remove());
  while(lowFPenList.remove());
  while(lowFBrushList.remove());
  while(restrPenList.remove());
  while(restrBrushList.remove());
  while(tmzPenList.remove());
  while(tmzBrushList.remove());

  while(roadPenList.remove());
  while(highwayPenList.remove());
  while(riverPenList.remove());
  while(railPenList.remove());
  while(cityPenList.remove());
  while(cityBrushList.remove());

  config->setGroup("Topography");
  READ_TOPO("SubTerrain", LEVEL_SUB)
  READ_TOPO("0M", LEVEL_0)
  READ_TOPO("10M", LEVEL_10)
  READ_TOPO("25M", LEVEL_25)
  READ_TOPO("50M", LEVEL_50)
  READ_TOPO("75M", LEVEL_75)
  READ_TOPO("100M", LEVEL_100)
  READ_TOPO("200M", LEVEL_200)
  READ_TOPO("300M", LEVEL_300)
  READ_TOPO("400M", LEVEL_400)
  READ_TOPO("500M", LEVEL_500)
  READ_TOPO("600M", LEVEL_600)
  READ_TOPO("700M", LEVEL_700)
  READ_TOPO("800M", LEVEL_800)
  READ_TOPO("900M", LEVEL_900)
  READ_TOPO("1000M", LEVEL_1000)
  READ_TOPO("1250M", LEVEL_1250)
  READ_TOPO("1500M", LEVEL_1500)
  READ_TOPO("1750M", LEVEL_1750)
  READ_TOPO("2000M", LEVEL_2000)
  READ_TOPO("2250M", LEVEL_2250)
  READ_TOPO("2500M", LEVEL_2500)
  READ_TOPO("2750M", LEVEL_2750)
  READ_TOPO("3000M", LEVEL_3000)
  READ_TOPO("3250M", LEVEL_3250)
  READ_TOPO("3500M", LEVEL_3500)
  READ_TOPO("3750M", LEVEL_3750)
  READ_TOPO("4000M", LEVEL_4000)
  READ_TOPO("4250M", LEVEL_4250)
  READ_TOPO("4500M", LEVEL_4500)
  READ_TOPO("4750M", LEVEL_4750)
  READ_TOPO("5000M", LEVEL_5000)
  READ_TOPO("5250M", LEVEL_5250)
  READ_TOPO("5500M", LEVEL_5500)
  READ_TOPO("5750M", LEVEL_5750)
  READ_TOPO("6000M", LEVEL_6000)
  READ_TOPO("6250M", LEVEL_6250)
  READ_TOPO("6500M", LEVEL_6500)
  READ_TOPO("6750M", LEVEL_6750)
  READ_TOPO("7000M", LEVEL_7000)
  READ_TOPO("7250M", LEVEL_7250)
  READ_TOPO("7500M", LEVEL_7500)
  READ_TOPO("7750M", LEVEL_7750)
  READ_TOPO("8000M", LEVEL_8000)
  READ_TOPO("8250M", LEVEL_8250)
  READ_TOPO("8500M", LEVEL_8500)
  READ_TOPO("8750M", LEVEL_8750)

  READ_PEN("Road", roadPenList, roadBorder,
        ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2)

  READ_PEN("River", riverPenList, riverBorder,
        RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
        PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
        RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2)

  READ_PEN("Rail", railPenList, railBorder,
        RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2)

  READ_PEN("Highway", highwayPenList, highwayBorder,
        HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2)
  /*
   * In version <= 2.0.1, the fillcolor of cities is called "Color" instead
   * of "Brush Color", so we must look, which version of configfile we read.
   */
  config->setGroup("General Options");
  if(config->hasKey("Version") && config->readEntry("Version") >= "2.0.2")
    {
      // PenStyle and BrushStyle are not used for cities ...
      READ_PEN_BRUSH("City", cityPenList, cityBorder, cityBrushList,
            CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
            CITY_COLOR_4, PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2,
            CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
            PRINT_CITY_PEN_1, PRINT_CITY_PEN_2,
            Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine,
            Qt::SolidLine, Qt::SolidLine,
            CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
            CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
            PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
            Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern,
            Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern)
    }
  else
    {
      // We assume to have an old configfile ...
      config->setGroup("City");
      cityPenList.append(new QPen(CITY_COLOR_1, 1));
      cityPenList.append(new QPen(CITY_COLOR_2, 1));
      cityPenList.append(new QPen(CITY_COLOR_3, 1));
      cityPenList.append(new QPen(CITY_COLOR_4, 1));
      cityPenList.append(new QPen(PRINT_CITY_COLOR_1, 1));
      cityPenList.append(new QPen(PRINT_CITY_COLOR_2, 1));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_1, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_2, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_3, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_4, Qt::SolidPattern));
      cityBrushList.append(new QBrush(PRINT_CITY_BRUSH_COLOR_1, Qt::SolidPattern));
      cityBrushList.append(new QBrush(PRINT_CITY_BRUSH_COLOR_2, Qt::SolidPattern));
      READ_BORDER(cityBorder);
    }

  READ_PEN_BRUSH("Airspace C", airCPenList, airCBorder, airCBrushList,
        AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4,
        PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2,
        AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
        PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
        AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4,
        PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace D", airDPenList, airDBorder, airDBrushList,
        AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4,
        PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2,
        AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
        PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
        AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4,
        PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace E low", airElPenList, airElBorder, airElBrushList,
        AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
        PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2,
        AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4,
        PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2,
        AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
        PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
        AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4,
        PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace E high", airEhPenList, airEhBorder, airEhBrushList,
        AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
        PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2,
        AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4,
        PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2,
        AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
        PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
        AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4,
        PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace F", airFPenList, airFBorder, airFBrushList,
        AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3, AIRF_COLOR_4,
        PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2,
        AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4,
        PRINT_AIRF_PEN_STYLE_1, PRINT_AIRF_PEN_STYLE_2,
        AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4,
        PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_STYLE_1, AIRF_BRUSH_STYLE_2,
        AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4,
        PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Control C", ctrCPenList, ctrCBorder,ctrCBrushList,
        CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4,
        PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2,
        CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
        PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
        CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4,
        PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Control D", ctrDPenList, ctrDBorder, ctrDBrushList,
        CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4,
        PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2,
        CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
        PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
        CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4,
        PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Danger", dangerPenList, dangerBorder, dangerBrushList,
        DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        PRINT_DNG_PEN_1, PRINT_DNG_PEN_2,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4,
        PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2,
        DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
        PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
        DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
        DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4,
        PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Low Flight", lowFPenList, lowFBorder,lowFBrushList,
        LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4,
        PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2,
        LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
        PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
        LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4,
        PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Restricted Area", restrPenList, restrBorder, restrBrushList,
        RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        PRINT_RES_COLOR_1, PRINT_RES_COLOR_2,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        PRINT_RES_PEN_1, PRINT_RES_PEN_2,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4,
        PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2,
        RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
        PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
        RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
        RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4,
        PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2)

  READ_PEN_BRUSH("TMZ", tmzPenList, tmzBorder, tmzBrushList,
        TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4,
        PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2,
        TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
        PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
        TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4,
        PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2)

  config->setGroup("Flight");
  drawFType = config->readNumEntry("Draw Type", MapConfig::Speed);

  config->setGroup(0);

  emit configChanged();
}

void MapConfig::slotSetFlightDataType(int type)  {  drawFType = type;  }

void MapConfig::slotSetMatrixValues(int index, bool sw)
{
  isSwitch = sw;
  scaleIndex = index;
}

void MapConfig::slotSetPrintMatrixValues(int index)
{
  // We increase the index by four, because we use four elements for drawing.
  printScaleIndex = index + 4;
}

QPen MapConfig::getPrintPen(unsigned int typeID)
{
  return __getPen(typeID, printScaleIndex);
}

QPen MapConfig::getDrawPen(struct flightPoint* fP)
{
  //
  // Dynamische Farben im Flug:
  //
  //   Farben als Legende ausgeben ???
  //
  //   Farbwerte müssen noch eingestellt werden. Konfigurierbar???
  //

  if(!isSwitch)  return QPen(QColor(0,100,200), 3);

  int red = 0, green = 0, blue = 0;
  int width = 4;

  switch(drawFType)
    {
      case MapConfig::Vario:
        if(fP->dH < 0)
          {
            red = 0;
            green = MAX(0, (int)(220 - 5.0 * -fP->dH));
            blue = 255;
          }
        else
          {
            red = 255;
            green = 0;
            blue = MAX(0, (int)(220 - 5.0 * fP->dH));
          }
        break;
      case MapConfig::Speed:
        red = 0;
        green = MIN(255, (int)(0.0 + (fP->dS / MAX(1, fP->dT)) * 4.0));
        blue = 255;
        break;
      case MapConfig::Altitude:
        if(fP->height < 500.0)
          {
            red = MAX(0, (int)(100.0 - fP->height * 0.2));
            green = 0;
            blue = MIN(255, (int)(100.0 + fP->height * 0.2));
          }
        else
          {
            red = 0;
            green = MIN(255, (int)(0.0 + 0.2 * (fP->height - 500.0)));
            blue = 255;
          }
        break;
      case MapConfig::Cycling:
        switch(fP->f_state)
          {
            case Flight::LeftTurn:
              red = 255;
              green = 50;
              blue = 0;
              break;
            case Flight::RightTurn:
              red = 50;
              green = 255;
              blue = 0;
              break;
            case Flight::MixedTurn:
              red = 200;
              green = 0;
              blue = 200;
              break;
            case Flight::Straight:
            default:
              red = 0;
              green = 50;
              blue = 255;
              break;
          }
        break;
    }

  return QPen(QColor(red, green, blue), width);
}

QPen MapConfig::getDrawPen(unsigned int typeID)
{
  return __getPen(typeID, scaleIndex);
}

QPen MapConfig::__getPen(unsigned int typeID, int sIndex)
{
  switch(typeID)
    {
      case BaseMapElement::Road:
          return *roadPenList.at(sIndex);
      case BaseMapElement::Highway:
          return *highwayPenList.at(sIndex);
      case BaseMapElement::Railway:
          return *railPenList.at(sIndex);
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return *riverPenList.at(sIndex);
      case BaseMapElement::City:
          return *cityPenList.at(sIndex);
      case BaseMapElement::AirC:
          return *airCPenList.at(sIndex);
      case BaseMapElement::AirD:
          return *airDPenList.at(sIndex);
      case BaseMapElement::AirElow:
          return *airElPenList.at(sIndex);
      case BaseMapElement::AirEhigh:
          return *airEhPenList.at(sIndex);
      case BaseMapElement::AirF:
          return *airFPenList.at(sIndex);
      case BaseMapElement::ControlC:
          return *ctrCPenList.at(sIndex);
      case BaseMapElement::ControlD:
          return *ctrDPenList.at(sIndex);
      case BaseMapElement::Danger:
          return *dangerPenList.at(sIndex);
      case BaseMapElement::LowFlight:
          return *lowFPenList.at(sIndex);
      case BaseMapElement::Restricted:
          return *restrPenList.at(sIndex);
      case BaseMapElement::TMZ:
          return *tmzPenList.at(sIndex);
      default:
          return *roadPenList.at(sIndex);
    }
}

bool MapConfig::isBorder(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Road:
          return roadBorder[scaleIndex];
      case BaseMapElement::Highway:
          return highwayBorder[scaleIndex];
      case BaseMapElement::Railway:
          return railBorder[scaleIndex];
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return riverBorder[scaleIndex];
      case BaseMapElement::City:
          return cityBorder[scaleIndex];
      case BaseMapElement::AirC:
          return airCBorder[scaleIndex];
      case BaseMapElement::AirD:
          return airDBorder[scaleIndex];
      case BaseMapElement::AirElow:
          return airElBorder[scaleIndex];
      case BaseMapElement::AirEhigh:
          return airEhBorder[scaleIndex];
      case BaseMapElement::AirF:
          return airFBorder[scaleIndex];
      case BaseMapElement::ControlC:
          return ctrCBorder[scaleIndex];
      case BaseMapElement::ControlD:
          return ctrDBorder[scaleIndex];
      case BaseMapElement::Danger:
          return dangerBorder[scaleIndex];
      case BaseMapElement::LowFlight:
          return lowFBorder[scaleIndex];
      case BaseMapElement::Restricted:
          return restrBorder[scaleIndex];
      case BaseMapElement::TMZ:
          return tmzBorder[scaleIndex];
    }

  /* Should never happen ... */
  return true;
}

bool MapConfig::isPrintBorder(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Road:
          return roadBorder[printScaleIndex];
      case BaseMapElement::Highway:
          return highwayBorder[printScaleIndex];
      case BaseMapElement::Railway:
          return railBorder[printScaleIndex];
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return riverBorder[printScaleIndex];
      case BaseMapElement::City:
          return cityBorder[printScaleIndex];
    }
  /* Should never happen ... */
  return true;
}

QColor MapConfig::getIsoColor(unsigned int heightIndex)
{
  return *topographyColorList.at(heightIndex);
}

QBrush MapConfig::getPrintBrush(unsigned int typeID)
{
  return __getBrush(typeID, printScaleIndex);
}

QBrush MapConfig::getDrawBrush(unsigned int typeID)
{
  return __getBrush(typeID, scaleIndex);
}

QBrush MapConfig::__getBrush(unsigned int typeID, int sIndex)
{
  switch(typeID)
    {
      case BaseMapElement::City:
          return *cityBrushList.at(sIndex);
      case BaseMapElement::Lake:
          return QBrush(riverPenList.at(sIndex)->color(), Qt::SolidPattern);
      case BaseMapElement::AirC:
          return *airCBrushList.at(sIndex);
      case BaseMapElement::AirD:
          return *airDBrushList.at(sIndex);
      case BaseMapElement::AirElow:
          return *airElBrushList.at(sIndex);
      case BaseMapElement::AirEhigh:
          return *airEhBrushList.at(sIndex);
      case BaseMapElement::AirF:
          return *airFBrushList.at(sIndex);
      case BaseMapElement::ControlC:
          return *ctrCBrushList.at(sIndex);
      case BaseMapElement::ControlD:
          return *ctrDBrushList.at(sIndex);
      case BaseMapElement::Danger:
          return *dangerBrushList.at(sIndex);
      case BaseMapElement::LowFlight:
          return *lowFBrushList.at(sIndex);
      case BaseMapElement::Restricted:
          return *restrBrushList.at(sIndex);
      case BaseMapElement::TMZ:
          return *tmzBrushList.at(sIndex);
    }
  return QBrush();
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch)
{
  QString iconName(getPixmapName(typeID, isWinch));

  if(isSwitch)
      return QPixmap(KGlobal::dirs()->findResource("appdata",
          "mapicons/" + iconName));

  return QPixmap(KGlobal::dirs()->findResource("appdata",
      "mapicons/small/" + iconName));
}

QString MapConfig::getPixmapName(unsigned int typeID, bool isWinch)
{
  QString iconName;

  switch(typeID)
    {
      case BaseMapElement::BaseMapElement::Airport:
      case BaseMapElement::BaseMapElement::IntAirport:
        iconName = "airport.xpm";
        break;
      case BaseMapElement::MilAirport:
        iconName = "milairport.xpm";
        break;
      case BaseMapElement::CivMilAirport:
        iconName = "civmilair.xpm";
        break;
      case BaseMapElement::Airfield:
        iconName = "airfield_c.xpm";
        break;
      case BaseMapElement::ClosedAirfield:
        iconName = "closed.xpm";
        break;
      case BaseMapElement::CivHeliport:
        iconName = "civheliport.xpm";
        break;
      case BaseMapElement::MilHeliport:
        iconName = "milheliport.xpm";
        break;
      case BaseMapElement::AmbHeliport:
        iconName = "ambheliport.xpm";
        break;
      case BaseMapElement::Glidersite:
        if(isWinch)
            iconName = "glider.xpm";
        else
            iconName = "glider2.xpm";
        break;
      case BaseMapElement::UltraLight:
        iconName = "ul.xpm";
        break;
      case BaseMapElement::HangGlider:
        iconName = "paraglider.xpm";
        break;
      case BaseMapElement::Parachute:
        iconName = "jump.xpm";
        break;
      case BaseMapElement::Ballon:
        iconName = "ballon.xpm";
        break;
      case BaseMapElement::CompPoint:
        iconName = "compoint.xpm";
        break;
      case BaseMapElement::Landmark:
        iconName = "landmark.xpm";
        break;
      case BaseMapElement::VOR:
        iconName = "vor.xpm";
        break;
      case BaseMapElement::VORDME:
        iconName = "vordme.xpm";
        break;
      case BaseMapElement::VORTAC:
        iconName = "vortac.xpm";
        break;
      case BaseMapElement::NDB:
        iconName = "ndb.xpm";
        break;
      default:
        iconName = "";
        break;
    }

  return iconName;
}
