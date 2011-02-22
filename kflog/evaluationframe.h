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

  public slots:
    /** */
    void slotVarioSmoothness(int s);
    /** */
    void slotBaroSmoothness(int s);
    /** */
    void slotSpeedSmoothness(int s);
    /** */
    void slotScale(int secWidth);
    /** */
    void slotShowFlight();
    /** */
    void slotShowGraph();
    /** */
    void slotUpdateCursorText(QString text);

  protected:

    virtual void resizeEvent(QResizeEvent* event);

  private:

    QScrollArea* graphFrame;
    EvaluationView* evalView;

    QTextBrowser* cursorLabel;

    QCheckBox* check_vario;
    QCheckBox* check_baro;
    QCheckBox* check_speed;

    QSlider* sliderVario;
    QSlider* sliderBaro;
    QSlider* sliderSpeed;
    QSpinBox* spinScale;

    int smoothness_va;
    int smoothness_v;
    int smoothness_h;

    int secWidth;
    int secWidthOld;

    time_t centerTime;

    Flight* flight;
};

#endif
