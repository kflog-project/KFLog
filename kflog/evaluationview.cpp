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
#include <kglobal.h>
#include <kstddirs.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qlayout.h>

#include "evaluationdialog.h"
#include <flight.h>
#include <resource.h>
#include <mapcalc.h>

#define X_ABSTAND 100
#define Y_ABSTAND 30
// auch in evalutionFrame ändern!!
#define KOORD_DISTANCE 60

EvaluationView::EvaluationView(QScrollView* parent, EvaluationDialog* dialog)
: QWidget(parent, "EvaluationView", false),
  startTime(0), secWidth(5), scrollFrame(parent), evalDialog(dialog)
{
  pixBuffer = new QPixmap;
  pixBuffer->resize(1,1);

  pixBufferYAxis = new QPixmap;
  pixBufferYAxis->resize(1,1);

  pixBufferKurve = new QPixmap;
  pixBufferKurve->resize(1,1);


  mouseB = NoButton | NotReached;
  cursor1 = 0;
  cursor2 = 86400;

  cursor_alt = 200000;

  isFlight = false;

  setMouseTracking(true);

  setBackgroundColor(QColor(white));
}

EvaluationView::~EvaluationView()
{
  delete pixBuffer;
}



QSize EvaluationView::sizeHint()
{
  return QWidget::sizeHint();
}

void EvaluationView::paintEvent(QPaintEvent* event = 0)
{
  bitBlt(pixBuffer, 0, 0, pixBufferKurve);

  if(!((vario && speed) || (vario && baro) || (baro && speed)))
    {
      bitBlt(pixBuffer, scrollFrame->contentsX(),0, pixBufferYAxis);
    }
  bitBlt(this, 0, 0, pixBuffer);

//  warning("paintEvent");
}

void EvaluationView::mousePressEvent(QMouseEvent* event)
{
  int x1 = ( cursor1 - startTime ) / secWidth
             + X_ABSTAND ;
  int x2 = ( cursor2 - startTime ) / secWidth
             + X_ABSTAND ;

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
      x = ( cursor1 - startTime ) / secWidth + X_ABSTAND ;
    }
  else if(cursor == 2)
    {
      cursor2 =  flight->getPointByTime(
                 ( event->pos().x() - X_ABSTAND ) * secWidth + startTime).time;

      if(cursor2 < cursor1) cursor2 = cursor1;
      x = ( cursor2 - startTime ) / secWidth + X_ABSTAND ;
    }
  else return;

  evalDialog->updateText(flight->getPointByTime_i(cursor1),
                         flight->getPointByTime_i(cursor2), true);

  __draw();

  paintEvent(0);
}

void EvaluationView::mouseMoveEvent(QMouseEvent* event)
{
  int x1 = (( cursor1 - startTime ) / secWidth)
             + X_ABSTAND ;
  int x2 = (( cursor2 - startTime ) / secWidth)
             + X_ABSTAND ;


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

      __paintCursor(( cursor - startTime ) / secWidth
                        + X_ABSTAND ,
                    ( cursor_alt - startTime ) / secWidth
                        + X_ABSTAND ,1,0);

      cursor_alt = flight->getPointByTime((event->pos().x() - X_ABSTAND
                           ) *
                          secWidth + startTime).time;

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
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND ;

  int gesamt = 0;

  for(int loop = 0; loop < MIN(gn, (i * 2 + 1)); loop++)
      gesamt += durch[loop];

  int y = this->height() - (int)( ( gesamt / MIN(gn, (i * 2 + 1)) ) / scale_h )
                                                  - Y_ABSTAND;

  return QPoint(x, y);
}

QPoint EvaluationView::__speedPoint(float speed, float durch[], int gn, int i)
{
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND ;
  //  = Abstand am Anfang der Kurve

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
  int x = ( curTime - startTime ) / secWidth + X_ABSTAND ;
  // PRE_GRAPH_DISTANCE = Abstand am Anfang der Kurve

  float gesamt = 0;
  for(int loop = 0; loop < MIN(gn, (i * 2 + 1)); loop++)
      gesamt += durch[loop];

  int y = (this->height() / 2) - (int)( ( gesamt / MIN(gn, (i * 2 + 1)) ) / scale_va );

  return QPoint(x, y);
}

void EvaluationView::__drawCsystem(QPainter* painter)
{
  /*
   * Die Schleife unten kann nicht terminieren, wenn scale_h negativ ist!
   */

  pixBufferYAxis->fill(white);
  QPainter painterText(pixBufferYAxis);

  if(scale_h < 0.0) return;
//  if(!baro && !vario && !speed) return;

  QString text;

  int breite = ((landTime - startTime) / secWidth)
                 + (KOORD_DISTANCE * 2) ;
  int hoehe = scrollFrame->viewport()->height();

  //Koordinatenachsen
  painter->setPen(QPen(QColor(0,0,0), 1));
  painter->drawLine(KOORD_DISTANCE,hoehe - Y_ABSTAND, breite,hoehe - Y_ABSTAND);
  painter->drawLine(KOORD_DISTANCE,hoehe - Y_ABSTAND, KOORD_DISTANCE, Y_ABSTAND);
  // Vario Null Linie
  if(vario)
    {
      painter->setPen(QPen(QColor(255,100,100), 2));
      painter->drawLine(KOORD_DISTANCE, hoehe / 2, breite, hoehe / 2);
    }


  // Zeitachse
  int time_plus, time_small_plus;
  if(secWidth > 22)
    {
      time_plus = 3600;
      time_small_plus = 1800;
    }
  else if (secWidth > 14)
    {
      time_plus = 1800;
      time_small_plus = 900;
    }
  else if (secWidth > 10)
    {
      time_plus = 900;
      time_small_plus = 300;
    }
  else
    {
      time_plus = 600;
      time_small_plus = 60;
    }

  int time = (((startTime - 1) / time_plus) + 1) * time_plus - startTime;
  int time_small = (((startTime - 1) / time_small_plus) + 1)
                       * time_small_plus - startTime;

  while(time / (int)secWidth < breite - 2*KOORD_DISTANCE)
    {
      painter->setPen(QPen(QColor(0,0,0), 2));
      painter->drawLine(X_ABSTAND + (time / secWidth) ,
                  hoehe - Y_ABSTAND,
                  X_ABSTAND + (time / secWidth) ,
                  hoehe - Y_ABSTAND + 10);

      painter->setPen(QPen(QColor(0,0,0), 1));
      text = printTime(startTime + time,false,false);
      painter->drawText(X_ABSTAND + (time / secWidth) - 40
                          ,
                         hoehe - 21,80,20, AlignCenter,text);

      time += time_plus;
    }

  while(time_small / (int)secWidth < breite - 2*KOORD_DISTANCE)
    {
      // "kleine" Striche (min)
      painter->setPen(QPen(QColor(0,0,0), 1));
      painter->drawLine(X_ABSTAND + (time_small / secWidth)
                ,
             hoehe - Y_ABSTAND,
             X_ABSTAND + (time_small / secWidth) ,
             hoehe - Y_ABSTAND + 5);

      time_small += time_small_plus;
    }



  // Y Achsen
  painterText.setPen(QPen(QColor(0,0,0), 1));
  painterText.drawLine(KOORD_DISTANCE,hoehe - Y_ABSTAND,
                       KOORD_DISTANCE, Y_ABSTAND);


  if(!vario && !speed && baro)
    {
      // Barogramm
      int dh = 100;
      if(scale_h > 10)     dh = 500;
      else if(scale_h > 8) dh = 250;
      else if(scale_h > 3) dh = 200;

      int h = dh;
      painterText.setFont(QFont("helvetica",10));

      while(h / scale_h < hoehe - (Y_ABSTAND * 2))
        {
          painterText.setPen(QPen(QColor(100,100,255), 1));
          text.sprintf("%d m",h);
          painterText.drawText(0,hoehe - (int)( h / scale_h ) - Y_ABSTAND - 10,
                           KOORD_DISTANCE - 3,20,Qt::AlignRight | Qt::AlignVCenter,text);


          if(h == 1000 || h == 2000 || h == 3000)
              painter->setPen(QPen(QColor(200,200,255), 2));
          else
              painter->setPen(QPen(QColor(200,200,255), 1));

          painter->drawLine(KOORD_DISTANCE - 3,
                      hoehe - (int)( h / scale_h ) - Y_ABSTAND,
                      breite - 20,hoehe - (int)( h / scale_h ) - Y_ABSTAND);

          h += dh;
        }
    }
  else if(!speed && !baro && vario)
    {
      // Variogramm
      painter->setPen(QPen(QColor(255,100,100), 1));
      painter->drawLine(KOORD_DISTANCE, (hoehe / 2), breite - 20, (hoehe / 2));

      float dva = 2.0;
      if(scale_va > 0.15)      dva = 5.0;
      else if(scale_va > 0.1) dva = 3.5;
      else if(scale_va > 0.08) dva = 2.5;

      float va = 0;
      painterText.setFont(QFont("helvetica",8));

      while(va / scale_va < (hoehe / 2) - Y_ABSTAND)
        {
          text.sprintf("%.1f m/s",va);
          painterText.setPen(QPen(QColor(255,100,100), 1));
          painterText.drawText(0,(hoehe / 2) - (int)( va / scale_va ) - 10,
                           KOORD_DISTANCE - 3,20,Qt::AlignRight | Qt::AlignVCenter,text);

          painter->setPen(QPen(QColor(255,200,200), 1));
          painter->drawLine(KOORD_DISTANCE - 3,(hoehe / 2) - (int)( va / scale_va ),
                   breite - 20,(hoehe / 2) - (int)( va / scale_va ));

          if(va != 0)
            {
              text.sprintf("-%.1f m/s",va);
              painterText.setPen(QPen(QColor(255,100,100), 1));
              painterText.drawText(0,(hoehe / 2) + (int)( va / scale_va ) -20,
                         KOORD_DISTANCE - 3,20,Qt::AlignRight | Qt::AlignVCenter,text);

              painter->setPen(QPen(QColor(255,200,200), 1));
              painter->drawLine(KOORD_DISTANCE,(hoehe / 2) + (int)( va / scale_va ) - 3,
                      breite - 20,(hoehe / 2) + (int)( va / scale_va ));

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

      int v = dv;
      painterText.setFont(QFont("helvetica",8));

      while(v / scale_v < hoehe - (Y_ABSTAND * 2))
        {
          text.sprintf("%d km/h",v);
          painterText.setPen(QPen(QColor(0,0,0), 1));
          painterText.drawText(0,hoehe - (int)( v / scale_v ) - Y_ABSTAND - 10,
                     KOORD_DISTANCE - 3,20,Qt::AlignRight | Qt::AlignVCenter,text);

          painter->setPen(QPen(QColor(200,200,200), 1));
          painter->drawLine(KOORD_DISTANCE - 3,
                      hoehe - (int)( v / scale_v ) - Y_ABSTAND,
                      breite - 20,hoehe - (int)( v / scale_v ) - Y_ABSTAND);

          v += dv;
        }
    }

    painterText.end();

}


void EvaluationView::drawCurve(Flight* current, bool arg_vario, bool arg_speed,
            bool arg_baro, unsigned int arg_glatt_va, unsigned int arg_glatt_v,
            unsigned int arg_glatt_h, unsigned int secW)
{
//  warning("drawCurve");
  isFlight = true;

  setMouseTracking(true);

  vario = arg_vario;
  speed = arg_speed;
  baro = arg_baro;
  glatt_va = arg_glatt_va;
  glatt_v = arg_glatt_v;
  glatt_h = arg_glatt_h;

  flight = current;
  startTime = flight->getStartTime();
  landTime = flight->getLandTime();

  cursor1 = MAX(startTime,cursor1);
  cursor2 = MAX(startTime,cursor2);
  cursor1 = MIN(landTime,cursor1);
  cursor2 = MIN(landTime,cursor2);

  secWidth = secW;

  this->resize((landTime - startTime) / secWidth
      + (KOORD_DISTANCE * 2)  ,
      scrollFrame->viewport()->height());

  pixBuffer->resize((landTime - startTime) / secWidth + (KOORD_DISTANCE * 2),
      scrollFrame->viewport()->height());
  pixBufferKurve->resize((landTime - startTime) / secWidth
       + (KOORD_DISTANCE * 2) ,
      scrollFrame->viewport()->height());
  pixBufferYAxis->resize(KOORD_DISTANCE + 1,
      scrollFrame->viewport()->height());


  scrollFrame->addChild(this);

  __draw();

  paintEvent(0);
}

void EvaluationView::__draw()
{
  // Skalierungsfaktor -- vertical
  scale_v = getSpeed(flight->getPoint(Flight::V_MAX)) /
          ((double)(this->height() - 2*Y_ABSTAND));
  scale_h = flight->getPoint(Flight::H_MAX).height /
          ((double)(this->height() - 2*Y_ABSTAND));
  scale_va = MAX(getVario(flight->getPoint(Flight::VA_MAX)),
              ( -1.0 * getVario(flight->getPoint(Flight::VA_MIN))) ) /
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
      baro_d[loop] = flight->getPoint(loop).height;
      baro_d_last[loop] = flight->getPoint(flight->getRouteLength() - loop - 1).height;
    }
  for(unsigned int loop = 0; loop < gn_v; loop++)
    {
      speed_d[loop] = getSpeed(flight->getPoint(loop));
      speed_d_last[loop] = getSpeed(flight->getPoint(flight->getRouteLength() - loop - 1));
    }
  for(unsigned int loop = 0; loop < gn_va; loop++)
    {
       vario_d[loop] = getVario(flight->getPoint(loop));
      vario_d_last[loop] =
          getVario(flight->getPoint(flight->getRouteLength() - loop - 1));
    }

  QPointArray baroArray(flight->getRouteLength());
  QPointArray varioArray(flight->getRouteLength());
  QPointArray speedArray(flight->getRouteLength());

  for(unsigned int loop = 0; loop < flight->getRouteLength(); loop++)
    {
      curTime = flight->getPoint(loop).time;

      /* Der Array wird hier noch falsch gefüllt. Wenn über 3 Punkte geglättet wird, stimmt
       * alles. Wenn jedoch z.B. über 5 Punkte geglättet wird, werden die Punkte
       * ( -4, -3, -2, -1, 0, 1) genommen, statt (-2, -1, 0, 1, 2). Das ist vermutlich
       * die Ursache dafür, dass die Kurve "wandert".
       */
      if(loop < flight->getRouteLength() - glatt_h && loop > glatt_h)
          baro_d[(loop - glatt_h - 1) % gn_h] = flight->getPoint(loop + glatt_h).height;

      if(loop < flight->getRouteLength() - glatt_v && loop > glatt_v)
          speed_d[(loop - glatt_v - 1) % gn_v] = getSpeed(flight->getPoint(loop + glatt_v));

      if(loop < flight->getRouteLength() - glatt_va && loop > glatt_va)
          vario_d[(loop - glatt_va - 1) % gn_va] = getVario(flight->getPoint(loop + glatt_va));

      /* Wenn das Glätten wie bei __speedPoint() erfolgt, können gn_? und loop auch als
       * unsigned übergeben werden ...
       */
      if(loop < flight->getRouteLength() - glatt_h)
          baroArray.setPoint(loop,
              __baroPoint(flight->getPoint(loop).height, baro_d, gn_h, loop));
      else
          baroArray.setPoint(loop,
              __baroPoint(flight->getPoint(loop).height, baro_d_last, gn_h,
                        flight->getRouteLength() - loop - 1));
      if(loop < flight->getRouteLength() - glatt_va)
          varioArray.setPoint(loop,
              __varioPoint(getVario(flight->getPoint(loop)), vario_d, gn_va, loop));
      else
          varioArray.setPoint(loop,
              __varioPoint(getVario(flight->getPoint(loop)), vario_d_last, gn_va,
                        flight->getRouteLength() - loop - 1));

      if(loop < flight->getRouteLength() - glatt_v)
          speedArray.setPoint(loop,
              __speedPoint(getSpeed(flight->getPoint(loop)), speed_d, gn_v, loop));
      else
          speedArray.setPoint(loop,
              __speedPoint(getSpeed(flight->getPoint(loop)), speed_d_last, gn_v,
                        flight->getRouteLength() - loop - 1));
    }



  pixBufferKurve->fill(white);
  QPainter paint(pixBufferKurve);

  __drawCsystem(&paint);

  int xpos = 0;

  // Wendepunkte
  QList<struct wayPoint>* wP;
  QString timeText = 0;

  wP = flight->getWPList();
  for(unsigned int n = 1; n < wP->count() - 1; n++)
    {
      xpos = (wP->at(n)->sector1 - startTime ) / secWidth
          + X_ABSTAND  ;

      paint.setPen(QPen(QColor(100,100,100), 3));
      paint.drawLine(xpos, this->height() - Y_ABSTAND, xpos, Y_ABSTAND + 5);
      paint.setPen(QPen(QColor(0,0,0), 3));
      paint.setFont(QFont("helvetica",8));
      paint.drawText (xpos - 40, Y_ABSTAND - 20 - 5,80,10, AlignCenter,
             wP->at(n)->name);
//      paint.drawText(xpos - 25, Y_ABSTAND - 5, wP->at(n)->name);
      if(wP->at(n)->sector1 != 0)
          timeText = printTime(wP->at(n)->sector1);
      else if(wP->at(n)->sector2 != 0)
          timeText = printTime(wP->at(n)->sector2);
      else if(wP->at(n)->sectorFAI != 0)
          timeText = printTime(wP->at(n)->sectorFAI);
      paint.setFont(QFont("helvetica",7));
      paint.drawText (xpos - 40, Y_ABSTAND - 10 - 5,80,10,
                          AlignCenter,timeText);
    }



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

   __paintCursor(( cursor1 - startTime ) / secWidth
        + X_ABSTAND ,-2000,0,1);
   __paintCursor(( cursor2 - startTime ) / secWidth
        + X_ABSTAND ,-2000,0,2);

}

void EvaluationView::__paintCursor(int xpos, int calt, int move, int cursor)
{
  // Bildschirmkoordinaten !!
  QPainter paint;

  //
  //  Bislang werden die Cursor durch Rasteroperationen gelöscht.
  //  Das klappt aber mit den Icons nicht, daher sollte ein
  //  Puffer-Speicher eingeführt werden, der dann wieder zurückkopiert
  //  wird.
  //

  if(move == 1)
    {
      paint.begin(this);
      if(cursor == 1)
          paint.setPen(QPen(QColor(0,200,0), 1));
      else
          paint.setPen(QPen(QColor(200,0,0), 1));

      paint.setRasterOp(XorROP);
      paint.drawLine(calt,this->height() - Y_ABSTAND, calt,Y_ABSTAND);
      paint.drawLine(xpos,this->height() - Y_ABSTAND, xpos,Y_ABSTAND);
      paint.end();
    }
   else
    {
      paint.begin(pixBufferKurve);

      if(cursor == 1)
        {
          paint.setPen(QPen(QColor(0,200,0), 1));
          paint.setBrush(QBrush(QColor(0,200,0), SolidPattern));
        }
      else
        {
          paint.setPen(QPen(QColor(200,0,0), 1));
          paint.setBrush(QBrush(QColor(200,0,0), SolidPattern));
        }

    QPixmap pixCursor1 = QPixmap(KGlobal::dirs()->findResource("appdata",
      "pics/flag_green.png"));
    QPixmap pixCursor2 = QPixmap(KGlobal::dirs()->findResource("appdata",
      "pics/flag_red.png"));


      // alte Linie übermalen
//      paint.drawLine(calt, this->height() - Y_ABSTAND, calt, Y_ABSTAND);

      // neue Linie malen
      paint.drawLine(xpos, this->height() - Y_ABSTAND, xpos, Y_ABSTAND);

      // Flaggen malen
      if(cursor == 1)
          paint.drawPixmap(xpos - 32, Y_ABSTAND - 30,pixCursor1);
      else
          paint.drawPixmap(xpos, Y_ABSTAND - 30,pixCursor2);

      paint.end();
    }
}
