/***********************************************************************
**
**   mapcontrolview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "mapcontrolview.h"

#include <kflog.h>
#include <mapcalc.h>
#include <mapmatrix.h>

#include <kiconloader.h>
#include <klocale.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>

#define DELTA 4

MapControlView::MapControlView(QWidget* parent)
  : QWidget(parent, "mapcontrolview")
{
  QLabel* mapControl = new QLabel("<B>" + i18n("Map-control:") + "</B>",
      parent);
  mapControl->setMinimumHeight(mapControl->sizeHint().height() + 5);

  QFrame* navFrame = new QFrame(parent);
  QPushButton* nwB = new QPushButton(navFrame);
  nwB->setPixmap(BarIcon("movemap_nw"));
  nwB->setFixedHeight(nwB->sizeHint().height() + DELTA);
  nwB->setFixedWidth(nwB->sizeHint().width() + DELTA);

  QPushButton* nB = new QPushButton(navFrame);
  nB->setPixmap(BarIcon("movemap_n"));
  nB->setFixedHeight(nB->sizeHint().height() + DELTA);
  nB->setFixedWidth(nB->sizeHint().width() + DELTA);

  QPushButton* neB = new QPushButton(navFrame);
  neB->setPixmap(BarIcon("movemap_ne"));
  neB->setFixedHeight(neB->sizeHint().height() + DELTA);
  neB->setFixedWidth(neB->sizeHint().width() + DELTA);

  QPushButton* wB = new QPushButton(navFrame);
  wB->setPixmap(BarIcon("movemap_e"));
  wB->setFixedHeight(wB->sizeHint().height() + DELTA);
  wB->setFixedWidth(wB->sizeHint().width() + DELTA);

  QPushButton* cenB = new QPushButton(navFrame);
  cenB->setPixmap(BarIcon("gohome"));
  cenB->setFixedHeight(cenB->sizeHint().height() + DELTA);
  cenB->setFixedWidth(cenB->sizeHint().width() + DELTA);

  QPushButton* eB = new QPushButton(navFrame);
  eB->setPixmap(BarIcon("movemap_w"));
  eB->setFixedHeight(eB->sizeHint().height() + DELTA);
  eB->setFixedWidth(eB->sizeHint().width() + DELTA);

  QPushButton* swB = new QPushButton(navFrame);
  swB->setPixmap(BarIcon("movemap_sw"));
  swB->setFixedHeight(swB->sizeHint().height() + DELTA);
  swB->setFixedWidth(swB->sizeHint().width() + DELTA);

  QPushButton* sB = new QPushButton(navFrame);
  sB->setPixmap(BarIcon("movemap_s"));
  sB->setFixedHeight(sB->sizeHint().height() + DELTA);
  sB->setFixedWidth(sB->sizeHint().width() + DELTA);

  QPushButton* seB = new QPushButton(navFrame);
  seB->setPixmap(BarIcon("movemap_se"));
  seB->setFixedHeight(seB->sizeHint().height() + DELTA);
  seB->setFixedWidth(seB->sizeHint().width() + DELTA);

  QGridLayout* navLayout = new QGridLayout(navFrame, 5, 5, 0, 2, "navLayout");
  navLayout->addWidget(nwB,1,1);
  navLayout->addWidget(nB,1,2);
  navLayout->addWidget(neB,1,3);
  navLayout->addWidget(wB,2,1);
  navLayout->addWidget(cenB,2,2);
  navLayout->addWidget(eB,2,3);
  navLayout->addWidget(swB,3,1);
  navLayout->addWidget(sB,3,2);
  navLayout->addWidget(seB,3,3);

  navLayout->setColStretch(0,1);
  navLayout->setColStretch(1,0);
  navLayout->setColStretch(2,0);
  navLayout->setColStretch(3,0);
  navLayout->setColStretch(4,1);

  navLayout->setRowStretch(0,1);
  navLayout->setRowStretch(1,0);
  navLayout->setRowStretch(2,0);
  navLayout->setRowStretch(3,0);
  navLayout->setRowStretch(4,1);
  navLayout->activate();

  QLabel* dimLabel = new QLabel(i18n("Height / Width [km]:"), parent);
  dimLabel->setMinimumHeight(dimLabel->sizeHint().height() + 5);
  dimText = new QLabel("125 / 130", parent);
  dimText->setAlignment( AlignCenter );

  dimText->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  dimText->setBackgroundMode( PaletteLight );

  QLabel* currentScaleLabel = new QLabel(i18n("Scale:"), parent);
  currentScaleLabel->setMinimumHeight(
          currentScaleLabel->sizeHint().height() + 10);
  currentScaleValue = new QLCDNumber(5,parent);

  QLabel* setScaleLabel = new QLabel(i18n("Set scale:"), parent);
  setScaleLabel->setMinimumWidth( setScaleLabel->sizeHint().width());

  setScaleLabel->setMinimumHeight(setScaleLabel->sizeHint().height());
  currentScaleSlider = new QSlider(2,105,1,0, QSlider::Horizontal,parent);
  currentScaleSlider->setMinimumHeight(
          currentScaleSlider->sizeHint().height());

  QGridLayout* controlLayout = new QGridLayout(parent,6,4,5,5, "controlLayout");

  controlLayout->addMultiCellWidget(mapControl,0,0,0,3);
  controlLayout->addMultiCellWidget(navFrame,1,3,0,1);
  controlLayout->addMultiCellWidget(dimLabel,1,1,2,3);
  controlLayout->addMultiCellWidget(dimText,2,2,2,3);
  controlLayout->addWidget(currentScaleLabel,3,2);
  controlLayout->addWidget(currentScaleValue,3,3);
  controlLayout->addWidget(setScaleLabel,4,0);
  controlLayout->addMultiCellWidget(currentScaleSlider,4,4,1,3);

  controlLayout->setColStretch(0,0);
  controlLayout->setColStretch(1,0);
  controlLayout->setColStretch(3,4);
  controlLayout->setRowStretch(0,0);
  controlLayout->setRowStretch(1,0);
  controlLayout->setRowStretch(2,0);
  controlLayout->setRowStretch(3,0);
  controlLayout->setRowStretch(4,0);
  controlLayout->setRowStretch(5,2);

  controlLayout->activate();

  connect(currentScaleSlider, SIGNAL(valueChanged(int)),
            SLOT(slotShowScaleChange(int)));
  connect(currentScaleSlider, SIGNAL(sliderReleased()),
            SLOT(slotSetScale()));

  extern MapMatrix _globalMapMatrix;
  connect(nwB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapNW()));
  connect(nB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapN()));
  connect(neB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapNE()));
  connect(wB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapW()));
  connect(cenB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotCenterToHome()));
  connect(eB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapE()));
  connect(swB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapSW()));
  connect(sB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapS()));
  connect(seB, SIGNAL(clicked()), &_globalMapMatrix, SLOT(slotMoveMapSE()));
}

MapControlView::~MapControlView()
{

}

void MapControlView::slotShowMapData(QSize mapSize)
{
  extern MapMatrix _globalMapMatrix;
  const double cScale = _globalMapMatrix.getScale();

  QString temp;

  temp.sprintf("<TT>%.1f / %.1f</TT>",
      mapSize.height() * cScale / 1000.0,
      mapSize.width() * cScale / 1000.0);
  dimText->setText(temp);

  currentScaleValue->display(cScale);
  currentScaleSlider->setValue(__getScaleValue(cScale));
}

void MapControlView::slotSetMinMaxValue(int min, int max)
{
  currentScaleSlider->setMinValue(__getScaleValue(min));
  currentScaleSlider->setMaxValue(__getScaleValue(max));
}

void MapControlView::slotSetScale()
{
  emit(scaleChanged(currentScaleValue->value()));
}

int MapControlView::__setScaleValue(int value)
{
  if(value <= 40) return (value * 5);
  else if(value <= 70) return (200 + (value - 40) * 10);
  else if(value <= 95) return (500 + (value - 70) * 20);
  else if(value <= 105) return (1000 + (value - 95) * 50);
  else return (2000 + (value - 105) * 100);
}

int MapControlView::__getScaleValue(double scale)
{
  if(scale <= 200) return ((int) scale / 5);
  else if(scale <= 500) return (((int) scale - 200) / 10 + 40);
  else if(scale <= 1000) return (((int) scale - 500) / 20 + 70);
  else if(scale <= 2000) return (((int) scale - 1000) / 50 + 95);
  else return (((int) scale - 2000) / 100 + 125);
}

void MapControlView::slotShowScaleChange(int value)
{
  extern MapMatrix _globalMapMatrix;

  currentScaleValue->display(__setScaleValue(value));

  if(currentScaleValue->value() > _globalMapMatrix.getScale(MapMatrix::UpperLimit))
      currentScaleSlider->setValue(_globalMapMatrix.getScale(MapMatrix::UpperLimit));

  if(currentScaleValue->value() < _globalMapMatrix.getScale(MapMatrix::LowerLimit))
      currentScaleSlider->setValue(_globalMapMatrix.getScale(MapMatrix::LowerLimit));
}
