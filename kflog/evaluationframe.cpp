/***********************************************************************
**
**   evaluationframe.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>

#include "evaluationframe.h"
#include "evaluationdialog.h"
#include "flight.h"
#include "mapcalc.h"
#include "mapcontents.h"

#define X_DISTANCE 100

extern QSettings _settings;

EvaluationFrame::EvaluationFrame(QWidget* parent, EvaluationDialog* dlg) :
  QWidget(parent),
  centerTime(0),
  flight(0)
{
  // variable control
  QSplitter* controlSplitter = new QSplitter(Qt::Horizontal, this);

  // View
  graphFrame = new QScrollArea(controlSplitter);
  graphFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  graphFrame->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  graphFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
  graphFrame->setBackgroundRole( QPalette::Light );
  graphFrame->setAutoFillBackground( true );

  evalView = new EvaluationView( graphFrame, dlg );
  graphFrame->setWidget( evalView );

  cursorLabel = new QLabel(this);
  cursorLabel->setFixedHeight(35);
  cursorLabel->setMinimumWidth(200);

  cursorLabel->setBackgroundRole( QPalette::Light );
  cursorLabel->setAutoFillBackground( true );
  cursorLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

  QVBoxLayout* scaleLayout = new QVBoxLayout;
  scaleLayout->setMargin(5);

  QLabel* label = new QLabel( tr("Time Scale") );
  label->setAlignment(Qt::AlignCenter);
  scaleLayout->addWidget( label );

  spinTime = new QSpinBox;
  spinTime->setRange( 1, 60 );
  spinTime->setSingleStep( 1 );
  spinTime->setButtonSymbols( QSpinBox::PlusMinus );
  spinTime->setSuffix( "s" );
  spinTime->setToolTip(tr("Time distance between two drawing points in seconds."));
  scaleLayout->addWidget( spinTime );

  label = new QLabel( tr("Vario Scale") );
  label->setAlignment(Qt::AlignCenter);
  scaleLayout->addWidget( label );

  spinVario = new QSpinBox;
  spinVario->setRange( 0, 25 );
  spinVario->setSingleStep( 1 );
  spinVario->setButtonSymbols( QSpinBox::PlusMinus );
  spinVario->setSuffix( "m/s" );
  spinVario->setSpecialValueText(tr("Auto"));
  spinVario->setToolTip(tr("Adjusts variometer scale."));
  scaleLayout->addWidget( spinVario );

  label = new QLabel( tr("Speed Scale") );
  label->setAlignment(Qt::AlignCenter);
  scaleLayout->addWidget( label );

  spinSpeed = new QSpinBox;
  spinSpeed->setRange( 0, 350 );
  spinSpeed->setSingleStep( 25 );
  spinSpeed->setButtonSymbols( QSpinBox::PlusMinus );
  spinSpeed->setSuffix( "km/h" );
  spinSpeed->setSpecialValueText(tr("Auto"));
  spinSpeed->setToolTip(tr("Adjusts speed scale."));
  scaleLayout->addWidget( spinSpeed );
  scaleLayout->addStretch( 10 );

  // Create smooth layout
  QGridLayout* smoothLayout = new QGridLayout;
  smoothLayout->setMargin( 5 );
  smoothLayout->setSpacing( 5 );

  QLabel* label_glaettung = new QLabel( tr("Smoothness") );
  label_glaettung->setAlignment(Qt::AlignCenter);
  smoothLayout->addWidget( label_glaettung, 0, 1, 1, 3 );

  sliderBaro  = new QSlider(Qt::Vertical);
  sliderBaro->setRange(0, 10);
  sliderBaro->setValue(1);
  sliderBaro->setMinimumHeight(60);
  sliderBaro->setToolTip(tr("Changes the altitude smoothness."));
  smoothLayout->addWidget( sliderBaro, 1, 1, Qt::AlignCenter );

  sliderVario = new QSlider(Qt::Vertical);
  sliderVario->setRange(0, 10);
  sliderVario->setValue(1);
  sliderVario->setMinimumHeight(60);
  sliderVario->setToolTip(tr("Changes the variometer smoothness."));
  smoothLayout->addWidget( sliderVario, 1, 2, Qt::AlignCenter );

  sliderSpeed = new QSlider(Qt::Vertical);
  sliderSpeed->setRange(0, 10);
  sliderSpeed->setValue(1);
  sliderSpeed->setMinimumHeight(60);
  sliderSpeed->setToolTip(tr("Changes the speed smoothness."));
  smoothLayout->addWidget( sliderSpeed, 1, 3, Qt::AlignCenter );

  check_baro  = new QCheckBox;
  check_baro->setToolTip(tr("Switches on/off altitude drawing."));
  smoothLayout->addWidget( check_baro, 2, 1, Qt::AlignCenter );

  check_vario = new QCheckBox;
  check_vario->setToolTip(tr("Switches on/off variometer drawing."));
  smoothLayout->addWidget( check_vario, 2, 2, Qt::AlignCenter );

  check_speed = new QCheckBox;
  check_speed->setToolTip(tr("Switches on/off speed drawing."));
  smoothLayout->addWidget( check_speed, 2, 3, Qt::AlignCenter );

  QLabel* label_baro  = new QLabel(tr("A"));
  smoothLayout->addWidget( label_baro, 3, 1, Qt::AlignCenter );

  QLabel* label_vario = new QLabel(tr("V"));
  smoothLayout->addWidget( label_vario, 3, 2, Qt::AlignCenter );

  QLabel* label_speed = new QLabel(tr("S"));
  smoothLayout->addWidget( label_speed, 3, 3, Qt::AlignCenter );

  smoothLayout->setColumnStretch( 0, 10 );
  smoothLayout->setRowStretch( 4, 5 );

  // create container layout
  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin( 0 );
  hbox->addLayout( smoothLayout );
  hbox->addLayout( scaleLayout );

  // Create a widget, which gets assigned the layouts.
  QWidget *cw = new QWidget( this );
  cw->setLayout( hbox );

  // Add widget to the splitter
  controlSplitter->addWidget( cw );
  controlSplitter->setCollapsible ( 0, false );
 //-----------------------------------------------------------------------------

  QVBoxLayout* vbox = new QVBoxLayout( this );
  vbox->setMargin( 0 );

  vbox->addWidget(controlSplitter);
  vbox->addWidget(cursorLabel);

  // Set default size of the window splitting
  QList<int> controlList;
  controlList.append(600);
  controlList.append(cw->minimumSizeHint().width());
  controlSplitter->setSizes(controlList);

  // load the settings from the configuration file
  timeScale = _settings.value("/Evaluation/ScaleTime", 10).toInt();
  timeScaleOld = timeScale;
  spinTime->setValue(timeScale);

  speedScale = _settings.value("/Evaluation/ScaleSpeed", 0).toInt();
  spinSpeed->setValue(speedScale);

  varioScale = _settings.value("/Evaluation/ScaleVario", 0).toInt();
  spinVario->setValue(varioScale);

  smoothness_va = _settings.value("/Evaluation/VarioSmoothness", 0).toInt();
  sliderVario->setValue(smoothness_va);

  smoothness_v = _settings.value("/Evaluation/SpeedSmoothness", 0).toInt();
  sliderSpeed->setValue(smoothness_v);

  smoothness_h = _settings.value("/Evaluation/AltitudeSmoothness", 0).toInt();
  sliderBaro->setValue(smoothness_h);

  check_vario->setChecked(_settings.value("/Evaluation/Vario",true).toBool());
  check_speed->setChecked(_settings.value("/Evaluation/Speed",true).toBool());
  check_baro->setChecked(_settings.value("/Evaluation/Altitude",true).toBool());

  connect(check_vario, SIGNAL(clicked()), SLOT(slotShowGraph()));
  connect(check_baro, SIGNAL(clicked()), SLOT(slotShowGraph()));
  connect(check_speed, SIGNAL(clicked()), SLOT(slotShowGraph()));

  connect(sliderVario, SIGNAL(valueChanged(int)),
          SLOT(slotVarioSmoothness(int)));
  connect(sliderSpeed, SIGNAL(valueChanged(int)),
          SLOT(slotSpeedSmoothness(int)));
  connect(sliderBaro, SIGNAL(valueChanged(int)),
          SLOT(slotBaroSmoothness(int)));

  connect(spinTime, SIGNAL(valueChanged(int)), SLOT(slotScaleTime(int)));
  connect(spinSpeed, SIGNAL(valueChanged(int)), SLOT(slotScaleSpeed(int)));
  connect(spinVario, SIGNAL(valueChanged(int)), SLOT(slotScaleVario(int)));
}

EvaluationFrame::~EvaluationFrame()
{
  // Save settings to the configuration file
  _settings.setValue("/Evaluation/ScaleTime", timeScale);
  _settings.setValue("/Evaluation/ScaleSpeed", speedScale);
  _settings.setValue("/Evaluation/ScaleVario", varioScale);
  _settings.setValue("/Evaluation/VarioSmoothness", smoothness_va);
  _settings.setValue("/Evaluation/AltitudeSmoothness", smoothness_h);
  _settings.setValue("/Evaluation/SpeedSmoothness", smoothness_v);
  _settings.setValue("/Evaluation/Vario", check_vario->isChecked());
  _settings.setValue("/Evaluation/Altitude", check_baro->isChecked());
  _settings.setValue("/Evaluation/Speed", check_speed->isChecked());
}

void EvaluationFrame::slotShowFlight( Flight* newFlight )
{
  flight = newFlight;
  slotShowGraph();
}

void EvaluationFrame::slotShowGraph()
{
  // draw the curves
  evalView->drawCurve( check_vario->isChecked(),
                       check_speed->isChecked(),
                       check_baro->isChecked(),
                       smoothness_va,
                       smoothness_v,
                       smoothness_h,
                       timeScale,
                       speedScale,
                       varioScale );

  if( flight && flight->getTypeID() == BaseMapElement::Flight )
    {
      int contentsX = ((centerTime - flight->getStartTime()) / timeScale) + X_DISTANCE;
      graphFrame->ensureVisible( contentsX, 0 );
    }
}

void EvaluationFrame::slotVarioSmoothness(int s)
{
  // set smoothness factor and redraw graph
  smoothness_va = s;
  slotShowGraph();
}

void EvaluationFrame::slotBaroSmoothness(int s)
{
  // set smoothness factor and redraw graph
  smoothness_h = s;
  slotShowGraph();
}

void EvaluationFrame::slotSpeedSmoothness(int s)
{
  // set smoothness factor and redraw graph
  smoothness_v = s;
  slotShowGraph();
}

void EvaluationFrame::slotScaleTime(int newValue)
{
  // set scale factor
  timeScale = newValue;

  if( flight == 0 )
    {
      timeScaleOld = timeScale;
      return;
    }

  int x = graphFrame->horizontalScrollBar()->value();

  int contentsX = x + ( graphFrame->width() / 2 );

  centerTime = flight->getPointByTime((contentsX - X_DISTANCE) *
                          timeScaleOld + flight->getStartTime()).time;

  timeScaleOld = timeScale;
  slotShowGraph();
}

void EvaluationFrame::slotScaleSpeed(int newScale)
{
  speedScale = newScale;

  if( flight == 0 )
    {
      return;
    }

  slotShowGraph();
}

void EvaluationFrame::slotScaleVario(int newScale)
{
  varioScale = newScale;

  if( flight == 0 )
    {
      return;
    }

  slotShowGraph();
}

void EvaluationFrame::slotUpdateCursorText(QString text)
{
  cursorLabel->setText(text);
}

void EvaluationFrame::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent( event );
  slotShowGraph();
}

unsigned int EvaluationFrame::getTaskStart()
{
  if( ! flight )
    {
      return 0;
    }

  return flight->getPointIndexByTime(evalView->getCursor1());
}

unsigned int EvaluationFrame::getTaskEnd()
{
  if( ! flight )
    {
      return 0;
    }

  return flight->getPointIndexByTime(evalView->getCursor2());
}
