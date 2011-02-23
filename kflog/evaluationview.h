/***********************************************************************
**
**   evaluationview.h
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

#ifndef EVALUATION_VIEW_H
#define EVALUATION_VIEW_H

#include <time.h>

#include <QBitmap>
#include <QPixmap>
#include <QScrollArea>
#include <QWidget>

class Flight;
class FlightPoint;
class EvaluationDialog;

/**
 * \class EvaluationView
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Draws the flight evaluation.
 *
 * \date 2000-2011
 *
 * \version $Id$
 */
class EvaluationView : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY( EvaluationView )

public:

  EvaluationView(QScrollArea* parent, EvaluationDialog* dialog);

  virtual ~EvaluationView();

  void drawCurve( bool vario, bool speed, bool baro,
                  unsigned int smoothness_va, unsigned int smoothness_v,
                  unsigned int smoothness_h, unsigned int secWidth );

  enum CursorStatus { Reached = 8, NotReached = 16};

  virtual QSize sizeHint();

  // Cursor Positions
  time_t cursor1;
  time_t cursor2;
  time_t cursor_alt;

protected:

  virtual void paintEvent(QPaintEvent* event);

  virtual void resizeEvent(QResizeEvent* event);

  virtual void mousePressEvent(QMouseEvent* event);

  virtual void mouseMoveEvent(QMouseEvent* event);

  virtual void mouseReleaseEvent(QMouseEvent* event);

public slots:

  /* Shows a pointer under the time axis to indicate the
   * position of FlightPoint fp in the graph. If fp=0,
   * then the flight point is removed.
   */
  void slotShowPointer(const FlightPoint* fp=0);

private:

  void __drawCsystem(QPainter* painter);
  /** */
  QPoint __baroPoint(int baro_d[], int gn, int i);
  /** */
  QPoint __varioPoint(float vario_d[], int gn, int i);
  /** */
  QPoint __speedPoint(float speed_d[], int gn, int i);

  /**
   * Prepares the flight pointer.
   */
  void preparePointer();
  /**
   * Makes a drawn flight pointer visible.
   */
  void makeFlightPointerVisible();

private:

  /**
   * Coordinates of last pointer position
   */
  QPoint lastPointerPosition;
   /**
    * Contains a reference to a buffer that contains the pointer
    * for the current position, so we only need to draw it once
    */
  QPixmap pixPointer;

  void __paintCursor(int xpos, int move, int cursor);
  /** Draw graphs */
  void __draw();
  /** Draw y-axis */
  void __drawYAxis();

  /** Stores the mouse move cursors. */
  QPixmap pixBufferMouse;

  /** Contains the Y-Axis. */
  QPixmap pixBufferYAxis;

  /** Contains the altitude, speed, variometer curves.*/
  QPixmap pixBufferKurve;

  time_t startTime;
  time_t landTime;
  time_t curTime;
  /**
    * Dieser Wert gibt den Abstand zwischen zwei Zeichenpunkten in
    * Sekunden an.
    */
  unsigned int secWidth;

  float scale_v;
  float scale_h;
  float scale_va;

  unsigned int smoothness_va;
  unsigned int smoothness_v;
  unsigned int smoothness_h;

  bool baro;
  bool vario;
  bool speed;

  QScrollArea* scrollFrame;
  EvaluationDialog* evalDialog;

  int mouseB;
  int leftB;

  /** Flight data which are displayed. */
  Flight* flight;

  /** Flight point at which a pointer is set. When Null, no pointer is set. */
  const FlightPoint* flightPointPointer;
};

#endif
