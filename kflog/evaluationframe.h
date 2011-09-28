/***********************************************************************
**
**   evaluationframe.h
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

#ifndef EVALUATION_FRAME_H
#define EVALUATION_FRAME_H

#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QTextBrowser>
#include <QResizeEvent>
#include <QWidget>

#include "evaluationview.h"

class EvaluationDialog;

/**
 * \class EvaluationFrame
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \date 2001-2011
 *
 * \version $Id$
 */
class EvaluationFrame : public QWidget
{
  Q_OBJECT

  private:

   Q_DISABLE_COPY( EvaluationFrame )

  public:
    /** */
    EvaluationFrame(QWidget* parent, EvaluationDialog* dlg);
    /** */
    virtual ~EvaluationFrame();
    /** */
    /** */
    unsigned int getTaskStart();
    /** */
    unsigned int getTaskEnd();

    /**
     * \return The current set flight or NULL, if no flight was set.
     */
    Flight* getFlight() const
      {
        return flight;
      };

  private slots:
    /** */
    void slotVarioSmoothness(int s);
    /** */
    void slotBaroSmoothness(int s);
    /** */
    void slotSpeedSmoothness(int s);
    /** */
    void slotScaleTime(int newScale);
    /** */
    void slotScaleSpeed(int newScale);
    /** */
    void slotScaleVario(int newScale);
    /** */
    void slotShowFlight( Flight* newFlight );
    /** */
    void slotShowGraph();
    /** */
    void slotUpdateCursorText(QString text);

  protected:

    virtual void resizeEvent(QResizeEvent* event);

  private:

    QScrollArea* graphFrame;
    EvaluationView* evalView;

    QLabel* cursorLabel;

    QCheckBox* check_vario;
    QCheckBox* check_baro;
    QCheckBox* check_speed;

    QSlider* sliderVario;
    QSlider* sliderBaro;
    QSlider* sliderSpeed;

    QSpinBox* spinTime;
    QSpinBox* spinVario;
    QSpinBox* spinSpeed;

    int smoothness_va;
    int smoothness_v;
    int smoothness_h;

    /** time scale values. */
    int timeScale;
    int timeScaleOld;

    /** variometer scale values. */
    int varioScale;

    /** speed scale values. */
    int speedScale;

    time_t centerTime;

    Flight* flight;
};

#endif
