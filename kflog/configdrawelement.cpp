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
#include <mapdefaults.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#define SAVE_PEN(a, b) \
    b[0] = border1->isChecked(); \
    a.at(0)->setColor(border1Color->color()); \
    a.at(0)->setWidth(border1Pen->value()); \
    a.at(0)->setStyle((Qt::PenStyle)(border1PenStyle->currentItem() + 1)); \
    b[1] = border2->isChecked(); \
    a.at(1)->setColor(border2Color->color()); \
    a.at(1)->setWidth(border2Pen->value()); \
    a.at(1)->setStyle((Qt::PenStyle)(border2PenStyle->currentItem() + 1)); \
    b[2] = border3->isChecked(); \
    a.at(2)->setColor(border3Color->color()); \
    a.at(2)->setWidth(border3Pen->value()); \
    a.at(2)->setStyle((Qt::PenStyle)(border3PenStyle->currentItem() + 1)); \
    b[3] = border4->isChecked(); \
    a.at(3)->setColor(border4Color->color()); \
    a.at(3)->setWidth(border4Pen->value()); \
    a.at(3)->setStyle((Qt::PenStyle)(border4PenStyle->currentItem() + 1)); \

#define SAVE_BRUSH(a) \
    a.at(0)->setColor(border1BrushColor->color()); \
    a.at(0)->setStyle((Qt::BrushStyle)border1BrushStyle->currentItem()); \
    a.at(1)->setColor(border2BrushColor->color()); \
    a.at(1)->setStyle((Qt::BrushStyle)border2BrushStyle->currentItem()); \
    a.at(2)->setColor(border3BrushColor->color()); \
    a.at(2)->setStyle((Qt::BrushStyle)border3BrushStyle->currentItem()); \
    a.at(3)->setColor(border4BrushColor->color()); \
    a.at(3)->setStyle((Qt::BrushStyle)border4BrushStyle->currentItem());

#define SHOW_PEN(a, b) \
    border1->setChecked(b[0]); \
    border1Color->setColor(a.at(0)->color()); \
    border1Pen->setValue(a.at(0)->width()); \
    border1PenStyle->setCurrentItem(a.at(0)->style() - 1); \
    border2->setChecked(b[1]); \
    border2Color->setColor(a.at(1)->color()); \
    border2Pen->setValue(a.at(1)->width()); \
    border2PenStyle->setCurrentItem(a.at(1)->style() - 1); \
    border3->setChecked(b[2]); \
    border3Color->setColor(a.at(2)->color()); \
    border3Pen->setValue(a.at(2)->width()); \
    border3PenStyle->setCurrentItem(a.at(2)->style() - 1); \
    border4->setChecked(b[3]); \
    border4Color->setColor(a.at(3)->color()); \
    border4Pen->setValue(a.at(3)->width()); \
    border4PenStyle->setCurrentItem(a.at(3)->style() - 1);

#define SHOW_BRUSH(a) \
    border1BrushColor->setColor(a.at(0)->color()); \
    border1BrushStyle->setCurrentItem(a.at(0)->style()); \
    border2BrushColor->setColor(a.at(1)->color()); \
    border2BrushStyle->setCurrentItem(a.at(1)->style()); \
    border3BrushColor->setColor(a.at(2)->color()); \
    border3BrushStyle->setCurrentItem(a.at(2)->style()); \
    border4BrushColor->setColor(a.at(3)->color()); \
    border4BrushStyle->setCurrentItem(a.at(3)->style());

#define READ_BORDER(a) \
    a[0] = config->readBoolEntry("Border 1", true); \
    a[1] = config->readBoolEntry("Border 2", true); \
    a[2] = config->readBoolEntry("Border 3", true); \
    a[3] = config->readBoolEntry("Border 4", true);

#define READ_PEN(A, C1, C2, C3, C4, P1, P2, P3, P4, S1, S2, S3, S4) \
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
        (Qt::PenStyle)config->readNumEntry("Pen Style 4", S4)));

#define READ_BRUSH(A, C1, C2, C3, C4, S1, S2, S3, S4) \
  A.append(new QBrush(config->readColorEntry("Brush Color 1", new C1), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 1", S1))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 2", new C2), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 2", S2))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 3", new C3), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 3", S3))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 4", new C4), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 4", S4))); \

#define WRITE_PEN(Group, a, b) \
    config->setGroup(Group); \
    config->writeEntry("Color 1", a.at(0)->color()); \
    config->writeEntry("Color 2", a.at(1)->color()); \
    config->writeEntry("Color 3", a.at(2)->color()); \
    config->writeEntry("Color 4", a.at(3)->color()); \
    config->writeEntry("Pen Size 1", a.at(0)->width()); \
    config->writeEntry("Pen Size 2", a.at(1)->width()); \
    config->writeEntry("Pen Size 3", a.at(2)->width()); \
    config->writeEntry("Pen Size 4", a.at(3)->width()); \
    config->writeEntry("Pen Style 1", a.at(0)->style()); \
    config->writeEntry("Pen Style 2", a.at(1)->style()); \
    config->writeEntry("Pen Style 3", a.at(2)->style()); \
    config->writeEntry("Pen Style 4", a.at(3)->style()); \
    config->writeEntry("Border 1", b[0]); \
    config->writeEntry("Border 2", ( b[1] && b[0] ) ); \
    config->writeEntry("Border 3", ( b[2] && b[1] && b[0] ) ); \
    config->writeEntry("Border 4", ( b[3] && b[2] && b[1] && b[0] ) );

#define WRITE_BRUSH(Group, a, b, c) \
    WRITE_PEN(Group,b,c) \
    config->writeEntry("Brush Color 1", a.at(0)->color()); \
    config->writeEntry("Brush Color 2", a.at(1)->color()); \
    config->writeEntry("Brush Color 3", a.at(2)->color()); \
    config->writeEntry("Brush Color 4", a.at(3)->color()); \
    config->writeEntry("Brush Style 1", a.at(0)->style()); \
    config->writeEntry("Brush Style 2", a.at(1)->style()); \
    config->writeEntry("Brush Style 3", a.at(2)->style()); \
    config->writeEntry("Brush Style 4", a.at(3)->style());

#define DEFAULT_PEN(a, b, C1, C2, C3, C4, P1, P2, P3, P4) \
    a.at(0)->setColor(C1); \
    a.at(0)->setWidth(P1); \
    a.at(1)->setColor(C2); \
    a.at(1)->setWidth(P2); \
    a.at(2)->setColor(C3); \
    a.at(2)->setWidth(P3); \
    a.at(3)->setColor(C4); \
    a.at(3)->setWidth(P4); \
    b[0] = true; \
    b[1] = true; \
    b[2] = true; \
    b[3] = true;

#define DEFAULT_PEN_BRUSH(A, b, a, C1, C2, C3, C4, P1, P2, P3, P4, \
    C5, C6, C7, C8, S5, S6, S7, S8) \
    DEFAULT_PEN(A, b, C1, C2, C3, C4, P1, P2, P3, P4) \
    a.at(0)->setColor(C5); \
    a.at(0)->setStyle(S5); \
    a.at(1)->setColor(C6); \
    a.at(1)->setStyle(S6); \
    a.at(2)->setColor(C7); \
    a.at(2)->setStyle(S7); \
    a.at(3)->setColor(C8); \
    a.at(3)->setStyle(S8);

// Qt::PenStyle-Enum starts with NoPen = 0, therefor we reduce the
// value by 1. We must use the same order as Qt::PenStyle.
// Qt::BrushStyle "NoBrush" is allowed ...
#define FILLSTYLE(pen,brush) \
  pen->insertItem(QPixmap(picDir + "solid.png"), Qt::SolidLine - 1); \
  pen->insertItem(QPixmap(picDir + "dashed.png"), Qt::DashLine - 1); \
  pen->insertItem(QPixmap(picDir + "dotted.png"), Qt::DotLine - 1); \
  pen->insertItem(QPixmap(picDir + "dashdot.png"), Qt::DashDotLine - 1); \
  pen->insertItem(QPixmap(picDir + "dashdotdot.png"), Qt::DashDotDotLine - 1); \
  brush->insertItem("no", Qt::NoBrush); \
  brush->insertItem(QPixmap(picDir + "brush0.png"), Qt::SolidPattern); \
  brush->insertItem(QPixmap(picDir + "brush1.png"), Qt::Dense1Pattern); \
  brush->insertItem(QPixmap(picDir + "brush2.png"), Qt::Dense2Pattern); \
  brush->insertItem(QPixmap(picDir + "brush3.png"), Qt::Dense3Pattern); \
  brush->insertItem(QPixmap(picDir + "brush4.png"), Qt::Dense4Pattern); \
  brush->insertItem(QPixmap(picDir + "brush5.png"), Qt::Dense5Pattern); \
  brush->insertItem(QPixmap(picDir + "brush6.png"), Qt::Dense6Pattern); \
  brush->insertItem(QPixmap(picDir + "brush7.png"), Qt::Dense7Pattern); \
  brush->insertItem(QPixmap(picDir + "brush8.png"), Qt::HorPattern); \
  brush->insertItem(QPixmap(picDir + "brush9.png"), Qt::VerPattern); \
  brush->insertItem(QPixmap(picDir + "brush10.png"), Qt::CrossPattern); \
  brush->insertItem(QPixmap(picDir + "brush11.png"), Qt::BDiagPattern); \
  brush->insertItem(QPixmap(picDir + "brush12.png"), Qt::FDiagPattern); \
  brush->insertItem(QPixmap(picDir + "brush13.png"), Qt::DiagCrossPattern);

#define BUTTONROW(penC, penW, penS, brushC, brushS, row) \
  penC = new KColorButton(parent); \
  penC->setMaximumWidth(35); \
  penW = new QSpinBox(1, 9, 1, parent); \
  penW->setMaximumWidth(40); \
  penS = new KComboBox(parent); \
  penS->setMaximumWidth(45); \
  brushC = new KColorButton(parent); \
  brushC->setMaximumWidth(35); \
  brushS = new KComboBox(parent); \
  brushS->setMaximumWidth(45); \
  FILLSTYLE(penS, brushS) \
  penC->setMaximumHeight(brushS->sizeHint().height()); \
  brushC->setMaximumHeight(brushS->sizeHint().height()); \
  elLayout->addWidget(penC, row, 3); \
  elLayout->addWidget(penW, row, 5); \
  elLayout->addWidget(penS, row, 7); \
  elLayout->addWidget(brushC, row, 9); \
  elLayout->addWidget(brushS, row, 11);

ConfigDrawElement::ConfigDrawElement(QWidget* parent, KConfig* cnf)
  : QFrame(parent, "configdrawelement"),
    config(cnf), oldElement(-1)
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

  config->setGroup("Road");
  READ_PEN(roadPenList, ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4)
  READ_BORDER(roadBorder);

  config->setGroup("River");
  READ_PEN(riverPenList, RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_2,
        RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4)
  READ_BORDER(riverBorder);

  config->setGroup("Rail");
  READ_PEN(railPenList, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4)
  READ_BORDER(railBorder);

  config->setGroup("Highway");
  READ_PEN(highwayPenList, HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4)
  READ_BORDER(highwayBorder);

  //
  // In version <= 2.0.1, the fillcolor of cities is called "Color" instead
  // of "Brush Color", so we must look, which version of configfile we read.
  //
  config->setGroup("General Options");
  if(config->hasKey("Version") && config->readEntry("Version") >= "2.0.2")
    {
      config->setGroup("City");
      // PenStyle and BrushStyle are not used for cities ...
      READ_PEN(cityPenList, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
            CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
            Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine)
      READ_BRUSH(cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
          CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, Qt::SolidPattern,
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
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_1, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_2, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_3, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_4, Qt::SolidPattern));
    }
  READ_BORDER(cityBorder);

  config->setGroup("Airspace A");
  READ_PEN(airAPenList, AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
        AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
        AIRA_PEN_STYLE_1, AIRA_PEN_STYLE_2, AIRA_PEN_STYLE_3, AIRA_PEN_STYLE_4)
  READ_BRUSH(airABrushList, AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
        AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4, AIRA_BRUSH_STYLE_1,
        AIRA_BRUSH_STYLE_2, AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4)
  READ_BORDER(airABorder);

  config->setGroup("Airspace B");
  READ_PEN(airBPenList, AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
        AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
        AIRB_PEN_STYLE_1, AIRB_PEN_STYLE_2, AIRB_PEN_STYLE_3, AIRB_PEN_STYLE_4)
  READ_BRUSH(airBBrushList, AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
        AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4, AIRB_BRUSH_STYLE_1,
        AIRB_BRUSH_STYLE_2, AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4)
  READ_BORDER(airBBorder);

  config->setGroup("Airspace C");
  READ_PEN(airCPenList, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4)
  READ_BRUSH(airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
        AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)
  READ_BORDER(airCBorder);

  config->setGroup("Airspace D");
  READ_PEN(airDPenList, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4)
  READ_BRUSH(airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
        AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)
  READ_BORDER(airDBorder);

  config->setGroup("Airspace E low");
  READ_PEN(airElPenList, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
        AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4)
  READ_BRUSH(airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
        AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4)
  READ_BORDER(airElBorder);

  config->setGroup("Airspace E high");
  READ_PEN(airEhPenList, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
        AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4)
  READ_BRUSH(airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
        AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4)
  READ_BORDER(airEhBorder);

  config->setGroup("Airspace F");
  READ_PEN(airFPenList, AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3,
        AIRF_COLOR_4, AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4)
  READ_BRUSH(airFBrushList, AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4, AIRF_BRUSH_STYLE_1,
        AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4)
  READ_BORDER(airFBorder);

  config->setGroup("Control C");
  READ_PEN(ctrCPenList, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4)
  READ_BRUSH(ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
        CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)
  READ_BORDER(ctrCBorder);

  config->setGroup("Control D");
  READ_PEN(ctrDPenList, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4)
  READ_BRUSH(ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
        CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)
  READ_BORDER(ctrDBorder);

  config->setGroup("Danger");
  READ_PEN(dangerPenList, DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4)
  READ_BRUSH(dangerBrushList, DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4, DNG_BRUSH_STYLE_1,
        DNG_BRUSH_STYLE_2, DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)
  READ_BORDER(dangerBorder);

  config->setGroup("Low Flight");
  READ_PEN(lowFPenList, LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4)
  READ_BRUSH(lowFBrushList, LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4, LOWF_BRUSH_STYLE_1,
        LOWF_BRUSH_STYLE_2, LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4)
  READ_BORDER(lowFBorder);

  config->setGroup("Restricted Area");
  READ_PEN(restrPenList, RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4)
  READ_BRUSH(restrBrushList, RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4, RES_BRUSH_STYLE_1,
        RES_BRUSH_STYLE_2, RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)
  READ_BORDER(restrBorder);

  config->setGroup("TMZ");
  READ_PEN(tmzPenList, TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4)
  READ_BRUSH(tmzBrushList, TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4, TMZ_BRUSH_STYLE_1,
        TMZ_BRUSH_STYLE_2, TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)
  READ_BORDER(tmzBorder);

  config->setGroup(0);

  border1 = new QCheckBox(i18n("border #1"), parent);
  border2 = new QCheckBox(i18n("border #2"), parent);
  border3 = new QCheckBox(i18n("border #3"), parent);
  border4 = new QCheckBox(i18n("scale-limit"), parent);

  QString picDir = KGlobal::dirs()->findResource("appdata", "pics/");

  border1Button = new QPushButton(parent);
  border1Button->setPixmap(BarIcon("down"));
  border1Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border1Button->setFixedHeight(border1Button->sizeHint().height() + 3);
  border2Button = new QPushButton(parent);
  border2Button->setPixmap(BarIcon("down"));
  border2Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border2Button->setFixedHeight(border1Button->sizeHint().height() + 3);
  border3Button = new QPushButton(parent);
  border3Button->setPixmap(BarIcon("down"));
  border3Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border3Button->setFixedHeight(border1Button->sizeHint().height() + 3);

  QGridLayout* elLayout = new QGridLayout(parent, 10, 15, 5, 1);
  elLayout->addWidget(new QLabel(i18n("draw up to"), parent), 1, 1);
  elLayout->addMultiCellWidget(new QLabel(i18n("Pen"), parent), 1, 1, 3, 7);
  elLayout->addMultiCellWidget(new QLabel(i18n("Brush"), parent), 1, 1, 9, 11);
  elLayout->addWidget(border1, 3, 1);
  elLayout->addWidget(border1Button, 3, 13);
  elLayout->addWidget(border2, 5, 1);
  elLayout->addWidget(border2Button, 5, 13);
  elLayout->addWidget(border3, 7, 1);
  elLayout->addWidget(border3Button, 7, 13);
  elLayout->addWidget(border4, 9, 1);

  BUTTONROW(border1Color, border1Pen, border1PenStyle, border1BrushColor,
      border1BrushStyle, 3);

  BUTTONROW(border2Color, border2Pen, border2PenStyle, border2BrushColor,
      border2BrushStyle, 5);

  BUTTONROW(border3Color, border3Pen, border3PenStyle, border3BrushColor,
      border3BrushStyle, 7);

  BUTTONROW(border4Color, border4Pen, border4PenStyle, border4BrushColor,
      border4BrushStyle, 9);

  elLayout->addRowSpacing(0, 2);
  elLayout->addRowSpacing(2, 5);
  elLayout->setRowStretch(2, 1);
  elLayout->setRowStretch(4, 1);
  elLayout->setRowStretch(6, 1);
  elLayout->setRowStretch(8, 1);
  elLayout->addRowSpacing(10, 5);

  elLayout->addColSpacing(0, 10);
  elLayout->setColStretch(2, 3);
  elLayout->setColStretch(4, 0);
  elLayout->addColSpacing(4, 5);
  elLayout->setColStretch(6, 0);
  elLayout->addColSpacing(6, 5);
  elLayout->setColStretch(8, 2);
  elLayout->addColSpacing(8, 8);
  elLayout->setColStretch(10, 0);
  elLayout->addColSpacing(10, 5);
  elLayout->setColStretch(12, 1);
  elLayout->addColSpacing(12, 8);
  elLayout->addColSpacing(14, 10);

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

  config->setGroup("General Options");
  config->writeEntry("Version", "2.0.2");

  WRITE_PEN("Road", roadPenList, roadBorder);

  WRITE_PEN("Highway", highwayPenList, highwayBorder);

  WRITE_PEN("Rail", railPenList, railBorder);

  WRITE_PEN("River", riverPenList, riverBorder);

  WRITE_PEN("City", cityPenList, cityBorder);

  WRITE_BRUSH("Airspace A", airABrushList, airAPenList, airABorder);

  WRITE_BRUSH("Airspace B", airBBrushList, airBPenList, airBBorder);

  WRITE_BRUSH("Airspace C", airCBrushList, airCPenList, airCBorder);

  WRITE_BRUSH("Airspace D", airDBrushList, airDPenList, airDBorder);

  WRITE_BRUSH("Airspace E low", airElBrushList, airElPenList, airElBorder);

  WRITE_BRUSH("Airspace E high", airEhBrushList, airEhPenList, airEhBorder);

  WRITE_BRUSH("Airspace F", airFBrushList, airFPenList, airFBorder);

  WRITE_BRUSH("Control C", ctrCBrushList, ctrCPenList, ctrCBorder);

  WRITE_BRUSH("Control D", ctrDBrushList, ctrDPenList, ctrDBorder);

  WRITE_BRUSH("Danger", dangerBrushList, dangerPenList, dangerBorder);

  WRITE_BRUSH("Low Flight", lowFBrushList, lowFPenList, lowFBorder);

  WRITE_BRUSH("Restricted Area", restrBrushList, restrPenList, restrBorder);

  WRITE_BRUSH("TMZ", tmzBrushList, tmzPenList, tmzBorder);

  config->sync();
  config->setGroup(0);
}

void ConfigDrawElement::slotDefaultElements()
{
  DEFAULT_PEN(roadPenList, roadBorder, ROAD_COLOR_1, ROAD_COLOR_2,
      ROAD_COLOR_3, ROAD_COLOR_4,
      ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4)

  DEFAULT_PEN(highwayPenList, highwayBorder, HIGH_COLOR_1, HIGH_COLOR_2,
      HIGH_COLOR_3, HIGH_COLOR_4,
      HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4)

  DEFAULT_PEN(riverPenList, riverBorder, RIVER_COLOR_1, RIVER_COLOR_2,
      RIVER_COLOR_3, RIVER_COLOR_4,
      RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4)

  DEFAULT_PEN(railPenList, railBorder, RAIL_COLOR_1, RAIL_COLOR_2,
      RAIL_COLOR_3, RAIL_COLOR_4,
      RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4)

  DEFAULT_PEN_BRUSH(cityPenList, cityBorder, cityBrushList,
      CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3, CITY_COLOR_4,
      CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
      CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4,
      CITY_BRUSH_STYLE_1, CITY_BRUSH_STYLE_2,
      CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airAPenList, airABorder, airABrushList,
      AIRA_COLOR_1, AIRA_COLOR_2, AIRA_COLOR_3, AIRA_COLOR_4,
      AIRA_PEN_1, AIRA_PEN_2, AIRA_PEN_3, AIRA_PEN_4,
      AIRA_BRUSH_COLOR_1, AIRA_BRUSH_COLOR_2,
      AIRA_BRUSH_COLOR_3, AIRA_BRUSH_COLOR_4,
      AIRA_BRUSH_STYLE_1, AIRA_BRUSH_STYLE_2,
      AIRA_BRUSH_STYLE_3, AIRA_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airBPenList, airBBorder, airBBrushList,
      AIRB_COLOR_1, AIRB_COLOR_2, AIRB_COLOR_3, AIRB_COLOR_4,
      AIRB_PEN_1, AIRB_PEN_2, AIRB_PEN_3, AIRB_PEN_4,
      AIRB_BRUSH_COLOR_1, AIRB_BRUSH_COLOR_2,
      AIRB_BRUSH_COLOR_3, AIRB_BRUSH_COLOR_4,
      AIRB_BRUSH_STYLE_1, AIRB_BRUSH_STYLE_2,
      AIRB_BRUSH_STYLE_3, AIRB_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airCPenList, airCBorder, airCBrushList,
      AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
      AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
      AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
      AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4,
      AIRC_BRUSH_STYLE_1, AIRC_BRUSH_STYLE_2,
      AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airDPenList, airDBorder, airDBrushList,
      AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
      AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
      AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
      AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4,
      AIRD_BRUSH_STYLE_1, AIRD_BRUSH_STYLE_2,
      AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airElPenList, airElBorder, airElBrushList,
      AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3, AIREL_COLOR_4,
      AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
      AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
      AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4,
      AIREL_BRUSH_STYLE_1, AIREL_BRUSH_STYLE_2,
      AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(airEhPenList, airEhBorder, airEhBrushList,
      AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3, AIREH_COLOR_4,
      AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
      AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
      AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4,
      AIREH_BRUSH_STYLE_1, AIREH_BRUSH_STYLE_2,
      AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(ctrCPenList, ctrCBorder, ctrCBrushList,
      CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
      CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
      CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
      CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4,
      CTRC_BRUSH_STYLE_1, CTRC_BRUSH_STYLE_2,
      CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(ctrDPenList, ctrDBorder, ctrDBrushList,
      CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
      CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
      CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
      CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4,
      CTRD_BRUSH_STYLE_1, CTRD_BRUSH_STYLE_2,
      CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(dangerPenList, dangerBorder, dangerBrushList,
      DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
      DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
      DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
      DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4,
      DNG_BRUSH_STYLE_1, DNG_BRUSH_STYLE_2,
      DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(restrPenList, restrBorder, restrBrushList,
      RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
      RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
      RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
      RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4,
      RES_BRUSH_STYLE_1, RES_BRUSH_STYLE_2,
      RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(lowFPenList, lowFBorder, lowFBrushList,
      LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
      LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
      LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
      LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4,
      LOWF_BRUSH_STYLE_1, LOWF_BRUSH_STYLE_2,
      LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4)

  DEFAULT_PEN_BRUSH(tmzPenList, tmzBorder, tmzBrushList,
      TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
      TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
      TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
      TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4,
      TMZ_BRUSH_STYLE_1, TMZ_BRUSH_STYLE_2,
      TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)

  oldElement = -1;
  slotSelectElement(currentElement);
}

void ConfigDrawElement::slotSelectElement(int elementID)
{
  currentElement = elementID;

  switch(oldElement)
    {
      case Road:
        SAVE_PEN(roadPenList, roadBorder)
        break;
      case Highway:
        SAVE_PEN(highwayPenList, highwayBorder)
        break;
      case Railway:
        SAVE_PEN(railPenList, railBorder)
        break;
      case River:
        SAVE_PEN(riverPenList, riverBorder)
        break;
      case City:
        SAVE_PEN(cityPenList, cityBorder)
        SAVE_BRUSH(cityBrushList)
        break;
      case AirA:
        SAVE_PEN(airAPenList, airABorder)
        SAVE_BRUSH(airABrushList)
        break;
      case AirB:
        SAVE_PEN(airBPenList, airBBorder)
        SAVE_BRUSH(airBBrushList)
        break;
      case AirC:
        SAVE_PEN(airCPenList, airCBorder)
        SAVE_BRUSH(airCBrushList)
        break;
      case AirD:
        SAVE_PEN(airDPenList, airDBorder)
        SAVE_BRUSH(airDBrushList)
        break;
      case AirElow:
        SAVE_PEN(airElPenList, airElBorder)
        SAVE_BRUSH(airElBrushList)
        break;
      case AirEhigh:
        SAVE_PEN(airEhPenList, airEhBorder)
        SAVE_BRUSH(airEhBrushList)
        break;
      case AirF:
        SAVE_PEN(airFPenList, airFBorder)
        SAVE_BRUSH(airFBrushList)
        break;
      case ControlC:
        SAVE_PEN(ctrCPenList, ctrCBorder)
        SAVE_BRUSH(ctrCBrushList)
        break;
      case ControlD:
        SAVE_PEN(ctrDPenList, ctrDBorder)
        SAVE_BRUSH(ctrDBrushList)
        break;
      case Danger:
        SAVE_PEN(dangerPenList, dangerBorder)
        SAVE_BRUSH(dangerBrushList)
        break;
      case LowFlight:
        SAVE_PEN(lowFPenList, lowFBorder)
        SAVE_BRUSH(lowFBrushList)
        break;
      case Restricted:
        SAVE_PEN(restrPenList, restrBorder)
        SAVE_BRUSH(restrBrushList)
        break;
      case TMZ:
        SAVE_PEN(tmzPenList, tmzBorder)
        SAVE_BRUSH(tmzBrushList)
        break;
      default:
        break;
    }

  switch(elementID)
    {
      case Road:
        SHOW_PEN(roadPenList, roadBorder)
        break;
      case Highway:
        SHOW_PEN(highwayPenList, highwayBorder)
        break;
      case Railway:
        SHOW_PEN(railPenList, railBorder)
        break;
      case River:
        SHOW_PEN(riverPenList, riverBorder)
        break;
      case City:
        SHOW_PEN(cityPenList, cityBorder)
        SHOW_BRUSH(cityBrushList)
        break;
      case AirA:
        SHOW_PEN(airAPenList, airABorder)
        SHOW_BRUSH(airABrushList)
        break;
      case AirB:
        SHOW_PEN(airBPenList, airBBorder)
        SHOW_BRUSH(airBBrushList)
        break;
      case AirC:
        SHOW_PEN(airCPenList, airCBorder)
        SHOW_BRUSH(airCBrushList)
        break;
      case AirD:
        SHOW_PEN(airDPenList, airDBorder)
        SHOW_BRUSH(airDBrushList)
        break;
      case AirElow:
        SHOW_PEN(airElPenList, airElBorder)
        SHOW_BRUSH(airElBrushList)
        break;
      case AirEhigh:
        SHOW_PEN(airEhPenList, airEhBorder)
        SHOW_BRUSH(airEhBrushList)
        break;
      case AirF:
        SHOW_PEN(airFPenList, airFBorder)
        SHOW_BRUSH(airFBrushList)
        break;
      case ControlC:
        SHOW_PEN(ctrCPenList, ctrCBorder)
        SHOW_BRUSH(ctrCBrushList)
        break;
      case ControlD:
        SHOW_PEN(ctrDPenList, ctrDBorder)
        SHOW_BRUSH(ctrDBrushList)
        break;
      case Danger:
        SHOW_PEN(dangerPenList, dangerBorder)
        SHOW_BRUSH(dangerBrushList)
        break;
      case LowFlight:
        SHOW_PEN(lowFPenList, lowFBorder)
        SHOW_BRUSH(lowFBrushList)
        break;
      case Restricted:
        SHOW_PEN(restrPenList, restrBorder)
        SHOW_BRUSH(restrBrushList)
        break;
      case TMZ:
        SHOW_PEN(tmzPenList, tmzBorder)
        SHOW_BRUSH(tmzBrushList)
        break;
    }

  oldElement = elementID;
  slotToggleFirst(border1->isChecked());
}

void ConfigDrawElement::slotToggleFirst(bool toggle)
{
  border1Button->setEnabled(toggle);
  border1Pen->setEnabled(toggle);
  border1Color->setEnabled(toggle);
  border2->setEnabled(toggle);

  switch(currentElement)
    {
      case City:
        border1PenStyle->setEnabled(false);
        border1BrushColor->setEnabled(toggle);
        border1BrushStyle->setEnabled(false);
        break;
      case AirA:
      case AirB:
      case AirC:
      case AirD:
      case AirElow:
      case AirEhigh:
      case AirF:
      case ControlC:
      case ControlD:
      case LowFlight:
      case Danger:
      case Restricted:
      case TMZ:
        border1PenStyle->setEnabled(toggle);
        border1BrushColor->setEnabled(toggle);
        border1BrushStyle->setEnabled(toggle);
        break;
      default:
        border1PenStyle->setEnabled(toggle);
        border1BrushColor->setEnabled(false);
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
  border2Color->setEnabled(toggle);
  border3->setEnabled(toggle);

  switch(currentElement)
    {
      case City:
        border2PenStyle->setEnabled(false);
        border2BrushColor->setEnabled(toggle);
        border2BrushStyle->setEnabled(false);
        break;
      case AirA:
      case AirB:
      case AirC:
      case AirD:
      case AirElow:
      case AirEhigh:
      case AirF:
      case ControlC:
      case ControlD:
      case LowFlight:
      case Danger:
      case Restricted:
      case TMZ:
        border2PenStyle->setEnabled(toggle);
        border2BrushColor->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        break;
      default:
        border2PenStyle->setEnabled(toggle);
        border2BrushColor->setEnabled(false);
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
  border3Color->setEnabled(toggle);
  border4->setEnabled(toggle);

  switch(currentElement)
    {
      case City:
        border3PenStyle->setEnabled(false);
        border3BrushColor->setEnabled(toggle);
        border3BrushStyle->setEnabled(false);
        break;
      case AirA:
      case AirB:
      case AirC:
      case AirD:
      case AirElow:
      case AirEhigh:
      case AirF:
      case ControlC:
      case ControlD:
      case LowFlight:
      case Danger:
      case Restricted:
      case TMZ:
        border3PenStyle->setEnabled(toggle);
        border3BrushColor->setEnabled(toggle);
        border3BrushStyle->setEnabled(toggle);
        break;
      default:
        border3PenStyle->setEnabled(toggle);
        border3BrushColor->setEnabled(false);
        border3BrushStyle->setEnabled(false);
    }

  if(!toggle)
      slotToggleForth(false);
  else
      slotToggleForth(border4->isChecked());
}

void ConfigDrawElement::slotToggleForth(bool toggle)
{
  border4Color->setEnabled(toggle);
  border4Pen->setEnabled(toggle);

  switch(currentElement)
    {
      case City:
        border4PenStyle->setEnabled(false);
        border4BrushColor->setEnabled(toggle);
        border4BrushStyle->setEnabled(false);
        break;
      case AirA:
      case AirB:
      case AirC:
      case AirD:
      case AirElow:
      case AirEhigh:
      case AirF:
      case ControlC:
      case ControlD:
      case LowFlight:
      case Danger:
      case Restricted:
      case TMZ:
        border4PenStyle->setEnabled(toggle);
        border4BrushColor->setEnabled(toggle);
        border4BrushStyle->setEnabled(toggle);
        break;
      default:
        border4PenStyle->setEnabled(toggle);
        border4BrushColor->setEnabled(false);
        border4BrushStyle->setEnabled(false);
    }
}

void ConfigDrawElement::slotSetSecond()
{
  border2Color->setColor(border1Color->color());
  border2Pen->setValue(border1Pen->value());
  border2PenStyle->setCurrentItem(border1PenStyle->currentItem());
  border2BrushColor->setColor(border1BrushColor->color());
  border2BrushStyle->setCurrentItem(border1BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetThird()
{
  border3Color->setColor(border2Color->color());
  border3Pen->setValue(border2Pen->value());
  border3PenStyle->setCurrentItem(border2PenStyle->currentItem());
  border3BrushColor->setColor(border2BrushColor->color());
  border3BrushStyle->setCurrentItem(border2BrushStyle->currentItem());
}

void ConfigDrawElement::slotSetForth()
{
  border4Color->setColor(border3Color->color());
  border4Pen->setValue(border3Pen->value());
  border4PenStyle->setCurrentItem(border3PenStyle->currentItem());
  border4BrushColor->setColor(border3BrushColor->color());
  border4BrushStyle->setCurrentItem(border3BrushStyle->currentItem());
}
