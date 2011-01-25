/***********************************************************************
**
**   configprintelement.cpp
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
#include <Qt3Support>

#include "configprintelement.h"
#include "kflogconfig.h"
#include "mapdefaults.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;
extern QSettings _settings;

ConfigPrintElement::ConfigPrintElement( QWidget* parent ) :
  QWidget(parent),
  oldElement(-1)
{
  airABorder = new bool[2];
  airBBorder = new bool[2];
  airCBorder = new bool[2];
  airDBorder = new bool[2];
  airElBorder = new bool[2];
  airEhBorder = new bool[2];
  airFBorder = new bool[2];
  ctrCBorder = new bool[2];
  ctrDBorder = new bool[2];
  lowFBorder = new bool[2];
  dangerBorder = new bool[2];
  restrBorder = new bool[2];
  tmzBorder = new bool[2];

  roadBorder = new bool[2];
  trailBorder = new bool[2];
  highwayBorder = new bool[2];
  railBorder = new bool[2];
  rail_dBorder = new bool[2];
  riverBorder = new bool[2];
  river_tBorder = new bool[2];
  cityBorder = new bool[2];
  forestBorder = new bool[2];
  glacierBorder = new bool[2];
  packiceBorder = new bool[2];
  canalBorder = new bool[2];
  aerialcableBorder = new bool[2];

  faiAreaLow500Border = new bool[2];
  faiAreaHigh500Border = new bool[2];

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
  trailPenList.setAutoDelete(true);
  highwayPenList.setAutoDelete(true);
  riverPenList.setAutoDelete(true);
  river_tPenList.setAutoDelete(true);
  river_tBrushList.setAutoDelete(true);
  railPenList.setAutoDelete(true);
  rail_dPenList.setAutoDelete(true);
  cityPenList.setAutoDelete(true);
  forestPenList.setAutoDelete(true);
  forestBrushList.setAutoDelete(true);
  glacierPenList.setAutoDelete(true);
  glacierBrushList.setAutoDelete(true);
  packicePenList.setAutoDelete(true);
  packiceBrushList.setAutoDelete(true);
  canalPenList.setAutoDelete(true);
  aerialcablePenList.setAutoDelete(true);

  faiAreaLow500PenList.setAutoDelete(true);
  faiAreaLow500BrushList.setAutoDelete(true);
  faiAreaHigh500PenList.setAutoDelete(true);
  faiAreaHigh500BrushList.setAutoDelete(true);

  __readPen("Road", &roadPenList, roadBorder,
        PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2);

  __readPen("Trail", &trailPenList, trailBorder,
        PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2,
        PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2,
        PRINT_TRAIL_PEN_STYLE_1, PRINT_TRAIL_PEN_STYLE_2);

  __readPen("River", &riverPenList, riverBorder,
        PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
        PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2);

  __readPen("Canal", &canalPenList, canalBorder,
           PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2,
           PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2,
           PRINT_CANAL_PEN_STYLE_1, PRINT_CANAL_PEN_STYLE_2);

  __readPen("Rail", &railPenList, railBorder,
        PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2);

  __readPen("Rail_D", &rail_dPenList, rail_dBorder,
        PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2,
        PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2,
        PRINT_RAIL_D_PEN_STYLE_1, PRINT_RAIL_D_PEN_STYLE_2);

  __readPen("Aerial Cable", &aerialcablePenList, aerialcableBorder,
           PRINT_AERIAL_CABLE_COLOR_1, PRINT_AERIAL_CABLE_COLOR_2,
           PRINT_AERIAL_CABLE_PEN_1, PRINT_AERIAL_CABLE_PEN_2,
           PRINT_AERIAL_CABLE_PEN_STYLE_1, PRINT_AERIAL_CABLE_PEN_STYLE_2);

  __readPen("Highway", &highwayPenList, highwayBorder,
        PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2);

  // PenStyle and BrushStyle are not used for cities ...
  __readPenBrush("City", &cityPenList, cityBorder, &cityBrushList,
        PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2,
        PRINT_CITY_PEN_1, PRINT_CITY_PEN_2,
        Qt::SolidLine, Qt::SolidLine,
        PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
        Qt::SolidPattern, Qt::SolidPattern);

  __readPenBrush("River_T", &river_tPenList, river_tBorder, &river_tBrushList,
                 PRINT_RIVER_T_COLOR_1, PRINT_RIVER_T_COLOR_2,
                 PRINT_RIVER_T_PEN_1, PRINT_RIVER_T_PEN_2,
                 PRINT_RIVER_T_PEN_STYLE_1, PRINT_RIVER_T_PEN_STYLE_2,
                 PRINT_RIVER_T_BRUSH_COLOR_1, PRINT_RIVER_T_BRUSH_COLOR_2,
                 PRINT_RIVER_T_BRUSH_STYLE_1, PRINT_RIVER_T_BRUSH_STYLE_2);

  __readPenBrush("Forest", &forestPenList, forestBorder, &forestBrushList,
        PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2,
        PRINT_FRST_PEN_1, PRINT_FRST_PEN_2,
        PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2,
        PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
        PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2);

  __readPenBrush("Glacier", &glacierPenList, glacierBorder, &glacierBrushList,
        PRINT_GLACIER_COLOR_1, PRINT_GLACIER_COLOR_2,
        PRINT_GLACIER_PEN_1, PRINT_GLACIER_PEN_2,
        PRINT_GLACIER_PEN_STYLE_1, PRINT_GLACIER_PEN_STYLE_2,
        PRINT_GLACIER_BRUSH_COLOR_1, PRINT_GLACIER_BRUSH_COLOR_2,
        PRINT_GLACIER_BRUSH_STYLE_1, PRINT_GLACIER_BRUSH_STYLE_2);

  __readPenBrush("Pack Ice", &packicePenList, packiceBorder, &packiceBrushList,
        PRINT_PACK_ICE_COLOR_1, PRINT_PACK_ICE_COLOR_2,
        PRINT_PACK_ICE_PEN_1, PRINT_PACK_ICE_PEN_2,
        PRINT_PACK_ICE_PEN_STYLE_1, PRINT_PACK_ICE_PEN_STYLE_2,
        PRINT_PACK_ICE_BRUSH_COLOR_1, PRINT_PACK_ICE_BRUSH_COLOR_2,
        PRINT_PACK_ICE_BRUSH_STYLE_1, PRINT_PACK_ICE_BRUSH_STYLE_2);

  __readPenBrush("Airspace A", &airAPenList, airABorder, &airABrushList,
        PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2,
        PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2,
        PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2,
        PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
        PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2);

  __readPenBrush("Airspace B", &airBPenList, airBBorder, &airBBrushList,
        PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2,
        PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2,
        PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2,
        PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
        PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2);

  __readPenBrush("Airspace C", &airCPenList, airCBorder, &airCBrushList,
        PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2,
        PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2,
        PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2,
        PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
        PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2);

  __readPenBrush("Airspace D", &airDPenList, airDBorder, &airDBrushList,
        PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2,
        PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2,
        PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2,
        PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
        PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2);

  __readPenBrush("Airspace E low", &airElPenList, airElBorder, &airElBrushList,
        PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2,
        PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2,
        PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2,
        PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
        PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2);

  __readPenBrush("Airspace E high", &airEhPenList, airEhBorder, &airEhBrushList,
        PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2,
        PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2,
        PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2,
        PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
        PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2);

  __readPenBrush("Airspace F", &airFPenList, airFBorder, &airFBrushList,
        PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2,
        PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2,
        PRINT_AIRF_PEN_STYLE_1, PRINT_AIRF_PEN_STYLE_2,
        PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
        PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2);

  __readPenBrush("Control C", &ctrCPenList, ctrCBorder, &ctrCBrushList,
        PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2,
        PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2,
        PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2,
        PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
        PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2);

  __readPenBrush("Control D", &ctrDPenList, ctrDBorder, &ctrDBrushList,
        PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2,
        PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2,
        PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2,
        PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
        PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2);

  __readPenBrush("Danger", &dangerPenList, dangerBorder, &dangerBrushList,
        PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2,
        PRINT_DNG_PEN_1, PRINT_DNG_PEN_2,
        PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2,
        PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
        PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2);

  __readPenBrush("Low Flight", &lowFPenList, lowFBorder, &lowFBrushList,
        PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2,
        PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2,
        PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2,
        PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
        PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2);

  __readPenBrush("Restricted Area", &restrPenList, restrBorder, &restrBrushList,
        PRINT_RES_COLOR_1, PRINT_RES_COLOR_2,
        PRINT_RES_PEN_1, PRINT_RES_PEN_2,
        PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2,
        PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
        PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2);

  __readPenBrush("TMZ", &tmzPenList, tmzBorder, &tmzBrushList,
        PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2,
        PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2,
        PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2,
        PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
        PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2);

  __readPenBrush("FAIAreaLow500", &faiAreaLow500PenList, faiAreaLow500Border, &faiAreaLow500BrushList,
        PRINT_FAI_LOW_500_COLOR_1, PRINT_FAI_LOW_500_COLOR_2,
        PRINT_FAI_LOW_500_PEN_1, PRINT_FAI_LOW_500_PEN_2,
        PRINT_FAI_LOW_500_PEN_STYLE_1, PRINT_FAI_LOW_500_PEN_STYLE_2,
        PRINT_FAI_LOW_500_BRUSH_COLOR_1, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
        PRINT_FAI_LOW_500_BRUSH_STYLE_1, PRINT_FAI_LOW_500_BRUSH_STYLE_2);

  __readPenBrush("FAIAreaHigh500", &faiAreaHigh500PenList, faiAreaHigh500Border, &faiAreaHigh500BrushList,
        PRINT_FAI_HIGH_500_COLOR_1, PRINT_FAI_HIGH_500_COLOR_2,
        PRINT_FAI_HIGH_500_PEN_1, PRINT_FAI_HIGH_500_PEN_2,
        PRINT_FAI_HIGH_500_PEN_STYLE_1, PRINT_FAI_HIGH_500_PEN_STYLE_2,
        PRINT_FAI_HIGH_500_BRUSH_COLOR_1, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
        PRINT_FAI_HIGH_500_BRUSH_STYLE_1, PRINT_FAI_HIGH_500_BRUSH_STYLE_2);

  border1 = new QCheckBox(tr("1:500.000"), this);
  border2 = new QCheckBox(tr("scale-limit"), this);

  border1Button = new QPushButton(this);
  border1Button-> setPixmap(_mainWindow->getPixmap("kde_down.png"));
  border1Button-> setFixedWidth(30);
  border1Button-> setFixedHeight(30);

  Q3GridLayout* elLayout = new Q3GridLayout(this, 10, 15, 5, 1);
  elLayout-> addWidget(new QLabel(tr("draw up to"), this), 1, 1);
  elLayout-> addMultiCellWidget(new QLabel(tr("Pen"), this), 1, 1, 3, 7);
  elLayout-> addMultiCellWidget(new QLabel(tr("Brush"), this), 1, 1, 9, 11);
  elLayout-> addWidget(border1, 3, 1);
  elLayout-> addWidget(border1Button, 3, 13);
  elLayout-> addWidget(border2, 5, 1);

  border1ColorButton = new QPushButton(this);
  border1ColorButton->setFixedHeight(24);
  border1ColorButton->setFixedWidth(55);
  connect(border1ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1Color()));
  border1Pen = new QSpinBox(1, 9, 1, this);
  border1Pen-> setMinimumWidth(35);
  border1PenStyle = new QComboBox(this);
  border1PenStyle-> setMinimumWidth(35);
  border1BrushColorButton = new QPushButton(this);
  border1BrushColorButton->setFixedHeight(24);
  border1BrushColorButton->setFixedWidth(55);
  connect(border1BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder1BrushColor()));
  border1BrushStyle = new QComboBox(this);
  border1BrushStyle-> setMinimumWidth(35);
  __fillStyle(border1PenStyle, border1BrushStyle);
  elLayout->addWidget(border1ColorButton, 3, 3);
  elLayout->addWidget(border1Pen, 3, 5);
  elLayout->addWidget(border1PenStyle, 3, 7);
  elLayout->addWidget(border1BrushColorButton, 3, 9);
  elLayout->addWidget(border1BrushStyle, 3, 11);

  border2ColorButton = new QPushButton(this);
  border2ColorButton->setFixedHeight(24);
  border2ColorButton->setFixedWidth(55);
  connect(border2ColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2Color()));
  border2Pen = new QSpinBox(1, 9, 1, this);
  border2Pen-> setMinimumWidth(35);
  border2PenStyle = new QComboBox(this);
  border2PenStyle-> setMinimumWidth(35);
  border2BrushColorButton = new QPushButton(this);
  border2BrushColorButton->setFixedHeight(24);
  border2BrushColorButton->setFixedWidth(55);
  connect(border2BrushColorButton, SIGNAL(clicked()), this, SLOT(slotSelectBorder2BrushColor()));
  border2BrushStyle = new QComboBox(this);
  border2BrushStyle-> setMinimumWidth(35);
  __fillStyle(border2PenStyle, border2BrushStyle);
  elLayout-> addWidget(border2ColorButton, 5, 3);
  elLayout-> addWidget(border2Pen, 5, 5);
  elLayout-> addWidget(border2PenStyle, 5, 7);
  elLayout-> addWidget(border2BrushColorButton, 5, 9);
  elLayout-> addWidget(border2BrushStyle, 5, 11);

  elLayout-> addRowSpacing(0, 2);
  elLayout-> addRowSpacing(2, 5);
  elLayout-> setRowStretch(2, 1);
  elLayout-> setRowStretch(4, 1);
  elLayout-> setRowStretch(6, 1);
  elLayout-> setRowStretch(8, 1);
  elLayout-> addRowSpacing(10, 5);

  elLayout-> addColSpacing(0, 10);
//  elLayout-> setColStretch(2, 1);
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
  connect(border1Button, SIGNAL(clicked()), SLOT(slotSetSecond()));
}

ConfigPrintElement::~ConfigPrintElement()
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

void ConfigPrintElement::slotOk()
{
  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

  __writePen("Road", &roadPenList, roadBorder);

  __writePen("Trail", &trailPenList, trailBorder);

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

void ConfigPrintElement::slotDefaultElements()
{
  __defaultPen(&roadPenList, roadBorder,
      PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
      PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
      PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2);

  __defaultPen(&trailPenList, trailBorder,
      PRINT_TRAIL_COLOR_1, PRINT_TRAIL_COLOR_2,
      PRINT_TRAIL_PEN_1, PRINT_TRAIL_PEN_2,
      PRINT_TRAIL_PEN_STYLE_1, PRINT_TRAIL_PEN_STYLE_2);

  __defaultPen(&highwayPenList, highwayBorder,
      PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
      PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
      PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2);

  __defaultPen(&riverPenList, riverBorder,
      PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
      PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
      PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2);

  __defaultPen(&canalPenList, canalBorder,
      PRINT_CANAL_COLOR_1, PRINT_CANAL_COLOR_2,
      PRINT_CANAL_PEN_1, PRINT_CANAL_PEN_2,
      PRINT_CANAL_PEN_STYLE_1, PRINT_CANAL_PEN_STYLE_2);

  __defaultPen(&railPenList, railBorder,
      PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
      PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
      PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2);

  __defaultPen(&rail_dPenList, rail_dBorder,
      PRINT_RAIL_D_COLOR_1, PRINT_RAIL_D_COLOR_2,
      PRINT_RAIL_D_PEN_1, PRINT_RAIL_D_PEN_2,
      PRINT_RAIL_D_PEN_STYLE_1, PRINT_RAIL_D_PEN_STYLE_2);

  __defaultPenBrush(&cityPenList, cityBorder, &cityBrushList,
      PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2,
      PRINT_CITY_PEN_1, PRINT_CITY_PEN_2,
      Qt::SolidLine, Qt::SolidLine,
      PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
      PRINT_CITY_BRUSH_STYLE_1, PRINT_CITY_BRUSH_STYLE_2);

  __defaultPenBrush(&river_tPenList, river_tBorder, &river_tBrushList,
      PRINT_RIVER_T_COLOR_1, PRINT_RIVER_T_COLOR_2,
      PRINT_RIVER_T_PEN_1, PRINT_RIVER_T_PEN_2,
      PRINT_RIVER_T_PEN_STYLE_1, PRINT_RIVER_T_PEN_STYLE_2,
      PRINT_RIVER_T_BRUSH_COLOR_1, PRINT_RIVER_T_BRUSH_COLOR_2,
      PRINT_RIVER_T_BRUSH_STYLE_1, PRINT_RIVER_T_BRUSH_STYLE_2);

  __defaultPenBrush(&forestPenList, forestBorder, &forestBrushList,
      PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2,
      PRINT_FRST_PEN_1, PRINT_FRST_PEN_2,
      PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2,
      PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
      PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2);

  __defaultPenBrush(&airAPenList, airABorder, &airABrushList,
      PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2,
      PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2,
      PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2,
      PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
      PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2);

  __defaultPenBrush(&airBPenList, airBBorder, &airBBrushList,
      PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2,
      PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2,
      PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2,
      PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
      PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2);

  __defaultPenBrush(&airCPenList, airCBorder, &airCBrushList,
      PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2,
      PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2,
      PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2,
      PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
      PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2);

  __defaultPenBrush(&airDPenList, airDBorder, &airDBrushList,
      PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2,
      PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2,
      PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2,
      PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
      PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2);

  __defaultPenBrush(&airElPenList, airElBorder, &airElBrushList,
      PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2,
      PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2,
      PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2,
      PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
      PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2);

  __defaultPenBrush(&airEhPenList, airEhBorder, &airEhBrushList,
      PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2,
      PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2,
      PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2,
      PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
      PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2);

  __defaultPenBrush(&airFPenList, airFBorder, &airFBrushList,
      PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2,
      PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2,
      PRINT_AIRF_PEN_STYLE_1, PRINT_AIRF_PEN_STYLE_2,
      PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
      PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2);

  __defaultPenBrush(&ctrCPenList, ctrCBorder, &ctrCBrushList,
      PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2,
      PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2,
      PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2,
      PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
      PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2);

  __defaultPenBrush(&ctrDPenList, ctrDBorder, &ctrDBrushList,
      PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2,
      PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2,
      PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2,
      PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
      PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2);

  __defaultPenBrush(&dangerPenList, dangerBorder, &dangerBrushList,
      PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2,
      PRINT_DNG_PEN_1, PRINT_DNG_PEN_2,
      PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2,
      PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
      PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2);

  __defaultPenBrush(&restrPenList, restrBorder, &restrBrushList,
      PRINT_RES_COLOR_1, PRINT_RES_COLOR_2,
      PRINT_RES_PEN_1, PRINT_RES_PEN_2,
      PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2,
      PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
      PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2);

  __defaultPenBrush(&lowFPenList, lowFBorder, &lowFBrushList,
      PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2,
      PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2,
      PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2,
      PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
      PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2);

  __defaultPenBrush(&tmzPenList, tmzBorder, &tmzBrushList,
      PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2,
      PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2,
      PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2,
      PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
      PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2);

  __defaultPenBrush(&faiAreaLow500PenList, faiAreaLow500Border, &faiAreaLow500BrushList,
      PRINT_FAI_LOW_500_COLOR_1, PRINT_FAI_LOW_500_COLOR_2,
      PRINT_FAI_LOW_500_PEN_1, PRINT_FAI_LOW_500_PEN_2,
      PRINT_FAI_LOW_500_PEN_STYLE_1, PRINT_FAI_LOW_500_PEN_STYLE_2,
      PRINT_FAI_LOW_500_BRUSH_COLOR_1, PRINT_FAI_LOW_500_BRUSH_COLOR_2,
      PRINT_FAI_LOW_500_BRUSH_STYLE_1, PRINT_FAI_LOW_500_BRUSH_STYLE_2);

  __defaultPenBrush(&faiAreaHigh500PenList, faiAreaHigh500Border, &faiAreaHigh500BrushList,
      PRINT_FAI_HIGH_500_COLOR_1, PRINT_FAI_HIGH_500_COLOR_2,
      PRINT_FAI_HIGH_500_PEN_1, PRINT_FAI_HIGH_500_PEN_2,
      PRINT_FAI_HIGH_500_PEN_STYLE_1, PRINT_FAI_HIGH_500_PEN_STYLE_2,
      PRINT_FAI_HIGH_500_BRUSH_COLOR_1, PRINT_FAI_HIGH_500_BRUSH_COLOR_2,
      PRINT_FAI_HIGH_500_BRUSH_STYLE_1, PRINT_FAI_HIGH_500_BRUSH_STYLE_2);


  oldElement = -1;
  slotSelectElement(currentElement);
}

void ConfigPrintElement::slotSelectBorder1Color()
{
  border1Color = QColorDialog::getColor(border1Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border1Color);
  border1ColorButton->setPixmap(*buttonPixmap);
}

void ConfigPrintElement::slotSelectBorder2Color()
{
  border2Color = QColorDialog::getColor(border2Color, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border2Color);
  border2ColorButton->setPixmap(*buttonPixmap);
}

void ConfigPrintElement::slotSelectBorder1BrushColor()
{
  border1BrushColor = QColorDialog::getColor(border1BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border1BrushColor);
  border1BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigPrintElement::slotSelectBorder2BrushColor()
{
  border2BrushColor = QColorDialog::getColor(border2BrushColor, this);
  QPixmap *buttonPixmap = new QPixmap();
  buttonPixmap->resize(45, 14);
  buttonPixmap->fill(border2BrushColor);
  border2BrushColorButton->setPixmap(*buttonPixmap);
}

void ConfigPrintElement::slotSelectElement(int elementID)
{
  currentElement = elementID;

  switch(oldElement)
    {
      case KFLogConfig::Road:
        __savePen(&roadPenList, roadBorder);
        break;
      case KFLogConfig::Trail:
        __savePen(&trailPenList, trailBorder);
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
      case KFLogConfig::River:
        __savePen(&riverPenList, riverBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __savePen(&aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::Canal:
        __savePen(&canalPenList, canalBorder);
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
      case KFLogConfig::AirE:
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
      case KFLogConfig::River_T:
        __savePen(&river_tPenList, river_tBorder);
        __saveBrush(&river_tBrushList);
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
      case KFLogConfig::Road:
        __showPen(&roadPenList, roadBorder);
        break;
      case KFLogConfig::Trail:
        __showPen(&trailPenList, trailBorder);
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
      case KFLogConfig::River:
        __showPen(&riverPenList, riverBorder);
        break;
      case KFLogConfig::Aerial_Cable:
        __showPen(&aerialcablePenList, aerialcableBorder);
        break;
      case KFLogConfig::Canal:
        __showPen(&canalPenList, canalBorder);
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
      case KFLogConfig::AirE:
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
        __showPen(&forestPenList, forestBorder);
        __showBrush(&forestBrushList);
        break;
      case KFLogConfig::River_T:
        __showPen(&river_tPenList, river_tBorder);
        __showBrush(&river_tBrushList);
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

void ConfigPrintElement::slotToggleFirst(bool toggle)
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
      case KFLogConfig::AirE:
      case KFLogConfig::AirF:
      case KFLogConfig::ControlC:
      case KFLogConfig::ControlD:
      case KFLogConfig::LowFlight:
      case KFLogConfig::Danger:
      case KFLogConfig::Restricted:
      case KFLogConfig::Glacier:
      case KFLogConfig::PackIce:
      case KFLogConfig::Tmz:
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
      case KFLogConfig::River_T:
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

void ConfigPrintElement::slotToggleSecond(bool toggle)
{
  border2Pen-> setEnabled(toggle);
  border2ColorButton-> setEnabled(toggle);

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
      case KFLogConfig::FAIAreaLow500:
      case KFLogConfig::FAIAreaHigh500:
      case KFLogConfig::River_T:
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
}

void ConfigPrintElement::slotSetSecond()
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

void ConfigPrintElement::__defaultPen(Q3PtrList<QPen> *penList, bool *b,
    QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2)
{
  penList->at(0)-> setColor(defaultColor1);
  penList->at(0)-> setWidth(defaultPenSize1);
  penList->at(0)-> setStyle(defaultPenStyle1);
  penList->at(1)-> setColor(defaultColor2);
  penList->at(1)-> setWidth(defaultPenSize2);
  penList->at(1)-> setStyle(defaultPenStyle2);
  b[0] = true;
  b[1] = true;
}

void ConfigPrintElement::__defaultPenBrush(Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
    QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
    Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2)
{
  __defaultPen(penList, b, defaultColor1, defaultColor2, defaultPenSize1, defaultPenSize2, defaultPenStyle1, defaultPenStyle2);
  brushList->at(0)->setColor(defaultBrushColor1);
  brushList->at(0)->setStyle(defaultBrushStyle1);
  brushList->at(1)->setColor(defaultBrushColor2);
  brushList->at(1)->setStyle(defaultBrushStyle2);
}

// Qt::PenStyle-Enum starts with NoPen = 0, therefore we reduce the
// value by 1. We must use the same order as Qt::PenStyle.
// Qt::BrushStyle "NoBrush" is allowed ...
void ConfigPrintElement::__fillStyle(QComboBox *pen, QComboBox *brush)
{
  pen-> insertItem(_mainWindow->getPixmap("solid.png"), Qt::SolidLine - 1);
  pen-> insertItem(_mainWindow->getPixmap("dashed.png"), Qt::DashLine - 1);
  pen-> insertItem(_mainWindow->getPixmap("dotted.png"), Qt::DotLine - 1);
  pen-> insertItem(_mainWindow->getPixmap("dashdot.png"), Qt::DashDotLine - 1);
  pen-> insertItem(_mainWindow->getPixmap("dashdotdot.png"), Qt::DashDotDotLine - 1);
  brush-> insertItem("no", Qt::NoBrush);
  brush-> insertItem(_mainWindow->getPixmap("brush0.png"), Qt::SolidPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush1.png"), Qt::Dense1Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush2.png"), Qt::Dense2Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush3.png"), Qt::Dense3Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush4.png"), Qt::Dense4Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush5.png"), Qt::Dense5Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush6.png"), Qt::Dense6Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush7.png"), Qt::Dense7Pattern);
  brush-> insertItem(_mainWindow->getPixmap("brush8.png"), Qt::HorPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush9.png"), Qt::VerPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush10.png"), Qt::CrossPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush11.png"), Qt::BDiagPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush12.png"), Qt::FDiagPattern);
  brush-> insertItem(_mainWindow->getPixmap("brush13.png"), Qt::DiagCrossPattern);
}

void ConfigPrintElement::__readBorder(QString group, bool *b)
{
  b[0] = _settings.value("/Map/"+group+"/PrintBorder1", true).toBool();
  b[1] = _settings.value("/Map/"+group+"/PrintBorder2", true).toBool();
}

void ConfigPrintElement::__readPen(QString group, Q3PtrList<QPen> *penList, bool *b,
    QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2)
{
  __readBorder(group, b);
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/PrintColor1", __color2String(defaultColor1))),
        _settings.readNumEntry("/Map/"+group+"/PrintPenSize1", defaultPenSize1),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PrintPenStyle1", defaultPenStyle1)));
  penList->append(new QPen(__string2Color(_settings.readEntry("/Map/"+group+"/PrintColor2", __color2String(defaultColor2))),
        _settings.readNumEntry("/Map/"+group+"/PrintPenSize2", defaultPenSize2),
        (Qt::PenStyle)_settings.readNumEntry("/Map/"+group+"/PrintPenStyle2", defaultPenStyle2)));
}

void ConfigPrintElement::__readPenBrush(QString group, Q3PtrList<QPen> *penList, bool *b, Q3PtrList<QBrush> *brushList,
    QColor defaultColor1, QColor defaultColor2, int defaultPenSize1, int defaultPenSize2,
    Qt::PenStyle defaultPenStyle1, Qt::PenStyle defaultPenStyle2, QColor defaultBrushColor1, QColor defaultBrushColor2,
    Qt::BrushStyle defaultBrushStyle1, Qt::BrushStyle defaultBrushStyle2)
{
  __readPen(group, penList, b, defaultColor1, defaultColor2, defaultPenSize1, defaultPenSize2,
      defaultPenStyle1, defaultPenStyle2);
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/PrintBrushColor1", __color2String(defaultBrushColor1))),
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/PrintBrushStyle1", defaultBrushStyle1)));
  brushList->append(new QBrush(__string2Color(_settings.readEntry("/Map/"+group+"/PrintBrushColor2", __color2String(defaultBrushColor2))),
        (Qt::BrushStyle)_settings.readNumEntry("/Map/"+group+"/PrintBrushStyle2", defaultBrushStyle2)));
}

void ConfigPrintElement::__saveBrush(Q3PtrList<QBrush> *brushList)
{
  brushList->at(0)->setColor(border1BrushColor);
  brushList->at(0)->setStyle((Qt::BrushStyle)border1BrushStyle->currentItem());
  brushList->at(1)->setColor(border2BrushColor);
  brushList->at(1)->setStyle((Qt::BrushStyle)border2BrushStyle->currentItem());
}

void ConfigPrintElement::__savePen(Q3PtrList<QPen> *penList, bool *b)
{
  b[0] = border1->isChecked();
  penList->at(0)->setColor(border1Color);
  penList->at(0)->setWidth(border1Pen->value());
  penList->at(0)->setStyle((Qt::PenStyle)(border1PenStyle->currentItem() + 1));
  b[1] = border2->isChecked();
  penList->at(1)->setColor(border2Color);
  penList->at(1)->setWidth(border2Pen->value());
  penList->at(1)->setStyle((Qt::PenStyle)(border2PenStyle->currentItem() + 1));
}

void ConfigPrintElement::__showBrush(Q3PtrList<QBrush> *brushList)
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
}

void ConfigPrintElement::__showPen(Q3PtrList<QPen> *penList, bool *b)
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
}

void ConfigPrintElement::__writeBrush(QString group, Q3PtrList<QBrush> *brushList, Q3PtrList<QPen> *penList, bool *b)
{
  __writePen(group, penList, b);
  _settings.setValue("/Map/"+group+"/PrintBrushColor1", __color2String(brushList->at(0)->color())); \
  _settings.setValue("/Map/"+group+"/PrintBrushColor2", __color2String(brushList->at(1)->color())); \
  _settings.setValue("/Map/"+group+"/PrintBrushStyle1", brushList->at(0)->style()); \
  _settings.setValue("/Map/"+group+"/PrintBrushStyle2", brushList->at(1)->style());
}

void ConfigPrintElement::__writePen(QString group, Q3PtrList<QPen> *penList, bool *b)
{
  _settings.setValue("/Map/"+group+"/PrintColor1", __color2String(penList->at(0)->color()));
  _settings.setValue("/Map/"+group+"/PrintColor2", __color2String(penList->at(1)->color()));
  _settings.setValue("/Map/"+group+"/PrintPenSize1", (int)penList->at(0)->width());
  _settings.setValue("/Map/"+group+"/PrintPenSize2", (int)penList->at(1)->width());
  _settings.setValue("/Map/"+group+"/PrintPenStyle1", penList->at(0)->style());
  _settings.setValue("/Map/"+group+"/PrintPenStyle2", penList->at(1)->style());
  _settings.setValue("/Map/"+group+"/PrintBorder1", b[0]);
  _settings.setValue("/Map/"+group+"/PrintBorder2", ( b[1] && b[0] ) );
}

/** this is a temporary function and it is not needed in Qt 4 */
QString ConfigPrintElement::__color2String(QColor color)
{
  QString colstr;
  colstr.sprintf("%d;%d;%d", color.red(), color.green(), color.blue());
  return colstr;
}

/** this is a temporary function and it is not needed in Qt 4 */
QColor ConfigPrintElement::__string2Color(QString colstr)
{
  QColor color(colstr.section(";", 0, 0).toInt(), colstr.section(";", 1, 1).toInt(), colstr.section(";", 2, 2).toInt());
  return color;
}
