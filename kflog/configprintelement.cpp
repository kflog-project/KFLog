/***********************************************************************
**
**   configprintelement.cpp
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

#include "configprintelement.h"
#include <mapdefaults.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#define SAVE_PEN(a,b) \
    b[0] = border1->isChecked(); \
    a.at(0)->setColor(border1Color->color()); \
    a.at(0)->setWidth(border1Pen->value()); \
    a.at(0)->setStyle((Qt::PenStyle)(border1PenStyle->currentItem() + 1)); \
    b[1] = border2->isChecked(); \
    a.at(1)->setColor(border2Color->color()); \
    a.at(1)->setWidth(border2Pen->value()); \
    a.at(1)->setStyle((Qt::PenStyle)(border2PenStyle->currentItem() + 1));

#define SAVE_BRUSH(a) \
    a.at(0)->setColor(border1BrushColor->color()); \
    a.at(0)->setStyle((Qt::BrushStyle)border1BrushStyle->currentItem()); \
    a.at(1)->setColor(border2BrushColor->color()); \
    a.at(1)->setStyle((Qt::BrushStyle)border2BrushStyle->currentItem());

#define SHOW_PEN(a,b) \
    border1->setChecked(b[0]); \
    border1Color->setColor(a.at(0)->color()); \
    border1Pen->setValue(a.at(0)->width()); \
    border1PenStyle->setCurrentItem(a.at(0)->style() - 1); \
    border2->setChecked(b[1]); \
    border2Color->setColor(a.at(1)->color()); \
    border2Pen->setValue(a.at(1)->width()); \
    border2PenStyle->setCurrentItem(a.at(1)->style() - 1);

#define SHOW_BRUSH(a) \
    border1BrushColor->setColor(a.at(0)->color()); \
    border1BrushStyle->setCurrentItem(a.at(0)->style()); \
    border2BrushColor->setColor(a.at(1)->color()); \
    border2BrushStyle->setCurrentItem(a.at(1)->style());

#define READ_BORDER(a) \
    a[0] = config->readBoolEntry("Print Border 1", true); \
    a[1] = config->readBoolEntry("Print Border 2", true);

#define READ_PEN(Group, A, b, C1, C2, P1, P2, S1, S2) \
  config->setGroup(Group); \
  READ_BORDER(b) \
  A.append(new QPen(config->readColorEntry("Print Color 1", new C1), \
        config->readNumEntry("Print Pen Size 1", P1), \
        (Qt::PenStyle)config->readNumEntry("Print Pen Style 1", S1))); \
  A.append(new QPen(config->readColorEntry("Print Color 2", new C2), \
        config->readNumEntry("Print Pen Size 2", P2), \
        (Qt::PenStyle)config->readNumEntry("Print Pen Style 2", S2)));

#define READ_PEN_BRUSH(Group, a, b, A, C1, C2, P1, P2, S1, S2, C3, C4, S3, S4) \
  READ_PEN(Group, a, b, C1, C2, P1, P2, S1, S2) \
  A.append(new QBrush(config->readColorEntry("Print Brush Color 1", new C3), \
        (Qt::BrushStyle)config->readNumEntry("Print Brush Style 1", S3))); \
  A.append(new QBrush(config->readColorEntry("Print Brush Color 2", new C4), \
        (Qt::BrushStyle)config->readNumEntry("Print Brush Style 2", S4)));

#define WRITE_PEN(Group,a,b) \
    config->setGroup(Group); \
    config->writeEntry("Print Color 1", a.at(0)->color()); \
    config->writeEntry("Print Color 2", a.at(1)->color()); \
    config->writeEntry("Print Pen Size 1", a.at(0)->width()); \
    config->writeEntry("Print Pen Size 2", a.at(1)->width()); \
    config->writeEntry("Print Pen Style 1", a.at(0)->style()); \
    config->writeEntry("Print Pen Style 2", a.at(1)->style()); \
    config->writeEntry("Print Border 1", b[0]); \
    config->writeEntry("Print Border 2", ( b[1] && b[0] ) );

#define WRITE_BRUSH(Group,a,b,c) \
    WRITE_PEN(Group,b,c) \
    config->writeEntry("Print Brush Color 1", a.at(0)->color()); \
    config->writeEntry("Print Brush Color 2", a.at(1)->color()); \
    config->writeEntry("Print Brush Style 1", a.at(0)->style()); \
    config->writeEntry("Print Brush Style 2", a.at(1)->style());

#define DEFAULT_PEN(a, b, C1, C2, P1, P2, S1, S2) \
    a.at(0)->setColor(C1); \
    a.at(0)->setWidth(P1); \
    a.at(0)->setStyle(S1); \
    a.at(1)->setColor(C2); \
    a.at(1)->setWidth(P2); \
    a.at(1)->setStyle(S2); \
    b[0] = true; \
    b[1] = true;

#define DEFAULT_PEN_BRUSH(A, b, a, C1, C2, P1, P2, S1, S2, C3, C4, S3, S4) \
    DEFAULT_PEN(A, b, C1, C2, P1, P2, S1, S2) \
    a.at(0)->setColor(C3); \
    a.at(0)->setStyle(S3); \
    a.at(1)->setColor(C4); \
    a.at(1)->setStyle(S4);

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

ConfigPrintElement::ConfigPrintElement(QWidget* parent, KConfig* cnf)
  : QFrame(parent, "ConfigPrintelement"),
    config(cnf), oldElement(-1)
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
  highwayBorder = new bool[2];
  railBorder = new bool[2];
  riverBorder = new bool[2];
  cityBorder = new bool[2];
  forestBorder = new bool[2];

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

  READ_PEN("Road", roadPenList, roadBorder,
        PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
        PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
        PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2)

  READ_PEN("River", riverPenList, riverBorder,
        PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
        PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
        PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2)

  READ_PEN("Rail", railPenList, railBorder,
        PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
        PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
        PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2)

  READ_PEN("Highway", highwayPenList, highwayBorder,
        PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
        PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
        PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2)

  //
  // In version <= 2.0.1, the fillcolor of cities is called "Color" instead
  // of "Brush Color", so we must look, which version of configfile we read.
  //
  config->setGroup("General Options");
  if(config->hasKey("Version") && config->readEntry("Version") >= "2.0.2")
    {
      // PenStyle and BrushStyle are not used for cities ...
      READ_PEN_BRUSH("City", cityPenList, cityBorder, cityBrushList,
            PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2,
            PRINT_CITY_PEN_1, PRINT_CITY_PEN_2,
            Qt::SolidLine, Qt::SolidLine,
            PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
            Qt::SolidPattern, Qt::SolidPattern)
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
      READ_BORDER(cityBorder);
    }

  READ_PEN_BRUSH("Forest", forestPenList, forestBorder, forestBrushList,
        PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2,
        PRINT_FRST_PEN_1, PRINT_FRST_PEN_2,
        PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2,
        PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
        PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace A", airAPenList, airABorder, airABrushList,
        PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2,
        PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2,
        PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2,
        PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
        PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace B", airBPenList, airBBorder, airBBrushList,
        PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2,
        PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2,
        PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2,
        PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
        PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace C", airCPenList, airCBorder, airCBrushList,
        PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2,
        PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2,
        PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2,
        PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
        PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace D", airDPenList, airDBorder, airDBrushList,
        PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2,
        PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2,
        PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2,
        PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
        PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace E low", airElPenList, airElBorder, airElBrushList,
        PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2,
        PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2,
        PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2,
        PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
        PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace E high", airEhPenList, airEhBorder, airEhBrushList,
        PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2,
        PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2,
        PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2,
        PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
        PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Airspace F", airFPenList, airFBorder, airFBrushList,
        PRINT_AIRF_COLOR_1, PRINT_AIRF_COLOR_2,
        PRINT_AIRF_PEN_1, PRINT_AIRF_PEN_2,
        PRINT_AIRF_PEN_STYLE_1, PRINT_AIRF_PEN_STYLE_2,
        PRINT_AIRF_BRUSH_COLOR_1, PRINT_AIRF_BRUSH_COLOR_2,
        PRINT_AIRF_BRUSH_STYLE_1, PRINT_AIRF_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Control C", ctrCPenList, ctrCBorder, ctrCBrushList,
        PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2,
        PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2,
        PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2,
        PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
        PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Control D", ctrDPenList, ctrDBorder, ctrDBrushList,
        PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2,
        PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2,
        PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2,
        PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
        PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Danger", dangerPenList, dangerBorder, dangerBrushList,
        PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2,
        PRINT_DNG_PEN_1, PRINT_DNG_PEN_2,
        PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2,
        PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
        PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Low Flight", lowFPenList, lowFBorder, lowFBrushList,
        PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2,
        PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2,
        PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2,
        PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
        PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2)

  READ_PEN_BRUSH("Restricted Area", restrPenList, restrBorder, restrBrushList,
        PRINT_RES_COLOR_1, PRINT_RES_COLOR_2,
        PRINT_RES_PEN_1, PRINT_RES_PEN_2,
        PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2,
        PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
        PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2)

  READ_PEN_BRUSH("TMZ", tmzPenList, tmzBorder, tmzBrushList,
        PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2,
        PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2,
        PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2,
        PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
        PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2)

  config->setGroup(0);

  border1 = new QCheckBox(i18n("1:500.000"), parent);
  border2 = new QCheckBox(i18n("scale-limit"), parent);

  QString picDir = KGlobal::dirs()->findResource("appdata", "pics/");

  border1Button = new QPushButton(parent);
  border1Button->setPixmap(BarIcon("down"));
  border1Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border1Button->setFixedHeight(border1Button->sizeHint().height() + 3);

  QGridLayout* elLayout = new QGridLayout(parent, 10, 15, 5, 1);
  elLayout->addWidget(new QLabel(i18n("draw up to"), parent), 1, 1);
  elLayout->addMultiCellWidget(new QLabel(i18n("Pen"), parent), 1, 1, 3, 7);
  elLayout->addMultiCellWidget(new QLabel(i18n("Brush"), parent), 1, 1, 9, 11);
  elLayout->addWidget(border1, 3, 1);
  elLayout->addWidget(border1Button, 3, 13);
  elLayout->addWidget(border2, 5, 1);

  BUTTONROW(border1Color, border1Pen, border1PenStyle, border1BrushColor,
      border1BrushStyle, 3);

  BUTTONROW(border2Color, border2Pen, border2PenStyle, border2BrushColor,
      border2BrushStyle, 5);

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
  connect(border1Button, SIGNAL(clicked()), SLOT(slotSetSecond()));
}

ConfigPrintElement::~ConfigPrintElement()
{

}

void ConfigPrintElement::slotOk()
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

  WRITE_BRUSH("Forest", forestBrushList, forestPenList, forestBorder);

  config->sync();
  config->setGroup(0);
}

void ConfigPrintElement::slotDefaultElements()
{
  DEFAULT_PEN(roadPenList, roadBorder,
      PRINT_ROAD_COLOR_1, PRINT_ROAD_COLOR_2,
      PRINT_ROAD_PEN_1, PRINT_ROAD_PEN_2,
      PRINT_ROAD_PEN_STYLE_1, PRINT_ROAD_PEN_STYLE_2)

  DEFAULT_PEN(highwayPenList, highwayBorder,
      PRINT_HIGH_COLOR_1, PRINT_HIGH_COLOR_2,
      PRINT_HIGH_PEN_1, PRINT_HIGH_PEN_2,
      PRINT_HIGH_PEN_STYLE_1, PRINT_HIGH_PEN_STYLE_2)

  DEFAULT_PEN(riverPenList, riverBorder,
      PRINT_RIVER_COLOR_1, PRINT_RIVER_COLOR_2,
      PRINT_RIVER_PEN_1, PRINT_RIVER_PEN_2,
      PRINT_RIVER_PEN_STYLE_1, PRINT_RIVER_PEN_STYLE_2)

  DEFAULT_PEN(railPenList, railBorder,
      PRINT_RAIL_COLOR_1, PRINT_RAIL_COLOR_2,
      PRINT_RAIL_PEN_1, PRINT_RAIL_PEN_2,
      PRINT_RAIL_PEN_STYLE_1, PRINT_RAIL_PEN_STYLE_2)

  DEFAULT_PEN_BRUSH(cityPenList, cityBorder, cityBrushList,
      PRINT_CITY_COLOR_1, PRINT_CITY_COLOR_2,
      PRINT_CITY_PEN_1, PRINT_CITY_PEN_2,
      Qt::SolidLine, Qt::SolidLine,
      PRINT_CITY_BRUSH_COLOR_1, PRINT_CITY_BRUSH_COLOR_2,
      PRINT_CITY_BRUSH_STYLE_1, PRINT_CITY_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(forestPenList, forestBorder, forestBrushList,
      PRINT_FRST_COLOR_1, PRINT_FRST_COLOR_2,
      PRINT_FRST_PEN_1, PRINT_FRST_PEN_2,
      PRINT_FRST_PEN_STYLE_1, PRINT_FRST_PEN_STYLE_2,
      PRINT_FRST_BRUSH_COLOR_1, PRINT_FRST_BRUSH_COLOR_2,
      PRINT_FRST_BRUSH_STYLE_1, PRINT_FRST_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airAPenList, airABorder, airABrushList,
      PRINT_AIRA_COLOR_1, PRINT_AIRA_COLOR_2,
      PRINT_AIRA_PEN_1, PRINT_AIRA_PEN_2,
      PRINT_AIRA_PEN_STYLE_1, PRINT_AIRA_PEN_STYLE_2,
      PRINT_AIRA_BRUSH_COLOR_1, PRINT_AIRA_BRUSH_COLOR_2,
      PRINT_AIRA_BRUSH_STYLE_1, PRINT_AIRA_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airBPenList, airBBorder, airBBrushList,
      PRINT_AIRB_COLOR_1, PRINT_AIRB_COLOR_2,
      PRINT_AIRB_PEN_1, PRINT_AIRB_PEN_2,
      PRINT_AIRB_PEN_STYLE_1, PRINT_AIRB_PEN_STYLE_2,
      PRINT_AIRB_BRUSH_COLOR_1, PRINT_AIRB_BRUSH_COLOR_2,
      PRINT_AIRB_BRUSH_STYLE_1, PRINT_AIRB_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airCPenList, airCBorder, airCBrushList,
      PRINT_AIRC_COLOR_1, PRINT_AIRC_COLOR_2,
      PRINT_AIRC_PEN_1, PRINT_AIRC_PEN_2,
      PRINT_AIRC_PEN_STYLE_1, PRINT_AIRC_PEN_STYLE_2,
      PRINT_AIRC_BRUSH_COLOR_1, PRINT_AIRC_BRUSH_COLOR_2,
      PRINT_AIRC_BRUSH_STYLE_1, PRINT_AIRC_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airDPenList, airDBorder, airDBrushList,
      PRINT_AIRD_COLOR_1, PRINT_AIRD_COLOR_2,
      PRINT_AIRD_PEN_1, PRINT_AIRD_PEN_2,
      PRINT_AIRD_PEN_STYLE_1, PRINT_AIRD_PEN_STYLE_2,
      PRINT_AIRD_BRUSH_COLOR_1, PRINT_AIRD_BRUSH_COLOR_2,
      PRINT_AIRD_BRUSH_STYLE_1, PRINT_AIRD_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airElPenList, airElBorder, airElBrushList,
      PRINT_AIREL_COLOR_1, PRINT_AIREL_COLOR_2,
      PRINT_AIREL_PEN_1, PRINT_AIREL_PEN_2,
      PRINT_AIREL_PEN_STYLE_1, PRINT_AIREL_PEN_STYLE_2,
      PRINT_AIREL_BRUSH_COLOR_1, PRINT_AIREL_BRUSH_COLOR_2,
      PRINT_AIREL_BRUSH_STYLE_1, PRINT_AIREL_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(airEhPenList, airEhBorder, airEhBrushList,
      PRINT_AIREH_COLOR_1, PRINT_AIREH_COLOR_2,
      PRINT_AIREH_PEN_1, PRINT_AIREH_PEN_2,
      PRINT_AIREH_PEN_STYLE_1, PRINT_AIREH_PEN_STYLE_2,
      PRINT_AIREH_BRUSH_COLOR_1, PRINT_AIREH_BRUSH_COLOR_2,
      PRINT_AIREH_BRUSH_STYLE_1, PRINT_AIREH_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(ctrCPenList, ctrCBorder, ctrCBrushList,
      PRINT_CTRC_COLOR_1, PRINT_CTRC_COLOR_2,
      PRINT_CTRC_PEN_1, PRINT_CTRC_PEN_2,
      PRINT_CTRC_PEN_STYLE_1, PRINT_CTRC_PEN_STYLE_2,
      PRINT_CTRC_BRUSH_COLOR_1, PRINT_CTRC_BRUSH_COLOR_2,
      PRINT_CTRC_BRUSH_STYLE_1, PRINT_CTRC_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(ctrDPenList, ctrDBorder, ctrDBrushList,
      PRINT_CTRD_COLOR_1, PRINT_CTRD_COLOR_2,
      PRINT_CTRD_PEN_1, PRINT_CTRD_PEN_2,
      PRINT_CTRD_PEN_STYLE_1, PRINT_CTRD_PEN_STYLE_2,
      PRINT_CTRD_BRUSH_COLOR_1, PRINT_CTRD_BRUSH_COLOR_2,
      PRINT_CTRD_BRUSH_STYLE_1, PRINT_CTRD_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(dangerPenList, dangerBorder, dangerBrushList,
      PRINT_DNG_COLOR_1, PRINT_DNG_COLOR_2,
      PRINT_DNG_PEN_1, PRINT_DNG_PEN_2,
      PRINT_DNG_PEN_STYLE_1, PRINT_DNG_PEN_STYLE_2,
      PRINT_DNG_BRUSH_COLOR_1, PRINT_DNG_BRUSH_COLOR_2,
      PRINT_DNG_BRUSH_STYLE_1, PRINT_DNG_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(restrPenList, restrBorder, restrBrushList,
      PRINT_RES_COLOR_1, PRINT_RES_COLOR_2,
      PRINT_RES_PEN_1, PRINT_RES_PEN_2,
      PRINT_RES_PEN_STYLE_1, PRINT_RES_PEN_STYLE_2,
      PRINT_RES_BRUSH_COLOR_1, PRINT_RES_BRUSH_COLOR_2,
      PRINT_RES_BRUSH_STYLE_1, PRINT_RES_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(lowFPenList, lowFBorder, lowFBrushList,
      PRINT_LOWF_COLOR_1, PRINT_LOWF_COLOR_2,
      PRINT_LOWF_PEN_1, PRINT_LOWF_PEN_2,
      PRINT_LOWF_PEN_STYLE_1, PRINT_LOWF_PEN_STYLE_2,
      PRINT_LOWF_BRUSH_COLOR_1, PRINT_LOWF_BRUSH_COLOR_2,
      PRINT_LOWF_BRUSH_STYLE_1, PRINT_LOWF_BRUSH_STYLE_2)

  DEFAULT_PEN_BRUSH(tmzPenList, tmzBorder, tmzBrushList,
      PRINT_TMZ_COLOR_1, PRINT_TMZ_COLOR_2,
      PRINT_TMZ_PEN_1, PRINT_TMZ_PEN_2,
      PRINT_TMZ_PEN_STYLE_1, PRINT_TMZ_PEN_STYLE_2,
      PRINT_TMZ_BRUSH_COLOR_1, PRINT_TMZ_BRUSH_COLOR_2,
      PRINT_TMZ_BRUSH_STYLE_1, PRINT_TMZ_BRUSH_STYLE_2)

  oldElement = -1;
  slotSelectElement(currentElement);
}

void ConfigPrintElement::slotSelectElement(int elementID)
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
      case Forest:
        SAVE_PEN(forestPenList, forestBorder)
        SAVE_BRUSH(forestBrushList)
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
      case Forest:
        SHOW_PEN(forestPenList, forestBorder)
        SHOW_BRUSH(forestBrushList)
        break;
    }

  oldElement = elementID;
  slotToggleFirst(border1->isChecked());
}

void ConfigPrintElement::slotToggleFirst(bool toggle)
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
      case Forest:
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

void ConfigPrintElement::slotToggleSecond(bool toggle)
{
  border2Pen->setEnabled(toggle);
  border2Color->setEnabled(toggle);

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
      case Forest:
        border2PenStyle->setEnabled(toggle);
        border2BrushColor->setEnabled(toggle);
        border2BrushStyle->setEnabled(toggle);
        break;
      default:
        border2PenStyle->setEnabled(toggle);
        border2BrushColor->setEnabled(false);
        border2BrushStyle->setEnabled(false);
    }
}

void ConfigPrintElement::slotSetSecond()
{
  border2Color->setColor(border1Color->color());
  border2Pen->setValue(border1Pen->value());
  border2PenStyle->setCurrentItem(border1PenStyle->currentItem());
  border2BrushColor->setColor(border1BrushColor->color());
  border2BrushStyle->setCurrentItem(border1BrushStyle->currentItem());
}
