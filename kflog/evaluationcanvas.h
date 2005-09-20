// /***********************************************************************
// **
// **   evaluationcanvas.h
// **
// **   This file is part of KFLog.
// **
// ************************************************************************
// **
// **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
// **
// **   This file is distributed under the terms of the General Public
// **   Licence. See the file COPYING for more information.
// **
// **   $Id$
// **
// ***********************************************************************/
//
// #ifndef EVALUATIONCANVAS_H
// #define EVALUATIONCANVAS_H
//
// #include <time.h>
// #include <qcanvas.h>
// #include <qpixmap.h>
// #include <qscrollview.h>
// #include <qwidget.h>
//
// #include "wp.h"
//
//
// class Flight;
// class EvaluationDialog;
//
//
// /**
//   * @author Heiner Lamprecht, Florian Ehinger
//   * @version $Id$
//   */
// class EvaluationCanvas : public QCanvasView
// {
//   Q_OBJECT
//
//  public:
//   /** */
//   EvaluationCanvas(QScrollView* parent, EvaluationDialog* dialog);
//   /** */
//   ~EvaluationCanvas();
//   /** */
//   void drawCurve(bool vario, bool speed, bool baro,
//            unsigned int glatt_va, unsigned int glatt_v,
//            unsigned int glatt_h, unsigned int secWidth);
//
//   /** */
//   enum CursorStatus { Reached = 8, NotReached = 16};
//
//   virtual QSize sizeHint();
//
//   // Cursor Positionen
//   time_t cursor1;
//   time_t cursor2;
//   time_t cursor_alt;
//
//  protected:
//   /**
//     * Redefinition of the paintEvent.
//     */
//   virtual void paintEvent(QPaintEvent* event);
//   /**
//     * Redefinition of the resizeEvent.
//     */
//  virtual void resizeEvent(QResizeEvent* event);
//   /**
//     */
//   virtual void mousePressEvent(QMouseEvent* event);
//
//   virtual void mouseMoveEvent(QMouseEvent* event);
//
//   virtual void mouseReleaseEvent(QMouseEvent* event);
//
//  private:
//   void __drawCsystem(QPainter* painter);
//   /** */
//   QPoint __baroPoint(int baro_d[], int gn, int i);
//   /** */
//   QPoint __varioPoint(float vario_d[], int gn, int i);
//   /** */
//   QPoint __speedPoint(float speed_d[], int gn, int i);
//
//   /**
//    * Prepares the buffers for the pointer.
//    */
//   void preparePointer();
//   /**
//    * Draws a pointer to indicate the current position
//    */
//   void drawPointer(const flightPoint * p);
//   /**
//    * Removes the pointer
//    */
//   void removePointer(bool);
//   /**
//    * Returns whether currently a pointer is being displayed
//    */
//   bool isShowingPointer();
//   /**
//    * Coordinates of last pointer position
//    */
//   QPoint lastPointerPosition;
//    /**
//     * Contains a reference to a buffer that contains the pointer for the current position, so we only need to draw it once
//     */
//   QPixmap* pixPointer;
//    /**
//     * Contains a reference to the mask for @ref pixPointer
//     */
//   QBitmap* bitPointerMask;
//    /**
//     * Contains a reference to a buffer that contains the contents of the graph under the position where the pointer was drawn
//     */
//   QPixmap* pixPointerBuffer;
//
//   void __paintCursor(int xpos, int calt, int move, int cursor);
//   /** Zeichnet die Kurven */
//   void __draw();
//   /** Zeichnet die Y Achse */
//   void __drawYAxis();
//
//   /** Behält den Inhalt der Zeichnung. */
//   QPixmap* pixBuffer;
//   QPixmap* pixBufferYAxis;
//   QPixmap* pixBufferKurve;
//   /* Wieso meckert der Compiler, wenn hier nur "QPixmap" statt eines
//    * Pointers steht ?????
//    */
//   time_t startTime;
//   time_t landTime;
//   time_t curTime;
//   /**
//     * Dieser Wert gibt den Abstand zwischen zwei Zeichenpunkten in
//     * Sekunden an.
//     */
//   unsigned int secWidth;
//
//   float scale_v;
//   float scale_h;
//   float scale_va;
//
//   unsigned int glatt_va;
//   unsigned int glatt_v;
//   unsigned int glatt_h;
//
//   bool baro;
//   bool vario;
//   bool speed;
//
//   QCanvas* canvas;
//
//   QScrollView* scrollFrame;
//
//   EvaluationDialog* evalDialog;
//
//   int mouseB;
//   int leftB;
//
//   Flight* flight;
//
// //  QPixmap pixCursor1;
// //  QPixmap pixCursor2;
// public slots: // Public slots
//   /** Shows a pointer under the time axis to indicate the position of flightPoint fp in the graph. If fp=0, then the flightpoint is removed. */
//   void slotShowPointer(const flightPoint * fp=0);
//
//
// };
//
// #endif
