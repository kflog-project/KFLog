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
#include <map.h>
#include <mapcalc.h>

#include <kiconloader.h>
#include <klocale.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>

MapControlView::MapControlView(QWidget* parent, Map* map)
: QWidget(parent)
{

  QLabel* mapControl = new QLabel(i18n("<B>Map-control:</B>"), parent);
  mapControl->setMinimumHeight(mapControl->sizeHint().height() + 5);

  QFrame* navFrame = new QFrame(parent);
  QPushButton* nwB = new QPushButton("NW", navFrame);
  nwB->setFixedHeight(35);
  nwB->setFixedWidth(35);
  QPushButton* nB = new QPushButton(navFrame);
  nB->setPixmap(BarIcon("1uparrow"));
  nB->setFixedHeight(35);
  nB->setFixedWidth(35);
  QPushButton* neB = new QPushButton("NE", navFrame);
  neB->setFixedHeight(35);
  neB->setFixedWidth(35);
  QPushButton* wB = new QPushButton(navFrame);
  wB->setPixmap(BarIcon("1leftarrow"));
  wB->setFixedHeight(35);
  wB->setFixedWidth(35);
  QPushButton* cenB = new QPushButton(navFrame);
  cenB->setPixmap(BarIcon("gohome"));
  cenB->setFixedHeight(35);
  cenB->setFixedWidth(35);
  QPushButton* eB = new QPushButton(navFrame);
  eB->setPixmap(BarIcon("1rightarrow"));
  eB->setFixedHeight(35);
  eB->setFixedWidth(35);
  QPushButton* swB = new QPushButton(navFrame);
  swB->setPixmap(BarIcon("1downleftarrow"));
  swB->setFixedHeight(35);
  swB->setFixedWidth(35);
  QPushButton* sB = new QPushButton(navFrame);
  sB->setPixmap(BarIcon("1downarrow"));
  sB->setFixedHeight(35);
  sB->setFixedWidth(35);
  QPushButton* seB = new QPushButton("SE", navFrame);
  seB->setFixedHeight(35);
  seB->setFixedWidth(35);

  QGridLayout* navLayout = new QGridLayout(navFrame, 3, 3, 0, 2, "navLayout");
  navLayout->addWidget(nwB,0,0);
  navLayout->addWidget(nB,0,1);
  navLayout->addWidget(neB,0,2);
  navLayout->addWidget(wB,1,0);
  navLayout->addWidget(cenB,1,1);
  navLayout->addWidget(eB,1,2);
  navLayout->addWidget(swB,2,0);
  navLayout->addWidget(sB,2,1);
  navLayout->addWidget(seB,2,2);

  navLayout->setColStretch(0,0);
  navLayout->setColStretch(1,0);
  navLayout->setColStretch(2,0);
  navLayout->setRowStretch(0,0);
  navLayout->setRowStretch(1,0);
  navLayout->setRowStretch(2,0);
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
  currentScaleSlider = new QSlider(2,135,1,0, QSlider::Horizontal,parent);
  currentScaleSlider->setMinimumHeight(
          currentScaleSlider->sizeHint().height());

  QGridLayout* controlLayout = new QGridLayout(parent,5,4,5,5,"controlLayout");

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

  controlLayout->activate();

  connect(currentScaleSlider, SIGNAL(valueChanged(int)),
            SLOT(slotShowScaleChange(int)));
  connect(currentScaleSlider, SIGNAL(sliderReleased()),
            SLOT(slotSetScale()));

  connect(nwB, SIGNAL(clicked()), map, SLOT(slotMoveMapNW()));
  connect(nB, SIGNAL(clicked()), map, SLOT(slotMoveMapN()));
  connect(neB, SIGNAL(clicked()), map, SLOT(slotMoveMapNE()));
  connect(wB, SIGNAL(clicked()), map, SLOT(slotMoveMapW()));
  connect(cenB, SIGNAL(clicked()), map, SLOT(slotCenterToHome()));
  connect(eB, SIGNAL(clicked()), map, SLOT(slotMoveMapE()));
  connect(swB, SIGNAL(clicked()), map, SLOT(slotMoveMapSW()));
  connect(sB, SIGNAL(clicked()), map, SLOT(slotMoveMapS()));
  connect(seB, SIGNAL(clicked()), map, SLOT(slotMoveMapSE()));
}

MapControlView::~MapControlView()
{

}

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
void MapControlView::slotShowMapData(QSize mapSize)
{
  extern const double _currentScale;

  QString temp;

  temp.sprintf("<TT>%.1f / %.1f</TT>",
      mapSize.height() * _currentScale / 1000.0,
      mapSize.width() * _currentScale / 1000.0);
  dimText->setText(temp);

  currentScaleValue->display(_currentScale);
  currentScaleSlider->setValue(__getScaleValue(_currentScale));
}

void MapControlView::slotSetScale()
{
  extern double _currentScale;

  if( _currentScale != __getScaleValue( currentScaleValue->value() ) )
    {
      extern MapMatrix _globalMapMatrix;
      _globalMapMatrix.setScale(currentScaleValue->value());
      emit(scaleChanged());
    }
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
  extern double _scale[];

  currentScaleValue->display(__setScaleValue(value));

  if(currentScaleValue->value() > _scale[9])
      currentScaleSlider->setValue(__getScaleValue(_scale[9]));

  if(currentScaleValue->value() < _scale[0])
      currentScaleSlider->setValue(__getScaleValue(_scale[0]));
}
