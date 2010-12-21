/***********************************************************************
**
**   mapconfig.cpp
**
**   This file is part of KFLog4.
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

#include <QtGui>

//Added by qt3to4:
#include <Q3PtrList>

#include "flight.h"
#include "mapdefaults.h"

extern QSettings _settings;

MapConfig::MapConfig( QObject* object ) :
  QObject(object),
  scaleIndex(0),
  printScaleIndex(0),
  isSwitch(false)
{
  qDebug() << "MapConfig()";

  airABorder = new bool[6];
  airBBorder = new bool[6];
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

  trailBorder = new bool[6];
  roadBorder = new bool[6];
  highwayBorder = new bool[6];
  railBorder = new bool[6];
  rail_dBorder = new bool[6];
  aerialcableBorder = new bool[6];
  riverBorder = new bool[6];
  river_tBorder = new bool[6];
  canalBorder = new bool[6];
  cityBorder = new bool[6];

  forestBorder = new bool[6];
  glacierBorder = new bool[6];
  packiceBorder = new bool[6];

  faiAreaLow500Border = new bool[6];
  faiAreaHigh500Border = new bool[6];
}

MapConfig::~MapConfig()
{
  qDebug() << "~MapConfig()";
}

void MapConfig::slotReadConfig()
{
  /* Unfortunately, a ~QList() doesn't work, so we must remove all
   * item manualy ;-(
   */
  while(topographyColorList.remove());

  while(airAPenList.remove());
  while(airABrushList.remove());
  while(airBPenList.remove());
  while(airBBrushList.remove());
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

  while(trailPenList.remove());
  while(roadPenList.remove());
  while(highwayPenList.remove());
  while(riverPenList.remove());
  while(river_tPenList.remove());
  while(river_tBrushList.remove());
  while(canalPenList.remove());
  while(railPenList.remove());
  while(rail_dPenList.remove());
  while(cityPenList.remove());
  while(cityBrushList.remove());
  while(forestPenList.remove());
  while(forestBrushList.remove());
  while(glacierPenList.remove());
  while(glacierBrushList.remove());
  while(packicePenList.remove());
  while(packiceBrushList.remove());
  while(faiAreaLow500PenList.remove());
  while(faiAreaLow500BrushList.remove());
  while(faiAreaHigh500PenList.remove());
  while(faiAreaHigh500BrushList.remove());

  __readTopo("SubTerrain", COLOR_LEVEL_SUB);
  __readTopo("0M",    COLOR_LEVEL_0);
  __readTopo("10M",   COLOR_LEVEL_10);
  __readTopo("25M",   COLOR_LEVEL_25);
  __readTopo("50M",   COLOR_LEVEL_50);
  __readTopo("75M",   COLOR_LEVEL_75);
  __readTopo("100M",  COLOR_LEVEL_100);
  __readTopo("150M",  COLOR_LEVEL_150);
  __readTopo("200M",  COLOR_LEVEL_200);
  __readTopo("250M",  COLOR_LEVEL_250);
  __readTopo("300M",  COLOR_LEVEL_300);
  __readTopo("350M",  COLOR_LEVEL_350);
  __readTopo("400M",  COLOR_LEVEL_400);
  __readTopo("450M",  COLOR_LEVEL_450);
  __readTopo("500M",  COLOR_LEVEL_500);
  __readTopo("600M",  COLOR_LEVEL_600);
  __readTopo("700M",  COLOR_LEVEL_700);
  __readTopo("800M",  COLOR_LEVEL_800);
  __readTopo("900M",  COLOR_LEVEL_900);
  __readTopo("1000M", COLOR_LEVEL_1000);
  __readTopo("1250M", COLOR_LEVEL_1250);
  __readTopo("1500M", COLOR_LEVEL_1500);
  __readTopo("1750M", COLOR_LEVEL_1750);
  __readTopo("2000M", COLOR_LEVEL_2000);
  __readTopo("2250M", COLOR_LEVEL_2250);
  __readTopo("2500M", COLOR_LEVEL_2500);
  __readTopo("2750M", COLOR_LEVEL_2750);
  __readTopo("3000M", COLOR_LEVEL_3000);
  __readTopo("3250M", COLOR_LEVEL_3250);
  __readTopo("3500M", COLOR_LEVEL_3500);
  __readTopo("3750M", COLOR_LEVEL_3750);
  __readTopo("4000M", COLOR_LEVEL_4000);
  __readTopo("4250M", COLOR_LEVEL_4250);
  __readTopo("4500M", COLOR_LEVEL_4500);
  __readTopo("4750M", COLOR_LEVEL_4750);
  __readTopo("5000M", COLOR_LEVEL_5000);
  __readTopo("5250M", COLOR_LEVEL_5250);
  __readTopo("5500M", COLOR_LEVEL_5500);
  __readTopo("5750M", COLOR_LEVEL_5750);
  __readTopo("6000M", COLOR_LEVEL_6000);
  __readTopo("6250M", COLOR_LEVEL_6250);
  __readTopo("6500M", COLOR_LEVEL_6500);
  __readTopo("6750M", COLOR_LEVEL_6750);
  __readTopo("7000M", COLOR_LEVEL_7000);
  __readTopo("7250M", COLOR_LEVEL_7250);
  __readTopo("7500M", COLOR_LEVEL_7500);
  __readTopo("7750M", COLOR_LEVEL_7750);
  __readTopo("8000M", COLOR_LEVEL_8000);
  __readTopo("8250M", COLOR_LEVEL_8250);
  __readTopo("8500M", COLOR_LEVEL_8500);
  __readTopo("8750M", COLOR_LEVEL_8750);


  __readPen("Road", &roadPenList, roadBorder,
        ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2);

  __readPen("Trail", &trailPenList, trailBorder,
        TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
        PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2,
        TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
        PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2,
        TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4,
        PRINT_TRAIL_PEN_STYLE_1, PRINT_TRAIL_PEN_STYLE_2);

  __readPen("River", &riverPenList, riverBorder,
        RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
        PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
        RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2);

  __readPen("Canal", &canalPenList, canalBorder,
        CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3, CANAL_COLOR_4,
        PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2,
        CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
        PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2,
        CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4,
        PRINT_CANAL_PEN_STYLE_1, PRINT_CANAL_PEN_STYLE_2);

  __readPen("Rail", &railPenList, railBorder,
        RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2);

  __readPen("Rail_D", &rail_dPenList, rail_dBorder,
        RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3, RAIL_D_COLOR_4,
        PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2,
        RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
        PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2,
        RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4,
        PRINT_RAIL_D_PEN_STYLE_1, PRINT_RAIL_D_PEN_STYLE_2);

  __readPen("Aerial_Cable", &aerialcablePenList, aerialcableBorder,
        AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2, AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
        PRINT_AERIAL_CABLE_COLOR_1, PRINT_AERIAL_CABLE_COLOR_2,
        AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
        PRINT_AERIAL_CABLE_PEN_1, PRINT_AERIAL_CABLE_PEN_2,
        AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2, AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4,
        PRINT_AERIAL_CABLE_PEN_STYLE_1, PRINT_AERIAL_CABLE_PEN_STYLE_2);


  __readPen("Highway", &highwayPenList, highwayBorder,
        HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2);

  __readPenBrush("City", &cityPenList, cityBorder, &cityBrushList,
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
        Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern);

  __readPenBrush("Forest", &forestPenList, forestBorder, &forestBrushList,
        FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
        PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2,
        FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
        PRINT_FRST_PEN_1, PRINT_FRST_PEN_2,
        FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3, FRST_PEN_STYLE_4,
        PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2,
        FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
        FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
        PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
        FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
        FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4,
        PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2);

  __readPenBrush("Glacier", &glacierPenList, glacierBorder, &glacierBrushList,
        GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
        PRINT_GLACIER_COLOR_1, PRINT_GLACIER_COLOR_2,
        GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
        PRINT_GLACIER_PEN_1, PRINT_GLACIER_PEN_2,
        GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3, GLACIER_PEN_STYLE_4,
        PRINT_GLACIER_PEN_STYLE_1, PRINT_GLACIER_PEN_STYLE_2,
        GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
        GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
        PRINT_GLACIER_BRUSH_COLOR_1, PRINT_GLACIER_BRUSH_COLOR_2,
        GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
        GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4,
        PRINT_GLACIER_BRUSH_STYLE_1, PRINT_GLACIER_BRUSH_STYLE_2);

  __readPenBrush("PackIce", &packicePenList, packiceBorder, &packiceBrushList,
        PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
        PRINT_PACK_ICE_COLOR_1, PRINT_PACK_ICE_COLOR_2,
        PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
        PRINT_PACK_ICE_PEN_1, PRINT_PACK_ICE_PEN_2,
        PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3, PACK_ICE_PEN_STYLE_4,
        PRINT_PACK_ICE_PEN_STYLE_1, PRINT_PACK_ICE_PEN_STYLE_2,
        PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
        PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
        PRINT_PACK_ICE_BRUSH_COLOR_1, PRINT_PACK_ICE_BRUSH_COLOR_2,
        PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
        PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4,
        PRINT_PACK_ICE_BRUSH_STYLE_1, PRINT_PACK_ICE_BRUSH_STYLE_2);

  __readPenBrush("River_T", &river_tPenList, river_tBorder, &river_tBrushList,
        RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
        PRINT_RIVER_T_COLOR_1, PRINT_RIVER_T_COLOR_2,
        RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
        PRINT_RIVER_T_PEN_1, PRINT_RIVER_T_PEN_2,
        RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4,
        PRINT_RIVER_T_PEN_STYLE_1, PRINT_RIVER_T_PEN_STYLE_2,
        RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
        RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
        PRINT_RIVER_T_BRUSH_COLOR_1, PRINT_RIVER_T_BRUSH_COLOR_2,
        RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
        RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4,
        PRINT_RIVER_T_BRUSH_STYLE_1, PRINT_RIVER_T_BRUSH_STYLE_2);

  __readPenBrush("Airspace A", &airAPenList, airABorder, &airABrushList,
        AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
        PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2,
        AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
        PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2,
        AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4,
        PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2,
        AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
        PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
        AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4,
        PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2);

  __readPenBrush("Airspace B", &airBPenList, airBBorder, &airBBrushList,
        AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
        PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2,
        AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
        PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2,
        AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4,
        PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2,
        AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
        PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
        AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4,
        PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2);

  __readPenBrush("Airspace C", &airCPenList, airCBorder, &airCBrushList,
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
        PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2);

  __readPenBrush("Airspace D", &airDPenList, airDBorder, &airDBrushList,
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
        PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2);

  __readPenBrush("Airspace E low", &airElPenList, airElBorder, &airElBrushList,
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
        PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2);

  __readPenBrush("Airspace E high", &airEhPenList, airEhBorder, &airEhBrushList,
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
        PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2);

  __readPenBrush("Airspace F", &airFPenList, airFBorder, &airFBrushList,
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
        PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2);

  __readPenBrush("Control C", &ctrCPenList, ctrCBorder, &ctrCBrushList,
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
        PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2);

  __readPenBrush("Control D", &ctrDPenList, ctrDBorder, &ctrDBrushList,
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
        PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2);

  __readPenBrush("Danger", &dangerPenList, dangerBorder, &dangerBrushList,
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
        PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2);

  __readPenBrush("Low Flight", &lowFPenList, lowFBorder, &lowFBrushList,
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
        PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2);

  __readPenBrush("Restricted Area", &restrPenList, restrBorder, &restrBrushList,
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
        PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2);

  __readPenBrush("TMZ", &tmzPenList, tmzBorder, &tmzBrushList,
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
        PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2);

    __readPenBrush("FAIAreaLow500", &faiAreaLow500PenList, faiAreaLow500Border, &faiAreaLow500BrushList,
        FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
        PRINT_FAI_LOW_500_COLOR_1, PRINT_FAI_LOW_500_COLOR_2,
        FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
        PRINT_FAI_LOW_500_PEN_1, PRINT_FAI_LOW_500_PEN_2,
        FAI_LOW_500_PEN_STYLE_1, FAI_LOW_500_PEN_STYLE_2, FAI_LOW_500_PEN_STYLE_3, FAI_LOW_500_PEN_STYLE_4,
        PRINT_FAI_LOW_500_PEN_STYLE_1, PRINT_FAI_LOW_500_PEN_STYLE_2,
        FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
        FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4,
        PRINT_FAI_LOW_500_BRUSH_COLOR_1, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
        FAI_LOW_500_BRUSH_STYLE_1, FAI_LOW_500_BRUSH_STYLE_2,
        FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4,
        PRINT_FAI_LOW_500_BRUSH_STYLE_1, PRINT_FAI_LOW_500_BRUSH_STYLE_2);

    __readPenBrush("FAIAreaHigh500", &faiAreaHigh500PenList, faiAreaHigh500Border, &faiAreaHigh500BrushList,
        FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
        PRINT_FAI_HIGH_500_COLOR_1, PRINT_FAI_HIGH_500_COLOR_2,
        FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
        PRINT_FAI_HIGH_500_PEN_1, PRINT_FAI_HIGH_500_PEN_2,
        FAI_HIGH_500_PEN_STYLE_1, FAI_HIGH_500_PEN_STYLE_2, FAI_HIGH_500_PEN_STYLE_3, FAI_HIGH_500_PEN_STYLE_4,
        PRINT_FAI_HIGH_500_PEN_STYLE_1, PRINT_FAI_HIGH_500_PEN_STYLE_2,
        FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
        FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4,
        PRINT_FAI_HIGH_500_BRUSH_COLOR_1, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
        FAI_HIGH_500_BRUSH_STYLE_1, FAI_HIGH_500_BRUSH_STYLE_2,
        FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4,
        PRINT_FAI_HIGH_500_BRUSH_STYLE_1, PRINT_FAI_HIGH_500_BRUSH_STYLE_2);

    drawFType = _settings.readNumEntry("/Flight/DrawType", MapConfig::Speed);

    _drawWpLabelScale = _settings.readNumEntry("/Scale/WaypointLabel", WPLABEL);

  emit configChanged();
}

void MapConfig::slotSetFlightDataType(int type)
{
  drawFType = type;
  _settings.setValue("/Flight/DrawType", type);
}

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

QPen MapConfig::getDrawPen(flightPoint* fP, float va_min/*=-10*/, float va_max/*=10*/, int altitude_max/*= 5000*/, float speed_max/*=80*/)
{
  //
  // Dynamische Farben im Flug:
  //
  //   Farben als Legende ausgeben ???
  //
  //   Farbwerte müssen noch eingestellt werden. Konfigurierbar???
  //   I would prefer colors adjusted for each flights histogram.
  //

  extern QSettings _settings;
  int width = _settings.readNumEntry("/Flight/flightPathWidth", 4);
  int red = 0, green = 0, blue = 0;
  float vario_range;
  QColor color;

  switch(drawFType)
    {
      case MapConfig::Vario:
        if(abs(va_min)>va_max)
          vario_range = 2*abs(va_min);
        else
          vario_range = 2*va_max;
        if(vario_range>10.0) //filter high vario values, probably due to a wrong GPS-fix
          vario_range = 10.0;
        color = getRainbowColor(0.5-(fP->dH/fP->dT)/vario_range);
        break;

      case MapConfig::Speed:
        speed_max -= 15;
        color = getRainbowColor(1-(fP->dS/std::max(1, fP->dT)-15)/speed_max);
        break;

      case MapConfig::Altitude:
        color = getRainbowColor((float)fP->height/altitude_max);
        break;

      case MapConfig::Cycling:
        switch(fP->f_state)
          {
            case Flight::LeftTurn:
              red = 255;
              green = 50;
              blue = 0;
              color = __string2Color(_settings.readEntry("/Flight/ColorLeftTurn", __color2String(QColor(red, green, blue))));
              break;
            case Flight::RightTurn:
              red = 50;
              green = 255;
              blue = 0;
              color = __string2Color(_settings.readEntry("/Flight/ColorRightTurn", __color2String(QColor(red, green, blue))));
              break;
            case Flight::MixedTurn:
              red = 200;
              green = 0;
              blue = 200;
              color = __string2Color(_settings.readEntry("/Flight/ColorMixedTurn", __color2String(QColor(red, green, blue))));
              break;
            case Flight::Straight:
            default:
              red = 0;
              green = 50;
              blue = 255;
              color = __string2Color(_settings.readEntry("/Flight/ColorStraight", __color2String(QColor(red, green, blue))));
              break;
          }
        break;

      case MapConfig::Solid:
      default:
        red = 0;
        green = 100;
        blue = 200;
        color = __string2Color(_settings.readEntry("/Flight/ColorSolid", __color2String(QColor(red, green, blue))));
        break;
    }

  // Simple aproach to see "engine was running"
  if ( fP->engineNoise > 350 ) {
    //  Put a white (or configured color) strip there in every case
    red = 255;
    green = 255;
    blue = 255;
    color = __string2Color(_settings.readEntry("/Flight/ColorEngineNoise", __color2String(QColor(red, green, blue))));
  }


  return QPen(color, width);
}

QColor MapConfig::getRainbowColor(float c)
{
  int red, green, blue;

  if(c<0.0)
  { //dark red
    red     = 100;
    green   = 0;
    blue    = 0;
  }
  else if(c<(1.0/6.0))
  { //dark red -> red
    red     = (int)(100+c*6*155);
    green   = 0;
    blue    = 0;
  }
  else if(c<(2.0/6.0))
  { //red -> yellow
    red     = 255;
    green   = (int)((c-1.0/6.0)*6*255);
    blue    = 0;
  }
  else if(c<(3.0/6.0))
  { //yellow -> green
    red     = (int)(255-(c-2.0/6.0)*6*255);
    green   = 255;
    blue    = 0;
  }
  else if(c<(4.0/6.0))
  { //green -> cyan
    red     = 0;
    green   = 255;
    blue    = (int)((c-3.0/6.0)*6*255);
  }
  else if(c<(5.0/6.0))
  { //cyan -> blue
    red     = 0;
    green   = (int)(255-(c-4.0/6.0)*6*255);
    blue    = 255;
  }
  else if(c<1.0)
  { //blue -> dark blue
    red     = 0;
    green   = 0;
    blue    = (int)(255-(c-5.0/6.0)*6*155);
  }
  else
  { //dark blue
    red     = 0;
    green   = 0;
    blue    = 100;
  }

  return QColor(red, green, blue);
}

QPen MapConfig::getDrawPen(unsigned int typeID)
{
  return __getPen(typeID, scaleIndex);
}

QPen MapConfig::__getPen(unsigned int typeID, int sIndex)
{
  switch(typeID)
    {
      case BaseMapElement::Trail:
          return *trailPenList.at(sIndex);
      case BaseMapElement::Road:
          return *roadPenList.at(sIndex);
      case BaseMapElement::Highway:
          return *highwayPenList.at(sIndex);
      case BaseMapElement::Railway:
          return *railPenList.at(sIndex);
      case BaseMapElement::Railway_D:
          return *rail_dPenList.at(sIndex);
      case BaseMapElement::Aerial_Cable:
          return *aerialcablePenList.at(sIndex);
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return *riverPenList.at(sIndex);
      case BaseMapElement::River_T:
      case BaseMapElement::Lake_T:
          return *river_tPenList.at(sIndex);
      case BaseMapElement::Canal:
          return *canalPenList.at(sIndex);
      case BaseMapElement::City:
          return *cityPenList.at(sIndex);
      case BaseMapElement::AirA:
          return *airAPenList.at(sIndex);
      case BaseMapElement::AirB:
          return *airBPenList.at(sIndex);
      case BaseMapElement::AirC:
          return *airCPenList.at(sIndex);
      case BaseMapElement::AirD:
          return *airDPenList.at(sIndex);
      case BaseMapElement::AirElow:
          return *airElPenList.at(sIndex);
      case BaseMapElement::AirE:
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
      case BaseMapElement::Tmz:
          return *tmzPenList.at(sIndex);
      case BaseMapElement::Forest:
          return *forestPenList.at(sIndex);
      case BaseMapElement::Glacier:
          return *glacierPenList.at(sIndex);
      case BaseMapElement::PackIce:
          return *packicePenList.at(sIndex);
      case BaseMapElement::FAIAreaLow500:
          return *faiAreaLow500PenList.at(sIndex);
      case BaseMapElement::FAIAreaHigh500:
          return *faiAreaHigh500PenList.at(sIndex);
      default:
          return *roadPenList.at(sIndex);
    }
}

bool MapConfig::isBorder(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Trail:
          return trailBorder[scaleIndex];
      case BaseMapElement::Road:
          return roadBorder[scaleIndex];
      case BaseMapElement::Highway:
          return highwayBorder[scaleIndex];
      case BaseMapElement::Railway:
          return railBorder[scaleIndex];
      case BaseMapElement::Railway_D:
          return rail_dBorder[scaleIndex];
      case BaseMapElement::Aerial_Cable:
          return aerialcableBorder[scaleIndex];

      case BaseMapElement::Canal:
          return canalBorder[scaleIndex];
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return riverBorder[scaleIndex];
      case BaseMapElement::River_T:
      case BaseMapElement::Lake_T:
          return river_tBorder[scaleIndex];

      case BaseMapElement::City:
          return cityBorder[scaleIndex];
      case BaseMapElement::AirA:
          return airABorder[scaleIndex];
      case BaseMapElement::AirB:
          return airBBorder[scaleIndex];
      case BaseMapElement::AirC:
          return airCBorder[scaleIndex];
      case BaseMapElement::AirD:
          return airDBorder[scaleIndex];
      case BaseMapElement::AirElow:
          return airElBorder[scaleIndex];
      case BaseMapElement::AirE:
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
      case BaseMapElement::Tmz:
          return tmzBorder[scaleIndex];
      case BaseMapElement::Forest:
          return forestBorder[scaleIndex];
      case BaseMapElement::Glacier:
          return glacierBorder[scaleIndex];
      case BaseMapElement::PackIce:
          return packiceBorder[scaleIndex];
      case BaseMapElement::FAIAreaLow500:
          return faiAreaLow500Border[scaleIndex];
      case BaseMapElement::FAIAreaHigh500:
          return faiAreaHigh500Border[scaleIndex];
    }

  /* Should never happen ... */
  return true;
}

bool MapConfig::isPrintBorder(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Trail:
          return trailBorder[printScaleIndex];
      case BaseMapElement::Road:
          return roadBorder[printScaleIndex];
      case BaseMapElement::Highway:
          return highwayBorder[printScaleIndex];
      case BaseMapElement::Railway:
          return railBorder[printScaleIndex];
      case BaseMapElement::Railway_D:
          return rail_dBorder[printScaleIndex];
      case BaseMapElement::Aerial_Cable:
          return aerialcableBorder[printScaleIndex];
      case BaseMapElement::Canal:
          return canalBorder[printScaleIndex];


      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return riverBorder[printScaleIndex];
      case BaseMapElement::River_T:
      case BaseMapElement::Lake_T:
          return river_tBorder[printScaleIndex];

      case BaseMapElement::City:
          return cityBorder[printScaleIndex];
      case BaseMapElement::Forest:
          return forestBorder[printScaleIndex];
      case BaseMapElement::Glacier:
          return glacierBorder[printScaleIndex];
      case BaseMapElement::PackIce:
          return packiceBorder[printScaleIndex];
      case BaseMapElement::FAIAreaLow500:
          return faiAreaLow500Border[printScaleIndex];
      case BaseMapElement::FAIAreaHigh500:
          return faiAreaHigh500Border[printScaleIndex];

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
      case BaseMapElement::Lake_T:
          return *river_tBrushList.at(sIndex);
      case BaseMapElement::AirA:
          return *airABrushList.at(sIndex);
      case BaseMapElement::AirB:
          return *airBBrushList.at(sIndex);
      case BaseMapElement::AirC:
          return *airCBrushList.at(sIndex);
      case BaseMapElement::AirD:
          return *airDBrushList.at(sIndex);
      case BaseMapElement::AirElow:
          return *airElBrushList.at(sIndex);
      case BaseMapElement::AirE:
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
      case BaseMapElement::Tmz:
          return *tmzBrushList.at(sIndex);
      case BaseMapElement::Forest:
          return *forestBrushList.at(sIndex);
      case BaseMapElement::Glacier:
          return *glacierBrushList.at(sIndex);
      case BaseMapElement::PackIce:
          return *packiceBrushList.at(sIndex);
      case BaseMapElement::FAIAreaLow500:
          return *faiAreaLow500BrushList.at(sIndex);
      case BaseMapElement::FAIAreaHigh500:
          return *faiAreaHigh500BrushList.at(sIndex);

    }
  return QBrush();
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch)
{
  QString iconName(getPixmapName(typeID, isWinch));

  if(isSwitch)
      return QPixmap(QDir::homePath() + "/.kflog/mapicons/" + iconName);

  return QPixmap(QDir::homePath() + "/.kflog/mapicons/small/" + iconName);
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch, bool smallIcon)
{
  QString iconName(getPixmapName(typeID, isWinch));

  if(smallIcon)
      return QPixmap(QDir::homePath() + "/.kflog/mapicons/small/" + iconName);

  return QPixmap(QDir::homePath() + "/.kflog/mapicons/" + iconName);
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
            iconName = "glider.xpm";     // winch, red icon
        else
            iconName = "glider2.xpm";   // aerotow, black icon
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
      case BaseMapElement::Balloon:
        iconName = "balloon.xpm";
        break;
      case BaseMapElement::CompPoint:
        iconName = "compoint.xpm";
        break;
      case BaseMapElement::Landmark:
        iconName = "landmark.xpm";
        break;
      case BaseMapElement::Vor:
        iconName = "vor.xpm";
        break;
      case BaseMapElement::VorDme:
        iconName = "vordme.xpm";
        break;
      case BaseMapElement::VorTac:
        iconName = "vortac.xpm";
        break;
      case BaseMapElement::Ndb:
        iconName = "ndb.xpm";
        break;
      case BaseMapElement::Outlanding:
        iconName = "outlanding.xpm";
        break;
      case BaseMapElement::Obstacle:
        iconName = "obstacle.xpm";
        break;
      case BaseMapElement::LightObstacle:
        iconName = "obst_light.xpm";
        break;
      case BaseMapElement::ObstacleGroup:
        iconName = "obst_group.xpm";
        break;
      case BaseMapElement::LightObstacleGroup:
        iconName = "obst_group_light.xpm";
        break;
      default:
        iconName = "";
        break;
    }

  return iconName;
}

/** Returns true if small icons are used, else returns false. */
bool MapConfig::useSmallIcons(){
  return !isSwitch;
}

/** Returns true if small icons are used, else returns false. */
bool MapConfig::drawWpLabels(){
  extern MapMatrix *_globalMapMatrix;
  return (_globalMapMatrix->getScale(MapMatrix::CurrentScale)<=_drawWpLabelScale);
}

Qt::PenStyle MapConfig::getIsoPenStyle(int height)
{
  // choose isoline style
  extern MapMatrix *_globalMapMatrix;
  Qt::PenStyle style=Qt::NoPen;

  if (_globalMapMatrix->getScale()<100.0)   // make configurable
  {
    if (height%1000==0)
      style=Qt::SolidLine;
    else if (height%500==0)
      style=Qt::DashLine;
    else if (height%250==0)
      style=Qt::DotLine;
    else if (height%100==0)
      style=Qt::DashDotLine;
    else if (height%50==0)
      style=Qt::DashDotDotLine;
  }
  return style;
}

void MapConfig::__readBorder(QString group, bool *b)
{
  b[0] = _settings.readBoolEntry("/Map/"+group+"/Border1", true);
  b[1] = _settings.readBoolEntry("/Map/"+group+"/Border2", true);
  b[2] = _settings.readBoolEntry("/Map/"+group+"/Border3", true);
  b[3] = _settings.readBoolEntry("/Map/"+group+"/Border4", true);
  b[4] = _settings.readBoolEntry("/Map/"+group+"/PrintBorder1", true);
  b[5] = _settings.readBoolEntry("/Map/"+group+"/PrintBorder2", true);
}

void MapConfig::__readPen(QString group, Q3PtrList<QPen> *penList, bool *b,
    QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4, QColor defaultColor5, QColor defaultColor6,
    int defaultPenSize1, int defaultPenSize2, int /*defaultPenSize3*/, int /*defaultPenSize4*/, int defaultPenSize5, int defaultPenSize6,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4, Qt::PenStyle defaultPenStyle5, Qt::PenStyle defaultPenStyle6)
{
  __readBorder(group, b);
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/Color1", __color2String(defaultColor1))),
        _settings.readNumEntry("/Map/"+group+"/PenSize1", defaultPenSize1),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PenStyle1", defaultPenStyle1)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/Color2", __color2String(defaultColor2))),
        _settings.readNumEntry("/Map/"+group+"/PenSize2", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PenStyle2", defaultPenStyle2)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/Color3", __color2String(defaultColor3))),
        _settings.readNumEntry("/Map/"+group+"/PenSize3", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PenStyle3", defaultPenStyle3)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/Color4", __color2String(defaultColor4))),
        _settings.readNumEntry("/Map/"+group+"/PenSize4", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PenStyle4", defaultPenStyle4)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/PrintColor1", __color2String(defaultColor5))),
        _settings.readNumEntry("/Map/"+group+"/PrintPenSize1", defaultPenSize5),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PrintPenStyle1", defaultPenStyle5)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/PrintColor2", __color2String(defaultColor6))),
        _settings.readNumEntry("/Map/"+group+"/PrintPenSize2", defaultPenSize6),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PrintPenStyle2", defaultPenStyle6)));
}

void MapConfig::__readPenBrush(QString group, Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
    QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4, QColor defaultColor5, QColor defaultColor6,
    int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4, int defaultPenSize5, int defaultPenSize6,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4, Qt::PenStyle defaultPenStyle5, Qt::PenStyle defaultPenStyle6,
    QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4, QColor defaultBrushColor5, QColor defaultBrushColor6,
    Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4, Qt::BrushStyle defaultBrushStyle5, Qt::BrushStyle defaultBrushStyle6)
{
  __readPen(group, penList, b,
      defaultColor1, defaultColor2, defaultColor3, defaultColor4, defaultColor5, defaultColor6,
      defaultPenSize1, defaultPenSize2, defaultPenSize3, defaultPenSize4, defaultPenSize5, defaultPenSize6,
      defaultPenStyle1, defaultPenStyle2, defaultPenStyle3, defaultPenStyle4, defaultPenStyle5, defaultPenStyle6);
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/BrushColor1", __color2String(defaultBrushColor1))), \
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/BrushStyle1", defaultBrushStyle1))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/BrushColor2", __color2String(defaultBrushColor2))), \
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/BrushStyle2", defaultBrushStyle2))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/BrushColor3", __color2String(defaultBrushColor3))), \
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/BrushStyle3", defaultBrushStyle3))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/BrushColor4", __color2String(defaultBrushColor4))), \
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/BrushStyle4", defaultBrushStyle4)));
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/PrintBrushColor1", __color2String(defaultBrushColor5))),
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/PrintBrushStyle1", defaultBrushStyle5)));
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/PrintBrushColor2", __color2String(defaultBrushColor6))),
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/PrintBrushStyle2", defaultBrushStyle6)));
}

void MapConfig::__readTopo(QString entry, QColor color)
{
  topographyColorList.append(new QColor(__string2Color(_settings.readEntry("/Map/Topography/" + entry, __color2String(color)))));
}

/** this is a temporary function and it is not needed in Qt 4 */
QString MapConfig::__color2String(QColor color)
{
  QString colstr;
  colstr.sprintf("%d;%d;%d", color.red(), color.green(), color.blue());
  return colstr;
}

/** this is a temporary function and it is not needed in Qt 4 */
QColor MapConfig::__string2Color(QString colstr)
{
  QColor color(colstr.section(";", 0, 0).toInt(), colstr.section(";", 1, 1).toInt(), colstr.section(";", 2, 2).toInt());
  return color;
}

