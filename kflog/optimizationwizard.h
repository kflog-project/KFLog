/***********************************************************************
**
**   optimizationwizard.h
**
**   This file is part of KFLog.
**
************************************************************************
**
** Created: Sam Mär 8 12:18:37 2003
**
************************************************************************
**
**   Copyright (c): 2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTextBrowser>
#include <QVariant>
#include <QWizard>

#include "evaluationdialog.h"
#include "flight.h"
#include "map.h"
#include "mapcontents.h"

class OptimizationWizard : public QWizard
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY( OptimizationWizard )

public:

  OptimizationWizard( QWidget* parent = 0 );

  virtual ~OptimizationWizard();

  QWizardPage *page;
  EvaluationDialog* evaluationDialog;
  QLabel* lblStartHeight;
  QLabel* lblStopTime;
  QLabel* lblDiffHeight;
  QLabel* textLabel1_3_2_2;
  QLabel* lblStartTime;
  QLabel* textLabel1_4_2;
  QLabel* textLabel1_2_2_2;
  QLabel* lblStopHeight;
  QLabel* lblDiffTime;
  QPushButton* timeButton;
  QWizardPage* page_2;
  QTextBrowser* olcResultBrowser;
  QProgressBar* progress;
  QPushButton* btnStart;
  QPushButton* btnStop;

  virtual void init();
  virtual double optimizationResult( unsigned int pointList[LEGS+3], double * points );

public slots:

  virtual void slotStartOptimization();
  virtual void slotStopOptimization();
  virtual void slotSetTimes();
  virtual void setMapContents( Map * _map );

protected:

  Flight* flight;
  QList<FlightPoint*> route;
  Optimization* optimization;

protected slots:

  virtual void languageChange();

};
