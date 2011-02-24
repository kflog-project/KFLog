/***********************************************************************
**
**   evaluationdialog.h
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

#ifndef EVALUATION_DIALOG_H
#define EVALUATION_DIALOG_H

#include <QPoint>
#include <QString>
#include <QTextBrowser>
#include <QWidget>

#include "evaluationframe.h"
#include "flightpoint.h"

class Flight;

/**
 * \class EvaluationDialog
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \date 2000-2011
 *
 * \version $Id$
 */
class EvaluationDialog : public QWidget
{
  Q_OBJECT

private:

 Q_DISABLE_COPY ( EvaluationDialog )

 public:

  EvaluationDialog( QWidget *parent );

  virtual ~EvaluationDialog();

  void updateText(int index1, int index2, bool updateAll = false);

  Flight* getFlight();

  unsigned int getTaskStart(){return evalFrame->getTaskStart();};

  unsigned int getTaskEnd(){return evalFrame->getTaskEnd();};

 protected:

  virtual void hideEvent( QHideEvent* event );

  virtual void showEvent( QShowEvent* event );

 signals:
  /** */
  void showCursor(const QPoint& p1, const QPoint& p2);
  /** */
  void flightChanged( Flight* newFlight );
  /** */
  void textChanged(QString text);
  /** No descriptions */
  void showFlightPoint(const FlightPoint* fp);

  /** Window was hidden. */
  void windowHidden();

  /** Emits the help text of the wigdet. */
  void evaluationHelp( QString& help );

 public slots:

  void slotShowFlightData();
  /** No descriptions */
  void slotShowFlightPoint(const QPoint&, const FlightPoint&);
  /** No descriptions */
  void slotRemoveFlightPoint();

 protected:

  virtual void resizeEvent(QResizeEvent* event);

 private:

  QTextBrowser* textDisplay;
  Flight* flight;
  EvaluationFrame* evalFrame;
};

#endif
