/***********************************************************************
**
**   evaluationframe.cpp
**
**   This file is part of KFLog4.
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

#include "evaluationframe.h"
#include "evaluationdialog.h"
#include "flight.h"
#include "mapcalc.h"
#include "mapcontents.h"


#include <qlayout.h>
#include <qsettings.h>
#include <qsplitter.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <Q3ValueList>
#include <QLabel>

#define X_DISTANCE 100

EvaluationFrame::EvaluationFrame(QWidget* parent, EvaluationDialog* dlg)
  : Q3Frame(parent),
  flight(0)
{
//warning("EvaluationFrame::EvaluationFrame");
  // variable control
  QSplitter* controlSplitter = new QSplitter(Qt::Horizontal, this);

  // View
  graphFrame = new Q3ScrollView(controlSplitter);
  graphFrame->setResizePolicy(Q3ScrollView::AutoOne);
  graphFrame->setHScrollBarMode(Q3ScrollView::AlwaysOn);
  graphFrame->setVScrollBarMode(Q3ScrollView::AlwaysOff);
  graphFrame->setFrameStyle(Q3Frame::Panel | Q3Frame::Sunken);
  graphFrame->setBackgroundMode(Qt::PaletteLight);

  evalView = new EvaluationView(graphFrame, dlg);
  graphFrame->addChild(evalView);

  cursorLabel = new Q3TextView(this);
  cursorLabel->setFixedHeight(35);
  cursorLabel->setVScrollBarMode(Q3ScrollView::AlwaysOff);
  cursorLabel->setHScrollBarMode(Q3ScrollView::AlwaysOff);

  // Control elements
  Q3Frame* control = new Q3Frame(controlSplitter);
  control->setMinimumWidth(1);

  QLabel* scale_label = new QLabel(tr("Time scale:"),control);
  scale_label->setAlignment(Qt::AlignHCenter);
  spinScale = new QSpinBox(1,60,1,control);

  QLabel* label_glaettung = new QLabel(tr("Smoothness:"),control);
  label_glaettung->setAlignment(Qt::AlignHCenter);

  sliderVario = new QSlider(0,10,1,0,Qt::Vertical,control);
  sliderBaro  = new QSlider(0,10,1,0,Qt::Vertical,control);
  sliderSpeed = new QSlider(0,10,1,0,Qt::Vertical,control);
  sliderVario->sizeHint().setHeight(20);
  sliderBaro->sizeHint().setHeight(20);
  sliderSpeed->sizeHint().setHeight(20);

  check_vario = new QCheckBox(control);
  check_baro = new QCheckBox(control);
  check_speed = new QCheckBox(control);

  //control->setMaximumWidth(scale_label->sizeHint().width() + 10);

  QLabel* label_vario = new QLabel(tr("V"),control);
  QLabel* label_baro  = new QLabel(tr("H"),control);
  QLabel* label_speed = new QLabel(tr("S"),control);
  label_vario->setAlignment(Qt::AlignHCenter);
  label_baro->setAlignment(Qt::AlignHCenter);
  label_speed->setAlignment(Qt::AlignHCenter);

  Q3GridLayout* centerlayout = new Q3GridLayout( this, 3, 1 );

  Q3GridLayout* controllayout = new Q3GridLayout( control, 12, 6, 5, 1 );

  centerlayout->addWidget(controlSplitter, 0, 0);
  centerlayout->addWidget(cursorLabel, 2, 0);

  centerlayout->setRowStretch( 0, 1 );
  centerlayout->setRowStretch( 1, 0 );
  centerlayout->addRowSpacing( 1, 5 );
  centerlayout->setRowStretch( 2, 0 );

  controllayout->addMultiCellWidget(scale_label,1,1,0,6);
  controllayout->addMultiCellWidget(spinScale,3,3,2,5);
  controllayout->addMultiCellWidget(label_glaettung,5,5,0,6);
  controllayout->addWidget(sliderBaro,7,1);
  controllayout->addWidget(sliderVario,7,3);
  controllayout->addWidget(sliderSpeed,7,5);
  controllayout->addWidget(check_baro,9,1);
  controllayout->addWidget(check_vario,9,3);
  controllayout->addWidget(check_speed,9,5);
  controllayout->addWidget(label_baro,11,1);
  controllayout->addWidget(label_vario,11,3);
  controllayout->addWidget(label_speed,11,5);

  controllayout->addColSpacing(1,sliderBaro->sizeHint().width() + 4);
  controllayout->addColSpacing(3,sliderVario->sizeHint().width() + 4);
  controllayout->addColSpacing(5,sliderSpeed->sizeHint().width() + 4);
  controllayout->setColStretch(0,2);
  controllayout->setColStretch(2,1);
  controllayout->setColStretch(4,1);
  controllayout->setColStretch(6,2);
  controllayout->addRowSpacing(2,5);
  controllayout->addRowSpacing(4,10);
  controllayout->addRowSpacing(6,5);
  controllayout->addRowSpacing(8,5);
  controllayout->addRowSpacing(10,5);
  controllayout->setRowStretch(12,3);
  
// Set default size of the window splitting
  typedef Q3ValueList<int> testList;
  testList controlList;
  controlList.append(400);
  controlList.append(scale_label->sizeHint().width() + 10);
  controlSplitter->setSizes(controlList);

//  controlSplitter->setResizeMode(graphFrame,QSplitter::FollowSizeHint);


  // load settings from config file
  extern QSettings _settings;

  secWidth = _settings.readNumEntry("/Evaluation/ScaleTime",10);
  spinScale->setValue(secWidth);
  smoothness_va = _settings.readNumEntry("/Evaluation/VarioSmoothness",0);
  sliderVario->setValue(smoothness_va);
  smoothness_v = _settings.readNumEntry("/Evaluation/SpeedSmoothness",0);
  sliderSpeed->setValue(smoothness_v);
  smoothness_h = _settings.readNumEntry("/Evaluation/AltitudeSmoothness",0);
  sliderBaro->setValue(smoothness_h);
  check_vario->setChecked(_settings.readBoolEntry("/Evaluation/Vario",true));
  check_speed->setChecked(_settings.readBoolEntry("/Evaluation/Speed",true));
  check_baro->setChecked(_settings.readBoolEntry("/Evaluation/Altitude",true));

  this->connect(check_vario, SIGNAL(clicked()),
        SLOT(slotShowGraph()));
  this->connect(check_baro, SIGNAL(clicked()),
        SLOT(slotShowGraph()));
  this->connect(check_speed, SIGNAL(clicked()),
        SLOT(slotShowGraph()));

  this->connect(sliderVario, SIGNAL(valueChanged(int)),
        SLOT(slotVarioSmoothness(int)));
  this->connect(sliderSpeed, SIGNAL(valueChanged(int)),
        SLOT(slotSpeedSmoothness(int)));
  this->connect(sliderBaro, SIGNAL(valueChanged(int)),
        SLOT(slotBaroSmoothness(int)));

  this->connect(spinScale, SIGNAL(valueChanged(int)),
        SLOT(slotScale(int)));
}

EvaluationFrame::~EvaluationFrame()
{
// warning("EvaluationFrame::~EvaluationFrame()");
  // Save settings
  extern QSettings _settings;

  _settings.setValue("/Evaluation/ScaleTime", secWidth);
  _settings.setValue("/Evaluation/VarioSmoothness", smoothness_va);
  _settings.setValue("/Evaluation/AltitudeSmoothness", smoothness_h);
  _settings.setValue("/Evaluation/SpeedSmoothness", smoothness_v);
  _settings.setValue("/Evaluation/Vario", check_vario->isChecked());
  _settings.setValue("/Evaluation/Altitude", check_baro->isChecked());
  _settings.setValue("/Evaluation/Speed", check_speed->isChecked());
}

void EvaluationFrame::slotShowFlight()
{
  extern MapContents *_globalMapContents;
  flight = (Flight *)_globalMapContents->getFlight();
  slotShowGraph();
}

void EvaluationFrame::slotShowGraph()
{
  // draw the curves
  evalView->drawCurve(check_vario->isChecked(), check_speed->isChecked(),
    check_baro->isChecked(), smoothness_va, smoothness_v, smoothness_h, secWidth);

  if (flight && flight->getTypeID() == BaseMapElement::Flight) {
    int contentsX = (( centerTime - flight->getStartTime() ) / secWidth)
               + X_DISTANCE ;
    graphFrame->center(contentsX,0);
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

  int contentsX = graphFrame->contentsX() + ( graphFrame->width() / 2 );
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
//  warning("EvaluationFrame::resizeEvent");

  slotShowGraph();
  Q3Frame::resizeEvent(event);
}

unsigned int EvaluationFrame::getTaskStart(){
  return flight->getPointIndexByTime(evalView->cursor1);
}

unsigned int EvaluationFrame::getTaskEnd(){
  return flight->getPointIndexByTime(evalView->cursor2);
}
