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
    b[1] = border2->isChecked(); \
    a.at(1)->setColor(border2Color->color()); \
    a.at(1)->setWidth(border2Pen->value()); \
    b[2] = border3->isChecked(); \
    a.at(2)->setColor(border3Color->color()); \
    a.at(2)->setWidth(border3Pen->value()); \
    b[3] = border4->isChecked(); \
    a.at(3)->setColor(border4Color->color()); \
    a.at(3)->setWidth(border4Pen->value());

#define SHOW_PEN(a,b) \
    border1->setChecked(b[0]); \
    border1Color->setColor(a.at(0)->color()); \
    border1Pen->setValue(a.at(0)->width()); \
    border2->setChecked(b[1]); \
    border2Color->setColor(a.at(1)->color()); \
    border2Pen->setValue(a.at(1)->width()); \
    border3->setChecked(b[2]); \
    border3Color->setColor(a.at(2)->color()); \
    border3Pen->setValue(a.at(2)->width()); \
    border4->setChecked(b[3]); \
    border4Color->setColor(a.at(3)->color()); \
    border4Pen->setValue(a.at(3)->width());

#define READ_BORDER(a) \
    a[0] = config->readBoolEntry("Border 1", true); \
    a[1] = config->readBoolEntry("Border 2", true); \
    a[2] = config->readBoolEntry("Border 3", true); \
    a[3] = config->readBoolEntry("Border 4", true); \
    a[4] = config->readBoolEntry("Border 5", true);

#define WRITE_DRAW_VALUES(a,b) \
    config->writeEntry("Color 1", a.at(0)->color()); \
    config->writeEntry("Color 2", a.at(1)->color()); \
    config->writeEntry("Color 3", a.at(2)->color()); \
    config->writeEntry("Color 4", a.at(3)->color()); \
    config->writeEntry("Pen Size 1", a.at(0)->width()); \
    config->writeEntry("Pen Size 2", a.at(1)->width()); \
    config->writeEntry("Pen Size 3", a.at(2)->width()); \
    config->writeEntry("Pen Size 4", a.at(3)->width()); \
    config->writeEntry("Border 1", b[0]); \
    config->writeEntry("Border 2", b[1]); \
    config->writeEntry("Border 3", b[2]); \
    config->writeEntry("Border 4", b[3]);

#define DEFAULT_PEN(a,b,C_1,C_2,C_3,C_4,P_1,P_2,P_3,P_4) \
    a.at(0)->setColor(C_1); \
    a.at(1)->setColor(C_2); \
    a.at(2)->setColor(C_3); \
    a.at(3)->setColor(C_4); \
    a.at(0)->setWidth(P_1); \
    a.at(1)->setWidth(P_2); \
    a.at(2)->setWidth(P_3); \
    a.at(3)->setWidth(P_4); \
    b[0] = true; \
    b[1] = true; \
    b[2] = true; \
    b[3] = true;

KFLogConfig::KFLogConfig(QWidget* parent, const char* name)
  : KDialogBase(IconList, i18n("KFlog Setup"), Ok|Cancel, Ok,
      parent, name, true, true),
    oldElement(-1)
{
  config = KGlobal::config();

  roadBorder = new bool[5];
  highwayBorder = new bool[5];
  railBorder = new bool[5];
  riverBorder = new bool[5];
  cityBorder = new bool[5];

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

  // Die aktuell angezeigten Angaben müssen noch gespeichert werden ...
  slotSelectElement(oldElement);

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
  config->writeEntry("Outline Color", cityPenList.at(4)->color());
  config->writeEntry("Outline Size", cityPenList.at(4)->width()); \
  config->writeEntry("Border 5", cityBorder[4]); \

  config->setGroup("Map Data");
  config->writeEntry("Homesite Latitude",
      MapContents::degreeToNum(homeLatE->text()));
  config->writeEntry("Homesite Longitude",
      MapContents::degreeToNum(homeLonE->text()));
  config->sync();

  extern MapContents _globalMapContents;
  _globalMapContents.readConfig();

  close();
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
  if(oldElement != -1)
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
            cityBorder[4] = outLine->isChecked();
            cityPenList.at(4)->setColor(outLineColor->color());
            cityPenList.at(4)->setWidth(outLinePen->value());
            break;
          }
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
        outLine->setChecked(cityBorder[4]);
        outLineColor->setColor(cityPenList.at(4)->color());
        break;
    }

  oldElement = elementID;
  slotToggleFirst(border1->isChecked());
}

void KFLogConfig::slotToggleFirst(bool toggle)
{
  border1Button->setEnabled(toggle);
  border1Color->setEnabled(toggle);
  border2->setEnabled(toggle);

  if(elementSelect->currentItem() == City)
    {
      border1Pen->setEnabled(false);
      outLine->setEnabled(toggle);
      slotToggleOutline(outLine->isChecked() && toggle);
    }
  else
    {
      border1Pen->setEnabled(toggle);
      outLine->setEnabled(false);
      outLine->setChecked(false);
      slotToggleOutline(false);
    }

  if(!toggle)
      slotToggleSecond(false);
  else
      slotToggleSecond(border2->isChecked());
}

void KFLogConfig::slotToggleSecond(bool toggle)
{
  border2Button->setEnabled(toggle);
  border2Color->setEnabled(toggle);
  border3->setEnabled(toggle);

  if(elementSelect->currentItem() == City)
      border2Pen->setEnabled(false);
  else
      border2Pen->setEnabled(toggle);

  if(!toggle)
      slotToggleThird(false);
  else
      slotToggleThird(border3->isChecked());
}

void KFLogConfig::slotToggleThird(bool toggle)
{
  border3Button->setEnabled(toggle);
  border3Color->setEnabled(toggle);
  border4->setEnabled(toggle);

  if(elementSelect->currentItem() == City)
      border3Pen->setEnabled(false);
  else
      border3Pen->setEnabled(toggle);

  if(!toggle)
      slotToggleForth(false);
  else
      slotToggleForth(border4->isChecked());
}

void KFLogConfig::slotToggleForth(bool toggle)
{
  border4Color->setEnabled(toggle);

  if(elementSelect->currentItem() == City)
      border4Pen->setEnabled(false);
  else
      border4Pen->setEnabled(toggle);
}

void KFLogConfig::slotToggleOutline(bool toggle)
{
  outLineColor->setEnabled(toggle);
  outLinePen->setEnabled(toggle);
}

void KFLogConfig::slotSetSecond()
{
  border2Color->setColor(border1Color->color());
  border2Pen->setValue(border1Pen->value());
}

void KFLogConfig::slotSetThird()
{
  border3Color->setColor(border2Color->color());
  border3Pen->setValue(border2Pen->value());
}

void KFLogConfig::slotSetForth()
{
  border4Color->setColor(border3Color->color());
  border4Pen->setValue(border3Pen->value());
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
      CITY_COLOR_4, 1, 1, 1, 1)

  cityPenList.at(4)->setColor(CITY_COLOR_5);
  cityPenList.at(4)->setWidth(CITY_OUTLINE);
  cityBorder[4] = true;

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
  mapPathE->setText(KGlobal::dirs()->findResource("data", "kflog/mapdata/"));
}

void KFLogConfig::__addMapTab()
{
  config->setGroup("Road");
  roadPenList.append(new QPen(config->readColorEntry("Color 1",
        new ROAD_COLOR_1),
        config->readNumEntry("Pen Size 1", ROAD_PEN_1)));
  roadPenList.append(new QPen(config->readColorEntry("Color 2",
        new ROAD_COLOR_2),
        config->readNumEntry("Pen Size 2", ROAD_PEN_2)));
  roadPenList.append(new QPen(config->readColorEntry("Color 3",
        new ROAD_COLOR_3),
        config->readNumEntry("Pen Size 3", ROAD_PEN_3)));
  roadPenList.append(new QPen(config->readColorEntry("Color 4",
        new ROAD_COLOR_4),
        config->readNumEntry("Pen Size 4", ROAD_PEN_4)));
  READ_BORDER(roadBorder);

  config->setGroup("River");
  riverPenList.append(new QPen(config->readColorEntry("Color 1",
        new RIVER_COLOR_1),
        config->readNumEntry("Pen Size 1", RIVER_PEN_1)));
  riverPenList.append(new QPen(config->readColorEntry("Color 2",
        new RIVER_COLOR_2),
        config->readNumEntry("Pen Size 2", RIVER_PEN_2)));
  riverPenList.append(new QPen(config->readColorEntry("Color 3",
        new RIVER_COLOR_3),
        config->readNumEntry("Pen Size 3", RIVER_PEN_3)));
  riverPenList.append(new QPen(config->readColorEntry("Color 4",
        new RIVER_COLOR_4),
        config->readNumEntry("Pen Size 4", RIVER_PEN_4)));
  READ_BORDER(riverBorder);

  config->setGroup("Rail");
  railPenList.append(new QPen(config->readColorEntry("Color 1",
        new RAIL_COLOR_1),
        config->readNumEntry("Pen Size 1", RAIL_PEN_1)));
  railPenList.append(new QPen(config->readColorEntry("Color 2",
        new RAIL_COLOR_2),
        config->readNumEntry("Pen Size 2", RAIL_PEN_2)));
  railPenList.append(new QPen(config->readColorEntry("Color 3",
        new RAIL_COLOR_3),
        config->readNumEntry("Pen Size 3", RAIL_PEN_3)));
  railPenList.append(new QPen(config->readColorEntry("Color 4",
        new RAIL_COLOR_4),
        config->readNumEntry("Pen Size 4", RAIL_PEN_4)));
  READ_BORDER(railBorder);

  config->setGroup("Highway");
  highwayPenList.append(new QPen(config->readColorEntry("Color 1",
        new HIGH_COLOR_1),
        config->readNumEntry("Pen Size 1", HIGH_PEN_1)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 2",
        new HIGH_COLOR_2),
        config->readNumEntry("Pen Size 2", HIGH_PEN_2)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 3",
        new HIGH_COLOR_3),
        config->readNumEntry("Pen Size 3", HIGH_PEN_3)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 4",
        new HIGH_COLOR_4),
        config->readNumEntry("Pen Size 4", HIGH_PEN_4)));
  READ_BORDER(highwayBorder);

  config->setGroup("City");
  cityPenList.append(new QPen(config->readColorEntry("Color 1",
        new CITY_COLOR_1),
        config->readNumEntry("Pen Size 1", 1)));
  cityPenList.append(new QPen(config->readColorEntry("Color 2",
        new CITY_COLOR_2),
        config->readNumEntry("Pen Size 2", 1)));
  cityPenList.append(new QPen(config->readColorEntry("Color 3",
        new CITY_COLOR_3),
        config->readNumEntry("Pen Size 3", 1)));
  cityPenList.append(new QPen(config->readColorEntry("Color 4",
        new CITY_COLOR_4),
        config->readNumEntry("Pen Size 4", 1)));
  cityPenList.append(new QPen(config->readColorEntry("Outline Color",
        new CITY_COLOR_5),
        config->readNumEntry("Outline Size", CITY_OUTLINE)));
  READ_BORDER(cityBorder);

  mapPage = addPage(i18n("Map-Elements"),i18n("Map Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("kflog", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGroupBox* elementBox = new QGroupBox(mapPage, "elementBox");
  elementBox->setTitle(i18n("visible Map-Elements"));

  elementSelect = new KComboBox(mapPage, "elementBox");
  // Diese Reihenfolge muss mit der von "ElementType" übereinstimmen
  // (siehe kflogconfig.h)
  elementSelect->insertItem(i18n("Road"));
  elementSelect->insertItem(i18n("Highway"));
  elementSelect->insertItem(i18n("Railway"));
  elementSelect->insertItem(i18n("River / Lake"));
  elementSelect->insertItem(i18n("City"));

  border1 = new QCheckBox(i18n("draw up to border #1"), mapPage);
  border2 = new QCheckBox(i18n("draw up to border #2"), mapPage);
  border3 = new QCheckBox(i18n("draw up to border #3"), mapPage);
  border4 = new QCheckBox(i18n("draw up to scale-limit"), mapPage);
  outLine = new QCheckBox(i18n("draw outline"), mapPage);

  border1Color = new KColorButton(mapPage);
  border2Color = new KColorButton(mapPage);
  border3Color = new KColorButton(mapPage);
  border4Color = new KColorButton(mapPage);
  outLineColor = new KColorButton(mapPage);

  border1Pen = new QSpinBox(1, 9, 1, mapPage);
  border2Pen = new QSpinBox(1, 9, 1, mapPage);
  border3Pen = new QSpinBox(1, 9, 1, mapPage);
  border4Pen = new QSpinBox(1, 9, 1, mapPage);
  outLinePen = new QSpinBox(1, 9, 1, mapPage);

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

  QGridLayout* elLayout = new QGridLayout(mapPage, 15, 9, 12, 1);
  elLayout->addMultiCellWidget(elementBox, 0, 12, 0, 8);
  elLayout->addWidget(elementSelect, 1, 1);
  elLayout->addWidget(border1, 3, 1);
  elLayout->addWidget(border1Color, 3, 3);
  elLayout->addWidget(border1Pen, 3, 5);
  elLayout->addWidget(border1Button, 3, 7);
  elLayout->addWidget(border2, 5, 1);
  elLayout->addWidget(border2Color, 5, 3);
  elLayout->addWidget(border2Pen, 5, 5);
  elLayout->addWidget(border2Button, 5, 7);
  elLayout->addWidget(border3, 7, 1);
  elLayout->addWidget(border3Color, 7, 3);
  elLayout->addWidget(border3Pen, 7, 5);
  elLayout->addWidget(border3Button, 7, 7);
  elLayout->addWidget(border4, 9, 1);
  elLayout->addWidget(border4Color, 9, 3);
  elLayout->addWidget(border4Pen, 9, 5);
  elLayout->addWidget(outLine, 11, 1);
  elLayout->addWidget(outLineColor, 11, 3);
  elLayout->addWidget(outLinePen, 11, 5);
  elLayout->addMultiCellWidget(defaultElements, 14, 14, 0, 1, AlignLeft);

  elLayout->addRowSpacing(0, 25);
  elLayout->addRowSpacing(10, 12);
  elLayout->addRowSpacing(12, 10);
  elLayout->addRowSpacing(13, 10);
  elLayout->setRowStretch(13, 1);

  elLayout->addColSpacing(0, 10);
  elLayout->setColStretch(2, 2);
  elLayout->setColStretch(4, 1);
  elLayout->setColStretch(6, 1);
  elLayout->addColSpacing(8, 10);

  connect(border1, SIGNAL(toggled(bool)), SLOT(slotToggleFirst(bool)));
  connect(border2, SIGNAL(toggled(bool)), SLOT(slotToggleSecond(bool)));
  connect(border3, SIGNAL(toggled(bool)), SLOT(slotToggleThird(bool)));
  connect(border4, SIGNAL(toggled(bool)), SLOT(slotToggleForth(bool)));
  connect(outLine, SIGNAL(toggled(bool)), SLOT(slotToggleOutline(bool)));
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
  lLimit->setMinimumWidth(250);
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
      KGlobal::dirs()->findResource("data", "kflog/mapdata/"));

  pathPage = addPage(i18n("Path"),i18n("Path Configuration"),
      KGlobal::instance()->iconLoader()->loadIcon("fileopen", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* pathLayout = new QGridLayout(pathPage, 13, 5, 12, 1);

  QGroupBox* igcGroup = new QGroupBox(pathPage, "igcGroup");
  igcGroup->setTitle(i18n("Flight-directory:"));

  igcPathE  = new QLineEdit(pathPage, "igcPathE");
  igcPathE->setMinimumWidth(200);
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

  QGroupBox* taskGroup = new QGroupBox(pathPage, "taskGroup");
  taskGroup->setTitle(i18n("Task-directory:"));

  taskPathE = new QLineEdit(pathPage, "taskPathE");
  taskPathE->setMinimumWidth(200);
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

  QGroupBox* mapGroup = new QGroupBox(pathPage, "mapGroup");
  mapGroup->setTitle(i18n("Map-directory:"));

  mapPathE = new QLineEdit(pathPage, "mapPathE");
  mapPathE->setMinimumWidth(200);
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
  int homeLat = config->readNumEntry("Homesite Latitude", 0);
  int homeLon = config->readNumEntry("Homesite Longitude", 0);

  idPage = addPage(i18n("Identity"),i18n("Personal Information"),
      KGlobal::instance()->iconLoader()->loadIcon("identity", KIcon::NoGroup,
          KIcon::SizeLarge));

  QGridLayout* idLayout = new QGridLayout(idPage, 13, 5, 12, 1);

  QGroupBox* homeGroup = new QGroupBox(idPage, "homeGroup");
  homeGroup->setTitle(i18n("Home-site:"));

  homeLatE  = new QLineEdit(idPage, "homeLatE");
  homeLatE->setMinimumWidth(200);
  homeLatE->setText(printPos(homeLat, true));
  homeLonE  = new QLineEdit(idPage, "homeLonE");
  homeLonE->setMinimumWidth(200);
  homeLonE->setText(printPos(homeLon, false));

  idLayout->addMultiCellWidget(homeGroup, 0, 4, 0, 4);
  idLayout->addWidget(new QLabel(i18n("Latitude"), idPage), 1, 1);
  idLayout->addWidget(homeLatE, 1, 3);
  idLayout->addWidget(new QLabel(i18n("Longitude"), idPage), 3, 1);
  idLayout->addWidget(homeLonE, 3, 3);

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
