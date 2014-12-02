/***********************************************************************
**
**   evaluationdialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef EVALUATION_DIALOG_H
#define EVALUATION_DIALOG_H

#include <ctime>

#include <QPoint>
#include <QString>
#include <QTextBrowser>
#include <QWidget>

#include "evaluationframe.h"
#include "flight.h"
#include "flightpoint.h"

class Flight;

/**
 * \class EvaluationDialog
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \date 2000-2014
 *
 * \version 1.0
 */
class EvaluationDialog : public QWidget
{
  Q_OBJECT

private:

 Q_DISABLE_COPY ( EvaluationDialog )

 public:

  EvaluationDialog( QWidget *parent = 0);

  virtual ~EvaluationDialog();

  void updateText(int index1, int index2, bool updateAll = false);

  Flight* getFlight()
    {
      if( m_flight && m_flight->getTypeID() == BaseMapElement::Flight )
        {
          return m_flight;
        }
      else
        {
          return static_cast<Flight *>(0);
        }
    };

  unsigned int getTaskStart()
    {
      return m_evalFrame->getTaskStart();
    };

  unsigned int getTaskEnd()
    {
      return m_evalFrame->getTaskEnd();
    };

  EvaluationFrame* getEvalFrame()
  {
    return m_evalFrame;
  }

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

  /** Set the cursors in the EvaluationFrame object. */
  void slotSetCursors(time_t NewCursor1, time_t NewCursor2);

  /** No descriptions */
  void slotShowFlightPoint(const QPoint&, const FlightPoint&);
  /** No descriptions */
  void slotRemoveFlightPoint();

 protected:

  virtual void resizeEvent(QResizeEvent* event);

 private:

  QTextBrowser*     m_textDisplay;
  Flight*           m_flight;
  EvaluationFrame*  m_evalFrame;
};

#endif
