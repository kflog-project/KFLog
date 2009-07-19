/***********************************************************************
**
**   evaluationframe.cpp
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

#include "evaluationframe.h"
#include "evaluationdialog.h"
#include "flight.h"
#include "mapcalc.h"
#include "mapcontents.h"

#include <kconfig.h>
#include <klocale.h>

#include <qlayout.h>
#include <qsplitter.h>

#define X_DISTANCE 100

EvaluationFrame::EvaluationFrame(QWidget* parent, EvaluationDialog* dlg)
  : QFrame(parent),
  flight(0)
{
warning("EvaluationFrame::EvaluationFrame");
  // variable control
  QSplitter* controlSplitter = new QSplitter(QSplitter::Horizontal, this);

  // View
  graphFrame = new QScrollView(controlSplitter);
  graphFrame->setResizePolicy(QScrollView::AutoOne);
  graphFrame->setHScrollBarMode(QScrollView::AlwaysOn);
  graphFrame->setVScrollBarMode(QScrollView::AlwaysOff);
  graphFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  graphFrame->setBackgroundMode(PaletteLight);

  evalView = new EvaluationView(graphFrame, dlg);
  graphFrame->addChild(evalView);

  cursorLabel = new QTextView(this);
  cursorLabel->setFixedHeight(35);
  cursorLabel->setVScrollBarMode(QScrollView::AlwaysOff);
  cursorLabel->setHScrollBarMode(QScrollView::AlwaysOff);

  // Control elements
  QFrame* control = new QFrame(controlSplitter);
  control->setMinimumWidth(1);

  QLabel* scale_label = new QLabel(i18n("Time scale:"),control);
  scale_label->setAlignment(AlignHCenter);
  spinScale = new QSpinBox(1,60,1,control);

  QLabel* label_glaettung = new QLabel(i18n("Smoothness:"),control);
  label_glaettung->setAlignment(AlignHCenter);

  sliderVario = new QSlider(0,10,1,0,QSlider::Vertical,control);
  sliderBaro  = new QSlider(0,10,1,0,QSlider::Vertical,control);
  sliderSpeed = new QSlider(0,10,1,0,QSlider::Vertical,control);
  sliderVario->sizeHint().setHeight(20);
  sliderBaro->sizeHint().setHeight(20);
  sliderSpeed->sizeHint().setHeight(20);

  check_vario = new QCheckBox(control);
  check_baro = new QCheckBox(control);
  check_speed = new QCheckBox(control);

  //control->setMaximumWidth(scale_label->sizeHint().width() + 10);

  QLabel* label_vario = new QLabel(i18n("V"),control);
  QLabel* label_baro  = new QLabel(i18n("H"),control);
  QLabel* label_speed = new QLabel(i18n("S"),control);
  label_vario->setAlignment(AlignHCenter);
  label_baro->setAlignment(AlignHCenter);
  label_speed->setAlignment(AlignHCenter);

  QGridLayout* centerlayout = new QGridLayout( this, 3, 1 );

  QGridLayout* controllayout = new QGridLayout( control, 12, 6, 5, 1 );

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
  typedef QValueList<int> testList;
  testList controlList;
  controlList.append(400);
  controlList.append(scale_label->sizeHint().width() + 10);
  controlSplitter->setSizes(controlList);

//  controlSplitter->setResizeMode(graphFrame,QSplitter::FollowSizeHint);


  // load settings from config file
  KConfig* config = KGlobal::config();

  config->setGroup("Evaluation");
  secWidth = config->readNumEntry("Scale Time",10);
  spinScale->setValue(secWidth);
  smoothness_va = config->readNumEntry("Vario Smoothness",0);
  sliderVario->setValue(smoothness_va);
  smoothness_v = config->readNumEntry("Speed Smoothness",0);
  sliderSpeed->setValue(smoothness_v);
  smoothness_h = config->readNumEntry("Altitude Smoothness",0);
  sliderBaro->setValue(smoothness_h);
  check_vario->setChecked(config->readBoolEntry("Vario",true));
  check_speed->setChecked(config->readBoolEntry("Speed",true));
  check_baro->setChecked(config->readBoolEntry("Altitude",true));

  config->setGroup(0);

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
 warning("EvaluationFrame::~EvaluationFrame()");
  // Save settings
  KConfig* config = KGlobal::config();

  config->setGroup("Evaluation");
  config->writeEntry("Scale Time",secWidth);
  config->writeEntry("Vario Smoothness",smoothness_va);
  config->writeEntry("Altitude Smoothness",smoothness_h);
  config->writeEntry("Speed Smoothness",smoothness_v);
  config->writeEntry("Vario",check_vario->isChecked());
  config->writeEntry("Altitude",check_baro->isChecked());
  config->writeEntry("Speed",check_speed->isChecked());
}

void EvaluationFrame::slotShowFlight()
{
  extern MapContents _globalMapContents;
  flight = (Flight *)_globalMapContents.getFlight();
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
  warning("EvaluationFrame::resizeEvent");

  slotShowGraph();
  QFrame::resizeEvent(event);
}

unsigned int EvaluationFrame::getTaskStart(){
  return flight->getPointIndexByTime(evalView->cursor1);
}

unsigned int EvaluationFrame::getTaskEnd(){
  return flight->getPointIndexByTime(evalView->cursor2);
}
