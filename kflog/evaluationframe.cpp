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
#include <evaluationdialog.h>
#include <flight.h>
#include <mapcalc.h>
#include <mapcontents.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <qlayout.h>
#include <qsplitter.h>

#define X_ABSTAND 100

EvaluationFrame::EvaluationFrame(QWidget* parent, EvaluationDialog* dlg)
  : QFrame(parent),
  flight(0)
{
  // variable Kontrolle
  QSplitter* kontSplitter = new QSplitter(QSplitter::Horizontal, this);

  // View
  graphFrame = new QScrollView(kontSplitter);
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

  // Kontrollelemente
  QFrame* kontrolle = new QFrame(kontSplitter);
  kontrolle->setMinimumWidth(1);

  QLabel* scale_label = new QLabel(i18n("Time scale:"),kontrolle);
  scale_label->setAlignment(AlignHCenter);
  spinScale = new QSpinBox(1,60,1,kontrolle);

  QLabel* label_glaettung = new QLabel(i18n("Smoothness:"),kontrolle);
  label_glaettung->setAlignment(AlignHCenter);

  sliderVario = new QSlider(0,10,1,0,QSlider::Vertical,kontrolle);
  sliderBaro  = new QSlider(0,10,1,0,QSlider::Vertical,kontrolle);
  sliderSpeed = new QSlider(0,10,1,0,QSlider::Vertical,kontrolle);
  sliderVario->sizeHint().setHeight(20);
  sliderBaro->sizeHint().setHeight(20);
  sliderSpeed->sizeHint().setHeight(20);

  check_vario = new QCheckBox(kontrolle);
  check_baro = new QCheckBox(kontrolle);
  check_speed = new QCheckBox(kontrolle);

  kontrolle->setMaximumWidth(scale_label->sizeHint().width() + 10);

  QLabel* label_vario = new QLabel(i18n("V"),kontrolle);
  QLabel* label_baro  = new QLabel(i18n("H"),kontrolle);
  QLabel* label_speed = new QLabel(i18n("S"),kontrolle);
  label_vario->setAlignment(AlignHCenter);
  label_baro->setAlignment(AlignHCenter);
  label_speed->setAlignment(AlignHCenter);

  QGridLayout* mittelayout = new QGridLayout( this, 3, 1 );

  QGridLayout* kontrolllayout = new QGridLayout( kontrolle, 12, 6, 5, 1 );

  mittelayout->addWidget(kontSplitter, 0, 0);
  mittelayout->addWidget(cursorLabel, 2, 0);

  mittelayout->setRowStretch( 0, 1 );
  mittelayout->setRowStretch( 1, 0 );
  mittelayout->addRowSpacing( 1, 5 );
  mittelayout->setRowStretch( 2, 0 );

  kontrolllayout->addMultiCellWidget(scale_label,1,1,0,6);
  kontrolllayout->addMultiCellWidget(spinScale,3,3,2,5);
  kontrolllayout->addMultiCellWidget(label_glaettung,5,5,0,6);
  kontrolllayout->addWidget(sliderBaro,7,1);
  kontrolllayout->addWidget(sliderVario,7,3);
  kontrolllayout->addWidget(sliderSpeed,7,5);
  kontrolllayout->addWidget(check_baro,9,1);
  kontrolllayout->addWidget(check_vario,9,3);
  kontrolllayout->addWidget(check_speed,9,5);
  kontrolllayout->addWidget(label_baro,11,1);
  kontrolllayout->addWidget(label_vario,11,3);
  kontrolllayout->addWidget(label_speed,11,5);

  kontrolllayout->addColSpacing(1,sliderBaro->sizeHint().width() + 4);
  kontrolllayout->addColSpacing(3,sliderVario->sizeHint().width() + 4);
  kontrolllayout->addColSpacing(5,sliderSpeed->sizeHint().width() + 4);
  kontrolllayout->setColStretch(0,2);
  kontrolllayout->setColStretch(2,1);
  kontrolllayout->setColStretch(4,1);
  kontrolllayout->setColStretch(6,2);
  kontrolllayout->addRowSpacing(2,5);
  kontrolllayout->addRowSpacing(4,10);
  kontrolllayout->addRowSpacing(6,5);
  kontrolllayout->addRowSpacing(8,5);
  kontrolllayout->addRowSpacing(10,5);
  kontrolllayout->setRowStretch(12,3);

// Setzt die Anfangsgrößen des Splitters
  typedef QValueList<int> testList;
  testList kontList;
  kontList.append(400);
  kontList.append(scale_label->sizeHint().width() + 10);
  kontSplitter->setSizes(kontList);

//  kontSplitter->setResizeMode(graphFrame,QSplitter::FollowSizeHint);


  // gespeicherte Daten
  KConfig* config = KGlobal::config();

  config->setGroup("Evaluation");
  secWidth = config->readNumEntry("Scale Time",10);
  spinScale->setValue(secWidth);
  glatt_va = config->readNumEntry("Vario Smoothness",0);
  sliderVario->setValue(glatt_va);
  glatt_v = config->readNumEntry("Speed Smoothness",0);
  sliderSpeed->setValue(glatt_v);
  glatt_h = config->readNumEntry("Elevation Smoothness",0);
  sliderBaro->setValue(glatt_h);
  check_vario->setChecked(config->readBoolEntry("Vario",true));
  check_speed->setChecked(config->readBoolEntry("Speed",true));
  check_baro->setChecked(config->readBoolEntry("Elevation",true));

  config->setGroup(0);

  this->connect(check_vario, SIGNAL(clicked()),
        SLOT(slotShowGraph()));
  this->connect(check_baro, SIGNAL(clicked()),
        SLOT(slotShowGraph()));
  this->connect(check_speed, SIGNAL(clicked()),
        SLOT(slotShowGraph()));

  this->connect(sliderVario, SIGNAL(valueChanged(int)),
        SLOT(slotVarioGlatt(int)));
  this->connect(sliderSpeed, SIGNAL(valueChanged(int)),
        SLOT(slotSpeedGlatt(int)));
  this->connect(sliderBaro, SIGNAL(valueChanged(int)),
        SLOT(slotBaroGlatt(int)));

  this->connect(spinScale, SIGNAL(valueChanged(int)),
        SLOT(slotScale(int)));
}

EvaluationFrame::~EvaluationFrame()
{
  // Save settings
  KConfig* config = KGlobal::config();

  config->setGroup("Evaluation");
  config->writeEntry("Scale Time",secWidth);
  config->writeEntry("Vario Smoothness",glatt_va);
  config->writeEntry("Elevation Smoothness",glatt_h);
  config->writeEntry("Speed Smoothness",glatt_v);
  config->writeEntry("Vario",check_vario->isChecked());
  config->writeEntry("Elevation",check_baro->isChecked());
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
/*  this->setCaption(i18n("Flightevaluation:") + flightList->at(n)->getPilot()
                    + "  " + flightList->at(n)->getDate().toString());
  // GRUNDWERTE setzen
  updateText(0,flightList->at(n)->getRouteLength() - 1, true);
*/


//  warning("EvaluationFrame::slotShowGraph");

  // Kurve malen
  evalView->drawCurve(check_vario->isChecked(), check_speed->isChecked(),
    check_baro->isChecked(), glatt_va, glatt_v, glatt_h, secWidth);

  if (flight && flight->getTypeID() == BaseMapElement::Flight) {
    int contentsX = (( centerTime - flight->getStartTime() ) / secWidth)
               + X_ABSTAND ;
    graphFrame->center(contentsX,0);
  }
//warning("Setze auf X: %d",centerTime);
}


void EvaluationFrame::slotVarioGlatt(int g)
{
  // gibt den Glästtungsfaktor zurück
  glatt_va = g;

  slotShowGraph();
}

void EvaluationFrame::slotBaroGlatt(int g)
{
  // gibt den Glättungsfaktor zurück
  glatt_h = g;

  slotShowGraph();
}

void EvaluationFrame::slotSpeedGlatt(int g)
{
  // gibt den Glättungsfaktor zurück
  glatt_v = g;

  slotShowGraph();
}

void EvaluationFrame::slotScale(int g)
{
  // gibt den Scalierungsfaktor zurück
  secWidth = g;

  int contentsX = graphFrame->contentsX() + ( graphFrame->width() / 2 );
  centerTime = flight->getPointByTime((contentsX - X_ABSTAND) *
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

  QFrame::resizeEvent(event);
  slotShowGraph();
}

unsigned int EvaluationFrame::getTaskStart(){
  return flight->getPointIndexByTime(evalView->cursor1);
}

unsigned int EvaluationFrame::getTaskEnd(){
  return flight->getPointIndexByTime(evalView->cursor2);
}
