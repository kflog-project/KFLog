/***********************************************************************
**
**   evaluationdialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef EVALUATIONDIALOG_H
#define EVALUATIONDIALOG_H

#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <q3scrollview.h>
#include <QSpinBox>
#include <QSlider>
#include <QTextEdit>

#include "wp.h"
#include "evaluationframe.h"

class Flight;

/**
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class EvaluationDialog : public QWidget
{
  Q_OBJECT

 public:
  /** */
  EvaluationDialog(QWidget *parent, const char name[]="");
  /** */
  ~EvaluationDialog();
  /**
   * Called, whenever a flight is loaded or closed, so that the listbox
   * contains all loaded flight.
   */
  void updateListBox();

  /**
   *
   */
  void updateText(int index1, int index2, bool updateAll = false);
  /** No descriptions */
  Flight* getFlight();

  unsigned int getTaskStart(){return evalFrame->getTaskStart();}
  unsigned int getTaskEnd(){return evalFrame->getTaskEnd();}

 signals:
  /** */
  void showCursor(const QPoint& p1, const QPoint& p2);
  /** */
  void flightChanged();
  /** */
  void textChanged(QString);
  /** No descriptions */
  void showFlightPoint(const flightPoint* fp);

 public slots:
  /** */
  void slotShowFlightData();
  void hide();
  /** No descriptions */
  void slotShowFlightPoint(const QPoint&, const flightPoint&);
  /** No descriptions */
  void slotRemoveFlightPoint();

 protected:
  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

 private:
  /*
   * Was ist mit diesen labels ???
   */
  QTextEdit* textLabel;
  QComboBox* combo_flight;
  Flight* flight;
  EvaluationFrame* evalFrame;
};

#endif
