/***********************************************************************
**
**   evaluationview.h
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

#ifndef EVALUATIONVIEW_H
#define EVALUATIONVIEW_H

#include <qpixmap.h>
#include <qscrollview.h>
#include <qwidget.h>

class Flight;
class EvaluationDialog;


/**
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class EvaluationView : public QWidget
{
  Q_OBJECT

 public:
  /** */
  EvaluationView(QScrollView* parent, EvaluationDialog* dialog);
  /** */
  ~EvaluationView();
  /** */
  void drawCurve(bool vario, bool speed, bool baro,
           unsigned int glatt_va, unsigned int glatt_v,
           unsigned int glatt_h, unsigned int secWidth);

  /** */
  enum CursorStatus { Reached = 8, NotReached = 16};

  virtual QSize sizeHint();

 protected:
  /**
    * Redefinition of the paintEvent.
    */
  virtual void paintEvent(QPaintEvent* event);
  /**
    * Redefinition of the resizeEvent.
    */
//  virtual void resizeEvent(QResizeEvent* event);
  /**
    */
  virtual void mousePressEvent(QMouseEvent* event);

  virtual void mouseMoveEvent(QMouseEvent* event);

  virtual void mouseReleaseEvent(QMouseEvent* event);

 private:
  void __drawCsystem(QPainter* painter);
  /** */
  QPoint __baroPoint(int baro_d[], int gn, int i);
  /** */
  QPoint __varioPoint(float vario_d[], int gn, int i);
  /** */
  QPoint __speedPoint(float speed_d[], int gn, int i);

  void __paintCursor(int xpos, int calt, int move, int cursor);
  /** Zeichnet die Kurven */
  void __draw();
  /** Zeichnet die Y Achse */
  void __drawYAxis();

  /** Behält den Inhalt der Zeichnung. */
  QPixmap* pixBuffer;
  QPixmap* pixBufferYAxis;
  QPixmap* pixBufferKurve;
  /* Wieso meckert der Compiler, wenn hier nur "QPixmap" statt eines
   * Pointers steht ?????
   */
  unsigned int startTime;
  unsigned int landTime;
  unsigned int curTime;
  /**
    * Dieser Wert gibt den Abstand zwischen zwei Zeichenpunkten in
    * Sekunden an.
    */
  unsigned int secWidth;

  float scale_v;
  float scale_h;
  float scale_va;

  unsigned int glatt_va;
  unsigned int glatt_v;
  unsigned int glatt_h;

  bool baro;
  bool vario;
  bool speed;

  QScrollView* scrollFrame;

  EvaluationDialog* evalDialog;

  int mouseB;
  int leftB;
  // Cursor Positionen
  unsigned int cursor1;
  unsigned int cursor2;
  unsigned int cursor_alt;

//  Flight* flight;

  bool isFlight;

//  QPixmap pixCursor1;
//  QPixmap pixCursor2;
};

#endif
