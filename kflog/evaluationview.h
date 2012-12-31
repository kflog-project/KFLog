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

#include <ctime>

#include <QBitmap>
#include <QPixmap>
#include <QPolygonF>
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

  void drawCurve( bool vario,
                  bool speed,
                  bool baro,
                  unsigned int smoothness_va,
                  unsigned int smoothness_v,
                  unsigned int smoothness_h,
                  unsigned int secWidth,
                  unsigned int speedScale,
                  unsigned int varioScale);

  enum CursorStatus { Reached = 8, NotReached = 16};

    void slotSetCursors(Flight * theFlight, time_t NewCursor1, time_t NewCursor2);

    // Cursor Positions
    time_t cursor1;
    time_t cursor2;
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

  /** Draws the coordinate system axis. */
  void __drawCsystem(QPainter* painter);

  /** Calculates the elevation point for the drawing polygon. */
  QPoint __baroPoint(int durch[], int gn, int i);

  /** Calculates the altitude point for the drawing polygon. */
  void _addBaroPoint( QPolygonF& vector, int baro_d[], int gn, int i);

  /** Calculates the variometer point for the drawing polygon. */
  void _addVarioPoint( QPolygonF& vector, float vario_d[], int gn, int i );

  /** Calculates the speed point for the drawing polygon. */
  void _addSpeedPoint( QPolygonF& vector, float speed_d[], int gn, int i);

  /**
   * Prepares the flight pointer.
   */
  void preparePointer();
  /**
   * Makes a drawn flight pointer visible.
   */
  void makeFlightPointerVisible();

  /**
   * Draws two different kind of cursors at the diagram.
   *
   * \param xpos The X-coordinate of the new cursor position.
   * \param move If true a move cursor (vertical bar) is drawn
   *             otherwise a flag cursor is drawn.
   * \param cursor Defines which cursor shall be drawn. There are two
   *               cursors (1 and 2) selectable.
   */
  void __drawCursor(const int xpos, const bool move, const int cursor);

  /** Draw graphs */
  void __draw();

  /** Draw y-axis */
  void __drawYAxis();

/**
   * Coordinates of last pointer position
   */
  QPoint lastPointerPosition;
   /**
    * Contains a reference to a buffer that contains the pointer
    * for the current position, so we only need to draw it once
    */
  QPixmap pixPointer;

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

  /** Maximum speed value, used for diagram drawing. */
  unsigned int speedScale;

  /** Maximum variometer value, used for diagram drawing. */
  unsigned int varioScale;

  double scale_v;
  double scale_h;
  double scale_va;

  int smoothness_va;
  int smoothness_v;
  int smoothness_h;

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
