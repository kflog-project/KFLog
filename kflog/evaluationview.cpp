/***********************************************************************
**
**   evaluationview.cpp
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

#include "evaluationview.h"

#include <kapp.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlayout.h>

#include "evaluationdialog.h"
#include <flight.h>
#include <resource.h>
#include <mapcalc.h>

#define X_ABSTAND 60
#define Y_ABSTAND 30

void EvaluationView::resizeEvent(QResizeEvent* event)
{
warning("  EvaluationView::resizeEvent()");
  pixBuffer->resize(event->size());
  scrollFrame->addChild(this);
}

void EvaluationView::paintEvent(QPaintEvent* event = 0)
{
  bitBlt(this, 0, 0, pixBuffer);
}


void EvaluationView::mousePressEvent(QMouseEvent* event)
{
  int x1 = ( cursor1 - startTime ) / secWidth + X_ABSTAND;
  int x2 = ( cursor2 - startTime ) / secWidth + X_ABSTAND;

  if(event->pos().x() < x1 + 5 && event->pos().x() > x1 - 5)
    {
      mouseB = event->button() | Reached;
      leftB = 1;
      cursor_alt = cursor1;
    }
  else if(event->pos().x() < x2 + 5 && event->pos().x() > x2 - 5)
    {
      mouseB = event->button() | Reached;
      leftB = 2;
      cursor_alt = cursor2;
    }
  else
    {
      mouseB = event->button() | NotReached;
      leftB = 0;
    }
}

void EvaluationView::mouseReleaseEvent(QMouseEvent* event)
{
  int time_alt, x;
  int cursor = -1;


  if(mouseB == (MidButton | Reached) ||
     mouseB == (MidButton | NotReached))
    {
      time_alt = cursor1;
      cursor = 1;
    }
  else if(mouseB == (RightButton | Reached) ||
          mouseB == (RightButton | NotReached))
    {
      time_alt = cursor2;
      cursor = 2;
    }
  else if(mouseB == (LeftButton | Reached))
    {
      if(leftB == 1)
        {
          time_alt =  cursor1;
          cursor = 1;
        }
      else
        {
          time_alt = cursor2;
          cursor = 2;
        }
    }

  mouseB = NoButton | NotReached;
  this->setCursor(arrowCursor);

 if(cursor == 1)
   {
      cursor1 =  flight->getPointByTime(
                ( event->pos().x() - X_ABSTAND ) * secWidth + startTime).time;

      if(cursor1 > cursor2) cursor1 = cursor2;
      x = ( cursor1 - startTime ) / secWidth + X_ABSTAND;
    }
  else if(cursor == 2)
    {
      cursor2 =  flight->getPointByTime(
                 ( event->pos().x() - X_ABSTAND ) * secWidth + startTime).time;

      if(cursor2 < cursor1) cursor2 = cursor1;
      x = ( cursor2 - startTime ) / secWidth + X_ABSTAND;
    }
  else return;



  evalDialog->updateText(flight->getPointByTime_i(cursor1),
                         flight->getPointByTime_i(cursor2), true);


  __paintCursor(x,(time_alt - startTime ) / secWidth + X_ABSTAND,0,cursor);
  bitBlt(this, 0, 0, pixBuffer);

}

void EvaluationView::mouseMoveEvent(QMouseEvent* event)
{
  int x1 = ( cursor1 - startTime ) / secWidth + X_ABSTAND;
  int x2 = ( cursor2 - startTime ) / secWidth + X_ABSTAND;

  if(mouseB == (NoButton | NotReached))
    {
      if(event->pos().x() < x1 + 5 && event->pos().x() > x1 - 5)
          this->setCursor(sizeHorCursor);
      else if(event->pos().x() < x2 + 5 && event->pos().x() > x2 - 5)
          this->setCursor(sizeHorCursor);
      else
        {
          this->setCursor(arrowCursor);
          return;
        }

    }
  else if(mouseB != (LeftButton | NotReached) &&
          mouseB != (NoButton | Reached))
    {
    // Koordinaten in Bildschirm Koordinaten
      int cursor = flight->getPointByTime((event->pos().x() - X_ABSTAND )
                           * secWidth + startTime).time;

      __paintCursor(( cursor - startTime ) / secWidth + X_ABSTAND,
                    ( cursor_alt - startTime ) / secWidth + X_ABSTAND,1,0);

      cursor_alt = flight->getPointByTime((event->pos().x() - X_ABSTAND )
                          * secWidth + startTime).time;


      //  kontinuierliches Update der Anzeige
      // was wird in Mouse Release noch gebraucht??
      int cursor_1 = cursor1;
      int cursor_2 = cursor2;

      if(mouseB == (MidButton | Reached) ||
        mouseB == (MidButton | NotReached))
      {
        cursor_1 = cursor;
      }
      else if(mouseB == (RightButton | Reached) ||
            mouseB == (RightButton | NotReached))
      {
        cursor_2 = cursor;
      }
      else if(mouseB == (LeftButton | Reached))
      {
        if(leftB == 1)
          {
            cursor_1 = cursor;
          }
        else
          {
            cursor_2 = cursor;
          }
      }

      evalDialog->updateText(flight->getPointByTime_i(cursor_1),
                             flight->getPointByTime_i(cursor_2));
    }
}


QPoint EvaluationView::__baroPoint(int height, int durch[], int gn, int i)
{
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND;

  int gesamt = 0;

  for(int loop = 0; loop < MIN(gn, (i * 2 + 1)); loop++)
      gesamt += durch[loop];

//  warning("-----> scale_h %.3f", scale_h);

  int y = this->height() - (int)( ( gesamt / MIN(gn, (i * 2 + 1)) ) / scale_h )
                                                  - Y_ABSTAND;

  return QPoint(x, y);
}

QPoint EvaluationView::__speedPoint(float speed, float durch[], int gn, int i)
{
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND;

  float gesamt = 0;
  /*
   * Jetzt läuft die Schleife immer durch. Wenn i < gn ist, nur bis i, sonst bis gn.
   * Das Ganze beruht auf der Annahme, dass die zur Berechnung nötigen Punkte am Anfang
   * des Arrays stehen. Das klappt am Anfang der Kurve sehr gut, am Ende würde es unter
   * Umständen zu Fehlern führen ...
   */
  for(int loop = 0; loop < MIN(gn, (i * 2 + 1)); loop++)
      gesamt += durch[loop];

  int y = this->height() - (int)( ( gesamt / MIN(gn, (i *  2 + 1)) ) / scale_v )
                                                  - Y_ABSTAND;

  return QPoint(x, y);
}

QPoint EvaluationView::__varioPoint(float vario, float durch[], int gn, int i)
{
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND;

  float gesamt = 0;
  for(int loop = 0; loop < MIN(gn, (i * 2 + 1)); loop++)
      gesamt += durch[loop];

  int y = (this->height() / 2) - (int)( ( gesamt / MIN(gn, (i * 2 + 1)) ) / scale_va );

  return QPoint(x, y);
}

void EvaluationView::__drawCsystem(QPainter* painter, bool vario, bool speed, bool baro)
{
  warning("__drawCsystem");
  /*
   * Die Schleife unten kann nicht terminieren, wenn scale_h negativ ist!
   */
  if(scale_h < 0.0) return;
  if(!baro && !vario && !speed) return;

  QString text;

  //Koordinatenachsen
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->drawLine(X_ABSTAND,this->height() - Y_ABSTAND,
                    this->width() - 20,this->height() - Y_ABSTAND);
  painter->drawLine(X_ABSTAND,this->height() - Y_ABSTAND,
                    X_ABSTAND, 20);
  if(vario)
    {
      painter->setPen(QPen(QColor(255,100,100), 2));
      painter->drawLine(X_ABSTAND, this->height() / 2,
                        this->width(), this->height() / 2);
    }

  int time = ((startTime / 900) + 1) * 900 - startTime;

  // Zeitachse
  while(time / (int)secWidth < this->width() - 2*X_ABSTAND)
    {
      painter->setPen(QPen(QColor(0,0,0), 2));
      painter->drawLine(X_ABSTAND + time / secWidth,this->height() - Y_ABSTAND,
                  X_ABSTAND + time / secWidth,this->height() - Y_ABSTAND + 10);



      time += 900;
    }
  time = ((startTime / 900) + 1) * 900 - startTime;
  while(time / (int)secWidth < this->width() - 2*X_ABSTAND)
    {
      painter->setPen(QPen(QColor(0,0,0), 1));
      text = printTime(startTime + time,false,false);
      painter->drawText(X_ABSTAND + time / secWidth - 27,this->height() - 5,
                  text);

      if(secWidth > 22) {
        time += 3600;
      } else if (secWidth > 14) {
        time += 1800;
      } else {
        time += 900;
      }
    }


  if(!vario && !speed && baro)
    {
      // Barogramm
      int dh = 100;
      if(scale_h > 10)     dh = 500;
      else if(scale_h > 8) dh = 250;
      else if(scale_h > 3) dh = 200;

      warning("scale_h: %.2f -- dh: %d",scale_h,dh);
      warning("this->height: %d", this->height());

      int h = dh;
      painter->setFont(QFont("helvetica",10));

      while(h / scale_h < this->height() - (Y_ABSTAND * 2))
        {
          painter->setPen(QPen(QColor(100,100,255), 1));
          text.sprintf("%d m",h);
          painter->drawText(3, this->height() - (int)( h / scale_h )
                      - Y_ABSTAND + 3, text);

          if(h == 1000 || h == 2000 || h == 3000)
            {
              painter->setPen(QPen(QColor(200,200,255), 2));
            }
          else
            {
              painter->setPen(QPen(QColor(200,200,255), 1));
            }
          painter->drawLine(X_ABSTAND - 3,
                      this->height() - (int)( h / scale_h ) - Y_ABSTAND,
                      this->width() - 20,this->height() - (int)( h / scale_h )
                            - Y_ABSTAND);

          h += dh;
        }
    }
  else if(!speed && !baro && vario)
    {
      // Variogramm
      painter->setPen(QPen(QColor(255,100,100), 1));
      painter->drawLine(X_ABSTAND,         (this->height() / 2),
                      this->width() - 20,(this->height() / 2));

      float dva = 2.0;
      if(scale_va > 0.15)      dva = 5.0;
      else if(scale_va > 0.1) dva = 3.5;
      else if(scale_va > 0.08) dva = 2.5;

      float va = 0;
      painter->setFont(QFont("helvetica",8));

      while(va / scale_va < (this->height() / 2) - Y_ABSTAND)
        {
          text.sprintf("%.2f m/s",va);
          painter->setPen(QPen(QColor(255,100,100), 1));
          painter->drawText(3, (this->height() / 2) - (int)( va / scale_va )
                  + 3, text);

          painter->setPen(QPen(QColor(255,200,200), 1));
          painter->drawLine(X_ABSTAND - 3,(this->height() / 2) - (int)( va / scale_va ),
                   this->width() - 20,(this->height() / 2) - (int)( va / scale_va ));

          if(va != 0)
            {
              text.sprintf("-%.2f m/s",va);
              painter->setPen(QPen(QColor(255,100,100), 1));
              painter->drawText(3, (this->height() / 2) + (int)( va / scale_va ) + 3, text);

              painter->setPen(QPen(QColor(255,200,200), 1));
              painter->drawLine(X_ABSTAND,(this->height() / 2) + (int)( va / scale_va ) - 3,
                      this->width() - 20,(this->height() / 2) + (int)( va / scale_va ));

            }
          va += dva;
        }
    }
  else if(!baro && !vario && speed)
    {
      // Speedogramm

      int dv = 10;
      if(scale_v > 0.9)     dv = 40;
      else if(scale_v > 0.6) dv = 25;
      else if(scale_v > 0.3) dv = 20;

      warning("scale_v: %.2f -- dv: %d",scale_v,dv);

      int v = dv;
      painter->setFont(QFont("helvetica",10));

      while(v / scale_v < this->height() - (Y_ABSTAND * 2))
        {
          text.sprintf("%d km/h",v);
          painter->setPen(QPen(QColor(0,0,0), 1));
          painter->drawText(3, this->height() - (int)( v / scale_v ) - Y_ABSTAND + 3,
                      text);

          painter->setPen(QPen(QColor(200,200,200), 1));
          painter->drawLine(X_ABSTAND + 3,
                      this->height() - (int)( v / scale_v ) - Y_ABSTAND,
                      this->width() - 20,this->height() - (int)( v / scale_v ) - Y_ABSTAND);
          v += dv;
        }
    }
}



void EvaluationView::drawCurve(Flight* current, bool vario, bool speed,
            bool baro, unsigned int glatt_va, unsigned int glatt_v,
            unsigned int glatt_h, unsigned int secW)
{
  warning("drawCurve");
  setMouseTracking(true);

  flight = current;
  startTime = flight->getStartTime();
  landTime = flight->getLandTime();


  cursor1 = MAX(startTime,cursor1);
  cursor2 = MAX(startTime,cursor2);
  cursor1 = MIN(landTime,cursor1);
  cursor2 = MIN(landTime,cursor2);

  secWidth = secW;

//  paint.begin(this);



  // Skalierungsfaktor -- horizontal
  // Sekunde / Punkt
//  warning("secWidth: %d", secWidth);
//  secWidth = (landTime - startTime) / ((double)(scrollFrame->viewport()->width() -
//                                              2*X_ABSTAND));

  warning("secWidth: %d", secWidth);

  this->resize((landTime - startTime) / secWidth + X_ABSTAND * 2,
      scrollFrame->viewport()->height());

//  this->resize(scrollFrame->viewport()->width(),
//      scrollFrame->viewport()->height());


  // Skalierungsfaktor -- vertical
  scale_v = getSpeed(flight->getPoint(Flight::V_MAX)) /
          ((double)(this->height() - 2*Y_ABSTAND));
  scale_h = flight->getPoint(Flight::H_MAX).height /
          ((double)(this->height() - 2*Y_ABSTAND));
  scale_va = MAX(getVario(flight->getPoint(Flight::VA_MAX)),
              ( -1.0 * getVario(current->getPoint(Flight::VA_MIN))) ) /
          ((double)(this->height() - 2*Y_ABSTAND) / 2.0);



  //
  unsigned int gn_v = glatt_v * 2 + 1;
  unsigned int gn_va = glatt_va * 2 + 1;
  unsigned int gn_h = glatt_h * 2 + 1;

  int baro_d[gn_h];
  int baro_d_last[gn_h];
  float speed_d[gn_v];
  float speed_d_last[gn_v];
  float vario_d[gn_va];
  float vario_d_last[gn_va];

  for(unsigned int loop = 0; loop < gn_h; loop++)
    {
      baro_d[loop] = current->getPoint(loop).height;
      baro_d_last[loop] = current->getPoint(current->getRouteLength() - loop - 1).height;
    }
  for(unsigned int loop = 0; loop < gn_v; loop++)
    {
      speed_d[loop] = getSpeed(current->getPoint(loop));
      speed_d_last[loop] = getSpeed(current->getPoint(current->getRouteLength() - loop - 1));
    }
  for(unsigned int loop = 0; loop < gn_va; loop++)
    {
       vario_d[loop] = getVario(current->getPoint(loop));
      vario_d_last[loop] =
          getVario(current->getPoint(current->getRouteLength() - loop - 1));
    }

  QPointArray baroArray(current->getRouteLength());
  QPointArray varioArray(current->getRouteLength());
  QPointArray speedArray(current->getRouteLength());


  for(unsigned int loop = 0; loop < current->getRouteLength(); loop++)
    {
      curTime = current->getPoint(loop).time;

      /* Der Array wird hier noch falsch gefüllt. Wenn über 3 Punkte geglättet wird, stimmt
       * alles. Wenn jedoch z.B. über 5 Punkte geglättet wird, werden die Punkte
       * ( -4, -3, -2, -1, 0, 1) genommen, statt (-2, -1, 0, 1, 2). Das ist vermutlich
       * die Ursache dafür, dass die Kurve "wandert".
       */
      if(loop < current->getRouteLength() - glatt_h && loop > glatt_h)
          baro_d[(loop - glatt_h - 1) % gn_h] = current->getPoint(loop + glatt_h).height;

      if(loop < current->getRouteLength() - glatt_v && loop > glatt_v)
          speed_d[(loop - glatt_v - 1) % gn_v] = getSpeed(current->getPoint(loop + glatt_v));

      if(loop < current->getRouteLength() - glatt_va && loop > glatt_va)
          vario_d[(loop - glatt_va - 1) % gn_va] = getVario(current->getPoint(loop + glatt_va));

      /* Wenn das Glätten wie bei __speedPoint() erfolgt, können gn_? und loop auch als
       * unsigned übergeben werden ...
       */
      if(loop < current->getRouteLength() - glatt_h)
          baroArray.setPoint(loop,
              __baroPoint(current->getPoint(loop).height, baro_d, gn_h, loop));
      else
          baroArray.setPoint(loop,
              __baroPoint(current->getPoint(loop).height, baro_d_last, gn_h,
                        current->getRouteLength() - loop - 1));
      if(loop < current->getRouteLength() - glatt_va)
          varioArray.setPoint(loop,
              __varioPoint(getVario(current->getPoint(loop)), vario_d, gn_va, loop));
      else
          varioArray.setPoint(loop,
              __varioPoint(getVario(current->getPoint(loop)), vario_d_last, gn_va,
                        current->getRouteLength() - loop - 1));

      if(loop < current->getRouteLength() - glatt_v)
          speedArray.setPoint(loop,
              __speedPoint(getSpeed(current->getPoint(loop)), speed_d, gn_v, loop));
      else
          speedArray.setPoint(loop,
              __speedPoint(getSpeed(current->getPoint(loop)), speed_d_last, gn_v,
                        current->getRouteLength() - loop - 1));
    }

  pixBuffer->fill(white);
  QPainter paint(pixBuffer);


  __drawCsystem(&paint, vario, speed, baro);



  int xpos = 0;

  // Wendepunkte
/*  QList* wP = flight->getWPList();
  for(unsigned int n = 0; n < flight->getRouteLength(); n++) {
      paint.setPen(QPen(QColor(100,255,0), 1));
//      paint.setRasterOp(XorROP);
      xpos = wP->at(n).sector1;
      paint.drawLine(xpos,this->height() - Y_ABSTAND, xpos,Y_ABSTAND);
      paint.end();
  }

  */


  if(vario)
    {
      paint.setPen(QPen(QColor(255,100,100), 1));
      paint.drawPolyline(varioArray);
    }
  if(speed)
    {
      paint.setPen(QPen(QColor(0,0,0), 1));
      paint.drawPolyline(speedArray);
    }
  if(baro)
    {
      paint.setPen(QPen(QColor(100,100, 255), 1));
      paint.drawPolyline(baroArray);
    }

  paint.end();

   __paintCursor(( cursor1 - startTime ) / secWidth + X_ABSTAND,-2000,0,1);
   __paintCursor(( cursor2 - startTime ) / secWidth + X_ABSTAND,-2000,0,2);
  bitBlt(this, 0, 0, pixBuffer);
}

void EvaluationView::__paintCursor(int xpos, int calt, int move, int cursor)
{
  // Bildschirmkoordinaten !!
  QPainter paint;

//  warning("Male Linie xpos: %d calt: %d",xpos,calt);
  if(move == 1)
    {
      paint.begin(this);
      if(cursor == 1)
        {
          paint.setPen(QPen(QColor(0,255,0), 1));
        }
      else
        {
          paint.setPen(QPen(QColor(255,0,0), 1));
        }
      paint.setRasterOp(XorROP);
      paint.drawLine(calt,this->height() - Y_ABSTAND, calt,Y_ABSTAND);
      paint.drawLine(xpos,this->height() - Y_ABSTAND, xpos,Y_ABSTAND);
      paint.end();
    }
   else
    {
      paint.begin(pixBuffer);
      paint.setRasterOp(XorROP);
      // Gemalt wird die komplementär-Farbe ...
      if(cursor == 1)
        {
          paint.setPen(QPen(QColor(255,0,255), 1));
          paint.setBrush(QBrush(QColor(255,0,255), SolidPattern));
        }
      else
      {
          paint.setPen(QPen(QColor(0,255,255), 1));
          paint.setBrush(QBrush(QColor(0,255,255), SolidPattern));
      }

      QPointArray flagArray(3);
      // alte Linie übermalen
      paint.drawLine(calt, this->height() - Y_ABSTAND, calt, Y_ABSTAND);
      if(cursor == 1)
        {
          flagArray.setPoint(0, QPoint(calt - 10, Y_ABSTAND));
          flagArray.setPoint(1, QPoint(calt - 4, Y_ABSTAND - 5));
          flagArray.setPoint(2, QPoint(calt - 10, Y_ABSTAND - 10));
        }
      else
        {
          flagArray.setPoint(0, QPoint(calt + 10, Y_ABSTAND));
          flagArray.setPoint(1, QPoint(calt + 4, Y_ABSTAND - 5));
          flagArray.setPoint(2, QPoint(calt + 10, Y_ABSTAND - 10));
        }
      paint.drawPolygon(flagArray);

      // neue Linie malen
      paint.drawLine(xpos, this->height() - Y_ABSTAND, xpos, Y_ABSTAND);
      flagArray.setPoint(1, QPoint(xpos, Y_ABSTAND - 5));
      if(cursor == 1)
        {
          flagArray.setPoint(0, QPoint(xpos - 10, Y_ABSTAND));
          flagArray.setPoint(1, QPoint(xpos - 4, Y_ABSTAND - 5));
          flagArray.setPoint(2, QPoint(xpos - 10, Y_ABSTAND - 10));
        }
      else
        {
          flagArray.setPoint(0, QPoint(xpos + 10, Y_ABSTAND));
          flagArray.setPoint(1, QPoint(xpos + 4, Y_ABSTAND - 5));
          flagArray.setPoint(2, QPoint(xpos + 10, Y_ABSTAND - 10));
        }
      paint.drawPolygon(flagArray);
      paint.end();
    }
}

EvaluationView::EvaluationView(QScrollView* parent, EvaluationDialog* dialog)
: QWidget(parent, "EvaluationView", false),
  startTime(0), secWidth(5), scrollFrame(parent), evalDialog(dialog)
{
  pixBuffer = new QPixmap;
  pixBuffer->resize(scrollFrame->viewport()->size());

  mouseB = NoButton | NotReached;
  cursor1 = 0;
  cursor2 = 86400;

  cursor_alt = 200000;

  setMouseTracking(true);
  warning("Maustracking; %d", this->hasMouseTracking());
  setBackgroundColor(QColor(white));
}

EvaluationView::~EvaluationView()
{
  delete pixBuffer;
}
