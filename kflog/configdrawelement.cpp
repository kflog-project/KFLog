/***********************************************************************
**
**   configdrawelement.cpp
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

#include "configdrawelement.h"
#include "kflogconfig.h"
#include "mapdefaults.h"

#include <qcolordialog.h>
#include <qdir.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsettings.h>
//Added by qt3to4:
#include <QPixmap>
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3PtrList>

extern QSettings _settings;

ConfigDrawElement::ConfigDrawElement(QWidget* parent)
  : Q3Frame(parent, "configdrawelement"),
    oldElement(-1)
{
  airABorder = new bool[4];
  airBBorder = new bool[4];
  airCBorder = new bool[4];
  airDBorder = new bool[4];
  airElBorder = new bool[4];
  airEhBorder = new bool[4];
  airFBorder = new bool[4];
  ctrCBorder = new bool[4];
  ctrDBorder = new bool[4];
  lowFBorder = new bool[4];
  dangerBorder = new bool[4];
  restrBorder = new bool[4];
  tmzBorder = new bool[4];

  roadBorder = new bool[4];
  highwayBorder = new bool[4];
  railBorder = new bool[4];
  riverBorder = new bool[4];
  cityBorder = new bool[4];
  forestBorder = new bool[4];

  trailBorder = new bool[4];
  rail_dBorder = new bool[4];
  aerialcableBorder = new bool[4];
  river_tBorder = new bool[4];
  canalBorder = new bool[4];
  glacierBorder = new bool[4];
  packiceBorder = new bool[4];

  faiAreaLow500Border = new bool[4];
  faiAreaHigh500Border = new bool[4];

  airAPenList.setAutoDelete(true);
  airABrushList.setAutoDelete(true);
  airBPenList.setAutoDelete(true);
  airBBrushList.setAutoDelete(true);
  airCPenList.setAutoDelete(true);
  airCBrushList.setAutoDelete(true);
  airDPenList.setAutoDelete(true);
  airDBrushList.setAutoDelete(true);
  airElPenList.setAutoDelete(true);
  airElBrushList.setAutoDelete(true);
  airEhPenList.setAutoDelete(true);
  airEhBrushList.setAutoDelete(true);
  ctrCPenList.setAutoDelete(true);
  ctrCBrushList.setAutoDelete(true);
  ctrDPenList.setAutoDelete(true);
  ctrDBrushList.setAutoDelete(true);
  lowFPenList.setAutoDelete(true);
  lowFBrushList.setAutoDelete(true);
  dangerPenList.setAutoDelete(true);
  dangerBrushList.setAutoDelete(true);
  restrPenList.setAutoDelete(true);
  restrBrushList.setAutoDelete(true);
  tmzPenList.setAutoDelete(true);
  tmzBrushList.setAutoDelete(true);

  roadPenList.setAutoDelete(true);
  highwayPenList.setAutoDelete(true);
  riverPenList.setAutoDelete(true);
  railPenList.setAutoDelete(true);
  cityPenList.setAutoDelete(true);

  forestPenList.setAutoDelete(true);
  forestBrushList.setAutoDelete(true);

  trailPenList.setAutoDelete(true);
  rail_dPenList.setAutoDelete(true);
  aerialcablePenList.setAutoDelete(true);
  river_tPenList.setAutoDelete(true);
  canalPenList.setAutoDelete(true);
  glacierPenList.setAutoDelete(true);
  packicePenList.setAutoDelete(true);

  glacierBrushList.setAutoDelete(true);
  packiceBrushList.setAutoDelete(true);

  faiAreaLow500PenList.setAutoDelete(true);
  faiAreaLow500BrushList.setAutoDelete(true);
  faiAreaHigh500PenList.setAutoDelete(true);
  faiAreaHigh500BrushList.setAutoDelete(true);

  __readPen("Trail", &trailPenList, TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
        TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
        TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4);
  __readBorder("Trail", trailBorder);

  __readPen("Road", &roadPenList, ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4);
  __readBorder("Road", roadBorder);

  __readPen("River", &riverPenList, RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3,
        RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4);
  __readBorder("River", riverBorder);

  __readPen("Canal", &canalPenList, CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3,
        CANAL_COLOR_4, CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
        CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4);
  __readBorder("Canal", canalBorder);

  __readPen("Rail", &railPenList, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4);
  __readBorder("Rail", railBorder);

  __readPen("Rail_D", &rail_dPenList, RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3,
        RAIL_D_COLOR_4, RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
        RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4);
  __readBorder("Rail_D", rail_dBorder);

  __readPen("Aerial Cable", &aerialcablePenList, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
        AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4, AERIAL_CABLE_PEN_1,
        AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
        AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2,
        AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4);
  __readBorder("Aerial Cable", aerialcableBorder);

  __readPen("Highway", &highwayPenList, HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4);
  __readBorder("Highway", highwayBorder);

  // PenStyle and BrushStyle are not used for cities ...
  __readPen("City", &cityPenList, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
        CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
        Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine);
  __readBrush("City", &cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, Qt::SolidPattern,
      Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern);
  __readBorder("City", cityBorder);

  __readPen("River_T", &river_tPenList, RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3,
        RIVER_T_COLOR_4, RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
        RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4);
  __readBrush("River_T", &river_tBrushList, RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
        RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4, RIVER_T_BRUSH_STYLE_1,
        RIVER_T_BRUSH_STYLE_2, RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);
  __readBorder("River_T", river_tBorder);

  __readPen("Forest", &forestPenList, FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3,
        FRST_COLOR_4, FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
        FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3,
        FRST_PEN_STYLE_4);
  __readBrush("Forest", &forestBrushList, FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
        FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4, FRST_BRUSH_STYLE_1,
        FRST_BRUSH_STYLE_2, FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4);
  __readBorder("Forest", forestBorder);

  __readPen("Glacier", &glacierPenList, GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3,
        GLACIER_COLOR_4, GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
        GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3,
        GLACIER_PEN_STYLE_4);
  __readBrush("Glacier", &glacierBrushList, GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
        GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4, GLACIER_BRUSH_STYLE_1,
        GLACIER_BRUSH_STYLE_2, GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4);
  __readBorder("Glacier", glacierBorder);

  __readPen("Pack Ice", &packicePenList, PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3,
        PACK_ICE_COLOR_4, PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
        PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3,
        PACK_ICE_PEN_STYLE_4);
  __readBrush("Pack Ice", &packiceBrushList, PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
        PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4, PACK_ICE_BRUSH_STYLE_1,
        PACK_ICE_BRUSH_STYLE_2, PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);
  __readBorder("Pack Ice", packiceBorder);

  __readPen("Airspace A", &airAPenList, AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
        AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
        AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4);
  __readBrush("Airspace A", &airABrushList, AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4, AIRA_BRUSH_STYLE_1,
        AIRA_BRUSH_STYLE_2, AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);
  __readBorder("Airspace A", airABorder);

  __readPen("Airspace B", &airBPenList, AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
        AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
        AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4);
  __readBrush("Airspace B", &airBBrushList, AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4, AIRB_BRUSH_STYLE_1,
        AIRB_BRUSH_STYLE_2, AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);
  __readBorder("Airspace B", airBBorder);

  __readPen("Airspace C", &airCPenList, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4);
  __readBrush("Airspace C", &airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
        AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);
  __readBorder("Airspace C", airCBorder);

  __readPen("Airspace D", &airDPenList, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4);
  __readBrush("Airspace D", &airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
        AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);
  __readBorder("Airspace D", airDBorder);

  __readPen("Airspace E low", &airElPenList, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
        AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4);
  __readBrush("Airspace E low", &airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
        AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);
  __readBorder("Airspace E low", airElBorder);

  __readPen("Airspace E high", &airEhPenList, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
        AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4);
  __readBrush("Airspace E high", &airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
        AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);
  __readBorder("Airspace E high", airEhBorder);

  __readPen("Airspace F", &airFPenList, AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3,
        AIRF_COLOR_4, AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4);
  __readBrush("Airspace F", &airFBrushList, AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4, AIRF_BRUSH_STYLE_1,
        AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4);
  __readBorder("Airspace F", airFBorder);

  __readPen("Control C", &ctrCPenList, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4);
  __readBrush("Control C", &ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
        CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);
  __readBorder("Control C", ctrCBorder);

  __readPen("Control D", &ctrDPenList, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4);
  __readBrush("Control D", &ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
        CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);
  __readBorder("Control D", ctrDBorder);

  __readPen("Danger", &dangerPenList, DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4);
  __readBrush("Danger", &dangerBrushList, DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4, DNG_BRUSH_STYLE_1,
        DNG_BRUSH_STYLE_2, DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);
  __readBorder("Danger", dangerBorder);

  __readPen("Low Flight", &lowFPenList, LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4);
  __readBrush("Low Flight", &lowFBrushList, LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4, LOWF_BRUSH_STYLE_1,
        LOWF_BRUSH_STYLE_2, LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);
  __readBorder("Low Flight", lowFBorder);

  __readPen("Restricted Area", &restrPenList, RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4);
  __readBrush("Restricted Area", &restrBrushList, RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4, RES_BRUSH_STYLE_1,
        RES_BRUSH_STYLE_2, RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);
  __readBorder("Restricted Area", restrBorder);

  __readPen("TMZ", &tmzPenList, TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4);
  __readBrush("TMZ", &tmzBrushList, TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4, TMZ_BRUSH_STYLE_1,
        TMZ_BRUSH_STYLE_2, TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);
  __readBorder("TMZ", tmzBorder);

  __readPen("FAIAreaLow500", &faiAreaLow500PenList, FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
        FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
        FAI_LOW_500_PEN_STYLE_1, FAI_LOW_500_PEN_STYLE_2, FAI_LOW_500_PEN_STYLE_3, FAI_LOW_500_PEN_STYLE_4);
  __readBrush("FAIAreaLow500", &faiAreaLow500BrushList, FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
        FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4, FAI_LOW_500_BRUSH_STYLE_1,
        FAI_LOW_500_BRUSH_STYLE_2, FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaLow500", faiAreaLow500Border);

  __readPen("FAIAreaHigh500", &faiAreaHigh500PenList, FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
        FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
        FAI_HIGH_500_PEN_STYLE_1, FAI_HIGH_500_PEN_STYLE_2, FAI_HIGH_500_PEN_STYLE_3, FAI_HIGH_500_PEN_STYLE_4);
  __readBrush("FAIAreaHigh500", &faiAreaHigh500BrushList, FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
        FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4, FAI_HIGH_500_BRUSH_STYLE_1,
        FAI_HIGH_500_BRUSH_STYLE_2, FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaHigh500", faiAreaHigh500Border);

  border1 = new QCheckBox(tr("threshold #1"), parent);
  border2 = new QCheckBox(tr("threshold #2"), parent);
  border3 = new QCheckBox(tr("threshold #3"), parent);
  border4 = new QCheckBox(tr("scale-limit"), parent);

  QString picDir = QDir::homeDirPath() + "/.kflog/pics/";

  border1Button = new QPushButton(parent);
  border1Button-> setPixmap(picDir + "kde_down.png");
  border1Button-> setFixedWidth(30);
  border1Button-> setFixedHeight(30);
  border2Button = new QPushButton(parent);
  border2Button-> setPixmap(picDir + "kde_down.png");
  border2Button-> setFixedWidth(30);
  border2Button-> setFixedHeight(30);
  border3Button = new QPushButton(parent);
  border3Button-> setPixmap(picDir + "kde_down.png");
  border3Button-> setFixedWidth(30);
  border3Button-> setFixedHeight(30);

  Q3GridLayout* elLayout = new Q3GridLayout(parent, 10, 15, 5, 1);
  elLayout-> addWidget(new QLabel(tr("draw up to"), parent), 1, 1);
  elLayout-> addMultiCellWidget(new QLabel(tr("Pen"), parent), 1, 1, 3, 7);
  elLayout-> addMultiCellWidget(new QLabel(tr("Brush"), parent), 1, 1, 9, 11);
  elLayout-> addWidget(border1, 3, 1);
  elLayout-> addWidget(border1Button, 3, 13);
  elLayout-> addWidget(border2, 5, 1);
  elLayout-> addWidget(border2Button, 5, 13);
  elLayout-> addWidget(border3, 7, 1);
  elLayout-> addWidget(border3Button, 7, 13);
  elLayout-> addWidget(border4, 9, 1);

  border1ColorButton = new QPushButton(parent);
  border1ColorButton->setFixedHeight(24);
  border1ColorButton->setFixedWidth(55);
  connect(border1ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1Color()));
  border1Pen = new QSpinBox(0, 9, 1, parent);
  border1Pen-> setMinimumWidth(35);
  border1PenStyle = new QComboBox(parent);
  border1PenStyle-> setMinimumWidth(35);
  border1BrushColorButton = new QPushButton(parent);
  border1BrushColorButton->setFixedHeight(24);
  border1BrushColorButton->setFixedWidth(55);
  connect(border1BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1BrushColor()));
  border1BrushStyle = new QComboBox(parent);
  border1BrushStyle-> setMinimumWidth(35);
  __fillStyle(border1PenStyle, border1BrushStyle);
  elLayout-> addWidget(border1ColorButton, 3, 3);
  elLayout-> addWidget(border1Pen, 3, 5);
  elLayout-> addWidget(border1PenStyle, 3, 7);
  elLayout-> addWidget(border1BrushColorButton, 3, 9);
  elLayout-> addWidget(border1BrushStyle, 3, 11);

  border2ColorButton = new QPushButton(parent);
  border2ColorButton->setFixedHeight(24);
  border2ColorButton->setFixedWidth(55);
  connect(border2ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2Color()));
  border2Pen = new QSpinBox(0, 9, 1, parent);
  border2Pen-> setMinimumWidth(35);
  border2PenStyle = new QComboBox(parent);
  border2PenStyle-> setMinimumWidth(35);
  border2BrushColorButton = new QPushButton(parent);
  border2BrushColorButton->setFixedHeight(24);
  border2BrushColorButton->setFixedWidth(55);
  connect(border2BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2BrushColor()));
  border2BrushStyle = new QComboBox(parent);
  border2BrushStyle-> setMinimumWidth(35);
  __fillStyle(border2PenStyle, border2BrushStyle);
  elLayout-> addWidget(border2ColorButton, 5, 3);
  elLayout-> addWidget(border2Pen, 5, 5);
  elLayout-> addWidget(border2PenStyle, 5, 7);
  elLayout-> addWidget(border2BrushColorButton, 5, 9);
  elLayout-> addWidget(border2BrushStyle, 5, 11);

  border3ColorButton = new QPushButton(parent);
  border3ColorButton->setFixedHeight(24);
  border3ColorButton->setFixedWidth(55);
  connect(border3ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3Color()));
  border3Pen = new QSpinBox(0, 9, 1, parent);
  border3Pen-> setMinimumWidth(35);
  border3PenStyle = new QComboBox(parent);
  border3PenStyle-> setMinimumWidth(35);
  border3BrushColorButton = new QPushButton(parent);
  border3BrushColorButton->setFixedHeight(24);
  border3BrushColorButton->setFixedWidth(55);
  connect(border3BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3BrushColor()));
  border3BrushStyle = new QComboBox(parent);
  border3BrushStyle-> setMinimumWidth(35);
  __fillStyle(border3PenStyle, border3BrushStyle);
  elLayout-> addWidget(border3ColorButton, 7, 3);
  elLayout-> addWidget(border3Pen, 7, 5);
  elLayout-> addWidget(border3PenStyle, 7, 7);
  elLayout-> addWidget(border3BrushColorButton, 7, 9);
  elLayout-> addWidget(border3BrushStyle, 7, 11);

  border4ColorButton = new QPushButton(parent);
  border4ColorButton->setFixedHeight(24);
  border4ColorButton->setFixedWidth(55);
  connect(border4ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4Color()));
  border4Pen = new QSpinBox(0, 9, 1, parent);
  border4Pen-> setMinimumWidth(35);
  border4PenStyle = new QComboBox(parent);
  border4PenStyle-> setMinimumWidth(35);
  border4BrushColorButton = new QPushButton(parent);
  border4BrushColorButton->setFixedHeight(24);
  border4BrushColorButton->setFixedWidth(55);
  connect(border4BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4BrushColor()));
  border4BrushStyle = new QComboBox(parent);
  border4BrushStyle-> setMinimumWidth(35);
  __fillStyle(border4PenStyle, border4BrushStyle);
  elLayout-> addWidget(border4ColorButton, 9, 3);
  elLayout-> addWidget(border4Pen, 9, 5);
  elLayout-> addWidget(border4PenStyle, 9, 7);
  elLayout-> addWidget(border4BrushColorButton, 9, 9);
  elLayout-> addWidget(border4BrushStyle, 9, 11);

  elLayout-> addRowSpacing(0, 2);
  elLayout-> addRowSpacing(2, 5);
  elLayout-> setRowStretch(2, 1);
  elLayout-> setRowStretch(4, 1);
  elLayout-> setRowStretch(6, 1);
  elLayout-> setRowStretch(8, 1);
  elLayout-> addRowSpacing(10, 5);

  elLayout-> addColSpacing(0, 10);
  elLayout-> setColStretch(3, 1);
  elLayout-> setColStretch(4, 0);
  elLayout-> addColSpacing(4, 5);
  elLayout-> setColStretch(5, 1);
  elLayout-> setColStretch(6, 0);
  elLayout-> addColSpacing(6, 5);
  elLayout-> setColStretch(7, 1);
  elLayout-> addColSpacing(8, 8);
  elLayout-> setColStretch(9, 1);
  elLayout-> setColStretch(10, 0);
  elLayout-> addColSpacing(10, 5);
  elLayout-> setColStretch(11, 1);
  elLayout-> addColSpacing(12, 2);
  elLayout-> addColSpacing(14, 10);

  connect(border1, SIGNAL(toggled(bool)), SLOT(slotToggleFirst(bool)));
  connect(border2, SIGNAL(toggled(bool)), SLOT(slotToggleSecond(bool)));
  connect(border3, SIGNAL(toggled(bool)), SLOT(slotToggleThird(bool)));
  connect(border4, SIGNAL(toggled(bool)), SLOT(slotToggleForth(bool)));
  connect(border1Button, SIGNAL(clicked()), SLOT(slotSetSecond()));
  connect(border2Button, SIGNAL(clicked()), SLOT(slotSetThird()));
  connect(border3Button, SIGNAL(clicked()), SLOT(slotSetForth()));
}

ConfigDrawElement::~ConfigDrawElement()
{
  delete airABorder;
  delete airBBorder;
  delete airCBorder;
  delete airDBorder;
  delete airElBorder;
  delete airEhBorder;
  delete airFBorder;
  delete ctrCBorder;
  delete ctrDBorder;
  delete lowFBorder;
  delete dangerBorder;
  delete restrBorder;
  delete tmzBorder;
  delete roadBorder;
  delete highwayBorder;
  delete railBorder;
  delete riverBorder;
  delete cityBorder;
  delete forestBorder;
  delete trailBorder;
  delete rail_dBorder;
  delete aerialcableBorder;
  delete river_tBorder;
  delete canalBorder;
  delete glacierBorder;
  delete packiceBorder;
  delete faiAreaLow500Border;
  delete faiAreaHigh500Border;
}

void ConfigDrawElement::slotOk()
{
  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

  __writePen("Trail", &trailPenList, trailBorder);

  __writePen("Road", &roadPenList, roadBorder);

  __writePen("Highway", &highwayPenList, highwayBorder);

  __writePen("Rail", &railPenList, railBorder);

  __writePen("Rail_D", &rail_dPenList, rail_dBorder);

  __writePen("Aerial Cable", &aerialcablePenList, aerialcableBorder);

  __writePen("River", &riverPenList, riverBorder);

  __writePen("Canal", &canalPenList, canalBorder);

  __writePen("City", &cityPenList, cityBorder);

  __writeBrush("Airspace A", &airABrushList, &airAPenList, airABorder);

  __writeBrush("Airspace B", &airBBrushList, &airBPenList, airBBorder);

  __writeBrush("Airspace C", &airCBrushList, &airCPenList, airCBorder);

  __writeBrush("Airspace D", &airDBrushList, &airDPenList, airDBorder);

  __writeBrush("Airspace E low", &airElBrushList, &airElPenList, airElBorder);

  __writeBrush("Airspace E high", &airEhBrushList, &airEhPenList, airEhBorder);

  __writeBrush("Airspace F", &airFBrushList, &airFPenList, airFBorder);

  __writeBrush("Control C", &ctrCBrushList, &ctrCPenList, ctrCBorder);

  __writeBrush("Control D", &ctrDBrushList, &ctrDPenList, ctrDBorder);

  __writeBrush("Danger", &dangerBrushList, &dangerPenList, dangerBorder);

  __writeBrush("Low Flight", &lowFBrushList, &lowFPenList, lowFBorder);

  __writeBrush("Restricted Area", &restrBrushList, &restrPenList, restrBorder);

  __writeBrush("TMZ", &tmzBrushList, &tmzPenList, tmzBorder);

  __writeBrush("Forest", &forestBrushList, &forestPenList, forestBorder);

  __writeBrush("River_T", &river_tBrushList, &river_tPenList, river_tBorder);

  __writeBrush("Glacier", &glacierBrushList, &glacierPenList, glacierBorder);

  __writeBrush("Pack Ice", &packiceBrushList, &packicePenList, packiceBorder);

  __writeBrush("FAIAreaLow500", &faiAreaLow500BrushList, &faiAreaLow500PenList, faiAreaLow500Border);

  __writeBrush("FAIAreaHigh500", &faiAreaHigh500BrushList, &faiAreaHigh500PenList, faiAreaHigh500Border);
}

void ConfigDrawElement::slotDefaultElements()
{
  __defaultPen(&trailPenList, trailBorder, TRAIL_COLOR_1, TRAIL_COLOR_2,
      TRAIL_COLOR_3, TRAIL_COLOR_4,
      TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4);

  __defaultPen(&roadPenList, roadBorder, ROAD_COLOR_1, ROAD_COLOR_2,
      ROAD_COLOR_3, ROAD_COLOR_4,
      ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4);

  __defaultPen(&highwayPenList, highwayBorder, HIGH_COLOR_1, HIGH_COLOR_2,
      HIGH_COLOR_3, HIGH_COLOR_4,
      HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4);

  __defaultPen(&riverPenList, riverBorder, RIVER_COLOR_1, RIVER_COLOR_2,
      RIVER_COLOR_3, RIVER_COLOR_4,
      RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4);

  __defaultPen(&canalPenList, canalBorder, CANAL_COLOR_1, CANAL_COLOR_2,
      CANAL_COLOR_3, CANAL_COLOR_4,
      CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4);

  __defaultPen(&railPenList, railBorder, RAIL_COLOR_1, RAIL_COLOR_2,
      RAIL_COLOR_3, RAIL_COLOR_4,
      RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4);

  __defaultPen(&rail_dPenList, rail_dBorder, RAIL_D_COLOR_1, RAIL_D_COLOR_2,
      RAIL_D_COLOR_3, RAIL_D_COLOR_4,
      RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4);

  __defaultPen(&aerialcablePenList, aerialcableBorder, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
      AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
      AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4);

  __defaultPenBrush(&river_tPenList, river_tBorder, &river_tBrushList,
      RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
      RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
      RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
      RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
      RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
      RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);


  __defaultPenBrush(&cityPenList, cityBorder, &cityBrushList,
      CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
      CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
      CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
      CITY_BRUSH_STYLE_1, CITY_BRUSH_STYLE_2,
      CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4);

  __defaultPenBrush(&forestPenList, forestBorder, &forestBrushList,
      FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
      FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
      FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
      FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
      FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
      FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4);

  __defaultPenBrush(&glacierPenList, glacierBorder, &glacierBrushList,
      GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
      GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
      GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
      GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
      GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
      GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4);

  __defaultPenBrush(&packicePenList, packiceBorder, &packiceBrushList,
      PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
      PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
      PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
      PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
      PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
      PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);


  __defaultPenBrush(&airAPenList, airABorder, &airABrushList,
      AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
      AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
      AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
      AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
      AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
      AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);

  __defaultPenBrush(&airBPenList, airBBorder, &airBBrushList,
      AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
      AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
      AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
      AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
      AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
      AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);

  __defaultPenBrush(&airCPenList, airCBorder, &airCBrushList,
      AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
      AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
      AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
      AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
      AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
      AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);

  __defaultPenBrush(&airDPenList, airDBorder, &airDBrushList,
      AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
      AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
      AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
      AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
      AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
      AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);

  __defaultPenBrush(&airElPenList, airElBorder, &airElBrushList,
      AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
      AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
      AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
      AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
      AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
      AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);

  __defaultPenBrush(&airEhPenList, airEhBorder, &airEhBrushList,
      AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
      AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
      AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
      AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
      AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
      AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);

  __defaultPenBrush(&ctrCPenList, ctrCBorder, &ctrCBrushList,
      CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
      CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
      CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
      CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
      CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
      CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);

  __defaultPenBrush(&ctrDPenList, ctrDBorder, &ctrDBrushList,
      CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
      CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
      CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
      CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
      CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
      CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);

  __defaultPenBrush(&dangerPenList, dangerBorder, &dangerBrushList,
      DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
      DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
      DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
      DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
      DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
      DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);

  __defaultPenBrush(&restrPenList, restrBorder, &restrBrushList,
      RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
      RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
      RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
      RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
      RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
      RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);

  __defaultPenBrush(&lowFPenList, lowFBorder, &lowFBrushList,
      LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
      LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
      LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
      LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
      LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
      LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);

  __defaultPenBrush(&tmzPenList, tmzBorder, &tmzBrushList,
      TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
      TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
      TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
      TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
      TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
      TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);

  __defaultPenBrush(&faiAreaLow500PenList, faiAreaLow500Border, &faiAreaLow500BrushList,
      FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
      FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
      FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
      FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4,
      FAI_LOW_500_BRUSH_STYLE_1, FAI_LOW_500_BRUSH_STYLE_2,
      FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);

  __defaultPenBrush(&faiAreaHigh500PenList, faiAreaHigh500Border, &faiAreaHigh500BrushList,
      FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
      FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
      FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
      FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4,
      FAI_HIGH_500_BRUSH_STYLE_1, FAI_HIGH_500_BRUSH_STYLE_2,
      FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4);

  oldElement = -1;
  slotSelectElement(currentElement);
}

void ConfigDrawElement::slotSelectBorder1Color()
{
  border1Color = QColorDialog::getColor(border1Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border1Color);
  border1ColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder2Color()
{
  border2Color = QColorDialog::getColor(border2Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border2Color);
  border2ColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder3Color()
{
  border3Color = QColorDialog::getColor(border3Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border3Color);
  border3ColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder4Color()
{
  border4Color = QColorDialog::getColor(border4Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border4Color);
  border4ColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder1BrushColor()
{
  border1BrushColor = QColorDialog::getColor(border1BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border1BrushColor);
  border1BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder2BrushColor()
{
  border2BrushColor = QColorDialog::getColor(border2BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border2BrushColor);
  border2BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder3BrushColor()
{
  border3BrushColor = QColorDialog::getColor(border3BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border3BrushColor);
  border3BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectBorder4BrushColor()
{
  border4BrushColor = QColorDialog::getColor(border4BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border4BrushColor);
  border4BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigDrawElement::slotSelectElement(int elementID)
{
  currentElement = elementID;

  switch(oldElement)
    {
      case KFLogConfig::Trail:
        __savePen(&trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __savePen(&roadPenList, roadBorder);
        break;
      case KFLogConfig::Highway:
        __savePen(&highwayPenList, highwayBorder);
        break;
      case KFLogConfig::Railway:
        __savePen(&railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __savePen(&rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __savePen(&aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __savePen(&riverPenList, riverBorder);
        break;
      case KFLogConfig::Canal:
        __savePen(&canalPenList, canalBorder);
        break;

      case KFLogConfig::River_T:
        __savePen(&river_tPenList, river_tBorder);
        __saveBrush(&river_tBrushList);
        break;
      case KFLogConfig::City:
        __savePen(&cityPenList, cityBorder);
        __saveBrush(&cityBrushList);
        break;
      case KFLogConfig::AirA:
        __savePen(&airAPenList, airABorder);
        __saveBrush(&airABrushList);
        break;
      case KFLogConfig::AirB:
        __savePen(&airBPenList, airBBorder);
        __saveBrush(&airBBrushList);
        break;
      case KFLogConfig::AirC:
        __savePen(&airCPenList, airCBorder);
        __saveBrush(&airCBrushList);
        break;
      case KFLogConfig::AirD:
        __savePen(&airDPenList, airDBorder);
        __saveBrush(&airDBrushList);
        break;
      case KFLogConfig::AirElow:
        __savePen(&airElPenList, airElBorder);
        __saveBrush(&airElBrushList);
        break;
      case KFLogConfig::AirEhigh:
        __savePen(&airEhPenList, airEhBorder);
        __saveBrush(&airEhBrushList);
        break;
      case KFLogConfig::AirF:
        __savePen(&airFPenList, airFBorder);
        __saveBrush(&airFBrushList);
        break;
      case KFLogConfig::ControlC:
        __savePen(&ctrCPenList, ctrCBorder);
        __saveBrush(&ctrCBrushList);
        break;
      case KFLogConfig::ControlD:
        __savePen(&ctrDPenList, ctrDBorder);
        __saveBrush(&ctrDBrushList);
        break;
      case KFLogConfig::Danger:
        __savePen(&dangerPenList, dangerBorder);
        __saveBrush(&dangerBrushList);
        break;
      case KFLogConfig::LowFlight:
        __savePen(&lowFPenList, lowFBorder);
        __saveBrush(&lowFBrushList);
        break;
      case KFLogConfig::Restricted:
        __savePen(&restrPenList, restrBorder);
        __saveBrush(&restrBrushList);
        break;
      case KFLogConfig::Tmz:
        __savePen(&tmzPenList, tmzBorder);
        __saveBrush(&tmzBrushList);
        break;
      case KFLogConfig::Forest:
        __savePen(&forestPenList, forestBorder);
        __saveBrush(&forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __savePen(&glacierPenList, glacierBorder);
        __saveBrush(&glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __savePen(&packicePenList, packiceBorder);
        __saveBrush(&packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __savePen(&faiAreaLow500PenList, faiAreaLow500Border);
        __saveBrush(&faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __savePen(&faiAreaHigh500PenList, faiAreaHigh500Border);
        __saveBrush(&faiAreaHigh500BrushList);
        break;
      default:
        break;
    }

  switch(elementID)
    {
      case KFLogConfig::Trail:
        __showPen(&trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __showPen(&roadPenList, roadBorder);
        break;
      case KFLogConfig::Highway:
        __showPen(&highwayPenList, highwayBorder);
        break;
      case KFLogConfig::Railway:
        __showPen(&railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __showPen(&rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __showPen(&aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __showPen(&riverPenList, riverBorder);
        break;
      case KFLogConfig::Canal:
        __showPen(&canalPenList, canalBorder);
        break;
      case KFLogConfig::River_T:
        __showPen(&river_tPenList, river_tBorder);
        __showBrush(&river_tBrushList);
        break;
      case KFLogConfig::City:
        __showPen(&cityPenList, cityBorder);
        __showBrush(&cityBrushList);
        break;
      case KFLogConfig::AirA:
        __showPen(&airAPenList, airABorder);
        __showBrush(&airABrushList);
        break;
      case KFLogConfig::AirB:
        __showPen(&airBPenList, airBBorder);
        __showBrush(&airBBrushList);
        break;
      case KFLogConfig::AirC:
        __showPen(&airCPenList, airCBorder);
        __showBrush(&airCBrushList);
        break;
      case KFLogConfig::AirD:
        __showPen(&airDPenList, airDBorder);
        __showBrush(&airDBrushList);
        break;
      case KFLogConfig::AirElow:
        __showPen(&airElPenList, airElBorder);
        __showBrush(&airElBrushList);
        break;
      case KFLogConfig::AirEhigh:
        __showPen(&airEhPenList, airEhBorder);
        __showBrush(&airEhBrushList);
        break;
      case KFLogConfig::AirF:
        __showPen(&airFPenList, airFBorder);
        __showBrush(&airFBrushList);
        break;
      case KFLogConfig::ControlC:
        __showPen(&ctrCPenList, ctrCBorder);
        __showBrush(&ctrCBrushList);
        break;
      case KFLogConfig::ControlD:
        __showPen(&ctrDPenList, ctrDBorder);
        __showBrush(&ctrDBrushList);
        break;
      case KFLogConfig::Danger:
        __showPen(&dangerPenList, dangerBorder);
        __showBrush(&dangerBrushList);
        break;
      case KFLogConfig::LowFlight:
        __showPen(&lowFPenList, lowFBorder);
        __showBrush(&lowFBrushList);
        break;
      case KFLogConfig::Restricted:
        __showPen(&restrPenList, restrBorder);
        __showBrush(&restrBrushList);
        break;
      case KFLogConfig::Tmz:
        __showPen(&tmzPenList, tmzBorder);
        __showBrush(&tmzBrushList);
        break;
      case KFLogConfig::Forest:
        //__showPen(&forestPenList, forestBorder);
        __showBrush(&forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __showPen(&glacierPenList, glacierBorder);
        __showBrush(&glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __showPen(&packicePenList, packiceBorder);
        __showBrush(&packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __showPen(&faiAreaLow500PenList, faiAreaLow500Border);
        __showBrush(&faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __showPen(&faiAreaHigh500PenList, faiAreaHigh500Border);
        __showBrush(&faiAreaHigh500BrushList);
        break;
    }

  oldElement = elementID;
  slotToggleFirst(border1-> isChecked());
}

void ConfigDrawElement::slotToggleFirst(bool toggle)
{
  border1Button-> setEnabled(toggle);
  border1Pen-> setEnabled(toggle);
  border1ColorButton-> setEnabled(toggle);
  border2-> setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border1PenStyle-> setEnabled(false);
        border1BrushColorButton-> setEnabled(toggle);
        border1BrushStyle-> setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirEhigh:
      case KFLogConfig::AirF:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border1PenStyle-> setEnabled(toggle);
        border1BrushColorButton-> setEnabled(toggle);
        border1BrushStyle-> setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border1ColorButton-> setEnabled(false);
        border1Pen-> setEnabled(false);
        border1PenStyle-> setEnabled(false);
        border1BrushColorButton-> setEnabled(toggle);
        border1BrushStyle-> setEnabled(toggle);
        break;
      default:
        border1PenStyle-> setEnabled(toggle);
        border1BrushColorButton-> setEnabled(false);
        border1BrushStyle-> setEnabled(false);
    }

  if(!toggle)
      slotToggleSecond(false);
  else
      slotToggleSecond(border2-> isChecked());
}

void ConfigDrawElement::slotToggleSecond(bool toggle)
{
  border2Button-> setEnabled(toggle);
  border2Pen-> setEnabled(toggle);
  border2ColorButton-> setEnabled(toggle);
  border3-> setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border2PenStyle-> setEnabled(false);
        border2BrushColorButton-> setEnabled(toggle);
        border2BrushStyle-> setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirEhigh:
      case KFLogConfig::AirF:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border2PenStyle-> setEnabled(toggle);
        border2BrushColorButton-> setEnabled(toggle);
        border2BrushStyle-> setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border2ColorButton-> setEnabled(false);
        border2Pen-> setEnabled(false);
        border2PenStyle-> setEnabled(false);
        border2BrushColorButton-> setEnabled(toggle);
        border2BrushStyle-> setEnabled(toggle);
        break;
      default:
        border2PenStyle-> setEnabled(toggle);
        border2BrushColorButton-> setEnabled(false);
        border2BrushStyle-> setEnabled(false);
    }

  if(!toggle)
      slotToggleThird(false);
  else
      slotToggleThird(border3-> isChecked());
}

void ConfigDrawElement::slotToggleThird(bool toggle)
{
  border3Button-> setEnabled(toggle);
  border3Pen-> setEnabled(toggle);
  border3ColorButton-> setEnabled(toggle);
  border4-> setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border3PenStyle-> setEnabled(false);
        border3BrushColorButton-> setEnabled(toggle);
        border3BrushStyle-> setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirEhigh:
      case KFLogConfig::AirF:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border3PenStyle-> setEnabled(toggle);
        border3BrushColorButton-> setEnabled(toggle);
        border3BrushStyle-> setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border3ColorButton-> setEnabled(false);
        border3Pen-> setEnabled(false);
        border3PenStyle-> setEnabled(false);
        border3BrushColorButton-> setEnabled(toggle);
        border3BrushStyle-> setEnabled(toggle);
        break;
      default:
        border3PenStyle-> setEnabled(toggle);
        border3BrushColorButton-> setEnabled(false);
        border3BrushStyle-> setEnabled(false);
    }

  if(!toggle)
      slotToggleForth(false);
  else
      slotToggleForth(border4-> isChecked());
}

void ConfigDrawElement::slotToggleForth(bool toggle)
{
  border4ColorButton-> setEnabled(toggle);
  border4Pen-> setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border4PenStyle-> setEnabled(false);
        border4BrushColorButton-> setEnabled(toggle);
        border4BrushStyle-> setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirEhigh:
      case KFLogConfig::AirF:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Tmz:
      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::River_T:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
        border4PenStyle-> setEnabled(toggle);
        border4BrushColorButton-> setEnabled(toggle);
        border4BrushStyle-> setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border4ColorButton-> setEnabled(false);
        border4Pen-> setEnabled(false);
        border4PenStyle-> setEnabled(false);
        border4BrushColorButton-> setEnabled(toggle);
        border4BrushStyle-> setEnabled(toggle);
        break;
      default:
        border4PenStyle-> setEnabled(toggle);
        border4BrushColorButton-> setEnabled(false);
        border4BrushStyle-> setEnabled(false);
    }
}

void ConfigDrawElement::slotSetSecond()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border2Color = border1Color;
  buttonPixmap->fill(border2Color);
  border2ColorButton->setPixmap(*buttonPixmap);
  border2Pen-> setValue(border1Pen->value());
  border2PenStyle-> setCurrentItem(border1PenStyle->currentItem());
  border2BrushColor = border1BrushColor;
  buttonPixmap->fill(border2BrushColor);
  border2BrushColorButton->setPixmap(*buttonPixmap);
  border2BrushStyle-> setCurrentItem(border1BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetThird()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border3Color = border2Color;
  buttonPixmap->fill(border3Color);
  border3ColorButton->setPixmap(*buttonPixmap);
  border3Pen-> setValue(border2Pen->value());
  border3PenStyle-> setCurrentItem(border2PenStyle->currentItem());
  border3BrushColor = border2BrushColor;
  buttonPixmap->fill(border3BrushColor);
  border3BrushColorButton->setPixmap(*buttonPixmap);
  border3BrushStyle-> setCurrentItem(border2BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetForth()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border4Color = border3Color;
  buttonPixmap->fill(border4Color);
  border4ColorButton->setPixmap(*buttonPixmap);
  border4Pen-> setValue(border3Pen->value());
  border4PenStyle-> setCurrentItem(border3PenStyle->currentItem());
  border4BrushColor = border3BrushColor;
  buttonPixmap->fill(border4BrushColor);
  border4BrushColorButton->setPixmap(*buttonPixmap);
  border4BrushStyle-> setCurrentItem(border3BrushStyle->currentItem());
}

void ConfigDrawElement::__defaultPen(Q3PtrList<QPen> *penList, bool *b,
    QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
    int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4)
{
  penList->at(0)-> setColor(defaultColor1);
  penList->at(0)-> setWidth(defaultPenSize1);
  penList->at(1)-> setColor(defaultColor2);
  penList->at(1)-> setWidth(defaultPenSize2);
  penList->at(2)-> setColor(defaultColor3);
  penList->at(2)-> setWidth(defaultPenSize3);
  penList->at(3)-> setColor(defaultColor4);
  penList->at(3)-> setWidth(defaultPenSize4);
  b[0] = true;
  b[1] = true;
  b[2] = true;
  b[3] = true;
}

void ConfigDrawElement::__defaultPenBrush(Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
    QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
    int defaultPenSize1, int defaultPenSize2, int defaultPenSize3, int defaultPenSize4,
    QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
    Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4)
{
  __defaultPen(penList, b, defaultColor1, defaultColor2, defaultColor3, defaultColor4,
      defaultPenSize1, defaultPenSize2, defaultPenSize3, defaultPenSize4);
  brushList->at(0)->setColor(defaultBrushColor1);
  brushList->at(0)->setStyle(defaultBrushStyle1);
  brushList->at(1)->setColor(defaultBrushColor2);
  brushList->at(1)->setStyle(defaultBrushStyle2);
  brushList->at(2)->setColor(defaultBrushColor3);
  brushList->at(2)->setStyle(defaultBrushStyle3);
  brushList->at(3)->setColor(defaultBrushColor4);
  brushList->at(3)->setStyle(defaultBrushStyle4);
}

// Qt::PenStyle-Enum starts with NoPen = 0, therefor we reduce the
// value by 1. We must use the same order as Qt::PenStyle.
// Qt::BrushStyle "NoBrush" is allowed ...
void ConfigDrawElement::__fillStyle(QComboBox *pen, QComboBox *brush)
{
  QString picDir = QDir::homeDirPath() + "/.kflog/pics/";

  pen-> insertItem(QPixmap(picDir + "solid.png"), Qt::SolidLine - 1);
  pen-> insertItem(QPixmap(picDir + "dashed.png"), Qt::DashLine - 1);
  pen-> insertItem(QPixmap(picDir + "dotted.png"), Qt::DotLine - 1);
  pen-> insertItem(QPixmap(picDir + "dashdot.png"), Qt::DashDotLine - 1);
  pen-> insertItem(QPixmap(picDir + "dashdotdot.png"), Qt::DashDotDotLine - 1);
  brush-> insertItem("no", Qt::NoBrush);
  brush-> insertItem(QPixmap(picDir + "brush0.png"), Qt::SolidPattern);
  brush-> insertItem(QPixmap(picDir + "brush1.png"), Qt::Dense1Pattern);
  brush-> insertItem(QPixmap(picDir + "brush2.png"), Qt::Dense2Pattern);
  brush-> insertItem(QPixmap(picDir + "brush3.png"), Qt::Dense3Pattern);
  brush-> insertItem(QPixmap(picDir + "brush4.png"), Qt::Dense4Pattern);
  brush-> insertItem(QPixmap(picDir + "brush5.png"), Qt::Dense5Pattern);
  brush-> insertItem(QPixmap(picDir + "brush6.png"), Qt::Dense6Pattern);
  brush-> insertItem(QPixmap(picDir + "brush7.png"), Qt::Dense7Pattern);
  brush-> insertItem(QPixmap(picDir + "brush8.png"), Qt::HorPattern);
  brush-> insertItem(QPixmap(picDir + "brush9.png"), Qt::VerPattern);
  brush-> insertItem(QPixmap(picDir + "brush10.png"), Qt::CrossPattern);
  brush-> insertItem(QPixmap(picDir + "brush11.png"), Qt::BDiagPattern);
  brush-> insertItem(QPixmap(picDir + "brush12.png"), Qt::FDiagPattern);
  brush-> insertItem(QPixmap(picDir + "brush13.png"), Qt::DiagCrossPattern);
}

void ConfigDrawElement::__readBorder(QString group, bool *b)
{
  b[0] = _settings.readBoolEntry("/KFLog/Map/"+group+"/Border1", true);
  b[1] = _settings.readBoolEntry("/KFLog/Map/"+group+"/Border2", true);
  b[2] = _settings.readBoolEntry("/KFLog/Map/"+group+"/Border3", true);
  b[3] = _settings.readBoolEntry("/KFLog/Map/"+group+"/Border4", true);
}

void ConfigDrawElement::__readPen(QString group, Q3PtrList<QPen> *penList,
    QColor defaultColor1, QColor defaultColor2, QColor defaultColor3, QColor defaultColor4,
    int defaultPenSize1, int defaultPenSize2, int /*defaultPenSize3*/, int /*defaultPenSize4*/,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, Qt::PenStyle defaultPenStyle3, Qt::PenStyle defaultPenStyle4)
{
  penList->append(new QPen(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/Color1", __color2String(defaultColor1))),
        _settings.readNumEntry("/KFLog/Map/"+group+"/PenSize1", defaultPenSize1),
        (Qt::PenStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/PenStyle1", defaultPenStyle1)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/Color2", __color2String(defaultColor2))),
        _settings.readNumEntry("/KFLog/Map/"+group+"/PenSize2", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/PenStyle2", defaultPenStyle2)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/Color3", __color2String(defaultColor3))),
        _settings.readNumEntry("/KFLog/Map/"+group+"/PenSize3", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/PenStyle3", defaultPenStyle3)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/Color4", __color2String(defaultColor4))),
        _settings.readNumEntry("/KFLog/Map/"+group+"/PenSize4", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/PenStyle4", defaultPenStyle4)));
}

void ConfigDrawElement::__readBrush(QString group, Q3PtrList<QBrush> *brushList,
    QColor defaultBrushColor1, QColor defaultBrushColor2, QColor defaultBrushColor3, QColor defaultBrushColor4,
    Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2, Qt::BrushStyle defaultBrushStyle3, Qt::BrushStyle defaultBrushStyle4)
{
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/BrushColor1", __color2String(defaultBrushColor1))), \
        (Qt::BrushStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/BrushStyle1", defaultBrushStyle1))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/BrushColor2", __color2String(defaultBrushColor2))), \
        (Qt::BrushStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/BrushStyle2", defaultBrushStyle2))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/BrushColor3", __color2String(defaultBrushColor3))), \
        (Qt::BrushStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/BrushStyle3", defaultBrushStyle3))); \
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/KFLog/Map/"+group+"/BrushColor4", __color2String(defaultBrushColor4))), \
        (Qt::BrushStyle)_settings.readNumEntry("/KFLog/Map/"+group+"/BrushStyle4", defaultBrushStyle4)));
}

void ConfigDrawElement::__saveBrush(Q3PtrList<QBrush> *brushList)
{
  brushList->at(0)->setColor(border1BrushColor);
  brushList->at(0)->setStyle((Qt::BrushStyle)border1BrushStyle->currentItem());
  brushList->at(1)->setColor(border2BrushColor);
  brushList->at(1)->setStyle((Qt::BrushStyle)border2BrushStyle->currentItem());
  brushList->at(2)->setColor(border3BrushColor);
  brushList->at(2)->setStyle((Qt::BrushStyle)border3BrushStyle->currentItem());
  brushList->at(3)->setColor(border4BrushColor);
  brushList->at(3)->setStyle((Qt::BrushStyle)border4BrushStyle->currentItem());
}

void ConfigDrawElement::__savePen(Q3PtrList<QPen> *penList, bool *b)
{
  b[0] = border1->isChecked();
  penList->at(0)->setColor(border1Color);
  penList->at(0)->setWidth(border1Pen->value());
  penList->at(0)->setStyle((Qt::PenStyle)(border1PenStyle->currentItem() + 1));
  b[1] = border2->isChecked();
  penList->at(1)->setColor(border2Color);
  penList->at(1)->setWidth(border2Pen->value());
  penList->at(1)->setStyle((Qt::PenStyle)(border2PenStyle->currentItem() + 1));
  b[2] = border3->isChecked();
  penList->at(2)->setColor(border3Color);
  penList->at(2)->setWidth(border3Pen->value());
  penList->at(2)->setStyle((Qt::PenStyle)(border3PenStyle->currentItem() + 1));
  b[3] = border4->isChecked();
  penList->at(3)->setColor(border4Color);
  penList->at(3)->setWidth(border4Pen->value());
  penList->at(3)->setStyle((Qt::PenStyle)(border4PenStyle->currentItem() + 1));
}

void ConfigDrawElement::__showBrush(Q3PtrList<QBrush> *brushList)
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border1BrushColor = brushList->at(0)->color();
  buttonPixmap->fill(border1BrushColor);
  border1BrushColorButton->setPixmap(*buttonPixmap);
  border1BrushStyle->setCurrentItem(brushList->at(0)->style());

  border2BrushColor = brushList->at(1)->color();
  buttonPixmap->fill(border1BrushColor);
  border2BrushColorButton->setPixmap(*buttonPixmap);
  border2BrushStyle->setCurrentItem(brushList->at(1)->style());

  border3BrushColor = brushList->at(2)->color();
  buttonPixmap->fill(border3BrushColor);
  border3BrushColorButton->setPixmap(*buttonPixmap);
  border3BrushStyle->setCurrentItem(brushList->at(2)->style());

  border4BrushColor = brushList->at(3)->color();
  buttonPixmap->fill(border4BrushColor);
  border4BrushColorButton->setPixmap(*buttonPixmap);
  border4BrushStyle->setCurrentItem(brushList->at(2)->style());
}

void ConfigDrawElement::__showPen(Q3PtrList<QPen> *penList, bool *b)
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border1->setChecked(b[0]);
  border1Color = penList->at(0)->color();
  buttonPixmap->fill(border1Color);
  border1ColorButton->setPixmap(*buttonPixmap);
  border1Pen->setValue(penList->at(0)->width());
  border1PenStyle->setCurrentItem(penList->at(0)->style() - 1);

  border2->setChecked(b[1]);
  border2Color = penList->at(1)->color();
  buttonPixmap->fill(border2Color);
  border2ColorButton->setPixmap(*buttonPixmap);
  border2Pen->setValue(penList->at(1)->width());
  border2PenStyle->setCurrentItem(penList->at(1)->style() - 1);

  border3->setChecked(b[2]);
  border3Color = penList->at(2)->color();
  buttonPixmap->fill(border3Color);
  border3ColorButton->setPixmap(*buttonPixmap);
  border3Pen->setValue(penList->at(2)->width());
  border3PenStyle->setCurrentItem(penList->at(2)->style() - 1);

  border4->setChecked(b[3]);
  border4Color = penList->at(3)->color();
  buttonPixmap->fill(border4Color);
  border4ColorButton->setPixmap(*buttonPixmap);
  border4Pen->setValue(penList->at(3)->width());
  border4PenStyle->setCurrentItem(penList->at(3)->style() - 1);
}

void ConfigDrawElement::__writeBrush(QString group, Q3PtrList<QBrush> *brushList, Q3PtrList<QPen> *penList, bool *b)
{
  __writePen(group, penList, b);
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushColor1", __color2String(brushList->at(0)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushColor2", __color2String(brushList->at(1)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushColor3", __color2String(brushList->at(2)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushColor4", __color2String(brushList->at(3)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushStyle1", brushList->at(0)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushStyle2", brushList->at(1)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushStyle3", brushList->at(2)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/BrushStyle4", brushList->at(3)->style());
}

void ConfigDrawElement::__writePen(QString group, Q3PtrList<QPen> *penList, bool *b)
{
  _settings.writeEntry("/KFLog/Map/"+group+"/Color1", __color2String(penList->at(0)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/Color2", __color2String(penList->at(1)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/Color3", __color2String(penList->at(2)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/Color4", __color2String(penList->at(3)->color()));
  _settings.writeEntry("/KFLog/Map/"+group+"/PenSize1", (int)penList->at(0)->width());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenSize2", (int)penList->at(1)->width());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenSize3", (int)penList->at(2)->width());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenSize4", (int)penList->at(3)->width());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenStyle1", penList->at(0)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenStyle2", penList->at(1)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenStyle3", penList->at(2)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/PenStyle4", penList->at(3)->style());
  _settings.writeEntry("/KFLog/Map/"+group+"/Border1", b[0]);
  _settings.writeEntry("/KFLog/Map/"+group+"/Border2", ( b[1] && b[0] ) );
  _settings.writeEntry("/KFLog/Map/"+group+"/Border3", ( b[2] && b[1] && b[0] ) );
  _settings.writeEntry("/KFLog/Map/"+group+"/Border4", ( b[3] && b[2] && b[1] && b[0] ) );
}

/** this is a temporary function and it is not needed in Qt 4 */
QString ConfigDrawElement::__color2String(QColor color)
{
  QString colstr;
  colstr.sprintf("%d;%d;%d", color.red(), color.green(), color.blue());
  return colstr;
}

/** this is a temporary function and it is not needed in Qt 4 */
QColor ConfigDrawElement::__string2Color(QString colstr)
{
  QColor color(colstr.section(";", 0, 0).toInt(), colstr.section(";", 1, 1).toInt(), colstr.section(";", 2, 2).toInt());
  return color;
}
