/***********************************************************************
**
**   evaluationframe.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "evaluationframe.h"
#include "evaluationdialog.h"
#include "flight.h"
#include "mapcalc.h"
#include "mapcontents.h"

#define X_DISTANCE 100

extern MapContents *_globalMapContents;
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
  cursorLabel->setBackgroundRole( QPalette::Light );
  cursorLabel->setAutoFillBackground( true );
  cursorLabel->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

  // Create Control layout
  QGridLayout* controlLayout = new QGridLayout;
  controlLayout->setMargin( 5 );
  controlLayout->setSpacing( 5 );

  QLabel* scale_label = new QLabel( tr("Time Scale") );
  scale_label->setAlignment(Qt::AlignCenter);
  controlLayout->addWidget( scale_label, 0, 1, 1, 3 );

  spinScale = new QSpinBox;
  spinScale->setRange( 1, 60 );
  spinScale->setSingleStep( 1 );
  spinScale->setButtonSymbols( QSpinBox::PlusMinus );
  spinScale->setSuffix( "s" );
  spinScale->setToolTip(tr("Time distance between two drawing points in seconds."));
  controlLayout->addWidget( spinScale, 1, 1, 1, 3 );

  QLabel* label_glaettung = new QLabel( tr("Smoothness") );
  label_glaettung->setAlignment(Qt::AlignCenter);
  controlLayout->addWidget( label_glaettung, 2, 1, 1, 3 );

  sliderBaro  = new QSlider(Qt::Vertical);
  sliderBaro->setRange(0, 10);
  sliderBaro->setValue(1);
  sliderBaro->sizeHint().setHeight(20);
  sliderBaro->setToolTip(tr("Changes the altitude smoothness."));
  controlLayout->addWidget( sliderBaro, 3, 1 );

  sliderVario = new QSlider(Qt::Vertical);
  sliderVario->setRange(0, 10);
  sliderVario->setValue(1);
  sliderVario->sizeHint().setHeight(20);
  sliderVario->setToolTip(tr("Changes the variometer smoothness."));
  controlLayout->addWidget( sliderVario, 3, 2 );

  sliderSpeed = new QSlider(Qt::Vertical);
  sliderSpeed->setRange(0, 10);
  sliderSpeed->setValue(1);
  sliderSpeed->sizeHint().setHeight(20);
  sliderSpeed->setToolTip(tr("Changes the speed smoothness."));
  controlLayout->addWidget( sliderSpeed, 3, 3 );

  check_baro  = new QCheckBox;
  check_baro->setToolTip(tr("Switches on/off altitude drawing."));
  controlLayout->addWidget( check_baro, 4, 1 );

  check_vario = new QCheckBox;
  check_vario->setToolTip(tr("Switches on/off variometer drawing."));
  controlLayout->addWidget( check_vario, 4, 2 );

  check_speed = new QCheckBox;
  check_speed->setToolTip(tr("Switches on/off speed drawing."));
  controlLayout->addWidget( check_speed, 4, 3 );

  QLabel* label_baro  = new QLabel(tr("A"));
  controlLayout->addWidget( label_baro, 5, 1 );

  QLabel* label_vario = new QLabel(tr("V"));
  controlLayout->addWidget( label_vario, 5, 2 );

  QLabel* label_speed = new QLabel(tr("S"));
  controlLayout->addWidget( label_speed, 5, 3 );

  controlLayout->setColumnStretch( 0, 5 );
  controlLayout->setColumnStretch( 4, 5 );
  controlLayout->setRowStretch( 6, 5 );
  controlLayout->setRowMinimumHeight( 3, 40 );

  // Create a widget, which gets assigned the layout.
  QWidget *cw = new QWidget( this );
  cw->setLayout( controlLayout );

  // Add widget to the splitter
  controlSplitter->addWidget( cw );
 //-----------------------------------------------------------------------------

  QVBoxLayout* vbox = new QVBoxLayout( this );
  vbox->setMargin( 0 );

  vbox->addWidget(controlSplitter);
  vbox->addWidget(cursorLabel);

  // Set default size of the window splitting
  QList<int> controlList;
  controlList.append(400);
  controlList.append(scale_label->sizeHint().width() + 10);
  controlSplitter->setSizes(controlList);

  // load the settings from the configuration file
  secWidth = _settings.value("/Evaluation/ScaleTime", 10).toInt();
  spinScale->setValue(secWidth);

  smoothness_va = _settings.value("/Evaluation/VarioSmoothness", 0).toInt();
  sliderVario->setValue(smoothness_va);

  smoothness_v = _settings.value("/Evaluation/SpeedSmoothness", 0).toInt();
  sliderSpeed->setValue(smoothness_v);

  smoothness_h = _settings.value("/Evaluation/AltitudeSmoothness", 0).toInt();
  sliderBaro->setValue(smoothness_h);

  check_vario->setChecked(_settings.value("/Evaluation/Vario",true).toBool());
  check_speed->setChecked(_settings.value("/Evaluation/Speed",true).toBool());
  check_baro->setChecked(_settings.value("/Evaluation/Altitude",true).toBool());

  this->connect(check_vario, SIGNAL(clicked()), SLOT(slotShowGraph()));
  this->connect(check_baro, SIGNAL(clicked()), SLOT(slotShowGraph()));
  this->connect(check_speed, SIGNAL(clicked()), SLOT(slotShowGraph()));

  this->connect(sliderVario, SIGNAL(valueChanged(int)),
        SLOT(slotVarioSmoothness(int)));
  this->connect(sliderSpeed, SIGNAL(valueChanged(int)),
        SLOT(slotSpeedSmoothness(int)));
  this->connect(sliderBaro, SIGNAL(valueChanged(int)),
        SLOT(slotBaroSmoothness(int)));

  this->connect(spinScale, SIGNAL(valueChanged(int)), SLOT(slotScale(int)));
}

EvaluationFrame::~EvaluationFrame()
{
  qDebug() << "~EvaluationFrame()";

  // Save settings to the configuration file
  _settings.setValue("/Evaluation/ScaleTime", secWidth);
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
                       secWidth );

  if( flight && flight->getObjectType() == BaseMapElement::Flight )
    {
      int contentsX = ((centerTime - flight->getStartTime()) / secWidth) + X_DISTANCE;
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

void EvaluationFrame::slotScale(int g)
{
  if(flight == NULL)  return;

  // set scale factor
  secWidth = g;

  int x = graphFrame->horizontalScrollBar()->value();

  int contentsX = x + ( graphFrame->width() / 2 );

  centerTime = flight->getPointByTime((contentsX - X_DISTANCE) *
                          secWidthOld + flight->getStartTime()).time;

  secWidthOld = secWidth;

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
  return flight->getPointIndexByTime(evalView->cursor1);
}

unsigned int EvaluationFrame::getTaskEnd()
{
  return flight->getPointIndexByTime(evalView->cursor2);
}
