/***********************************************************************
**
**   mapcontrolview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "distance.h"
#include "mapcontrolview.h"
#include "mapcalc.h"
#include "mapmatrix.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

#define DELTA 4

extern MapMatrix* _globalMapMatrix;

MapControlView::MapControlView(QWidget* parent) : QWidget(parent)
{
  setObjectName( "MapControlView" );
  setHelpText();

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
  cenB->setToolTip( tr("Center to Home position") );
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

  m_dimLabel = new QLabel( tr("Height / Width:"), this );
  m_dimLabel->setMinimumHeight(m_dimLabel->sizeHint().height() + 5);

  m_dimText = new QLabel("", this);
  m_dimText->setMargin( 5 );
  m_dimText->setAlignment( Qt::AlignCenter );
  m_dimText->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  m_dimText->setBackgroundRole( QPalette::Light );
  m_dimText->setAutoFillBackground( true );

  QLabel* currentScaleLabel = new QLabel(tr("Scale:"), this);
  currentScaleLabel->setMinimumHeight(currentScaleLabel->sizeHint().height() + 10);
  m_currentScaleValue = new QLCDNumber(5,this);
  m_currentScaleValue->setBackgroundRole( QPalette::Light );
  m_currentScaleValue->setAutoFillBackground( true );

  QLabel* setScaleLabel = new QLabel(tr("Change Scale:"), this);
  setScaleLabel->setMinimumWidth( setScaleLabel->sizeHint().width());

  setScaleLabel->setMinimumHeight(setScaleLabel->sizeHint().height());
  m_currentScaleSlider = new QSlider( Qt::Horizontal,this );
  m_currentScaleSlider->setMinimum(1);
  m_currentScaleSlider->setMaximum(2);
  m_currentScaleSlider->setPageStep(1);
  m_currentScaleSlider->setValue(0);
  m_currentScaleSlider->setMinimumHeight(m_currentScaleSlider->sizeHint().height());

  QGridLayout* controlLayout = new QGridLayout( this );
  controlLayout->setMargin( 5 );
  controlLayout->setSpacing( 10 );

  controlLayout->addWidget( navFrame, 0, 0, 3, 1 );
  controlLayout->addWidget( m_dimLabel, 0, 1, 1, 2, Qt::AlignCenter );
  controlLayout->addWidget( m_dimText,  1, 1, 1, 2 );
  controlLayout->addWidget( currentScaleLabel, 2, 1 );
  controlLayout->addWidget( m_currentScaleValue, 2, 2 );
  controlLayout->addWidget( setScaleLabel, 3, 0 );
  controlLayout->addWidget( m_currentScaleSlider, 3, 1, 1, 3 );

  controlLayout->setColumnStretch( 4, 10 );
  controlLayout->setRowStretch( 4, 10 );

  connect( m_currentScaleSlider, SIGNAL(valueChanged(int)),
           SLOT(slotShowScaleChange(int)) );
  connect( m_currentScaleSlider, SIGNAL(sliderReleased()),
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

void MapControlView::setHelpText()
{
  setWhatsThis( tr(
   "<html><b>The map control help</b><br><br>"
   "The map control provides the following actions:"
   "<ul>"
   "<li><i>Moving</i> the map by pressing an arrow button."
   "<li><i>Center</i> the map to the <i>Home</i> position by pressing the house button."
   "<li><i>Zooming</i> the map by moving the slider."
   "</ul>"
   "<br><br></html>"
  ) );
}

void MapControlView::slotShowMapData( QSize mapSize )
{
  m_mapSize = mapSize;

  const double cScale = _globalMapMatrix->getScale();

  // Distances in meters
  Distance height = Distance(mapSize.height() * cScale);
  Distance width  = Distance(mapSize.width() * cScale);

  QString text = QString("<html><TT>%1/%2 [%3]</TT></html>")
                 .arg( height.getText( false, 1 ) )
		 .arg( width.getText( false, 1) )
		 .arg( Distance::getUnitText() );

  m_dimText->setText( text );
  m_currentScaleValue->display( cScale );
  m_currentScaleSlider->setValue( __getScaleValue( cScale ) );
}

void MapControlView::slotSetMinMaxValue(int min, int max)
{
  m_currentScaleSlider->setMinimum( __getScaleValue( min ) );
  m_currentScaleSlider->setMaximum( __getScaleValue( max ) );
}

void MapControlView::slotSetScale()
{
  emit( scaleChanged(m_currentScaleValue->value()) );
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
  m_currentScaleValue->display(__setScaleValue(value));

  if(m_currentScaleValue->value() > _globalMapMatrix->getScale(MapMatrix::UpperLimit))
    {
      m_currentScaleSlider->setValue((int)rint(_globalMapMatrix->getScale(MapMatrix::UpperLimit)));
    }

  if(m_currentScaleValue->value() < _globalMapMatrix->getScale(MapMatrix::LowerLimit))
    {
      m_currentScaleSlider->setValue((int)rint(_globalMapMatrix->getScale(MapMatrix::LowerLimit)));
    }
}
