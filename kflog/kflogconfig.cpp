/***********************************************************************
**
**   kflogconfig.cpp
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

#include "kflogconfig.h"
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapdefaults.h>

#include <unistd.h>
#include <pwd.h>
//#include <sys/types.h>
#include <stdlib.h>

#include <kfiledialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#define SAVE_PEN(a,b) \
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

#define SHOW_PEN(a,b) \
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

#define WRITE_DRAW_VALUES(a,b) \
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
    config->writeEntry("Border 2", b[1]); \
    config->writeEntry("Border 3", b[2]); \
    config->writeEntry("Border 4", b[3]);

#define WRITE_BRUSH_VALUES(a) \
    config->writeEntry("Brush Color 1", a.at(0)->color()); \
    config->writeEntry("Brush Color 2", a.at(1)->color()); \
    config->writeEntry("Brush Color 3", a.at(2)->color()); \
    config->writeEntry("Brush Color 4", a.at(3)->color()); \
    config->writeEntry("Brush Style 1", a.at(0)->style()); \
    config->writeEntry("Brush Style 2", a.at(1)->style()); \
    config->writeEntry("Brush Style 3", a.at(2)->style()); \
    config->writeEntry("Brush Style 4", a.at(3)->style());

#define DEFAULT_PEN(a,b,C_1,C_2,C_3,C_4,P_1,P_2,P_3,P_4) \
    a.at(0)->setColor(C_1); \
    a.at(0)->setWidth(P_1); \
    a.at(1)->setColor(C_2); \
    a.at(1)->setWidth(P_2); \
    a.at(2)->setColor(C_3); \
    a.at(2)->setWidth(P_3); \
    a.at(3)->setColor(C_4); \
    a.at(3)->setWidth(P_4); \
    b[0] = true; \
    b[1] = true; \
    b[2] = true; \
    b[3] = true;

#define DEFAULT_BRUSH(a, C_1, C_2, C_3, C_4, S_1, S_2, S_3, S_4) \
    a.at(0)->setColor(C_1); \
    a.at(0)->setStyle(S_1); \
    a.at(1)->setColor(C_2); \
    a.at(1)->setStyle(S_2); \
    a.at(2)->setColor(C_3); \
    a.at(2)->setStyle(S_3); \
    a.at(3)->setColor(C_4); \
    a.at(3)->setStyle(S_4);

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
  brush->insertItem(QPixmap(picDir + "brush12.png"), Qt::FDiagPattern);

#define BUTTONROW(penC, penW, penS, brushC, brushS, row) \
  penC = new KColorButton(mapPage); \
  penC->setMaximumWidth(35); \
  penW = new QSpinBox(1, 9, 1, mapPage); \
  penW->setMaximumWidth(40); \
  penS = new KComboBox(mapPage); \
  penS->setMaximumWidth(45); \
  brushC = new KColorButton(mapPage); \
  brushC->setMaximumWidth(35); \
  brushS = new KComboBox(mapPage); \
  brushS->setMaximumWidth(45); \
  FILLSTYLE(penS, brushS) \
  penC->setMaximumHeight(brushS->sizeHint().height()); \
  brushC->setMaximumHeight(brushS->sizeHint().height()); \
  elLayout->addWidget(penC, row, 3); \
  elLayout->addWidget(penW, row, 5); \
  elLayout->addWidget(penS, row, 7); \
  elLayout->addWidget(brushC, row, 9); \
  elLayout->addWidget(brushS, row, 11);


KFLogConfig::KFLogConfig(QWidget* parent, KConfig* cnf, const char* name)
  : KDialogBase(IconList, i18n("KFlog Setup"), Ok|Cancel, Ok,
      parent, name, true, true),
    config(cnf), oldElement(-1)
{
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

  __addIDTab();
  __addPathTab();
  __addScaleTab();
  __addMapTab();
//  __addPrintTab();

  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

KFLogConfig::~KFLogConfig()
{

}

void KFLogConfig::slotOk()
{
  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

  config->setGroup("General Options");
  config->writeEntry("Version", "2.0.2");

  config->setGroup("Path");
  config->writeEntry("DefaultFlightDirectory", igcPathE->text());
  config->writeEntry("DefaultTaskDirectory", taskPathE->text());
  config->writeEntry("DefaultMapDirectory", mapPathE->text());

  config->setGroup("Scale");
  config->writeEntry("Lower Limit", lLimitN->value());
  config->writeEntry("Upper Limit", uLimitN->value());
  config->writeEntry("Switch Scale", switchScaleN->value());
  config->writeEntry("Border 1", reduce1N->value());
  config->writeEntry("Border 2", reduce2N->value());
  config->writeEntry("Border 3", reduce3N->value());

  config->setGroup("Road");
  WRITE_DRAW_VALUES(roadPenList, roadBorder);

  config->setGroup("Highway");
  WRITE_DRAW_VALUES(highwayPenList, highwayBorder);

  config->setGroup("Rail");
  WRITE_DRAW_VALUES(railPenList, railBorder);

  config->setGroup("River");
  WRITE_DRAW_VALUES(riverPenList, riverBorder);

  config->setGroup("City");
  WRITE_DRAW_VALUES(cityPenList, cityBorder);

  config->setGroup("Airspace C");
  WRITE_DRAW_VALUES(airCPenList, airCBorder);
  WRITE_BRUSH_VALUES(airCBrushList);

  config->setGroup("Airspace D");
  WRITE_DRAW_VALUES(airDPenList, airDBorder);
  WRITE_BRUSH_VALUES(airDBrushList);

  config->setGroup("Airspace E low");
  WRITE_DRAW_VALUES(airElPenList, airElBorder);
  WRITE_BRUSH_VALUES(airElBrushList);

  config->setGroup("Airspace E high");
  WRITE_DRAW_VALUES(airEhPenList, airEhBorder);
  WRITE_BRUSH_VALUES(airEhBrushList);

  config->setGroup("Airspace F");
  WRITE_DRAW_VALUES(airFPenList, airFBorder);
  WRITE_BRUSH_VALUES(airFBrushList);

  config->setGroup("Control C");
  WRITE_DRAW_VALUES(ctrCPenList, ctrCBorder);
  WRITE_BRUSH_VALUES(ctrCBrushList);

  config->setGroup("Control D");
  WRITE_DRAW_VALUES(ctrDPenList, ctrDBorder);
  WRITE_BRUSH_VALUES(ctrDBrushList);

  config->setGroup("Danger");
  WRITE_DRAW_VALUES(dangerPenList, dangerBorder);
  WRITE_BRUSH_VALUES(dangerBrushList);

  config->setGroup("Low Flight");
  WRITE_DRAW_VALUES(lowFPenList, lowFBorder);
  WRITE_BRUSH_VALUES(lowFBrushList);

  config->setGroup("Restricted Area");
  WRITE_DRAW_VALUES(restrPenList, restrBorder);
  WRITE_BRUSH_VALUES(restrBrushList);

  config->setGroup("TMZ");
  WRITE_DRAW_VALUES(tmzPenList, tmzBorder);
  WRITE_BRUSH_VALUES(tmzBrushList);

  config->setGroup("Map Data");
  config->writeEntry("Homesite Latitude",
      MapContents::degreeToNum(homeLatE->text()));
  config->writeEntry("Homesite Longitude",
      MapContents::degreeToNum(homeLonE->text()));
  config->sync();

  emit scaleChanged(lLimitN->value(), uLimitN->value());
  accept();
}

void KFLogConfig::slotSearchFlightPath()
{
  QString temp = KFileDialog::getExistingDirectory(igcPathE->text(), this,
      i18n("Select Default-Directory for the IGC-Files"));

  if(temp != 0)  igcPathE->setText(temp);
}

void KFLogConfig::slotSearchMapPath()
{
  QString temp = KFileDialog::getExistingDirectory(mapPathE->text(), this,
      i18n("Select Directory for the Map-Files"));

  if(temp != 0)  mapPathE->setText(temp);
}

void KFLogConfig::slotSearchTaskPath()
{
  QString temp = KFileDialog::getExistingDirectory(taskPathE->text(), this,
      i18n("Select Default-Directory for the Task-Files"));

  if(temp != 0)  taskPathE->setText(temp);
}

void KFLogConfig::slotSelectElement(int elementID)
{
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

void KFLogConfig::slotToggleFirst(bool toggle)
{
  border1Button->setEnabled(toggle);
  border1Pen->setEnabled(toggle);
  border1Color->setEnabled(toggle);
  border2->setEnabled(toggle);

  switch(elementSelect->currentItem())
    {
      case City:
        border1PenStyle->setEnabled(false);
        border1BrushColor->setEnabled(toggle);
        border1BrushStyle->setEnabled(false);
        break;
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

void KFLogConfig::slotToggleSecond(bool toggle)
{
  border2Button->setEnabled(toggle);
  border2Pen->setEnabled(toggle);
  border2Color->setEnabled(toggle);
  border3->setEnabled(toggle);

  switch(elementSelect->currentItem())
    {
      case City:
        border2PenStyle->setEnabled(false);
        border2BrushColor->setEnabled(toggle);
        border2BrushStyle->setEnabled(false);
        break;
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

void KFLogConfig::slotToggleThird(bool toggle)
{
  border3Button->setEnabled(toggle);
  border3Pen->setEnabled(toggle);
  border3Color->setEnabled(toggle);
  border4->setEnabled(toggle);

  switch(elementSelect->currentItem())
    {
      case City:
        border3PenStyle->setEnabled(false);
        border3BrushColor->setEnabled(toggle);
        border3BrushStyle->setEnabled(false);
        break;
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

void KFLogConfig::slotToggleForth(bool toggle)
{
  border4Color->setEnabled(toggle);
  border4Pen->setEnabled(toggle);

  switch(elementSelect->currentItem())
    {
      case City:
        border4PenStyle->setEnabled(false);
        border4BrushColor->setEnabled(toggle);
        border4BrushStyle->setEnabled(false);
        break;
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

void KFLogConfig::slotSetSecond()
{
  border2Color->setColor(border1Color->color());
  border2Pen->setValue(border1Pen->value());
  border2PenStyle->setCurrentItem(border1PenStyle->currentItem());
  border2BrushColor->setColor(border1BrushColor->color());
  border2BrushStyle->setCurrentItem(border1BrushStyle->currentItem());
}

void KFLogConfig::slotSetThird()
{
  border3Color->setColor(border2Color->color());
  border3Pen->setValue(border2Pen->value());
  border3PenStyle->setCurrentItem(border2PenStyle->currentItem());
  border3BrushColor->setColor(border2BrushColor->color());
  border3BrushStyle->setCurrentItem(border2BrushStyle->currentItem());
}

void KFLogConfig::slotSetForth()
{
  border4Color->setColor(border3Color->color());
  border4Pen->setValue(border3Pen->value());
  border4PenStyle->setCurrentItem(border3PenStyle->currentItem());
  border4BrushColor->setColor(border3BrushColor->color());
  border4BrushStyle->setCurrentItem(border3BrushStyle->currentItem());
}

void KFLogConfig::slotShowLowerLimit(int value)
{
  lLimitN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowUpperLimit(int value)
{
  uLimitN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowSwitchScale(int value)
{
  switchScaleN->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleA(int value)
{
  reduce1N->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleB(int value)
{
  reduce2N->display(__setScaleValue(value));
}

void KFLogConfig::slotShowReduceScaleC(int value)
{
  reduce3N->display(__setScaleValue(value));
}

void KFLogConfig::slotDefaultElements()
{
  DEFAULT_PEN(roadPenList, roadBorder, ROAD_COLOR_1, ROAD_COLOR_2,
      ROAD_COLOR_3, ROAD_COLOR_4, ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4)

  DEFAULT_PEN(highwayPenList, highwayBorder, HIGH_COLOR_1, HIGH_COLOR_2,
      HIGH_COLOR_3, HIGH_COLOR_4, HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4)

  DEFAULT_PEN(riverPenList, riverBorder, RIVER_COLOR_1, RIVER_COLOR_2,
      RIVER_COLOR_3, RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3,
      RIVER_PEN_4)

  DEFAULT_PEN(railPenList, railBorder, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3,
      RAIL_COLOR_4, RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4)

  DEFAULT_PEN(cityPenList, cityBorder, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
      CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4)
  DEFAULT_BRUSH(cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
      CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, CITY_BRUSH_STYLE_1,
      CITY_BRUSH_STYLE_2, CITY_BRUSH_STYLE_3, CITY_BRUSH_STYLE_4)

  DEFAULT_PEN(airCPenList, airCBorder, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3,
      AIRC_COLOR_4, AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4)
  DEFAULT_BRUSH(airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
      AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
      AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)

  DEFAULT_PEN(airDPenList, airDBorder, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3,
      AIRD_COLOR_4, AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4)
  DEFAULT_BRUSH(airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
      AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
      AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)

  DEFAULT_PEN(airElPenList, airElBorder, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
      AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4)
  DEFAULT_BRUSH(airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
      AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
      AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4)

  DEFAULT_PEN(airEhPenList, airEhBorder, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
      AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4)
  DEFAULT_BRUSH(airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
      AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
      AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4)

  DEFAULT_PEN(ctrCPenList, ctrCBorder, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3,
      CTRC_COLOR_4, CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4)
  DEFAULT_BRUSH(ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
      CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
      CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)

  DEFAULT_PEN(ctrDPenList, ctrDBorder, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3,
      CTRD_COLOR_4, CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4)
  DEFAULT_BRUSH(ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
      CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
      CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)

  oldElement = -1;
  slotSelectElement(elementSelect->currentItem());
}

void KFLogConfig::slotDefaultScale()
{
  lLimit->setValue(__getScaleValue(L_LIMIT));
  lLimitN->display(L_LIMIT);
  uLimit->setValue(__getScaleValue(U_LIMIT));
  uLimitN->display(U_LIMIT);
  switchScale->setValue(__getScaleValue(SWITCH_S));
  switchScaleN->display(SWITCH_S);
  reduce1->setValue(__getScaleValue(BORDER_1));
  reduce1N->display(BORDER_1);
  reduce2->setValue(__getScaleValue(BORDER_2));
  reduce2N->display(BORDER_2);
  reduce3->setValue(__getScaleValue(BORDER_3));
  reduce3N->display(BORDER_3);
}

void KFLogConfig::slotDefaultPath()
{
  igcPathE->setText(getpwuid(getuid())->pw_dir);
  taskPathE->setText(getpwuid(getuid())->pw_dir);
  mapPathE->setText(KGlobal::dirs()->findResource("appdata", "mapdata/"));
}

void KFLogConfig::__addMapTab()
{
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

  /*
   * In version <= 2.0.1, the fillcolor of cities is called "Color" instead
   * of "Brush Color", so we must look, which version of configfile we read.
   */
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

  mapPage = addPage(i18n("Map-Elements"),i18n("Map Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("kflog", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* elementBox = new QGroupBox(mapPage, "elementBox");
  elementBox->setTitle(i18n("visible Map-Elements"));

  elementSelect = new KComboBox(mapPage, "elementBox");
  elementSelect->setMaximumWidth(300);
  elementSelect->insertItem(i18n("Road"), Road);
  elementSelect->insertItem(i18n("Highway"), Highway);
  elementSelect->insertItem(i18n("Railway"), Railway);
  elementSelect->insertItem(i18n("River / Lake"), River);
  elementSelect->insertItem(i18n("Canal"), Canal);
  elementSelect->insertItem(i18n("City"), City);
  elementSelect->insertItem(i18n("Airspace C"), AirC);
  elementSelect->insertItem(i18n("Airspace D"), AirD);
  elementSelect->insertItem(i18n("Airspace E (low)"), AirElow);
  elementSelect->insertItem(i18n("Airspace E (high)"), AirEhigh);
  elementSelect->insertItem(i18n("Airspace F"), AirF);
  elementSelect->insertItem(i18n("Control C"), ControlC);
  elementSelect->insertItem(i18n("Control D"), ControlD);
  elementSelect->insertItem(i18n("Danger"), Danger);
  elementSelect->insertItem(i18n("Low flight area"), LowFlight);
  elementSelect->insertItem(i18n("Restricted"), Restricted);
  elementSelect->insertItem(i18n("TMZ"), TMZ);

  border1 = new QCheckBox(i18n("border #1"), mapPage);
  border2 = new QCheckBox(i18n("border #2"), mapPage);
  border3 = new QCheckBox(i18n("border #3"), mapPage);
  border4 = new QCheckBox(i18n("scale-limit"), mapPage);

  QString picDir = KGlobal::dirs()->findResource("appdata", "pics/");

  border1Button = new QPushButton(mapPage);
  border1Button->setPixmap(BarIcon("down"));
  border1Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border1Button->setFixedHeight(border1Button->sizeHint().height() + 3);
  border2Button = new QPushButton(mapPage);
  border2Button->setPixmap(BarIcon("down"));
  border2Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border2Button->setFixedHeight(border1Button->sizeHint().height() + 3);
  border3Button = new QPushButton(mapPage);
  border3Button->setPixmap(BarIcon("down"));
  border3Button->setFixedWidth(border1Button->sizeHint().width() + 3);
  border3Button->setFixedHeight(border1Button->sizeHint().height() + 3);

  QPushButton* defaultElements = new QPushButton(i18n("Default"), mapPage,
      "defaultElements");
  defaultElements->setMaximumWidth(defaultElements->sizeHint().width() + 10);
  defaultElements->setMinimumHeight(defaultElements->sizeHint().height() + 2);

  QGridLayout* elLayout = new QGridLayout(mapPage, 15, 15, 12, 1);
  elLayout->addMultiCellWidget(elementBox, 0, 12, 0, 14);
  elLayout->addMultiCellWidget(elementSelect, 1, 1, 1, 5, AlignLeft);
  elLayout->addWidget(new QLabel(i18n("draw up to"), mapPage), 3, 1);
  elLayout->addMultiCellWidget(new QLabel(i18n("Pen"), mapPage), 3, 3, 3, 7);
  elLayout->addMultiCellWidget(new QLabel(i18n("Brush"), mapPage), 3, 3, 9, 11);
  elLayout->addWidget(border1, 5, 1);
  elLayout->addWidget(border1Button, 5, 13);
  elLayout->addWidget(border2, 7, 1);
  elLayout->addWidget(border2Button, 7, 13);
  elLayout->addWidget(border3, 9, 1);
  elLayout->addWidget(border3Button, 9, 13);
  elLayout->addWidget(border4, 11, 1);

  BUTTONROW(border1Color, border1Pen, border1PenStyle, border1BrushColor,
      border1BrushStyle, 5);

  BUTTONROW(border2Color, border2Pen, border2PenStyle, border2BrushColor,
      border2BrushStyle, 7);

  BUTTONROW(border3Color, border3Pen, border3PenStyle, border3BrushColor,
      border3BrushStyle, 9);

  BUTTONROW(border4Color, border4Pen, border4PenStyle, border4BrushColor,
      border4BrushStyle, 11);

  elLayout->addMultiCellWidget(defaultElements, 14, 14, 0, 1, AlignLeft);

  elLayout->addRowSpacing(0, 25);
  elLayout->addRowSpacing(2, 5);
  elLayout->setRowStretch(2, 1);
  elLayout->setRowStretch(4, 1);
  elLayout->setRowStretch(6, 1);
  elLayout->setRowStretch(8, 1);
  elLayout->setRowStretch(10, 1);
  elLayout->addRowSpacing(12, 10);
  elLayout->addRowSpacing(13, 15);
  elLayout->setRowStretch(13, 3);

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

  connect(defaultElements, SIGNAL(clicked()), SLOT(slotDefaultElements()));
  connect(elementSelect, SIGNAL(activated(int)),
      SLOT(slotSelectElement(int)));

  slotSelectElement(elementSelect->currentItem());
}

void KFLogConfig::__addScaleTab()
{
  config->setGroup("Scale");
  int ll = config->readNumEntry("Lower Limit", L_LIMIT);
  int ul = config->readNumEntry("Upper Limit", U_LIMIT);
  int sw = config->readNumEntry("Switch Scale", SWITCH_S);
  int b1 = config->readNumEntry("Border 1", BORDER_1);
  int b2 = config->readNumEntry("Border 2", BORDER_2);
  int b3 = config->readNumEntry("Border 3", BORDER_3);

  scalePage = addPage(i18n("Map-Scales"),i18n("Map-Scale Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("viewmag", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* scaleLimits = new QGroupBox(scalePage, "scaleLimitBox");
  scaleLimits->setTitle(i18n("Scale-Range:"));

  QLabel* lLimitText = new QLabel(i18n("lower limit"), scalePage);
  lLimit = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  lLimit->setMinimumHeight(lLimit->sizeHint().height() + 5);
  lLimit->setMaximumHeight(lLimit->sizeHint().height() + 20);
  lLimit->setMinimumWidth(200);
  lLimitN = new QLCDNumber(5, scalePage);
  lLimitN->setMinimumWidth(lLimitN->sizeHint().width() + 10);
  lLimit->setValue(__getScaleValue(ll));
  lLimitN->display(ll);

  QLabel* uLimitText = new QLabel(i18n("upper limit"), scalePage);
  uLimit = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  uLimit->setMinimumHeight(uLimit->sizeHint().height() + 5);
  uLimit->setMaximumHeight(uLimit->sizeHint().height() + 20);
  uLimitN = new QLCDNumber(5, scalePage);
  uLimit->setValue(__getScaleValue(ul));
  uLimitN->display(ul);

  QGroupBox* borderBox = new QGroupBox(scalePage, "borderBox");
  borderBox->setTitle("Scale-Borders:");

  QLabel* switchText = new QLabel(i18n("use small icons"), scalePage);

  switchScale = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  switchScale->setMinimumHeight(switchScale->sizeHint().height() + 5);
  switchScale->setMaximumHeight(switchScale->sizeHint().height() + 20);
  switchScaleN = new QLCDNumber(5, scalePage);
  switchScale->setValue(__getScaleValue(sw));
  switchScaleN->display(sw);

  QLabel* reduce1Text = new QLabel(i18n("border #1"), scalePage);
  reduce1 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce1->setMinimumHeight(reduce1->sizeHint().height() + 5);
  reduce1->setMaximumHeight(reduce1->sizeHint().height() + 20);
  reduce1N = new QLCDNumber(5, scalePage);
  reduce1->setValue(__getScaleValue(b1));
  reduce1N->display(b1);

  QLabel* reduce2Text = new QLabel(i18n("border #2"), scalePage);
  reduce2 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce2->setMinimumHeight(reduce2->sizeHint().height() + 5);
  reduce2->setMaximumHeight(reduce2->sizeHint().height() + 20);
  reduce2N = new QLCDNumber(5, scalePage);
  reduce2->setValue(__getScaleValue(b2));
  reduce2N->display(b2);

  QLabel* reduce3Text = new QLabel(i18n("border #3"), scalePage);
  reduce3 = new QSlider(2,105,1,0, QSlider::Horizontal, scalePage);
  reduce3->setMinimumHeight(reduce3->sizeHint().height() + 5);
  reduce3->setMaximumHeight(reduce3->sizeHint().height() + 20);
  reduce3N = new QLCDNumber(5, scalePage);
  reduce3->setValue(__getScaleValue(b3));
  reduce3N->display(b3);

  QPushButton* defaultScale = new QPushButton(i18n("Default"), scalePage,
      "defaultScale");
  defaultScale->setMaximumWidth(defaultScale->sizeHint().width() + 10);
  defaultScale->setMinimumHeight(defaultScale->sizeHint().height() + 2);

  QGridLayout* scaleLayout = new QGridLayout(scalePage, 17, 7, 12, 1);
  scaleLayout->addMultiCellWidget(scaleLimits, 0, 4, 0, 6);
  scaleLayout->addWidget(lLimitText, 1, 1);
  scaleLayout->addWidget(lLimit, 1, 3);
  scaleLayout->addWidget(lLimitN, 1, 5);
  scaleLayout->addWidget(uLimitText, 3, 1);
  scaleLayout->addWidget(uLimit, 3, 3);
  scaleLayout->addWidget(uLimitN, 3, 5);

  scaleLayout->addMultiCellWidget(borderBox, 6, 14, 0, 6);
  scaleLayout->addWidget(switchText, 7, 1);
  scaleLayout->addWidget(switchScale, 7, 3);
  scaleLayout->addWidget(switchScaleN, 7, 5);
  scaleLayout->addWidget(reduce1Text, 9, 1);
  scaleLayout->addWidget(reduce1, 9, 3);
  scaleLayout->addWidget(reduce1N, 9, 5);
  scaleLayout->addWidget(reduce2Text, 11, 1);
  scaleLayout->addWidget(reduce2, 11, 3);
  scaleLayout->addWidget(reduce2N, 11, 5);
  scaleLayout->addWidget(reduce3Text, 13, 1);
  scaleLayout->addWidget(reduce3, 13, 3);
  scaleLayout->addWidget(reduce3N, 13, 5);

  scaleLayout->addMultiCellWidget(defaultScale, 16, 16, 0, 1, AlignLeft);

  scaleLayout->addColSpacing(0, 10);
  scaleLayout->addColSpacing(2, 5);
  scaleLayout->addColSpacing(4, 10);
  scaleLayout->addColSpacing(6, 10);

  scaleLayout->setColStretch(3, 1);

  scaleLayout->addRowSpacing(0, 20);
  scaleLayout->addRowSpacing(4, 10);
  scaleLayout->addRowSpacing(5, 10);
  scaleLayout->addRowSpacing(6, 20);
  scaleLayout->addRowSpacing(14, 10);
  scaleLayout->addRowSpacing(15, 10);

  connect(defaultScale, SIGNAL(clicked()), SLOT(slotDefaultScale()));
  connect(lLimit, SIGNAL(valueChanged(int)), SLOT(slotShowLowerLimit(int)));
  connect(uLimit, SIGNAL(valueChanged(int)), SLOT(slotShowUpperLimit(int)));
  connect(switchScale, SIGNAL(valueChanged(int)),
      SLOT(slotShowSwitchScale(int)));
  connect(reduce1, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleA(int)));
  connect(reduce2, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleB(int)));
  connect(reduce3, SIGNAL(valueChanged(int)), SLOT(slotShowReduceScaleC(int)));
}

void KFLogConfig::__addPathTab()
{
  config->setGroup("Path");
  QString flightDir = config->readEntry("DefaultFlightDirectory",
      getpwuid(getuid())->pw_dir);
  QString taskDir = config->readEntry("DefaultTaskDirectory",
      getpwuid(getuid())->pw_dir);
  QString mapDir = config->readEntry("DefaultMapDirectory",
      KGlobal::dirs()->findResource("appdata", "mapdata/"));

  pathPage = addPage(i18n("Path"),i18n("Path Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("fileopen", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* pathLayout = new QGridLayout(pathPage, 13, 5, 12, 1);

  QGroupBox* igcGroup = new QGroupBox(pathPage, "igcGroup");
  igcGroup->setTitle(i18n("Flight-directory:"));

  igcPathE  = new QLineEdit(pathPage, "igcPathE");
  igcPathE->setMinimumWidth(150);
  igcPathE->setText(flightDir);

  QPushButton* igcPathSearch = new QPushButton(pathPage);
  igcPathSearch->setPixmap(BarIcon("fileopen"));
  igcPathSearch->setMinimumWidth(igcPathSearch->sizeHint().width() + 5);
  igcPathSearch->setMinimumHeight(igcPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(igcGroup, 0, 2, 0, 4);
  pathLayout->addWidget(igcPathE, 1, 1);
  pathLayout->addWidget(igcPathSearch, 1, 3);

  pathLayout->addColSpacing(0, 10);
  pathLayout->addColSpacing(2, 10);
  pathLayout->addColSpacing(4, 10);

  pathLayout->addRowSpacing(0, 20);
  pathLayout->addRowSpacing(1, 10);
  pathLayout->addRowSpacing(2, 10);

  pathLayout->addRowSpacing(3, 20);
  pathLayout->setRowStretch(3, 1);

  QGroupBox* taskGroup = new QGroupBox(pathPage, "taskGroup");
  taskGroup->setTitle(i18n("Task-directory:"));

  taskPathE = new QLineEdit(pathPage, "taskPathE");
  taskPathE->setMinimumWidth(150);
  taskPathE->setText(taskDir);

  QPushButton* taskPathSearch = new QPushButton(pathPage);
  taskPathSearch->setPixmap(BarIcon("fileopen"));
  taskPathSearch->setMinimumWidth(taskPathSearch->sizeHint().width() + 5);
  taskPathSearch->setMinimumHeight(taskPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(taskGroup, 4, 6, 0, 4);
  pathLayout->addWidget(taskPathE, 5, 1);
  pathLayout->addWidget(taskPathSearch, 5, 3);

  pathLayout->addRowSpacing(4, 20);
  pathLayout->addRowSpacing(5, 10);
  pathLayout->addRowSpacing(6, 10);

  pathLayout->addRowSpacing(7, 20);
  pathLayout->setRowStretch(7, 1);

  QGroupBox* mapGroup = new QGroupBox(pathPage, "mapGroup");
  mapGroup->setTitle(i18n("Map-directory:"));

  mapPathE = new QLineEdit(pathPage, "mapPathE");
  mapPathE->setMinimumWidth(150);
  mapPathE->setText(mapDir);

  QPushButton* mapPathSearch = new QPushButton(pathPage);
  mapPathSearch->setPixmap(BarIcon("fileopen"));
  mapPathSearch->setMinimumWidth(mapPathSearch->sizeHint().width() + 5);
  mapPathSearch->setMinimumHeight(mapPathSearch->sizeHint().height() + 5);

  pathLayout->addMultiCellWidget(mapGroup, 8, 10, 0, 4);
  pathLayout->addWidget(mapPathE, 9, 1);
  pathLayout->addWidget(mapPathSearch, 9, 3);

  pathLayout->addRowSpacing(8, 20);
  pathLayout->addRowSpacing(9, 10);
  pathLayout->addRowSpacing(10, 10);

  QPushButton* defaultPath = new QPushButton(i18n("Default"), pathPage,
      "defaultPath");
  defaultPath->setMaximumWidth(defaultPath->sizeHint().width() + 10);
  defaultPath->setMinimumHeight(defaultPath->sizeHint().height() + 2);

  pathLayout->addMultiCellWidget(defaultPath, 12, 12, 0, 1, AlignLeft);

  pathLayout->addRowSpacing(11, 10);
  pathLayout->setRowStretch(11, 1);

  connect(igcPathSearch, SIGNAL(clicked()), SLOT(slotSearchFlightPath()));
  connect(mapPathSearch, SIGNAL(clicked()), SLOT(slotSearchMapPath()));
  connect(taskPathSearch, SIGNAL(clicked()), SLOT(slotSearchTaskPath()));
  connect(defaultPath, SIGNAL(clicked()), SLOT(slotDefaultPath()));
}

void KFLogConfig::__addPrintTab()
{
  printPage = addPage(i18n("Print"),i18n("Printer Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("fileprint", KIcon::NoGroup,
          KIcon::SizeLarge));
}

void KFLogConfig::__addIDTab()
{
  config->setGroup("Map Data");
  int homeLat = config->readNumEntry("Homesite Latitude", HOME_DEFAULT_LAT);
  int homeLon = config->readNumEntry("Homesite Longitude", HOME_DEFAULT_LON);
  QString homeName = config->readEntry("Homesite", "");

  idPage = addPage(i18n("Identity"),i18n("Personal Information"),
      KGlobal::instance()->iconLoader()->loadIcon("identity", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* idLayout = new QGridLayout(idPage, 13, 5, 12, 1);

  QGroupBox* homeGroup = new QGroupBox(idPage, "homeGroup");
  homeGroup->setTitle(i18n("Homesite:"));

  homeNameE = new QLineEdit(idPage, "homeNameE");
  homeNameE->setMinimumWidth(150);
  homeLatE  = new QLineEdit(idPage, "homeLatE");
  homeLatE->setMinimumWidth(150);
  homeLatE->setText(printPos(homeLat, true));
  homeLonE  = new QLineEdit(idPage, "homeLonE");
  homeLonE->setMinimumWidth(150);
  homeLonE->setText(printPos(homeLon, false));

  idLayout->addMultiCellWidget(homeGroup, 0, 6, 0, 4);
  idLayout->addWidget(new QLabel(i18n("Homesite"), idPage), 1, 1);
  idLayout->addWidget(homeNameE, 1, 3);
  idLayout->addWidget(new QLabel(i18n("Latitude"), idPage), 3, 1);
  idLayout->addWidget(homeLatE, 3, 3);
  idLayout->addWidget(new QLabel(i18n("Longitude"), idPage), 5, 1);
  idLayout->addWidget(homeLonE, 5, 3);

  idLayout->addColSpacing(0, 10);
  idLayout->addColSpacing(2, 10);
  idLayout->addColSpacing(4, 10);

  idLayout->addRowSpacing(0, 20);
  idLayout->addRowSpacing(1, 10);
  idLayout->addRowSpacing(2, 10);

  idLayout->addRowSpacing(3, 20);
}

int KFLogConfig::__setScaleValue(int value)
{
  if(value <= 40) return (value * 5);
  else if(value <= 70) return (200 + (value - 40) * 10);
  else if(value <= 95) return (500 + (value - 70) * 20);
  else if(value <= 105) return (1000 + (value - 95) * 50);
  else return (2000 + (value - 105) * 100);
}

int KFLogConfig::__getScaleValue(double scale)
{
  if(scale <= 200) return ((int) scale / 5);
  else if(scale <= 500) return (((int) scale - 200) / 10 + 40);
  else if(scale <= 1000) return (((int) scale - 500) / 20 + 70);
  else if(scale <= 2000) return (((int) scale - 1000) / 50 + 95);
  else return (((int) scale - 2000) / 100 + 125);
}
