/***********************************************************************
**
**   mapconfig.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtGui>

#include "flight.h"
#include "mapdefaults.h"

extern QSettings _settings;

MapConfig::MapConfig( QObject* object ) :
  QObject(object),
  scaleIndex(0),
  printScaleIndex(0),
  isSwitch(false),
  _drawWpLabelScale(WPLABEL)
{
  defaultOpacity[0] = AS_OPACITY_1;
  defaultOpacity[1] = AS_OPACITY_2;
  defaultOpacity[2] = AS_OPACITY_3;
  defaultOpacity[3] = AS_OPACITY_4;
}

MapConfig::~MapConfig()
{
}

void MapConfig::slotReadConfig()
{
  topographyColorList.clear();

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

  __readPen("Road", roadPenList, roadBorder,
        ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2);

  __readPen("Trail", trailPenList, trailBorder,
        TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
        PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2,
        TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
        PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2,
        TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4,
        PRINT_TRAIL_PEN_STYLE_1, PRINT_TRAIL_PEN_STYLE_2);

  __readPen("Canal", canalPenList, canalBorder,
        CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3, CANAL_COLOR_4,
        PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2,
        CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
        PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2,
        CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4,
        PRINT_CANAL_PEN_STYLE_1, PRINT_CANAL_PEN_STYLE_2);

  __readPen("Rail", railPenList, railBorder,
        RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2);

  __readPen("Rail_D", rail_dPenList, rail_dBorder,
        RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3, RAIL_D_COLOR_4,
        PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2,
        RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
        PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2,
        RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4,
        PRINT_RAIL_D_PEN_STYLE_1, PRINT_RAIL_D_PEN_STYLE_2);

  __readPen("Aerial_Cable", aerialcablePenList, aerialcableBorder,
        AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2, AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
        PRINT_AERIAL_CABLE_COLOR_1, PRINT_AERIAL_CABLE_COLOR_2,
        AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
        PRINT_AERIAL_CABLE_PEN_1, PRINT_AERIAL_CABLE_PEN_2,
        AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2, AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4,
        PRINT_AERIAL_CABLE_PEN_STYLE_1, PRINT_AERIAL_CABLE_PEN_STYLE_2);

  __readPen("Highway", highwayPenList, highwayBorder,
        HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2);

  __readPenBrush("City", cityPenList, cityBorder, cityBrushList,
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

  __readPenBrush("Forest", forestPenList, forestBorder, forestBrushList,
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

  __readPenBrush("Glacier", glacierPenList, glacierBorder, glacierBrushList,
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

  __readPenBrush("PackIce", packicePenList, packiceBorder, packiceBrushList,
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

  __readPenBrush("River", riverPenList, riverBorder, riverBrushList,
        RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3, RIVER_COLOR_4,
        PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
        RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2,
        RIVER_BRUSH_COLOR_1, RIVER_BRUSH_COLOR_2,
        RIVER_BRUSH_COLOR_3, RIVER_BRUSH_COLOR_4,
        PRINT_RIVER_BRUSH_COLOR_1, PRINT_RIVER_BRUSH_COLOR_2,
        RIVER_BRUSH_STYLE_1, RIVER_BRUSH_STYLE_2,
        RIVER_BRUSH_STYLE_3, RIVER_BRUSH_STYLE_4,
        PRINT_RIVER_BRUSH_STYLE_1, PRINT_RIVER_BRUSH_STYLE_2);

  __readPenBrush("River_T", river_tPenList, river_tBorder, river_tBrushList,
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

  __readPenBrush(AS_A, airAPenList, airABorder, airABrushList,
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
  __readAsOpacity(AS_A, airAOpacityList);

  __readPenBrush(AS_B, airBPenList, airBBorder, airBBrushList,
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
  __readAsOpacity(AS_B, airBOpacityList);

  __readPenBrush(AS_C, airCPenList, airCBorder, airCBrushList,
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
  __readAsOpacity(AS_C, airCOpacityList);

  __readPenBrush(AS_D, airDPenList, airDBorder, airDBrushList,
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
  __readAsOpacity(AS_D, airDOpacityList);

  __readPenBrush(AS_EL, airElPenList, airElBorder, airElBrushList,
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
  __readAsOpacity(AS_EL, airElOpacityList);

  __readPenBrush(AS_E, airEPenList, airEBorder, airEBrushList,
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
  __readAsOpacity(AS_E, airEOpacityList);

  __readPenBrush(AS_F, airFPenList, airFBorder, airFBrushList,
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
  __readAsOpacity(AS_F, airFOpacityList);

  __readPenBrush(AS_FIR, airFirPenList, airFirBorder, airFirBrushList,
        AIRFIR_COLOR_1, AIRFIR_COLOR_2, AIRFIR_COLOR_3, AIRFIR_COLOR_4,
        PRINT_AIRFIR_COLOR_1, PRINT_AIRFIR_COLOR_2,
        AIRFIR_PEN_1, AIRFIR_PEN_2, AIRFIR_PEN_3, AIRFIR_PEN_4,
        PRINT_AIRFIR_PEN_1, PRINT_AIRFIR_PEN_2,
        AIRFIR_PEN_STYLE_1, AIRFIR_PEN_STYLE_2, AIRFIR_PEN_STYLE_3, AIRFIR_PEN_STYLE_4,
        PRINT_AIRFIR_PEN_STYLE_1, PRINT_AIRFIR_PEN_STYLE_2,
        AIRFIR_BRUSH_COLOR_1, AIRFIR_BRUSH_COLOR_2,
        AIRFIR_BRUSH_COLOR_3, AIRFIR_BRUSH_COLOR_4,
        PRINT_AIRFIR_BRUSH_COLOR_1, PRINT_AIRFIR_BRUSH_COLOR_2,
        AIRFIR_BRUSH_STYLE_1, AIRFIR_BRUSH_STYLE_2,
        AIRFIR_BRUSH_STYLE_3, AIRFIR_BRUSH_STYLE_4,
        PRINT_AIRFIR_BRUSH_STYLE_1, PRINT_AIRFIR_BRUSH_STYLE_2);
  __readAsOpacity(AS_FIR, airFirOpacityList);

  __readPenBrush(AS_CTR, ctrPenList, ctrBorder, ctrBrushList,
        CTR_COLOR_1, CTR_COLOR_2, CTR_COLOR_3, CTR_COLOR_4,
        PRINT_CTR_COLOR_1, PRINT_CTR_COLOR_2,
        CTR_PEN_1, CTR_PEN_2, CTR_PEN_3, CTR_PEN_4,
        PRINT_CTR_PEN_1, PRINT_CTR_PEN_2,
        CTR_PEN_STYLE_1, CTR_PEN_STYLE_2, CTR_PEN_STYLE_3, CTR_PEN_STYLE_4,
        PRINT_CTR_PEN_STYLE_1, PRINT_CTR_PEN_STYLE_2,
        CTR_BRUSH_COLOR_1, CTR_BRUSH_COLOR_2,
        CTR_BRUSH_COLOR_3, CTR_BRUSH_COLOR_4,
        PRINT_CTR_BRUSH_COLOR_1, PRINT_CTR_BRUSH_COLOR_2,
        CTR_BRUSH_STYLE_1, CTR_BRUSH_STYLE_2,
        CTR_BRUSH_STYLE_3, CTR_BRUSH_STYLE_4,
        PRINT_CTR_BRUSH_STYLE_1, PRINT_CTR_BRUSH_STYLE_2);
  __readAsOpacity(AS_CTR, ctrOpacityList);

  __readPenBrush(AS_DANGER, dangerPenList, dangerBorder, dangerBrushList,
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
  __readAsOpacity(AS_DANGER, dangerOpacityList);

  __readPenBrush(AS_LF, lowFPenList, lowFBorder, lowFBrushList,
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
  __readAsOpacity(AS_LF, lowFOpacityList);

  __readPenBrush(AS_RESTRICTED, restrPenList, restrBorder, restrBrushList,
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
  __readAsOpacity(AS_RESTRICTED, restrOpacityList);

  __readPenBrush(AS_RMZ, rmzPenList, rmzBorder, rmzBrushList,
        RMZ_COLOR_1, RMZ_COLOR_2, RMZ_COLOR_3, RMZ_COLOR_4,
        PRINT_RMZ_COLOR_1, PRINT_RMZ_COLOR_2,
        RMZ_PEN_1, RMZ_PEN_2, RMZ_PEN_3, RMZ_PEN_4,
        PRINT_RMZ_PEN_1, PRINT_RMZ_PEN_2,
        RMZ_PEN_STYLE_1, RMZ_PEN_STYLE_2, RMZ_PEN_STYLE_3, RMZ_PEN_STYLE_4,
        PRINT_RMZ_PEN_STYLE_1, PRINT_RMZ_PEN_STYLE_2,
        RMZ_BRUSH_COLOR_1, RMZ_BRUSH_COLOR_2,
        RMZ_BRUSH_COLOR_3, RMZ_BRUSH_COLOR_4,
        PRINT_RMZ_BRUSH_COLOR_1, PRINT_RMZ_BRUSH_COLOR_2,
        RMZ_BRUSH_STYLE_1, RMZ_BRUSH_STYLE_2,
        RMZ_BRUSH_STYLE_3, RMZ_BRUSH_STYLE_4,
        PRINT_RMZ_BRUSH_STYLE_1, PRINT_RMZ_BRUSH_STYLE_2);
  __readAsOpacity(AS_RMZ, rmzOpacityList);

  __readPenBrush(AS_TMZ, tmzPenList, tmzBorder, tmzBrushList,
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
  __readAsOpacity(AS_TMZ, tmzOpacityList);

  __readPenBrush(AS_GS, gsPenList, gsBorder, gsBrushList,
        GLIDER_SECTOR_COLOR_1, GLIDER_SECTOR_COLOR_2, GLIDER_SECTOR_COLOR_3, GLIDER_SECTOR_COLOR_4,
        PRINT_GLIDER_SECTOR_COLOR_1, PRINT_GLIDER_SECTOR_COLOR_2,
        GLIDER_SECTOR_PEN_1, GLIDER_SECTOR_PEN_2, GLIDER_SECTOR_PEN_3, GLIDER_SECTOR_PEN_4,
        PRINT_GLIDER_SECTOR_PEN_1, PRINT_GLIDER_SECTOR_PEN_2,
        GLIDER_SECTOR_PEN_STYLE_1, GLIDER_SECTOR_PEN_STYLE_2, GLIDER_SECTOR_PEN_STYLE_3, GLIDER_SECTOR_PEN_STYLE_4,
        PRINT_GLIDER_SECTOR_PEN_STYLE_1, PRINT_GLIDER_SECTOR_PEN_STYLE_2,
        GLIDER_SECTOR_BRUSH_COLOR_1, GLIDER_SECTOR_BRUSH_COLOR_2,
        GLIDER_SECTOR_BRUSH_COLOR_3, GLIDER_SECTOR_BRUSH_COLOR_4,
        PRINT_GLIDER_SECTOR_BRUSH_COLOR_1, PRINT_GLIDER_SECTOR_BRUSH_COLOR_2,
        GLIDER_SECTOR_BRUSH_STYLE_1, GLIDER_SECTOR_BRUSH_STYLE_2,
        GLIDER_SECTOR_BRUSH_STYLE_3, GLIDER_SECTOR_BRUSH_STYLE_4,
        PRINT_GLIDER_SECTOR_BRUSH_STYLE_1, PRINT_GLIDER_SECTOR_BRUSH_STYLE_2);
  __readAsOpacity(AS_GS, gsOpacityList);

  __readPenBrush(AS_WW, wwPenList, wwBorder, wwBrushList,
        WAVE_WINDOW_COLOR_1, WAVE_WINDOW_COLOR_2, WAVE_WINDOW_COLOR_3, WAVE_WINDOW_COLOR_4,
        PRINT_WAVE_WINDOW_COLOR_1, PRINT_WAVE_WINDOW_COLOR_2,
        WAVE_WINDOW_PEN_1, WAVE_WINDOW_PEN_2, WAVE_WINDOW_PEN_3, WAVE_WINDOW_PEN_4,
        PRINT_WAVE_WINDOW_PEN_1, PRINT_WAVE_WINDOW_PEN_2,
        WAVE_WINDOW_PEN_STYLE_1, WAVE_WINDOW_PEN_STYLE_2, WAVE_WINDOW_PEN_STYLE_3, WAVE_WINDOW_PEN_STYLE_4,
        PRINT_WAVE_WINDOW_PEN_STYLE_1, PRINT_WAVE_WINDOW_PEN_STYLE_2,
        WAVE_WINDOW_BRUSH_COLOR_1, WAVE_WINDOW_BRUSH_COLOR_2,
        WAVE_WINDOW_BRUSH_COLOR_3, WAVE_WINDOW_BRUSH_COLOR_4,
        PRINT_WAVE_WINDOW_BRUSH_COLOR_1, PRINT_WAVE_WINDOW_BRUSH_COLOR_2,
        WAVE_WINDOW_BRUSH_STYLE_1, WAVE_WINDOW_BRUSH_STYLE_2,
        WAVE_WINDOW_BRUSH_STYLE_3, WAVE_WINDOW_BRUSH_STYLE_4,
        PRINT_WAVE_WINDOW_BRUSH_STYLE_1, PRINT_WAVE_WINDOW_BRUSH_STYLE_2);
  __readAsOpacity(AS_WW, wwOpacityList);


  __readPenBrush("FAIAreaLow500", faiAreaLow500PenList, faiAreaLow500Border, faiAreaLow500BrushList,
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

  __readPenBrush("FAIAreaHigh500", faiAreaHigh500PenList, faiAreaHigh500Border, faiAreaHigh500BrushList,
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

  _drawWpLabelScale = _settings.value("/Scale/WaypointLabel", WPLABEL).toInt();

  emit configChanged();
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

QPen& MapConfig::getPrintPen(unsigned int typeID)
{
  return __getPen(typeID, printScaleIndex);
}

QPen MapConfig::getDrawPen( FlightPoint* fP,
                            float va_min/*=-10*/,
                            float va_max/*=10*/,
                            int altitude_max/*= 5000*/,
                            float speed_max/*=80*/,
                            enum MapConfig::DrawFlightPointType dfpt )
{
  //
  // Dynamische Farben im Flug:
  //
  //   Farben als Legende ausgeben ???
  //
  //   I would prefer colors adjusted for each flights histogram.
  //
  // Colors are configurable in KFLogConfig.
  int width = 4;
  float vario_range;
  QColor color;

  switch( dfpt )
    {
      case MapConfig::Vario:

        if( abs( va_min ) > va_max )
          {
            vario_range = 2 * abs( va_min );
          }
        else
          {
            vario_range = 2 * va_max;
          }

        if(vario_range>10.0) //filter high vario values, probably due to a wrong GPS-fix
          {
            vario_range = 10.0;
          }

        color = getRainbowColor( 0.5 - (fP->dH / fP->dT) / vario_range );
        width = _settings.value( "/FlightPathLine/Vario", FlightPathLineWidth ).toInt();
        break;

      case MapConfig::Speed:
        speed_max -= 15;
        color = getRainbowColor(1-(fP->dS/qMax(1, fP->dT)-15)/speed_max);
        width = _settings.value("/FlightPathLine/Speed", FlightPathLineWidth).toInt();
        break;

      case MapConfig::Altitude:
        color = getRainbowColor((float)fP->height/altitude_max);
        width = _settings.value("/FlightPathLine/Altitude", FlightPathLineWidth).toInt();
        break;

      case MapConfig::Cycling:

        width = _settings.value("/FlightPathLine/Cycling", FlightPathLineWidth).toInt();

        switch(fP->f_state)
          {
            case Flight::LeftTurn:
              color = _settings.value( "/FlightColor/LeftTurn", FlightTypeLeftTurnColor.name() ).value<QColor>();
              break;

            case Flight::RightTurn:
              color = _settings.value( "/FlightColor/RightTurn", FlightTypeRightTurnColor.name() ).value<QColor>();
              break;

            case Flight::MixedTurn:
              color = _settings.value( "/FlightColor/MixedTurn", FlightTypeMixedTurnColor.name() ).value<QColor>();
              break;

            case Flight::Straight:
            default:
              color = _settings.value( "/FlightColor/Straight", FlightTypeStraightColor.name() ).value<QColor>();
              break;
          }
        break;

      case MapConfig::Airspace:
          {
            if( fP->isAirspaceIntersected == true )
              {
                color = Qt::magenta;
              }
            else
              {
                color = Qt::black;
              }

            width = _settings.value("/FlightPathLine/Solid", FlightPathLineWidth).toInt();
          }

          break;

      case MapConfig::Solid:
      default:

        width = _settings.value("/FlightPathLine/Solid", FlightPathLineWidth).toInt();
        color = _settings.value( "/FlightColor/Solid", FlightTypeSolidColor.name() ).value<QColor>();
        break;
    }

  // Simple approach to see "engine was running"
  if( fP->engineNoise > 350 )
    {
      width = _settings.value("/FlightPathLine/Engine", FlightPathLineWidth).toInt();
      //  Put a white (or configured color) strip there in every case
      color = _settings.value( "/FlightColor/EngineNoise",
                               FlightTypeEngineNoiseColor.name() ).value<QColor>();
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

QPen& MapConfig::getDrawPen(unsigned int typeID)
{
  return __getPen(typeID, scaleIndex);
}

QPen& MapConfig::__getPen( unsigned int typeID, int sIndex )
{
  switch(typeID)
    {
      case BaseMapElement::Trail:
          return trailPenList[sIndex];
      case BaseMapElement::Road:
          return roadPenList[sIndex];
      case BaseMapElement::Motorway:
          return highwayPenList[sIndex];
      case BaseMapElement::Railway:
          return railPenList[sIndex];
      case BaseMapElement::Railway_D:
          return rail_dPenList[sIndex];
      case BaseMapElement::Aerial_Cable:
          return aerialcablePenList[sIndex];
      case BaseMapElement::River:
      case BaseMapElement::Lake:
          return riverPenList[sIndex];
      case BaseMapElement::River_T:
      case BaseMapElement::Lake_T:
          return river_tPenList[sIndex];
      case BaseMapElement::Canal:
          return canalPenList[sIndex];
      case BaseMapElement::City:
          return cityPenList[sIndex];

      case BaseMapElement::AirA:
          return airAPenList[sIndex];
      case BaseMapElement::AirB:
          return airBPenList[sIndex];
      case BaseMapElement::AirC:
          return airCPenList[sIndex];
      case BaseMapElement::AirD:
          return airDPenList[sIndex];
      case BaseMapElement::AirElow:
          return airElPenList[sIndex];
      case BaseMapElement::AirE:
          return airEPenList[sIndex];
      case BaseMapElement::AirF:
          return airFPenList[sIndex];
      case BaseMapElement::AirFir:
          return airFirPenList[sIndex];
      case BaseMapElement::Ctr:
          return ctrPenList[sIndex];
      case BaseMapElement::Danger:
      case BaseMapElement::Prohibited:
          return dangerPenList[sIndex];
      case BaseMapElement::LowFlight:
          return lowFPenList[sIndex];
      case BaseMapElement::Restricted:
          return restrPenList[sIndex];
      case BaseMapElement::Rmz:
          return rmzPenList[sIndex];
      case BaseMapElement::Tmz:
          return tmzPenList[sIndex];
      case BaseMapElement::GliderSector:
          return gsPenList[sIndex];
      case BaseMapElement::WaveWindow:
          return wwPenList[sIndex];

      case BaseMapElement::Forest:
          return forestPenList[sIndex];
      case BaseMapElement::Glacier:
          return glacierPenList[sIndex];
      case BaseMapElement::PackIce:
          return packicePenList[sIndex];
      case BaseMapElement::FAIAreaLow500:
          return faiAreaLow500PenList[sIndex];
      case BaseMapElement::FAIAreaHigh500:
          return faiAreaHigh500PenList[sIndex];

      default:
          qWarning() << "MapConfig::__getPen(): Cannot handle type identifier"
                     << typeID;

          return roadPenList[sIndex];
    }
}

int MapConfig::getAsOpacity( uint asType )
{
  switch( asType )
    {
      case BaseMapElement::AirA:
          return airAOpacityList[scaleIndex];
      case BaseMapElement::AirB:
          return airBOpacityList[scaleIndex];
      case BaseMapElement::AirC:
          return airCOpacityList[scaleIndex];
      case BaseMapElement::AirD:
          return airDOpacityList[scaleIndex];
      case BaseMapElement::AirElow:
          return airElOpacityList[scaleIndex];
      case BaseMapElement::AirE:
          return airEOpacityList[scaleIndex];
      case BaseMapElement::AirF:
          return airFOpacityList[scaleIndex];
      case BaseMapElement::AirFir:
          return airFirOpacityList[scaleIndex];
      case BaseMapElement::Ctr:
          return ctrOpacityList[scaleIndex];
      case BaseMapElement::Danger:
      case BaseMapElement::Prohibited:
          return dangerOpacityList[scaleIndex];
      case BaseMapElement::LowFlight:
          return lowFOpacityList[scaleIndex];
      case BaseMapElement::Restricted:
          return restrOpacityList[scaleIndex];
      case BaseMapElement::Rmz:
          return rmzOpacityList[scaleIndex];
      case BaseMapElement::Tmz:
          return tmzOpacityList[scaleIndex];
      case BaseMapElement::GliderSector:
          return gsOpacityList[scaleIndex];
      case BaseMapElement::WaveWindow:
          return wwOpacityList[scaleIndex];

      default:
        qWarning() << "MapConfig::getAsOpacity(): unknown  Airspace Type"
                   << asType;
        break;
    }

  return defaultOpacity[scaleIndex];
}

bool MapConfig::isBorder( unsigned int typeID )
{
  switch( typeID )
    {
      case BaseMapElement::Trail:
          return trailBorder[scaleIndex];
      case BaseMapElement::Road:
          return roadBorder[scaleIndex];
      case BaseMapElement::Motorway:
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
          return airEBorder[scaleIndex];
      case BaseMapElement::AirF:
          return airFBorder[scaleIndex];
      case BaseMapElement::AirFir:
          return airFirBorder[scaleIndex];
      case BaseMapElement::Ctr:
          return ctrBorder[scaleIndex];
      case BaseMapElement::Danger:
      case BaseMapElement::Prohibited:
          return dangerBorder[scaleIndex];
      case BaseMapElement::LowFlight:
          return lowFBorder[scaleIndex];
      case BaseMapElement::Restricted:
          return restrBorder[scaleIndex];
      case BaseMapElement::Rmz:
          return rmzBorder[scaleIndex];
      case BaseMapElement::Tmz:
          return tmzBorder[scaleIndex];
      case BaseMapElement::GliderSector:
          return gsBorder[scaleIndex];
      case BaseMapElement::WaveWindow:
          return wwBorder[scaleIndex];

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

      default:
        qWarning() << "MapConfig::isBorder(): Cannot handle type identifier"
                   << typeID;
        break;
    }

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
      case BaseMapElement::Motorway:
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

      default:
        qWarning() << "MapConfig::isPrintBorder(): Cannot handle type identifier"
                   << typeID;
        break;
    }
  /* Should never happen ... */
  return true;
}

QColor& MapConfig::getIsoColor(unsigned int heightIndex)
{
  return topographyColorList[heightIndex];
}

QBrush& MapConfig::getPrintBrush(unsigned int typeID)
{
  return __getBrush(typeID, printScaleIndex);
}

QBrush& MapConfig::getDrawBrush(unsigned int typeID)
{
  return __getBrush(typeID, scaleIndex);
}

QBrush& MapConfig::__getBrush(unsigned int typeID, int index)
{
  static QBrush emptyBrush;

  switch(typeID)
    {
      case BaseMapElement::City:
          return cityBrushList[index];
      case BaseMapElement::Lake:
          return riverBrushList[index];
      case BaseMapElement::Lake_T:
          return river_tBrushList[index];

      case BaseMapElement::AirA:
          return airABrushList[index];
      case BaseMapElement::AirB:
          return airBBrushList[index];
      case BaseMapElement::AirC:
          return airCBrushList[index];
      case BaseMapElement::AirD:
          return airDBrushList[index];
      case BaseMapElement::AirElow:
          return airElBrushList[index];
      case BaseMapElement::AirE:
          return airEBrushList[index];
      case BaseMapElement::AirF:
          return airFBrushList[index];
      case BaseMapElement::AirFir:
          return airFirBrushList[index];
      case BaseMapElement::Ctr:
          return ctrBrushList[index];
      case BaseMapElement::Danger:
      case BaseMapElement::Prohibited:
          return dangerBrushList[index];
      case BaseMapElement::LowFlight:
          return lowFBrushList[index];
      case BaseMapElement::Restricted:
          return restrBrushList[index];
      case BaseMapElement::Rmz:
          return rmzBrushList[index];
      case BaseMapElement::Tmz:
          return tmzBrushList[index];
      case BaseMapElement::GliderSector:
          return gsBrushList[index];
      case BaseMapElement::WaveWindow:
          return wwBrushList[index];

      case BaseMapElement::Forest:
          return forestBrushList[index];
      case BaseMapElement::Glacier:
          return glacierBrushList[index];
      case BaseMapElement::PackIce:
          return packiceBrushList[index];
      case BaseMapElement::FAIAreaLow500:
          return faiAreaLow500BrushList[index];
      case BaseMapElement::FAIAreaHigh500:
          return faiAreaHigh500BrushList[index];

      default:
        qWarning() << "MapConfig::__getBrush: Cannot handle type identifier"
                   << typeID;
        break;
    }

  return emptyBrush;
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch)
{
  QString iconName(getPixmapName(typeID, isWinch));
  return loadPixmap( iconName, ! isSwitch );
}

QPixmap MapConfig::getPixmap(unsigned int typeID, bool isWinch, bool smallIcon)
{
  QString iconName(getPixmapName(typeID, isWinch));
  return loadPixmap( iconName, smallIcon );
}

QString MapConfig::getIconPath()
{
  return QString( ":/mapicons" );
}

/* Loads a pixmap from the cache. If not contained there, insert it. */
QPixmap MapConfig::loadPixmap( const QString& pixmapName, bool smallIcon )
{
  static bool firstCall = true;
  static QPixmap smallEmptyPixmap( 16, 16 );
  static QPixmap emptyPixmap( 32, 32 );

  if( firstCall )
    {
      firstCall = false;
      smallEmptyPixmap.fill( Qt::transparent );
      emptyPixmap.fill( Qt::transparent );
    }

  QString path = getIconPath();

  if( smallIcon )
    {
      path += "/small";
    }

  path += "/" + pixmapName;

  QPixmap pm;

  if( QPixmapCache::find( path, pm ) )
    {
      return pm;
    }

  if( pm.load( path ) )
    {
      QPixmapCache::insert( path, pm );
      return pm;
    }

  qWarning( "Could not load Pixmap file '%s'. Maybe it was not installed?",
             path.toLatin1().data() );

  // Return an empty transparent pixmap as default
  if( smallIcon )
    {
      return smallEmptyPixmap;
    }

  return emptyPixmap;
}

QPixmap MapConfig::getPixmapRotatable(unsigned int typeID, bool hasWinch)
{
  QString iconName(getPixmapName(typeID, hasWinch, true));

  // qDebug("PixmapNameRotable: %d %s",typeID, iconName.toLatin1().constData() );
  return loadPixmap( iconName, ! isSwitch );
}

QString MapConfig::getPixmapName( unsigned int typeID,
                                  bool hasWinch,
                                  bool rotatable )
{
  QString iconName;

  switch(typeID)
    {
      case BaseMapElement::Airport:
      case BaseMapElement::IntAirport:
        iconName = "airport";
        break;
      case BaseMapElement::MilAirport:
        iconName = "milairport";
        break;
      case BaseMapElement::CivMilAirport:
        iconName = "civmilair";
        break;
      case BaseMapElement::Airfield:
            iconName = rotatable ? "airfield" : "airfield_c";
        break;
      case BaseMapElement::ClosedAirfield:
        iconName = "closed";
        break;
      case BaseMapElement::CivHeliport:
        iconName = "civheliport";
        break;
      case BaseMapElement::MilHeliport:
        iconName = "milheliport";
        break;
      case BaseMapElement::AmbHeliport:
        iconName = "ambheliport";
        break;
      case BaseMapElement::Gliderfield:
        iconName = hasWinch ? "glider_winch" : "glider";
        break;
      case BaseMapElement::UltraLight:
        iconName = "ul";
        break;
      case BaseMapElement::HangGlider:
        iconName = "paraglider";
        break;
      case BaseMapElement::Parachute:
        iconName = "jump";
        break;
      case BaseMapElement::Balloon:
        iconName = "balloon";
        break;
      case BaseMapElement::CompPoint:
        iconName = "compoint";
        break;
      case BaseMapElement::Landmark:
        iconName = "landmark";
        break;
      case BaseMapElement::Vor:
        iconName = "vor";
        break;
      case BaseMapElement::VorDme:
        iconName = "vordme";
        break;
      case BaseMapElement::VorTac:
        iconName = "vortac";
        break;
      case BaseMapElement::Tacan:
	// TODO provide an icon for TACAN
        iconName = "vortac";
        break;
      case BaseMapElement::Ndb:
        iconName = "ndb";
        break;
      case BaseMapElement::Outlanding:
        iconName = "outlanding";
        break;
      case BaseMapElement::Obstacle:
        iconName = "obstacle";
        break;
      case BaseMapElement::LightObstacle:
        iconName = "obst_light";
        break;
      case BaseMapElement::ObstacleGroup:
        iconName = "obst_group";
        break;
      case BaseMapElement::LightObstacleGroup:
        iconName = "obst_group_light";
        break;
      case BaseMapElement::Village:
        iconName = "village";
        break;
      case BaseMapElement::Railway:
        iconName = "railway";
        break;
      case BaseMapElement::AerialRailway:
        iconName = "waypoint";
        break;
      case BaseMapElement::Turnpoint:
        iconName = "waypoint";
        break;
      case BaseMapElement::Thermal:
        iconName = "thermal";
        break;
      case BaseMapElement::City:
        iconName = "waypoint";
        break;
      case BaseMapElement::EmptyPoint:
        iconName = "empty";
        break;

      default:
        qWarning() << "MapConfig::getPixmapName: No pixmap mapping found for typeId"
                   << typeID;
        iconName = "empty";
        break;
    }

  if( rotatable )
    {
      iconName += "-18.png";  // airfield icons can be rotated 10 degree wise
    }
  else
    {
      iconName += ".xpm";
    }

  return iconName;
}

bool MapConfig::isRotatable( unsigned int typeID ) const
  {
    switch (typeID)
      {
      case BaseMapElement::Airport:
      case BaseMapElement::IntAirport:
      case BaseMapElement::CivMilAirport:
      case BaseMapElement::Airfield:
      case BaseMapElement::Gliderfield:
        return true;
      default:
        return false;
      }
  }

/** Returns true if small icons are used, else returns false. */
bool MapConfig::useSmallIcons()
{
  return !isSwitch;
}

/** Returns true if small icons are used, else returns false. */
bool MapConfig::drawWpLabels()
{
  extern MapMatrix *_globalMapMatrix;
  return (_globalMapMatrix->getScale( MapMatrix::CurrentScale ) <= _drawWpLabelScale);
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

void MapConfig::__readBorder( QString group, bool *b )
{
  b[0] = _settings.value( "/Map/" + group + "/Border1", true ).toBool();
  b[1] = _settings.value( "/Map/" + group + "/Border2", true ).toBool();
  b[2] = _settings.value( "/Map/" + group + "/Border3", true ).toBool();
  b[3] = _settings.value( "/Map/" + group + "/Border4", true ).toBool();
  b[4] = _settings.value( "/Map/" + group + "/PrintBorder1", true ).toBool();
  b[5] = _settings.value( "/Map/" + group + "/PrintBorder2", true ).toBool();
}

void MapConfig::__readPen( QString group,
                            QList<QPen> &penList,
                            bool *b,
                            QColor defaultColor1,
                            QColor defaultColor2,
                            QColor defaultColor3,
                            QColor defaultColor4,
                            QColor defaultColor5,
                            QColor defaultColor6,
                            int defaultPenSize1,
                            int defaultPenSize2,
                            int defaultPenSize3,
                            int defaultPenSize4,
                            int defaultPenSize5,
                            int defaultPenSize6,
                            Qt::PenStyle defaultPenStyle1,
                            Qt::PenStyle defaultPenStyle2,
                            Qt::PenStyle defaultPenStyle3,
                            Qt::PenStyle defaultPenStyle4,
                            Qt::PenStyle defaultPenStyle5,
                            Qt::PenStyle defaultPenStyle6 )
{
  Q_UNUSED( defaultPenSize3 )
  Q_UNUSED( defaultPenSize4 )
  // Read border data
  __readBorder( group, b );

  QColor c1( _settings.value( "/Map/" + group + "/Color1", defaultColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + "/Color2", defaultColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + "/Color3", defaultColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + "/Color4", defaultColor4.name() ).value<QColor>() );
  QColor c5( _settings.value( "/Map/" + group + "/PrintColor1", defaultColor5.name() ).value<QColor>() );
  QColor c6( _settings.value( "/Map/" + group + "/PrintColor2", defaultColor6.name() ).value<QColor>() );

  int ps1 = _settings.value( "/Map/" + group + "/PenSize1", defaultPenSize1 ).toInt();
  int ps2 = _settings.value( "/Map/" + group + "/PenSize2", defaultPenSize2 ).toInt();
  int ps3 = _settings.value( "/Map/" + group + "/PenSize3", defaultPenSize2 ).toInt();
  int ps4 = _settings.value( "/Map/" + group + "/PenSize4", defaultPenSize2 ).toInt();
  int ps5 = _settings.value( "/Map/" + group + "/PrintPenSize1", defaultPenSize5 ).toInt();
  int ps6 = _settings.value( "/Map/" + group + "/PrintPenSize2", defaultPenSize6 ).toInt();

  Qt::PenStyle pst1 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle1", (int)defaultPenStyle1).toInt());
  Qt::PenStyle pst2 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle2", (int)defaultPenStyle2).toInt());
  Qt::PenStyle pst3 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle3", (int)defaultPenStyle3).toInt());
  Qt::PenStyle pst4 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle4", (int)defaultPenStyle4).toInt());
  Qt::PenStyle pst5 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PrintPenStyle1", (int)defaultPenStyle5).toInt());
  Qt::PenStyle pst6 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PrintPenStyle2", (int)defaultPenStyle6).toInt());

  QPen pen1(c1);
  QPen pen2(c2);
  QPen pen3(c3);
  QPen pen4(c4);
  QPen pen5(c5);
  QPen pen6(c6);

  pen1.setWidth( ps1 );
  pen2.setWidth( ps2 );
  pen3.setWidth( ps3 );
  pen4.setWidth( ps4 );
  pen5.setWidth( ps5 );
  pen6.setWidth( ps6 );

  pen1.setStyle( pst1 );
  pen2.setStyle( pst2 );
  pen3.setStyle( pst3 );
  pen4.setStyle( pst4 );
  pen5.setStyle( pst5 );
  pen6.setStyle( pst6 );

  // first remove all old entries.
  penList.clear();
  penList.append( pen1 );
  penList.append( pen2 );
  penList.append( pen3 );
  penList.append( pen4 );
  penList.append( pen5 );
  penList.append( pen6 );
}

void MapConfig::__readPenBrush( QString group,
                                QList<QPen> &penList,
                                bool *b,
                                QList<QBrush> &brushList,
                                QColor defaultColor1,
                                QColor defaultColor2,
                                QColor defaultColor3,
                                QColor defaultColor4,
                                QColor defaultColor5,
                                QColor defaultColor6,
                                int defaultPenSize1,
                                int defaultPenSize2,
                                int defaultPenSize3,
                                int defaultPenSize4,
                                int defaultPenSize5,
                                int defaultPenSize6,
                                Qt::PenStyle defaultPenStyle1,
                                Qt::PenStyle defaultPenStyle2,
                                Qt::PenStyle defaultPenStyle3,
                                Qt::PenStyle defaultPenStyle4,
                                Qt::PenStyle defaultPenStyle5,
                                Qt::PenStyle defaultPenStyle6,
                                QColor defaultBrushColor1,
                                QColor defaultBrushColor2,
                                QColor defaultBrushColor3,
                                QColor defaultBrushColor4,
                                QColor defaultBrushColor5,
                                QColor defaultBrushColor6,
                                Qt::BrushStyle defaultBrushStyle1,
                                Qt::BrushStyle defaultBrushStyle2,
                                Qt::BrushStyle defaultBrushStyle3,
                                Qt::BrushStyle defaultBrushStyle4,
                                Qt::BrushStyle defaultBrushStyle5,
                                Qt::BrushStyle defaultBrushStyle6 )
{
  __readPen( group, penList, b,
             defaultColor1, defaultColor2, defaultColor3, defaultColor4, defaultColor5, defaultColor6,
             defaultPenSize1, defaultPenSize2, defaultPenSize3, defaultPenSize4, defaultPenSize5, defaultPenSize6,
             defaultPenStyle1, defaultPenStyle2, defaultPenStyle3, defaultPenStyle4, defaultPenStyle5, defaultPenStyle6 );

  QColor c1( _settings.value( "/Map/" + group + "/BrushColor1", defaultBrushColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + "/BrushColor2", defaultBrushColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + "/BrushColor3", defaultBrushColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + "/BrushColor4", defaultBrushColor4.name() ).value<QColor>() );
  QColor c5( _settings.value( "/Map/" + group + "/PrintBrushColor1", defaultBrushColor5.name() ).value<QColor>() );
  QColor c6( _settings.value( "/Map/" + group + "/PrintBrushColor2", defaultBrushColor6.name() ).value<QColor>() );

  Qt::BrushStyle bs1 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle1", (int)defaultBrushStyle1 ).toInt();
  Qt::BrushStyle bs2 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle2", (int)defaultBrushStyle2 ).toInt();
  Qt::BrushStyle bs3 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle3", (int)defaultBrushStyle3 ).toInt();
  Qt::BrushStyle bs4 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle4", (int)defaultBrushStyle4 ).toInt();
  Qt::BrushStyle bs5 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/PrintBrushStyle1", (int)defaultBrushStyle5 ).toInt();
  Qt::BrushStyle bs6 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/PrintBrushStyle2", (int)defaultBrushStyle6 ).toInt();

  // first remove all old entries.
  brushList.clear();
  brushList.append( QBrush( c1, bs1 ) );
  brushList.append( QBrush( c2, bs2 ) );
  brushList.append( QBrush( c3, bs3 ) );
  brushList.append( QBrush( c4, bs4 ) );
  brushList.append( QBrush( c5, bs5 ) );
  brushList.append( QBrush( c6, bs6 ) );
}

void MapConfig::__readAsOpacity( QString group, QList<int> &opacityList )
{
  int o1 = _settings.value( "/Map/" + group + "/Opacity1", AS_OPACITY_1 ).toInt();
  int o2 = _settings.value( "/Map/" + group + "/Opacity2", AS_OPACITY_2 ).toInt();
  int o3 = _settings.value( "/Map/" + group + "/Opacity3", AS_OPACITY_3 ).toInt();
  int o4 = _settings.value( "/Map/" + group + "/Opacity4", AS_OPACITY_4 ).toInt();

  // first remove all old entries.
  opacityList.clear();
  opacityList << o1 << o2 << o3 << o4;
}

void MapConfig::__readAsOpacity( QString group, QList<int> &opacityList,
                         int defaultOpacity1,
                         int defaultOpacity2,
                         int defaultOpacity3,
                         int defaultOpacity4)
{
  int o1 = _settings.value( "/Map/" + group + "/Opacity1", defaultOpacity1 ).toInt();
  int o2 = _settings.value( "/Map/" + group + "/Opacity2", defaultOpacity2 ).toInt();
  int o3 = _settings.value( "/Map/" + group + "/Opacity3", defaultOpacity3 ).toInt();
  int o4 = _settings.value( "/Map/" + group + "/Opacity4", defaultOpacity4 ).toInt();

  // first remove all old entries.
  opacityList.clear();
  opacityList << o1 << o2 << o3 << o4;
}

void MapConfig::__readTopo( QString entry, QColor color )
{
  QColor c = _settings.value( "/ElevationColors/" + entry, color.name() ).value<QColor>();

  topographyColorList.append( c );
}
