/***********************************************************************
**
**   evaluationframe.h
**
**   This file is part of KFLog4.
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

#ifndef EVALUATIONFRAME_H
#define EVALUATIONFRAME_H

#include "evaluationview.h"

#include <qcheckbox.h>
#include <q3frame.h>
#include <q3scrollview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <q3textview.h>
//Added by qt3to4:
#include <QResizeEvent>

class EvaluationDialog;

/**
 * @author Heiner Lamprecht
 */
class EvaluationFrame : public Q3Frame
{
  Q_OBJECT

  public:
    /** */
    EvaluationFrame(QWidget* parent, EvaluationDialog* dlg);
    /** */
    ~EvaluationFrame();
    /** */
    /** */
    unsigned int getTaskStart();
    /** */
    unsigned int getTaskEnd();
//    void updateCursorText(QString text);

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

    Q3ScrollView* graphFrame;
    EvaluationView* evalView;

    Q3TextView* cursorLabel;

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
