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

#include <qdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlist.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qtextview.h>

#include "wp.h"

class Flight;

/**
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class EvaluationDialog : public QDialog
{
  Q_OBJECT

 public:
  /** */
  EvaluationDialog(QWidget *parent);
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

 signals:
  /** */
  void showCursor(QPoint p1, QPoint p2);
  /** */
  void flightChanged();
  /** */
  void textChanged(QString);

 public slots:
  /** */
  void slotShowFlightData();

 protected:
  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

 private:
  /*
   * Was ist mit diesen labels ???
   */
  QScrollView* graphFrame;
  QTextView* textLabel;
  QComboBox* combo_flight;
  Flight* flight;
};

#endif
