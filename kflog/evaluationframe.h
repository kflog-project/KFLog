/***********************************************************************
**
**   evaluationframe.h
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

#ifndef EVALUATIONFRAME_H
#define EVALUATIONFRAME_H

#include <evaluationview.h>

#include <qcheckbox.h>
#include <qframe.h>
#include <qscrollview.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qtextview.h>

class EvaluationDialog;

/**
 * @author Heiner Lamprecht
 */
class EvaluationFrame : public QFrame
{
  Q_OBJECT

  public:
    /** */
    EvaluationFrame(QWidget* parent, EvaluationDialog* dlg);
    /** */
    ~EvaluationFrame();
    /** */
//    void updateCursorText(QString text);

  public slots:
    /** */
    void slotVarioGlatt(int glatt);
    /** */
    void slotBaroGlatt(int glatt);
    /** */
    void slotSpeedGlatt(int glatt);
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

    QScrollView* graphFrame;
    EvaluationView* evalView;

    QTextView* cursorLabel;

    QCheckBox* check_vario;
    QCheckBox* check_baro;
    QCheckBox* check_speed;

    QSlider* sliderVario;
    QSlider* sliderBaro;
    QSlider* sliderSpeed;
    QSpinBox* spinScale;

    int glatt_va;
    int glatt_v;
    int glatt_h;

    int secWidth;
    int secWidthOld;

    int centerTime;

    Flight* flight;

};

#endif
