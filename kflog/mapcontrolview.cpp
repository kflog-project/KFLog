/***********************************************************************
**
**   mapcontrolview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "mapcontrolview.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

#define DELTA 4

extern MapMatrix* _globalMapMatrix;

MapControlView::MapControlView(QWidget* parent) : QWidget(parent)
{
  QFrame* navFrame = new QFrame(this);

  QPushButton* nwB = new QPushButton(navFrame);
  nwB->setIcon(_mainWindow->getPixmap("movemap_nw_22.png"));
  nwB->setFixedHeight(nwB->sizeHint().height() + DELTA);
  nwB->setFixedWidth(nwB->sizeHint().width() + DELTA);

  QPushButton* nB = new QPushButton(navFrame);
  nB->setIcon(_mainWindow->getPixmap("movemap_n_22.png"));
  nB->setFixedHeight(nB->sizeHint().height() + DELTA);
  nB->setFixedWidth(nB->sizeHint().width() + DELTA);

  QPushButton* neB = new QPushButton(navFrame);
  neB->setIcon(_mainWindow->getPixmap("movemap_ne_22.png"));
  neB->setFixedHeight(neB->sizeHint().height() + DELTA);
  neB->setFixedWidth(neB->sizeHint().width() + DELTA);

  QPushButton* wB = new QPushButton(navFrame);
  wB->setIcon(_mainWindow->getPixmap("movemap_w_22.png"));
  wB->setFixedHeight(wB->sizeHint().height() + DELTA);
  wB->setFixedWidth(wB->sizeHint().width() + DELTA);

  QPushButton* cenB = new QPushButton(navFrame);
  cenB->setIcon(_mainWindow->getPixmap("kde_gohome_22.png"));
  cenB->setFixedHeight(cenB->sizeHint().height() + DELTA);
  cenB->setFixedWidth(cenB->sizeHint().width() + DELTA);

  QPushButton* eB = new QPushButton(navFrame);
  eB->setIcon(_mainWindow->getPixmap("movemap_e_22.png"));
  eB->setFixedHeight(eB->sizeHint().height() + DELTA);
  eB->setFixedWidth(eB->sizeHint().width() + DELTA);

  QPushButton* swB = new QPushButton(navFrame);
  swB->setIcon(_mainWindow->getPixmap("movemap_sw_22.png"));
  swB->setFixedHeight(swB->sizeHint().height() + DELTA);
  swB->setFixedWidth(swB->sizeHint().width() + DELTA);

  QPushButton* sB = new QPushButton(navFrame);
  sB->setIcon(_mainWindow->getPixmap("movemap_s_22.png"));
  sB->setFixedHeight(sB->sizeHint().height() + DELTA);
  sB->setFixedWidth(sB->sizeHint().width() + DELTA);

  QPushButton* seB = new QPushButton(navFrame);
  seB->setIcon(_mainWindow->getPixmap("movemap_se_22.png"));
  seB->setFixedHeight(seB->sizeHint().height() + DELTA);
  seB->setFixedWidth(seB->sizeHint().width() + DELTA);

  QGridLayout* navLayout = new QGridLayout( navFrame );
  navLayout->setMargin( 0 );
  navLayout->setSpacing( 2 );

  navLayout->addWidget( nwB, 1, 1 );
  navLayout->addWidget( nB, 1, 2 );
  navLayout->addWidget( neB, 1, 3 );
  navLayout->addWidget( wB, 2, 1 );
  navLayout->addWidget( cenB, 2, 2 );
  navLayout->addWidget( eB, 2, 3 );
  navLayout->addWidget( swB, 3, 1 );
  navLayout->addWidget( sB, 3, 2 );
  navLayout->addWidget( seB, 3, 3 );

  navLayout->setColumnStretch( 0, 1 );
  navLayout->setColumnStretch( 1, 0 );
  navLayout->setColumnStretch( 2, 0 );
  navLayout->setColumnStretch( 3, 0 );
  navLayout->setColumnStretch( 4, 1 );

  navLayout->setRowStretch( 0, 1 );
  navLayout->setRowStretch( 1, 0 );
  navLayout->setRowStretch( 2, 0 );
  navLayout->setRowStretch( 3, 0 );
  navLayout->setRowStretch( 4, 1 );

  QLabel* dimLabel = new QLabel( tr("Height/Width [km]:"), this );

  dimLabel->setMinimumHeight(dimLabel->sizeHint().height() + 5);
  dimText = new QLabel("125/130", this);
  dimText->setMargin( 5 );
  dimText->setAlignment( Qt::AlignCenter );
  dimText->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  dimText->setBackgroundRole( QPalette::Light );
  dimText->setAutoFillBackground( true );

  QLabel* currentScaleLabel = new QLabel(tr("Scale:"), this);
  currentScaleLabel->setMinimumHeight(currentScaleLabel->sizeHint().height() + 10);
  currentScaleValue = new QLCDNumber(5,this);
  currentScaleValue->setBackgroundRole( QPalette::Light );
  currentScaleValue->setAutoFillBackground( true );

  QLabel* setScaleLabel = new QLabel(tr("Change Scale:"), this);
  setScaleLabel->setMinimumWidth( setScaleLabel->sizeHint().width());

  setScaleLabel->setMinimumHeight(setScaleLabel->sizeHint().height());
  currentScaleSlider = new QSlider(2,105,1,0, Qt::Horizontal,this);
  currentScaleSlider->setMinimumHeight(currentScaleSlider->sizeHint().height());

  QGridLayout* controlLayout = new QGridLayout( this );
  controlLayout->setMargin( 5 );
  controlLayout->setSpacing( 10 );

  controlLayout->addWidget( navFrame, 0, 0, 3, 1 );
  controlLayout->addWidget( dimLabel, 0, 1, 1, 2, Qt::AlignCenter );
  controlLayout->addWidget( dimText,  1, 1, 1, 2 );
  controlLayout->addWidget( currentScaleLabel, 2, 1 );
  controlLayout->addWidget( currentScaleValue, 2, 2 );
  controlLayout->addWidget( setScaleLabel, 3, 0 );
  controlLayout->addWidget( currentScaleSlider, 3, 1, 1, 3 );

  controlLayout->setColumnStretch( 4, 10 );
  controlLayout->setRowStretch( 4, 10 );

  connect( currentScaleSlider, SIGNAL(valueChanged(int)),
           SLOT(slotShowScaleChange(int)) );
  connect( currentScaleSlider, SIGNAL(sliderReleased()),
           SLOT(slotSetScale()) );

  connect(nwB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapNW()));
  connect(nB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapN()));
  connect(neB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapNE()));
  connect(wB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapW()));
  connect(cenB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotCenterToHome()));
  connect(eB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapE()));
  connect(swB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapSW()));
  connect(sB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapS()));
  connect(seB, SIGNAL(clicked()), _globalMapMatrix, SLOT(slotMoveMapSE()));
}

MapControlView::~MapControlView()
{
}

void MapControlView::slotShowMapData(QSize mapSize)
{
  const double cScale = _globalMapMatrix->getScale();

  QString temp;

  temp.sprintf( "<html><TT>%.1f/%.1f</TT></html>",
                mapSize.height() * cScale / 1000.0,
                mapSize.width() * cScale / 1000.0);
  dimText->setText( temp );

  currentScaleValue->display( cScale );
  currentScaleSlider->setValue( __getScaleValue( cScale ) );
}

void MapControlView::slotSetMinMaxValue(int min, int max)
{
  currentScaleSlider->setMinValue( __getScaleValue( min ) );
  currentScaleSlider->setMaxValue( __getScaleValue( max ) );
}

void MapControlView::slotSetScale()
{
  emit( scaleChanged(currentScaleValue->value()) );
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
  currentScaleValue->display(__setScaleValue(value));

  if(currentScaleValue->value() > _globalMapMatrix->getScale(MapMatrix::UpperLimit))
    {
      currentScaleSlider->setValue((int)rint(_globalMapMatrix->getScale(MapMatrix::UpperLimit)));
    }

  if(currentScaleValue->value() < _globalMapMatrix->getScale(MapMatrix::LowerLimit))
    {
      currentScaleSlider->setValue((int)rint(_globalMapMatrix->getScale(MapMatrix::LowerLimit)));
    }
}
