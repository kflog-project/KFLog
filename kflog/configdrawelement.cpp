/***********************************************************************
**
**   configdrawelement.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <Qt3Support>

#include "configdrawelement.h"
#include "kflogconfig.h"
#include "mapdefaults.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;
extern QSettings _settings;

ConfigDrawElement::ConfigDrawElement( QWidget* parent ) :
  QWidget(parent),
  oldElement(-1)
{
  __readPen("Trail", trailPenList, TRAIL_COLOR_1, TRAIL_COLOR_2, TRAIL_COLOR_3, TRAIL_COLOR_4,
        TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4,
        TRAIL_PEN_STYLE_1, TRAIL_PEN_STYLE_2, TRAIL_PEN_STYLE_3, TRAIL_PEN_STYLE_4);
  __readBorder("Trail", trailBorder);

  __readPen("Road", roadPenList, ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4);
  __readBorder("Road", roadBorder);

  __readPen("River", riverPenList, RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_3,
        RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4);
  __readBorder("River", riverBorder);

  __readPen("Canal", canalPenList, CANAL_COLOR_1, CANAL_COLOR_2, CANAL_COLOR_3,
        CANAL_COLOR_4, CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4,
        CANAL_PEN_STYLE_1, CANAL_PEN_STYLE_2, CANAL_PEN_STYLE_3, CANAL_PEN_STYLE_4);
  __readBorder("Canal", canalBorder);

  __readPen("Rail", railPenList, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4);
  __readBorder("Rail", railBorder);

  __readPen("Rail_D", rail_dPenList, RAIL_D_COLOR_1, RAIL_D_COLOR_2, RAIL_D_COLOR_3,
        RAIL_D_COLOR_4, RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4,
        RAIL_D_PEN_STYLE_1, RAIL_D_PEN_STYLE_2, RAIL_D_PEN_STYLE_3, RAIL_D_PEN_STYLE_4);
  __readBorder("Rail_D", rail_dBorder);

  __readPen("Aerial Cable", aerialcablePenList, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
        AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4, AERIAL_CABLE_PEN_1,
        AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4,
        AERIAL_CABLE_PEN_STYLE_1, AERIAL_CABLE_PEN_STYLE_2,
        AERIAL_CABLE_PEN_STYLE_3, AERIAL_CABLE_PEN_STYLE_4);
  __readBorder("Aerial Cable", aerialcableBorder);

  __readPen("Highway", highwayPenList, HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4);
  __readBorder("Highway", highwayBorder);

  // PenStyle and BrushStyle are not used for cities ...
  __readPen("City", cityPenList, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
        CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
        Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine);
  __readBrush("City", cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, Qt::SolidPattern,
      Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern);
  __readBorder("City", cityBorder);

  __readPen("River_T", river_tPenList, RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3,
        RIVER_T_COLOR_4, RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
        RIVER_T_PEN_STYLE_1, RIVER_T_PEN_STYLE_2, RIVER_T_PEN_STYLE_3, RIVER_T_PEN_STYLE_4);
  __readBrush("River_T", river_tBrushList, RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
        RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4, RIVER_T_BRUSH_STYLE_1,
        RIVER_T_BRUSH_STYLE_2, RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);
  __readBorder("River_T", river_tBorder);

  __readPen("Forest", forestPenList, FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3,
        FRST_COLOR_4, FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
        FRST_PEN_STYLE_1, FRST_PEN_STYLE_2, FRST_PEN_STYLE_3,
        FRST_PEN_STYLE_4);
  __readBrush("Forest", forestBrushList, FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
        FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4, FRST_BRUSH_STYLE_1,
        FRST_BRUSH_STYLE_2, FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4);
  __readBorder("Forest", forestBorder);

  __readPen("Glacier", glacierPenList, GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3,
        GLACIER_COLOR_4, GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
        GLACIER_PEN_STYLE_1, GLACIER_PEN_STYLE_2, GLACIER_PEN_STYLE_3,
        GLACIER_PEN_STYLE_4);
  __readBrush("Glacier", glacierBrushList, GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
        GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4, GLACIER_BRUSH_STYLE_1,
        GLACIER_BRUSH_STYLE_2, GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4);
  __readBorder("Glacier", glacierBorder);

  __readPen("Pack Ice", packicePenList, PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3,
        PACK_ICE_COLOR_4, PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
        PACK_ICE_PEN_STYLE_1, PACK_ICE_PEN_STYLE_2, PACK_ICE_PEN_STYLE_3,
        PACK_ICE_PEN_STYLE_4);
  __readBrush("Pack Ice", packiceBrushList, PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
        PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4, PACK_ICE_BRUSH_STYLE_1,
        PACK_ICE_BRUSH_STYLE_2, PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);
  __readBorder("Pack Ice", packiceBorder);

  __readPen("Airspace A", airAPenList, AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
        AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
        AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4);
  __readBrush("Airspace A", airABrushList, AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4, AIRA_BRUSH_STYLE_1,
        AIRA_BRUSH_STYLE_2, AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);
  __readBorder("Airspace A", airABorder);

  __readPen("Airspace B", airBPenList, AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
        AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
        AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4);
  __readBrush("Airspace B", airBBrushList, AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4, AIRB_BRUSH_STYLE_1,
        AIRB_BRUSH_STYLE_2, AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);
  __readBorder("Airspace B", airBBorder);

  __readPen("Airspace C", airCPenList, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4);
  __readBrush("Airspace C", airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
        AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);
  __readBorder("Airspace C", airCBorder);

  __readPen("Airspace D", airDPenList, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4);
  __readBrush("Airspace D", airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
        AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);
  __readBorder("Airspace D", airDBorder);

  __readPen("Airspace E low", airElPenList, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
        AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4);
  __readBrush("Airspace E low", airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
        AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);
  __readBorder("Airspace E low", airElBorder);

  __readPen("Airspace E high", airEhPenList, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
        AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4);
  __readBrush("Airspace E high", airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
        AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);
  __readBorder("Airspace E high", airEhBorder);

  __readPen("Airspace F", airFPenList, AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3,
        AIRF_COLOR_4, AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4);
  __readBrush("Airspace F", airFBrushList, AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4, AIRF_BRUSH_STYLE_1,
        AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4);
  __readBorder("Airspace F", airFBorder);

  __readPen("Control C", ctrCPenList, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4);
  __readBrush("Control C", ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
        CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);
  __readBorder("Control C", ctrCBorder);

  __readPen("Control D", ctrDPenList, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4);
  __readBrush("Control D", ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
        CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);
  __readBorder("Control D", ctrDBorder);

  __readPen("Danger", dangerPenList, DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4);
  __readBrush("Danger", dangerBrushList, DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4, DNG_BRUSH_STYLE_1,
        DNG_BRUSH_STYLE_2, DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);
  __readBorder("Danger", dangerBorder);

  __readPen("Low Flight", lowFPenList, LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4);
  __readBrush("Low Flight", lowFBrushList, LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4, LOWF_BRUSH_STYLE_1,
        LOWF_BRUSH_STYLE_2, LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);
  __readBorder("Low Flight", lowFBorder);

  __readPen("Restricted Area", restrPenList, RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4);
  __readBrush("Restricted Area", restrBrushList, RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4, RES_BRUSH_STYLE_1,
        RES_BRUSH_STYLE_2, RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);
  __readBorder("Restricted Area", restrBorder);

  __readPen("TMZ", tmzPenList, TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4);
  __readBrush("TMZ", tmzBrushList, TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4, TMZ_BRUSH_STYLE_1,
        TMZ_BRUSH_STYLE_2, TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);
  __readBorder("TMZ", tmzBorder);

  __readPen("FAIAreaLow500", faiAreaLow500PenList, FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
        FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
        FAI_LOW_500_PEN_STYLE_1, FAI_LOW_500_PEN_STYLE_2, FAI_LOW_500_PEN_STYLE_3, FAI_LOW_500_PEN_STYLE_4);
  __readBrush("FAIAreaLow500", faiAreaLow500BrushList, FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
        FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4, FAI_LOW_500_BRUSH_STYLE_1,
        FAI_LOW_500_BRUSH_STYLE_2, FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaLow500", faiAreaLow500Border);

  __readPen("FAIAreaHigh500", faiAreaHigh500PenList, FAI_HIGH_500_COLOR_1, FAI_HIGH_500_COLOR_2, FAI_HIGH_500_COLOR_3, FAI_HIGH_500_COLOR_4,
        FAI_HIGH_500_PEN_1, FAI_HIGH_500_PEN_2, FAI_HIGH_500_PEN_3, FAI_HIGH_500_PEN_4,
        FAI_HIGH_500_PEN_STYLE_1, FAI_HIGH_500_PEN_STYLE_2, FAI_HIGH_500_PEN_STYLE_3, FAI_HIGH_500_PEN_STYLE_4);
  __readBrush("FAIAreaHigh500", faiAreaHigh500BrushList, FAI_HIGH_500_BRUSH_COLOR_1, FAI_HIGH_500_BRUSH_COLOR_2,
        FAI_HIGH_500_BRUSH_COLOR_3, FAI_HIGH_500_BRUSH_COLOR_4, FAI_HIGH_500_BRUSH_STYLE_1,
        FAI_HIGH_500_BRUSH_STYLE_2, FAI_HIGH_500_BRUSH_STYLE_3, FAI_HIGH_500_BRUSH_STYLE_4);
  __readBorder("FAIAreaHigh500", faiAreaHigh500Border);

  border1 = new QCheckBox(tr("Threshold #1"), this);
  border2 = new QCheckBox(tr("Threshold #2"), this);
  border3 = new QCheckBox(tr("Threshold #3"), this);
  border4 = new QCheckBox(tr("Scale limit"), this);

  border1Button = new QPushButton(this);
  border1Button->setPixmap(_mainWindow->getPixmap("kde_down.png"));
  border1Button->setFixedWidth(30);
  border1Button->setFixedHeight(30);

  border2Button = new QPushButton(this);
  border2Button->setPixmap(_mainWindow->getPixmap("kde_down.png"));
  border2Button->setFixedWidth(30);
  border2Button->setFixedHeight(30);

  border3Button = new QPushButton(this);
  border3Button->setPixmap(_mainWindow->getPixmap("kde_down.png"));
  border3Button->setFixedWidth(30);
  border3Button->setFixedHeight(30);

  QGridLayout* elLayout = new QGridLayout( this );
  elLayout->setMargin( 10 );
  elLayout->setSpacing( 5 );

  elLayout->addWidget( new QLabel(tr("Draw up to"), this), 0, 0 );
  elLayout->addWidget( new QLabel(tr("Pen"), this), 0, 1, 1, 3 );
  elLayout->addWidget( new QLabel(tr("Brush"), this), 0, 4, 1, 3 );

  elLayout->addWidget(border1, 1, 0);
  elLayout->addWidget(border1Button, 1, 6);
  elLayout->addWidget(border2, 2, 0);
  elLayout->addWidget(border2Button, 2, 6);
  elLayout->addWidget(border3, 3, 0);
  elLayout->addWidget(border3Button, 3, 6);
  elLayout->addWidget(border4, 4, 0);

  border1ColorButton = new QPushButton(this);
  border1ColorButton->setFixedHeight(24);
  border1ColorButton->setFixedWidth(55);
  connect(border1ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1Color()));

  border1Pen = new QSpinBox( 0, 9, 1, this );
  border1Pen->setMinimumWidth( 35 );
  border1Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );

  border1PenStyle = new QComboBox(this);
  border1PenStyle->setMinimumWidth(60);
  border1PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border1BrushColorButton = new QPushButton(this);
  border1BrushColorButton->setFixedHeight(24);
  border1BrushColorButton->setFixedWidth(55);
  connect(border1BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1BrushColor()));

  border1BrushStyle = new QComboBox(this);
  border1BrushStyle->setMinimumWidth(60);
  border1BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  __fillStyle(border1PenStyle, border1BrushStyle);

  elLayout->addWidget(border1ColorButton, 1, 1);
  elLayout->addWidget(border1Pen, 1, 2);
  elLayout->addWidget(border1PenStyle, 1, 3);
  elLayout->addWidget(border1BrushColorButton, 1, 4);
  elLayout->addWidget(border1BrushStyle, 1, 5);

  border2ColorButton = new QPushButton(this);
  border2ColorButton->setFixedHeight(24);
  border2ColorButton->setFixedWidth(55);
  connect(border2ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2Color()));

  border2Pen = new QSpinBox(0, 9, 1, this);
  border2Pen->setMinimumWidth(35);
  border2Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );

  border2PenStyle = new QComboBox(this);
  border2PenStyle->setMinimumWidth(35);
  border2PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border2BrushColorButton = new QPushButton(this);
  border2BrushColorButton->setFixedHeight(24);
  border2BrushColorButton->setFixedWidth(55);
  connect(border2BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2BrushColor()));

  border2BrushStyle = new QComboBox(this);
  border2BrushStyle->setMinimumWidth(35);
  border2BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  __fillStyle(border2PenStyle, border2BrushStyle);

  elLayout->addWidget(border2ColorButton, 2, 1);
  elLayout->addWidget(border2Pen, 2, 2);
  elLayout->addWidget(border2PenStyle, 2, 3);
  elLayout->addWidget(border2BrushColorButton, 2, 4);
  elLayout->addWidget(border2BrushStyle, 2, 5);

  border3ColorButton = new QPushButton(this);
  border3ColorButton->setFixedHeight(24);
  border3ColorButton->setFixedWidth(55);
  connect(border3ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3Color()));

  border3Pen = new QSpinBox(0, 9, 1, this);
  border3Pen->setMinimumWidth(35);
  border3Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );

  border3PenStyle = new QComboBox(this);
  border3PenStyle->setMinimumWidth(35);
  border3PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border3BrushColorButton = new QPushButton(this);
  border3BrushColorButton->setFixedHeight(24);
  border3BrushColorButton->setFixedWidth(55);
  connect(border3BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder3BrushColor()));

  border3BrushStyle = new QComboBox(this);
  border3BrushStyle->setMinimumWidth(35);
  border3BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  __fillStyle(border3PenStyle, border3BrushStyle);

  elLayout->addWidget(border3ColorButton, 3, 1);
  elLayout->addWidget(border3Pen, 3, 2);
  elLayout->addWidget(border3PenStyle, 3, 3);
  elLayout->addWidget(border3BrushColorButton, 3, 4);
  elLayout->addWidget(border3BrushStyle, 3, 5);

  border4ColorButton = new QPushButton(this);
  border4ColorButton->setFixedHeight(24);
  border4ColorButton->setFixedWidth(55);
  connect(border4ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4Color()));

  border4Pen = new QSpinBox(0, 9, 1, this);
  border4Pen->setMinimumWidth(35);
  border4Pen->setButtonSymbols( QAbstractSpinBox::PlusMinus );

  border4PenStyle = new QComboBox(this);
  border4PenStyle->setMinimumWidth(35);
  border4PenStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );

  border4BrushColorButton = new QPushButton(this);
  border4BrushColorButton->setFixedHeight(24);
  border4BrushColorButton->setFixedWidth(55);
  connect(border4BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder4BrushColor()));

  border4BrushStyle = new QComboBox(this);
  border4BrushStyle->setMinimumWidth(35);
  border4BrushStyle->setSizeAdjustPolicy( QComboBox::AdjustToContents );
  __fillStyle(border4PenStyle, border4BrushStyle);

  elLayout->addWidget(border4ColorButton, 4, 1);
  elLayout->addWidget(border4Pen, 4, 2);
  elLayout->addWidget(border4PenStyle, 4, 3);
  elLayout->addWidget(border4BrushColorButton, 4, 4);
  elLayout->addWidget(border4BrushStyle, 4, 5);

  elLayout->setColStretch( 7, 10 );

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
}

void ConfigDrawElement::slotOk()
{
  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

  __writePen("Trail", trailPenList, trailBorder);

  __writePen("Road", roadPenList, roadBorder);

  __writePen("Highway", highwayPenList, highwayBorder);

  __writePen("Rail", railPenList, railBorder);

  __writePen("Rail_D", rail_dPenList, rail_dBorder);

  __writePen("Aerial Cable", aerialcablePenList, aerialcableBorder);

  __writePen("River", riverPenList, riverBorder);

  __writePen("Canal", canalPenList, canalBorder);

  __writePen("City", cityPenList, cityBorder);

  __writeBrush("Airspace A", airABrushList, airAPenList, airABorder);

  __writeBrush("Airspace B", airBBrushList, airBPenList, airBBorder);

  __writeBrush("Airspace C", airCBrushList, airCPenList, airCBorder);

  __writeBrush("Airspace D", airDBrushList, airDPenList, airDBorder);

  __writeBrush("Airspace E low", airElBrushList, airElPenList, airElBorder);

  __writeBrush("Airspace E high", airEhBrushList, airEhPenList, airEhBorder);

  __writeBrush("Airspace F", airFBrushList, airFPenList, airFBorder);

  __writeBrush("Control C", ctrCBrushList, ctrCPenList, ctrCBorder);

  __writeBrush("Control D", ctrDBrushList, ctrDPenList, ctrDBorder);

  __writeBrush("Danger", dangerBrushList, dangerPenList, dangerBorder);

  __writeBrush("Low Flight", lowFBrushList, lowFPenList, lowFBorder);

  __writeBrush("Restricted Area", restrBrushList, restrPenList, restrBorder);

  __writeBrush("TMZ", tmzBrushList, tmzPenList, tmzBorder);

  __writeBrush("Forest", forestBrushList, forestPenList, forestBorder);

  __writeBrush("River_T", river_tBrushList, river_tPenList, river_tBorder);

  __writeBrush("Glacier", glacierBrushList, glacierPenList, glacierBorder);

  __writeBrush("Pack Ice", packiceBrushList, packicePenList, packiceBorder);

  __writeBrush("FAIAreaLow500", faiAreaLow500BrushList, faiAreaLow500PenList, faiAreaLow500Border);

  __writeBrush("FAIAreaHigh500", faiAreaHigh500BrushList, faiAreaHigh500PenList, faiAreaHigh500Border);
}

void ConfigDrawElement::slotDefaultElements()
{
  __defaultPen( trailPenList, trailBorder, TRAIL_COLOR_1, TRAIL_COLOR_2,
      TRAIL_COLOR_3, TRAIL_COLOR_4,
      TRAIL_PEN_1, TRAIL_PEN_2, TRAIL_PEN_3, TRAIL_PEN_4 );

  __defaultPen( roadPenList, roadBorder, ROAD_COLOR_1, ROAD_COLOR_2,
      ROAD_COLOR_3, ROAD_COLOR_4,
      ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4 );

  __defaultPen( highwayPenList, highwayBorder, HIGH_COLOR_1, HIGH_COLOR_2,
      HIGH_COLOR_3, HIGH_COLOR_4,
      HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4 );

  __defaultPen( riverPenList, riverBorder, RIVER_COLOR_1, RIVER_COLOR_2,
      RIVER_COLOR_3, RIVER_COLOR_4,
      RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4 );

  __defaultPen( canalPenList, canalBorder, CANAL_COLOR_1, CANAL_COLOR_2,
      CANAL_COLOR_3, CANAL_COLOR_4,
      CANAL_PEN_1, CANAL_PEN_2, CANAL_PEN_3, CANAL_PEN_4 );

  __defaultPen( railPenList, railBorder, RAIL_COLOR_1, RAIL_COLOR_2,
      RAIL_COLOR_3, RAIL_COLOR_4,
      RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4 );

  __defaultPen( rail_dPenList, rail_dBorder, RAIL_D_COLOR_1, RAIL_D_COLOR_2,
      RAIL_D_COLOR_3, RAIL_D_COLOR_4,
      RAIL_D_PEN_1, RAIL_D_PEN_2, RAIL_D_PEN_3, RAIL_D_PEN_4 );

  __defaultPen( aerialcablePenList, aerialcableBorder, AERIAL_CABLE_COLOR_1, AERIAL_CABLE_COLOR_2,
      AERIAL_CABLE_COLOR_3, AERIAL_CABLE_COLOR_4,
      AERIAL_CABLE_PEN_1, AERIAL_CABLE_PEN_2, AERIAL_CABLE_PEN_3, AERIAL_CABLE_PEN_4 );

  __defaultPenBrush( river_tPenList, river_tBorder, river_tBrushList,
      RIVER_T_COLOR_1, RIVER_T_COLOR_2, RIVER_T_COLOR_3, RIVER_T_COLOR_4,
      RIVER_T_PEN_1, RIVER_T_PEN_2, RIVER_T_PEN_3, RIVER_T_PEN_4,
      RIVER_T_BRUSH_COLOR_1, RIVER_T_BRUSH_COLOR_2,
      RIVER_T_BRUSH_COLOR_3, RIVER_T_BRUSH_COLOR_4,
      RIVER_T_BRUSH_STYLE_1, RIVER_T_BRUSH_STYLE_2,
      RIVER_T_BRUSH_STYLE_3, RIVER_T_BRUSH_STYLE_4);

  __defaultPenBrush( cityPenList, cityBorder, cityBrushList,
      CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
      CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
      CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
      CITY_BRUSH_STYLE_1, CITY_BRUSH_STYLE_2,
      CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4 );

  __defaultPenBrush( forestPenList, forestBorder, forestBrushList,
      FRST_COLOR_1, FRST_COLOR_2, FRST_COLOR_3, FRST_COLOR_4,
      FRST_PEN_1, FRST_PEN_2, FRST_PEN_3, FRST_PEN_4,
      FRST_BRUSH_COLOR_1, FRST_BRUSH_COLOR_2,
      FRST_BRUSH_COLOR_3, FRST_BRUSH_COLOR_4,
      FRST_BRUSH_STYLE_1, FRST_BRUSH_STYLE_2,
      FRST_BRUSH_STYLE_3, FRST_BRUSH_STYLE_4 );

  __defaultPenBrush( glacierPenList, glacierBorder, glacierBrushList,
      GLACIER_COLOR_1, GLACIER_COLOR_2, GLACIER_COLOR_3, GLACIER_COLOR_4,
      GLACIER_PEN_1, GLACIER_PEN_2, GLACIER_PEN_3, GLACIER_PEN_4,
      GLACIER_BRUSH_COLOR_1, GLACIER_BRUSH_COLOR_2,
      GLACIER_BRUSH_COLOR_3, GLACIER_BRUSH_COLOR_4,
      GLACIER_BRUSH_STYLE_1, GLACIER_BRUSH_STYLE_2,
      GLACIER_BRUSH_STYLE_3, GLACIER_BRUSH_STYLE_4 );

  __defaultPenBrush(packicePenList, packiceBorder, packiceBrushList,
      PACK_ICE_COLOR_1, PACK_ICE_COLOR_2, PACK_ICE_COLOR_3, PACK_ICE_COLOR_4,
      PACK_ICE_PEN_1, PACK_ICE_PEN_2, PACK_ICE_PEN_3, PACK_ICE_PEN_4,
      PACK_ICE_BRUSH_COLOR_1, PACK_ICE_BRUSH_COLOR_2,
      PACK_ICE_BRUSH_COLOR_3, PACK_ICE_BRUSH_COLOR_4,
      PACK_ICE_BRUSH_STYLE_1, PACK_ICE_BRUSH_STYLE_2,
      PACK_ICE_BRUSH_STYLE_3, PACK_ICE_BRUSH_STYLE_4);


  __defaultPenBrush(airAPenList, airABorder, airABrushList,
      AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
      AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
      AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
      AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
      AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
      AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4);

  __defaultPenBrush(airBPenList, airBBorder, airBBrushList,
      AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
      AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
      AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
      AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
      AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
      AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4);

  __defaultPenBrush(airCPenList, airCBorder, airCBrushList,
      AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
      AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
      AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
      AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
      AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
      AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4);

  __defaultPenBrush(airDPenList, airDBorder, airDBrushList,
      AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
      AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
      AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
      AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
      AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
      AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4);

  __defaultPenBrush(airElPenList, airElBorder, airElBrushList,
      AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
      AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
      AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
      AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
      AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
      AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4);

  __defaultPenBrush(airEhPenList, airEhBorder, airEhBrushList,
      AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
      AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
      AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
      AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
      AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
      AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4);

  __defaultPenBrush(ctrCPenList, ctrCBorder, ctrCBrushList,
      CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
      CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
      CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
      CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
      CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
      CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4);

  __defaultPenBrush(ctrDPenList, ctrDBorder, ctrDBrushList,
      CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
      CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
      CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
      CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
      CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
      CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4);

  __defaultPenBrush(dangerPenList, dangerBorder, dangerBrushList,
      DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
      DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
      DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
      DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
      DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
      DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4);

  __defaultPenBrush(restrPenList, restrBorder, restrBrushList,
      RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
      RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
      RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
      RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
      RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
      RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4);

  __defaultPenBrush(lowFPenList, lowFBorder, lowFBrushList,
      LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
      LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
      LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
      LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
      LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
      LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4);

  __defaultPenBrush(tmzPenList, tmzBorder, tmzBrushList,
      TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
      TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
      TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
      TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
      TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
      TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4);

  __defaultPenBrush(faiAreaLow500PenList, faiAreaLow500Border, faiAreaLow500BrushList,
      FAI_LOW_500_COLOR_1, FAI_LOW_500_COLOR_2, FAI_LOW_500_COLOR_3, FAI_LOW_500_COLOR_4,
      FAI_LOW_500_PEN_1, FAI_LOW_500_PEN_2, FAI_LOW_500_PEN_3, FAI_LOW_500_PEN_4,
      FAI_LOW_500_BRUSH_COLOR_1, FAI_LOW_500_BRUSH_COLOR_2,
      FAI_LOW_500_BRUSH_COLOR_3, FAI_LOW_500_BRUSH_COLOR_4,
      FAI_LOW_500_BRUSH_STYLE_1, FAI_LOW_500_BRUSH_STYLE_2,
      FAI_LOW_500_BRUSH_STYLE_3, FAI_LOW_500_BRUSH_STYLE_4);

  __defaultPenBrush(faiAreaHigh500PenList, faiAreaHigh500Border, faiAreaHigh500BrushList,
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
        __savePen(trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __savePen(roadPenList, roadBorder);
        break;
      case KFLogConfig::Highway:
        __savePen(highwayPenList, highwayBorder);
        break;
      case KFLogConfig::Railway:
        __savePen(railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __savePen(rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __savePen(aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __savePen(riverPenList, riverBorder);
        break;
      case KFLogConfig::Canal:
        __savePen(canalPenList, canalBorder);
        break;

      case KFLogConfig::River_T:
        __savePen(river_tPenList, river_tBorder);
        __saveBrush(river_tBrushList);
        break;
      case KFLogConfig::City:
        __savePen(cityPenList, cityBorder);
        __saveBrush(cityBrushList);
        break;
      case KFLogConfig::AirA:
        __savePen(airAPenList, airABorder);
        __saveBrush(airABrushList);
        break;
      case KFLogConfig::AirB:
        __savePen(airBPenList, airBBorder);
        __saveBrush(airBBrushList);
        break;
      case KFLogConfig::AirC:
        __savePen(airCPenList, airCBorder);
        __saveBrush(airCBrushList);
        break;
      case KFLogConfig::AirD:
        __savePen(airDPenList, airDBorder);
        __saveBrush(airDBrushList);
        break;
      case KFLogConfig::AirElow:
        __savePen(airElPenList, airElBorder);
        __saveBrush(airElBrushList);
        break;
      case KFLogConfig::AirE:
        __savePen(airEhPenList, airEhBorder);
        __saveBrush(airEhBrushList);
        break;
      case KFLogConfig::AirF:
        __savePen(airFPenList, airFBorder);
        __saveBrush(airFBrushList);
        break;
      case KFLogConfig::ControlC:
        __savePen(ctrCPenList, ctrCBorder);
        __saveBrush(ctrCBrushList);
        break;
      case KFLogConfig::ControlD:
        __savePen(ctrDPenList, ctrDBorder);
        __saveBrush(ctrDBrushList);
        break;
      case KFLogConfig::Danger:
        __savePen(dangerPenList, dangerBorder);
        __saveBrush(dangerBrushList);
        break;
      case KFLogConfig::LowFlight:
        __savePen(lowFPenList, lowFBorder);
        __saveBrush(lowFBrushList);
        break;
      case KFLogConfig::Restricted:
        __savePen(restrPenList, restrBorder);
        __saveBrush(restrBrushList);
        break;
      case KFLogConfig::Tmz:
        __savePen(tmzPenList, tmzBorder);
        __saveBrush(tmzBrushList);
        break;
      case KFLogConfig::Forest:
        __savePen(forestPenList, forestBorder);
        __saveBrush(forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __savePen(glacierPenList, glacierBorder);
        __saveBrush(glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __savePen(packicePenList, packiceBorder);
        __saveBrush(packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __savePen(faiAreaLow500PenList, faiAreaLow500Border);
        __saveBrush(faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __savePen(faiAreaHigh500PenList, faiAreaHigh500Border);
        __saveBrush(faiAreaHigh500BrushList);
        break;
      default:
        break;
    }

  switch(elementID)
    {
      case KFLogConfig::Trail:
        __showPen(trailPenList, trailBorder);
        break;
      case KFLogConfig::Road:
        __showPen(roadPenList, roadBorder);
        break;
      case KFLogConfig::Highway:
        __showPen(highwayPenList, highwayBorder);
        break;
      case KFLogConfig::Railway:
        __showPen(railPenList, railBorder);
        break;
      case KFLogConfig::Railway_D:
        __showPen(rail_dPenList, rail_dBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __showPen(aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::River:
        __showPen(riverPenList, riverBorder);
        break;
      case KFLogConfig::Canal:
        __showPen(canalPenList, canalBorder);
        break;
      case KFLogConfig::River_T:
        __showPen(river_tPenList, river_tBorder);
        __showBrush(river_tBrushList);
        break;
      case KFLogConfig::City:
        __showPen(cityPenList, cityBorder);
        __showBrush(cityBrushList);
        break;
      case KFLogConfig::AirA:
        __showPen(airAPenList, airABorder);
        __showBrush(airABrushList);
        break;
      case KFLogConfig::AirB:
        __showPen(airBPenList, airBBorder);
        __showBrush(airBBrushList);
        break;
      case KFLogConfig::AirC:
        __showPen(airCPenList, airCBorder);
        __showBrush(airCBrushList);
        break;
      case KFLogConfig::AirD:
        __showPen(airDPenList, airDBorder);
        __showBrush(airDBrushList);
        break;
      case KFLogConfig::AirElow:
        __showPen(airElPenList, airElBorder);
        __showBrush(airElBrushList);
        break;
      case KFLogConfig::AirE:
        __showPen(airEhPenList, airEhBorder);
        __showBrush(airEhBrushList);
        break;
      case KFLogConfig::AirF:
        __showPen(airFPenList, airFBorder);
        __showBrush(airFBrushList);
        break;
      case KFLogConfig::ControlC:
        __showPen(ctrCPenList, ctrCBorder);
        __showBrush(ctrCBrushList);
        break;
      case KFLogConfig::ControlD:
        __showPen(ctrDPenList, ctrDBorder);
        __showBrush(ctrDBrushList);
        break;
      case KFLogConfig::Danger:
        __showPen(dangerPenList, dangerBorder);
        __showBrush(dangerBrushList);
        break;
      case KFLogConfig::LowFlight:
        __showPen(lowFPenList, lowFBorder);
        __showBrush(lowFBrushList);
        break;
      case KFLogConfig::Restricted:
        __showPen(restrPenList, restrBorder);
        __showBrush(restrBrushList);
        break;
      case KFLogConfig::Tmz:
        __showPen(tmzPenList, tmzBorder);
        __showBrush(tmzBrushList);
        break;
      case KFLogConfig::Forest:
        //__showPen(forestPenList, forestBorder);
        __showBrush(forestBrushList);
        break;
      case KFLogConfig::Glacier:
        __showPen(glacierPenList, glacierBorder);
        __showBrush(glacierBrushList);
        break;
      case KFLogConfig::PackIce:
        __showPen(packicePenList, packiceBorder);
        __showBrush(packiceBrushList);
        break;
      case KFLogConfig::FAIAreaLow500:
        __showPen(faiAreaLow500PenList, faiAreaLow500Border);
        __showBrush(faiAreaLow500BrushList);
        break;
      case KFLogConfig::FAIAreaHigh500:
        __showPen(faiAreaHigh500PenList, faiAreaHigh500Border);
        __showBrush(faiAreaHigh500BrushList);
        break;
      default:
        qWarning() << "ConfigDrawElement::slotSelectElement: Can not handle Id="
                   << elementID;
    }

  oldElement = elementID;
  slotToggleFirst(border1->isChecked());
}

void ConfigDrawElement::slotToggleFirst(bool toggle)
{
  border1Button->setEnabled(toggle);
  border1Pen->setEnabled(toggle);
  border1ColorButton->setEnabled(toggle);
  border2->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border1PenStyle->setEnabled(false);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
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
        border1PenStyle->setEnabled(toggle);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border1ColorButton->setEnabled(false);
        border1Pen->setEnabled(false);
        border1PenStyle->setEnabled(false);
        border1BrushColorButton->setEnabled(toggle);
        border1BrushStyle->setEnabled(toggle);
        break;
      default:
        border1PenStyle->setEnabled(toggle);
        border1BrushColorButton->setEnabled(false);
        border1BrushStyle->setEnabled(false);
    }

  if(!toggle)
      slotToggleSecond(false);
  else
      slotToggleSecond(border2->isChecked());
}

void ConfigDrawElement::slotToggleSecond(bool toggle)
{
  border2Button->setEnabled(toggle);
  border2Pen->setEnabled(toggle);
  border2ColorButton->setEnabled(toggle);
  border3->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border2PenStyle->setEnabled(false);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
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
        border2PenStyle->setEnabled(toggle);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border2ColorButton->setEnabled(false);
        border2Pen->setEnabled(false);
        border2PenStyle->setEnabled(false);
        border2BrushColorButton->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        break;
      default:
        border2PenStyle->setEnabled(toggle);
        border2BrushColorButton->setEnabled(false);
        border2BrushStyle->setEnabled(false);
    }

  if(!toggle)
      slotToggleThird(false);
  else
      slotToggleThird(border3->isChecked());
}

void ConfigDrawElement::slotToggleThird(bool toggle)
{
  border3Button->setEnabled(toggle);
  border3Pen->setEnabled(toggle);
  border3ColorButton->setEnabled(toggle);
  border4->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border3PenStyle->setEnabled(false);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
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
        border3PenStyle->setEnabled(toggle);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border3ColorButton->setEnabled(false);
        border3Pen->setEnabled(false);
        border3PenStyle->setEnabled(false);
        border3BrushColorButton->setEnabled(toggle);
        border3BrushStyle->setEnabled(toggle);
        break;
      default:
        border3PenStyle->setEnabled(toggle);
        border3BrushColorButton->setEnabled(false);
        border3BrushStyle->setEnabled(false);
    }

  if(!toggle)
      slotToggleForth(false);
  else
      slotToggleForth(border4->isChecked());
}

void ConfigDrawElement::slotToggleForth(bool toggle)
{
  border4ColorButton->setEnabled(toggle);
  border4Pen->setEnabled(toggle);

  switch(currentElement)
    {
      case KFLogConfig::City:
        border4PenStyle->setEnabled(false);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(false);
        break;
      case KFLogConfig::AirA:
      case KFLogConfig::AirB:
      case KFLogConfig::AirC:
      case KFLogConfig::AirD:
      case KFLogConfig::AirElow:
      case KFLogConfig::AirE:
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
        border4PenStyle->setEnabled(toggle);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(toggle);
        break;
      case KFLogConfig::Forest:
        border4ColorButton->setEnabled(false);
        border4Pen->setEnabled(false);
        border4PenStyle->setEnabled(false);
        border4BrushColorButton->setEnabled(toggle);
        border4BrushStyle->setEnabled(toggle);
        break;
      default:
        border4PenStyle->setEnabled(toggle);
        border4BrushColorButton->setEnabled(false);
        border4BrushStyle->setEnabled(false);
    }
}

void ConfigDrawElement::slotSetSecond()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border2Color = border1Color;
  buttonPixmap->fill(border2Color);
  border2ColorButton->setPixmap(*buttonPixmap);
  border2Pen->setValue(border1Pen->value());
  border2PenStyle->setCurrentItem(border1PenStyle->currentItem());
  border2BrushColor = border1BrushColor;
  buttonPixmap->fill(border2BrushColor);
  border2BrushColorButton->setPixmap(*buttonPixmap);
  border2BrushStyle->setCurrentItem(border1BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetThird()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border3Color = border2Color;
  buttonPixmap->fill(border3Color);
  border3ColorButton->setPixmap(*buttonPixmap);
  border3Pen->setValue(border2Pen->value());
  border3PenStyle->setCurrentItem(border2PenStyle->currentItem());
  border3BrushColor = border2BrushColor;
  buttonPixmap->fill(border3BrushColor);
  border3BrushColorButton->setPixmap(*buttonPixmap);
  border3BrushStyle->setCurrentItem(border2BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetForth()
{
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);

  border4Color = border3Color;
  buttonPixmap->fill(border4Color);
  border4ColorButton->setPixmap(*buttonPixmap);
  border4Pen->setValue(border3Pen->value());
  border4PenStyle->setCurrentItem(border3PenStyle->currentItem());
  border4BrushColor = border3BrushColor;
  buttonPixmap->fill(border4BrushColor);
  border4BrushColorButton->setPixmap(*buttonPixmap);
  border4BrushStyle->setCurrentItem(border3BrushStyle->currentItem());
}

void ConfigDrawElement::__defaultPen( QList<QPen> &penList,
                                      bool *b,
                                      QColor defaultColor1,
                                      QColor defaultColor2,
                                      QColor defaultColor3,
                                      QColor defaultColor4,
                                      int defaultPenSize1,
                                      int defaultPenSize2,
                                      int defaultPenSize3,
                                      int defaultPenSize4 )
{
  penList[0].setColor(defaultColor1);
  penList[0].setWidth(defaultPenSize1);
  penList[1].setColor(defaultColor2);
  penList[1].setWidth(defaultPenSize2);
  penList[2].setColor(defaultColor3);
  penList[2].setWidth(defaultPenSize3);
  penList[3].setColor(defaultColor4);
  penList[3].setWidth(defaultPenSize4);

  b[0] = true;
  b[1] = true;
  b[2] = true;
  b[3] = true;
}

void ConfigDrawElement::__defaultPenBrush( QList<QPen> &penList,
                                           bool *b,
                                           QList<QBrush> &brushList,
                                           QColor defaultColor1,
                                           QColor defaultColor2,
                                           QColor defaultColor3,
                                           QColor defaultColor4,
                                           int defaultPenSize1,
                                           int defaultPenSize2,
                                           int defaultPenSize3,
                                           int defaultPenSize4,
                                           QColor defaultBrushColor1,
                                           QColor defaultBrushColor2,
                                           QColor defaultBrushColor3,
                                           QColor defaultBrushColor4,
                                           Qt::BrushStyle defaultBrushStyle1,
                                           Qt::BrushStyle defaultBrushStyle2,
                                           Qt::BrushStyle defaultBrushStyle3,
                                           Qt::BrushStyle defaultBrushStyle4 )
{
  __defaultPen( penList, b, defaultColor1, defaultColor2, defaultColor3, defaultColor4,
                defaultPenSize1, defaultPenSize2, defaultPenSize3, defaultPenSize4);

  brushList[0].setColor(defaultBrushColor1);
  brushList[0].setStyle(defaultBrushStyle1);
  brushList[1].setColor(defaultBrushColor2);
  brushList[1].setStyle(defaultBrushStyle2);
  brushList[2].setColor(defaultBrushColor3);
  brushList[2].setStyle(defaultBrushStyle3);
  brushList[3].setColor(defaultBrushColor4);
  brushList[3].setStyle(defaultBrushStyle4);
}

// Qt::PenStyle-Enum starts with NoPen = 0, therefor we reduce the
// value by 1. We must use the same order as Qt::PenStyle.
// Qt::BrushStyle "NoBrush" is allowed ...
void ConfigDrawElement::__fillStyle( QComboBox *pen, QComboBox *brush )
{
  QPixmap pixmap = _mainWindow->getPixmap("solid.png");
  pen->setIconSize( pixmap.size() );

  pen->addItem(_mainWindow->getPixmap("solid.png"), "", Qt::SolidLine);
  pen->addItem(_mainWindow->getPixmap("dashed.png"), "", Qt::DashLine);
  pen->addItem(_mainWindow->getPixmap("dotted.png"), "", Qt::DotLine);
  pen->addItem(_mainWindow->getPixmap("dashdot.png"), "", Qt::DashDotLine);
  pen->addItem(_mainWindow->getPixmap("dashdotdot.png"), "", Qt::DashDotDotLine);

  pixmap = _mainWindow->getPixmap("brush0.png");
  brush->setIconSize( pixmap.size() );

  brush->addItem( tr("none"), Qt::NoBrush);
  brush->addItem(_mainWindow->getPixmap("brush0.png"), "", Qt::SolidPattern);
  brush->addItem(_mainWindow->getPixmap("brush1.png"), "", Qt::Dense1Pattern);
  brush->addItem(_mainWindow->getPixmap("brush2.png"), "", Qt::Dense2Pattern);
  brush->addItem(_mainWindow->getPixmap("brush3.png"), "", Qt::Dense3Pattern);
  brush->addItem(_mainWindow->getPixmap("brush4.png"), "", Qt::Dense4Pattern);
  brush->addItem(_mainWindow->getPixmap("brush5.png"), "", Qt::Dense5Pattern);
  brush->addItem(_mainWindow->getPixmap("brush6.png"), "", Qt::Dense6Pattern);
  brush->addItem(_mainWindow->getPixmap("brush7.png"), "", Qt::Dense7Pattern);
  brush->addItem(_mainWindow->getPixmap("brush8.png"), "", Qt::HorPattern);
  brush->addItem(_mainWindow->getPixmap("brush9.png"), "", Qt::VerPattern);
  brush->addItem(_mainWindow->getPixmap("brush10.png"), "", Qt::CrossPattern);
  brush->addItem(_mainWindow->getPixmap("brush11.png"), "", Qt::BDiagPattern);
  brush->addItem(_mainWindow->getPixmap("brush12.png"), "", Qt::FDiagPattern);
  brush->addItem(_mainWindow->getPixmap("brush13.png"), "", Qt::DiagCrossPattern);
}

void ConfigDrawElement::__readBorder( QString group, bool *array )
{
  array[0] = _settings.value("/Map/"+group+"/Border1", true).toBool();
  array[1] = _settings.value("/Map/"+group+"/Border2", true).toBool();
  array[2] = _settings.value("/Map/"+group+"/Border3", true).toBool();
  array[3] = _settings.value("/Map/"+group+"/Border4", true).toBool();
}

void ConfigDrawElement::__readPen( QString group,
                                   QList<QPen> &penList,
                                   QColor defaultColor1,
                                   QColor defaultColor2,
                                   QColor defaultColor3,
                                   QColor defaultColor4,
                                   int defaultPenSize1,
                                   int defaultPenSize2,
                                   int /*defaultPenSize3*/,
                                   int /*defaultPenSize4*/,
                                   Qt::PenStyle defaultPenStyle1,
                                   Qt::PenStyle defaultPenStyle2,
                                   Qt::PenStyle defaultPenStyle3,
                                   Qt::PenStyle defaultPenStyle4 )
{
  QColor c1( _settings.value( "/Map/" + group + "/Color1", defaultColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + "/Color2", defaultColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + "/Color3", defaultColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + "/Color4", defaultColor4.name() ).value<QColor>() );

  int ps1 = _settings.value( "/Map/" + group + "/PenSize1", defaultPenSize1 ).toInt();
  int ps2 = _settings.value( "/Map/" + group + "/PenSize2", defaultPenSize2 ).toInt();
  int ps3 = _settings.value( "/Map/" + group + "/PenSize3", defaultPenSize2 ).toInt();
  int ps4 = _settings.value( "/Map/" + group + "/PenSize4", defaultPenSize2 ).toInt();

  Qt::PenStyle pst1 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle1", defaultPenStyle1).toInt());
  Qt::PenStyle pst2 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle2", defaultPenStyle2).toInt());
  Qt::PenStyle pst3 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle3", defaultPenStyle3).toInt());
  Qt::PenStyle pst4 = static_cast<Qt::PenStyle> (_settings.value("/Map/" + group + "/PenStyle4", defaultPenStyle4).toInt());

  QPen pen1(c1);
  QPen pen2(c2);
  QPen pen3(c3);
  QPen pen4(c4);

  pen1.setWidth( ps1 );
  pen2.setWidth( ps2 );
  pen3.setWidth( ps3 );
  pen4.setWidth( ps4 );

  pen1.setStyle( pst1 );
  pen2.setStyle( pst2 );
  pen3.setStyle( pst3 );
  pen4.setStyle( pst4 );

  penList.append( pen1 );
  penList.append( pen2 );
  penList.append( pen3 );
  penList.append( pen4 );
}

void ConfigDrawElement::__readBrush( QString group,
                                     QList<QBrush> &brushList,
                                     QColor defaultBrushColor1,
                                     QColor defaultBrushColor2,
                                     QColor defaultBrushColor3,
                                     QColor defaultBrushColor4,
                                     Qt::BrushStyle defaultBrushStyle1,
                                     Qt::BrushStyle defaultBrushStyle2,
                                     Qt::BrushStyle defaultBrushStyle3,
                                     Qt::BrushStyle defaultBrushStyle4 )
{
  QColor c1( _settings.value( "/Map/" + group + "/BrushColor1", defaultBrushColor1.name() ).value<QColor>() );
  QColor c2( _settings.value( "/Map/" + group + "/BrushColor2", defaultBrushColor2.name() ).value<QColor>() );
  QColor c3( _settings.value( "/Map/" + group + "/BrushColor3", defaultBrushColor3.name() ).value<QColor>() );
  QColor c4( _settings.value( "/Map/" + group + "/BrushColor4", defaultBrushColor4.name() ).value<QColor>() );

  Qt::BrushStyle bs1 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle1", defaultBrushStyle1 ).toInt();
  Qt::BrushStyle bs2 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle2", defaultBrushStyle2 ).toInt();
  Qt::BrushStyle bs3 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle3", defaultBrushStyle3 ).toInt();
  Qt::BrushStyle bs4 = (Qt::BrushStyle) _settings.value( "/Map/" + group + "/BrushStyle4", defaultBrushStyle4 ).toInt();

  brushList.append( QBrush( c1, bs1 ) );
  brushList.append( QBrush( c2, bs2 ) );
  brushList.append( QBrush( c3, bs3 ) );
  brushList.append( QBrush( c4, bs4 ) );
}

void ConfigDrawElement::__saveBrush(QList<QBrush> &brushList)
{
  brushList[0].setColor(border1BrushColor);
  brushList[0].setStyle((Qt::BrushStyle) border1BrushStyle->currentIndex());
  brushList[1].setColor(border2BrushColor);
  brushList[1].setStyle((Qt::BrushStyle) border2BrushStyle->currentIndex());
  brushList[2].setColor(border3BrushColor);
  brushList[2].setStyle((Qt::BrushStyle) border3BrushStyle->currentIndex());
  brushList[3].setColor(border4BrushColor);
  brushList[3].setStyle((Qt::BrushStyle) border4BrushStyle->currentIndex());
}

void ConfigDrawElement::__savePen(QList<QPen> &penList, bool *b)
{
  b[0] = border1->isChecked();
  penList[0].setColor(border1Color);
  penList[0].setWidth(border1Pen->value());
  penList[0].setStyle((Qt::PenStyle) (border1PenStyle->currentIndex() + 1));

  b[1] = border2->isChecked();
  penList[1].setColor(border2Color);
  penList[1].setWidth(border2Pen->value());
  penList[1].setStyle((Qt::PenStyle) (border2PenStyle->currentIndex() + 1));

  b[2] = border3->isChecked();
  penList[2].setColor(border3Color);
  penList[2].setWidth(border3Pen->value());
  penList[2].setStyle((Qt::PenStyle) (border3PenStyle->currentIndex() + 1));

  b[3] = border4->isChecked();
  penList[3].setColor(border4Color);
  penList[3].setWidth(border4Pen->value());
  penList[3].setStyle((Qt::PenStyle) (border4PenStyle->currentIndex() + 1));
}

void ConfigDrawElement::__showBrush(QList<QBrush> &brushList)
{
  QPixmap buttonPixmap(45, 14);

  border1BrushColor = brushList[0].color();
  buttonPixmap.fill(border1BrushColor);
  border1BrushColorButton->setPixmap(buttonPixmap);
  border1BrushStyle->setCurrentItem(brushList[0].style());

  border2BrushColor = brushList[1].color();
  buttonPixmap.fill(border1BrushColor);
  border2BrushColorButton->setPixmap(buttonPixmap);
  border2BrushStyle->setCurrentItem(brushList[1].style());

  border3BrushColor = brushList[2].color();
  buttonPixmap.fill(border3BrushColor);
  border3BrushColorButton->setPixmap(buttonPixmap);
  border3BrushStyle->setCurrentItem(brushList[2].style());

  border4BrushColor = brushList[3].color();
  buttonPixmap.fill(border4BrushColor);
  border4BrushColorButton->setPixmap(buttonPixmap);
  border4BrushStyle->setCurrentItem(brushList[3].style());
}

void ConfigDrawElement::__showPen(QList<QPen> &penList, bool *b)
{
  QPixmap buttonPixmap(45, 14);

  border1->setChecked(b[0]);
  border1Color = penList[0].color();
  buttonPixmap.fill(border1Color);
  border1ColorButton->setPixmap(buttonPixmap);
  border1Pen->setValue(penList[0].width());
  border1PenStyle->setCurrentItem(penList[0].style() - 1);

  border2->setChecked(b[1]);
  border2Color = penList[1].color();
  buttonPixmap.fill(border2Color);
  border2ColorButton->setPixmap(buttonPixmap);
  border2Pen->setValue(penList[1].width());
  border2PenStyle->setCurrentItem(penList[1].style() - 1);

  border3->setChecked(b[2]);
  border3Color = penList[2].color();
  buttonPixmap.fill(border3Color);
  border3ColorButton->setPixmap(buttonPixmap);
  border3Pen->setValue(penList[2].width());
  border3PenStyle->setCurrentItem(penList[2].style() - 1);

  border4->setChecked(b[3]);
  border4Color = penList[3].color();
  buttonPixmap.fill(border4Color);
  border4ColorButton->setPixmap(buttonPixmap);
  border4Pen->setValue(penList[3].width());
  border4PenStyle->setCurrentItem(penList[3].style() - 1);
}

void ConfigDrawElement::__writeBrush(QString group, QList<QBrush> &brushList, QList<QPen> &penList, bool *b)
{
  __writePen(group, penList, b);
  _settings.setValue("/Map/" + group + "/BrushColor1", brushList.at(0).color().name());
  _settings.setValue("/Map/" + group + "/BrushColor2", brushList.at(1).color().name());
  _settings.setValue("/Map/" + group + "/BrushColor3", brushList.at(2).color().name());
  _settings.setValue("/Map/" + group + "/BrushColor4", brushList.at(3).color().name());

  _settings.setValue("/Map/" + group + "/BrushStyle1", brushList.at(0).style());
  _settings.setValue("/Map/" + group + "/BrushStyle2", brushList.at(1).style());
  _settings.setValue("/Map/" + group + "/BrushStyle3", brushList.at(2).style());
  _settings.setValue("/Map/" + group + "/BrushStyle4", brushList.at(3).style());
}

void ConfigDrawElement::__writePen(QString group, QList<QPen> &penList, bool *b)
{
  _settings.setValue("/Map/" + group + "/Color1", penList.at(0).color().name());
  _settings.setValue("/Map/" + group + "/Color2", penList.at(1).color().name());
  _settings.setValue("/Map/" + group + "/Color3", penList.at(2).color().name());
  _settings.setValue("/Map/" + group + "/Color4", penList.at(3).color().name());
  _settings.setValue("/Map/" + group + "/PenSize1", (int) penList.at(0).width());
  _settings.setValue("/Map/" + group + "/PenSize2", (int) penList.at(1).width());
  _settings.setValue("/Map/" + group + "/PenSize3", (int) penList.at(2).width());
  _settings.setValue("/Map/" + group + "/PenSize4", (int) penList.at(3).width());
  _settings.setValue("/Map/" + group + "/PenStyle1", penList.at(0).style());
  _settings.setValue("/Map/" + group + "/PenStyle2", penList.at(1).style());
  _settings.setValue("/Map/" + group + "/PenStyle3", penList.at(2).style());
  _settings.setValue("/Map/" + group + "/PenStyle4", penList.at(3).style());
  _settings.setValue("/Map/" + group + "/Border1", b[0]);
  _settings.setValue("/Map/" + group + "/Border2", ( b[1] && b[0] ) );
  _settings.setValue("/Map/" + group + "/Border3", ( b[2] && b[1] && b[0] ) );
  _settings.setValue("/Map/" + group + "/Border4", ( b[3] && b[2] && b[1] && b[0] ) );
}
