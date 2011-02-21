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

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <q3scrollview.h>
#include <QSpinBox>
#include <QSlider>
#include <QTextEdit>

#include "evaluationframe.h"
#include "flightpoint.h"

class Flight;

/**
 * @author Heiner Lamprecht, Florian Ehinger
 *
 * @version $Id$
 */
class EvaluationDialog : public QWidget
{
  Q_OBJECT

 public:
  /** */
  EvaluationDialog( QWidget *parent );
  /** */
  ~EvaluationDialog();
  /**
   * Called, whenever a flight is loaded or closed, so that the listbox
   * contains all loaded flight.
   */
  void updateListBox();

  void updateText(int index1, int index2, bool updateAll = false);

  Flight* getFlight();

  unsigned int getTaskStart(){return evalFrame->getTaskStart();}
  unsigned int getTaskEnd(){return evalFrame->getTaskEnd();}

 protected:

  virtual void hideEvent( QHideEvent * event );

 signals:
  /** */
  void showCursor(const QPoint& p1, const QPoint& p2);
  /** */
  void flightChanged();
  /** */
  void textChanged(QString);
  /** No descriptions */
  void showFlightPoint(const FlightPoint* fp);

  /** Window was hidden. */
  void windowHidden();

 public slots:

  void slotShowFlightData();
  /** No descriptions */
  void slotShowFlightPoint(const QPoint&, const FlightPoint&);
  /** No descriptions */
  void slotRemoveFlightPoint();

 protected:

  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

 private:

  QTextEdit* textLabel;
  QComboBox* combo_flight;
  Flight* flight;
  EvaluationFrame* evalFrame;
};

#endif
