/***********************************************************************
**
**   map.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>
#include <Qt3Support>

#include "airfield.h"
#include "airspace.h"
#include "flight.h"
#include "flightgroup.h"
#include "map.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "radiopoint.h"
#include "resource.h"
#include "singlepoint.h"
#include "translationlist.h"
#include "waypoints.h"
#include "waypointdialog.h"
#include "wgspoint.h"
#include "whatsthat.h"
#include "mainwindow.h"

// These values control the borders at which to pan the map
// NOTE: These values are only for testing, and need revision.
#define MIN_X_TO_PAN 30
#define MAX_X_TO_PAN QApplication::desktop()->width()-30
#define MIN_Y_TO_PAN 30
#define MAX_Y_TO_PAN QApplication::desktop()->height()-30
#define MAP_INFO_DELAY 1000

/** External references */
extern MainWindow   *_mainWindow;
extern MapConfig    *_globalMapConfig;
extern MapContents  *_globalMapContents;
extern MapMatrix    *_globalMapMatrix;
extern QSettings    _settings;

Map::Map( QWidget* parent ) :
  QFrame(parent),
  prePos(-50, -50),
  preCur1(-50, -50),
  preCur2(-50, -50),
  planning(-1),
  tempTask(""),
  isDrawing(false),
  redrawRequest(false),
  preSnapPoint(-999, -999)
{
  QBitmap bitCursorMask;
  bitCursorMask.resize(40,40);
  bitCursorMask.fill(Qt::color0);
  pixCursor.resize(40,40);
  pixCursor.fill(Qt::transparent);

  QPainter cursor(&pixCursor);
  cursor.setPen(QPen(QColor(255,0,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  cursor.begin(&bitCursorMask);
  cursor.setPen(QPen(Qt::color1, 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(Qt::color1, 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  pixCursor.setMask(bitCursorMask);

  pixCursor1 = _mainWindow->getPixmap("flag_green.png");
  pixCursor2 = _mainWindow->getPixmap("flag_red.png");

  pixCursorBuffer1.resize(32,32);
  pixCursorBuffer1.fill(Qt::white);

  pixCursorBuffer2.resize(32,32);
  pixCursorBuffer2.fill(Qt::white);

//  pixAnimate.resize(32,32);
//  pixAnimate.fill(white);

  __setCursor();
  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setAcceptDrops(true);

  // For Planning
  pixPlan.fill(Qt::white);
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);

  setWhatsThis( QObject::tr(
     "<html><B>The map</B>"
     "<P>To move or scale the map, please use the buttons in the "
     "<B>Map control area</B>. Or center the map to the current "
     "cursor position by using the right mouse button.</P>"
     "<P>To zoom in or out, use the slider or the two buttons on the "
     "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" or \"+\" (zoom in) "
     "and \"&lt;Ctrl&gt;&lt;-&gt;\" or \"-\"(zoom out).</P>"
     "<P>The cursor keys and the keys on the NumPad can also pan the map, if "
     "NumLock is switched on.</P>"
     "<P>With the menu item <b>Options-&gt;Configure KFLog</b> you can "
     "configure, which map elements should be displayed at which "
     "scale.</P></html>"
      ) );

  __createPopupMenu();

  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), this,
           SLOT(slotAnimateFlightTimeout()) );

  mapInfoTimer = new QTimer(this);
  connect (mapInfoTimer, SIGNAL(timeout()), this, SLOT(slotMapInfoTimeout()));

  /** Create a timer for queuing draw events. */
  redrawMapTimer = new QTimer(this);
  redrawMapTimer->setSingleShot(true);

  connect( redrawMapTimer, SIGNAL(timeout()),
           this, SLOT(slotRedrawMap()));

  isZoomRect=false;
  dragZoomRect=false;
}

Map::~Map()
{
  qDebug() << "~Map()";
}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern MapContents *_globalMapContents;
  const QPoint current = event->pos();
  Waypoint *w;

  QPoint vector = event->pos() - mapInfoTimerStartpoint;

  if(vector.manhattanLength() > 4)
  {
    mapInfoTimer->stop();
    // don't start the timer when in planning mode
    if ((planning != 1) &&
        (planning != 3))
    {
      mapInfoTimer->start(MAP_INFO_DELAY, true);
      mapInfoTimerStartpoint = event->pos();
    }
  }

  if(planning == 1 || planning == 3)
    {
      double delta(8.0);

      if(_globalMapMatrix->isSwitchScale()) delta = 16.0;

      // If we are near to the last temp-point of the task,
      // and SHIFT is not pressed, we can exit ...
      if( (abs(current.x() - preSnapPoint.x()) <= delta) &&
          (abs(current.y() - preSnapPoint.y()) <= delta) &&
          event->state() != Qt::ShiftModifier )  return;

      BaseFlightElement *f = _globalMapContents->getFlight();

      if(!f)  return;

      QList<Waypoint*> taskPointList = f->getWPList();
      QList<Waypoint*> tempTaskPointList = f->getWPList();
      QList<Waypoint*> *wpList = _globalMapContents->getWaypointList();

      // 3: Task beendet verschieben eines Punktes

      QPoint preSitePos, nextSitePos;
      QPoint point(current);

      if (!taskPointList.isEmpty())
        {
          if(planning == 1)
              preSitePos = _globalMapMatrix->map(taskPointList.last()->projP);
          else if(planning == 3)
            {
              if(moveWPindex > 0)
                  preSitePos = _globalMapMatrix->map(taskPointList.at(moveWPindex - 1)->projP);

              if(moveWPindex + 1 < (int)taskPointList.count())
                  nextSitePos = _globalMapMatrix->map(taskPointList.at(moveWPindex + 1)->projP);
            }

          // If we are near to the last point of the current task,
          // we can exit ...
          if( (abs(current.x() - preSitePos.x()) <= delta) &&
              (abs(current.y() - preSitePos.y()) <= delta) )
            {
              return;
            }

          if(isSnapping)
            {
              // Snapping Bereich verlassen?
              if( (abs(prePlanPos.x() - current.x()) > delta) ||
                  (abs(prePlanPos.y() - current.y()) > delta) )
                {
                  isSnapping = false;

                  // delete temp WP
                  __drawPlannedTask();
                  __showLayer();
                }
            }

          if(!isSnapping)
            {
              Waypoint wp;
              bool found = __getTaskWaypoint(current, &wp, taskPointList);
              if (!found) // check wp catalog
              found = __getTaskWaypoint(current, &wp, *wpList);
              if(found)
//              if(__getTaskWaypoint(current, &wp, taskPointList))
                {
                  isSnapping = true;

                  point = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(wp.origP));

                  // Eigentlich k�nnte man dann auf point verzichten?
                  preSnapPoint = point;
                }
              else
                {
                  preSnapPoint.setX(-999);
                  preSnapPoint.setY(-999);
                }

              if(isSnapping)
                {
                  // add temp WP
                  if(planning == 3)
                    {
                      //verschieben
                      tempTaskPointList.insert(moveWPindex,new Waypoint);
                      w = tempTaskPointList.at(moveWPindex);
                    }
                  else
                    {
                      //anh�ngen
                      tempTaskPointList.append(new Waypoint);
                      w = tempTaskPointList.last();
                    }

                   w->name = wp.name;
                   w->origP = wp.origP;
                   w->elevation = wp.elevation;
                   w->projP = wp.projP;
                   w->description = wp.description;
                   w->type = wp.type;
                   w->elevation = wp.elevation;
                   w->icao = wp.icao;
                   w->frequency = wp.frequency;
                   w->runway = wp.runway;
                   w->length = wp.length;
                   w->surface = wp.surface;
                   w->comment = wp.comment;
                   w->isLandable = wp.isLandable;

                  tempTask.setWaypointList(tempTaskPointList);
                  __drawPlannedTask(false);

                  if((current.x() > 0 && current.x() < this->width()) &&
                     (current.y() > 0 && current.y() < this->height()))
                    {
                        prePlanPos = point;
                    }
                  else
                    {
                      // au�erhalb der Karte
                      prePlanPos.setX(-999);
                      prePlanPos.setY(-999);
                    }

                  qWarning() << "wp.name";
                }
            }

          if(!isSnapping)
            {
              if((current.x() > 0 && current.x() < this->width()) &&
                 (current.y() > 0 && current.y() < this->height()))
                {
                  // delete temp WP
//                  warning(" ---> 3");
//                  __drawPlannedTask();

                  if(event->state() == Qt::ShiftModifier)
                    {
//                      bitPlanMask.fill(Qt::color0);
//                      pixPlan.fill(white);
//                      __showLayer();
                      // tempor�rer Weg
                      if(planning == 3)
                        {
                          //verschieben
                          tempTaskPointList.insert(moveWPindex,new Waypoint);
                          tempTaskPointList.at(moveWPindex)->name = "";
                          QPoint tmp(_globalMapMatrix->mapToWgs(event->pos()));
                          tempTaskPointList.at(moveWPindex)->origP = WGSPoint(tmp.y(), tmp.x());
                          tempTaskPointList.at(moveWPindex)->projP =
                                _globalMapMatrix->wgsToMap(tempTaskPointList.at(moveWPindex)->origP);
                        }
                      else
                        {
                          //anh�ngen
                          tempTaskPointList.append(new Waypoint);
                          tempTaskPointList.last()->name = "test";
                          QPoint tmp(_globalMapMatrix->mapToWgs(event->pos()));
                          tempTaskPointList.last()->origP = WGSPoint(tmp.y(), tmp.x());
                          tempTaskPointList.last()->projP =
                                _globalMapMatrix->wgsToMap(tempTaskPointList.last()->origP);
                        }


                    tempTask.setWaypointList(tempTaskPointList);
//                    __drawPlannedTask(false);
                    emit showTaskText(&tempTask);

                  }
                prePlanPos = point;
              }
            else
              {
                // au�erhalb der Karte
                prePlanPos.setX(-999);
                prePlanPos.setY(-999);
              }
          }
      }

    //      emit showTaskText(task,_globalMapMatrix->mapToWgs(point));

    //
    // Planen ende
    //
    //////////////
  }

  if (!timerAnimate->isActive())
    {
      if(prePos.x() >= 0)
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
             prePos.x() - 20, prePos.y() - 20, 40, 40);
        }

      BaseFlightElement *f = _globalMapContents->getFlight();

      if(f)
        {
          flightPoint cP;
          int index;
          if ((index = f->searchPoint(event->pos(), cP)) != -1)
            {
              emit showFlightPoint(_globalMapMatrix->mapToWgs(event->pos()), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
          else
            {
              emit showPoint(_globalMapMatrix->mapToWgs(event->pos()));
            }
        }
      else
        {
          emit showPoint(_globalMapMatrix->mapToWgs(event->pos()));
          prePos.setX(-5000);
          prePos.setY(-5000);
        }

    }

    // Assume, that mouse position must be converted to map coordinates.
    __findElevation( _globalMapMatrix->invertToMap(current) );

    if (dragZoomRect){
        QPainter zoomPainter;
        zoomPainter.begin(this);
        zoomPainter.setCompositionMode(QPainter::CompositionMode_Xor);

        double width = event->pos().x() - beginDrag.x();
        double height = event->pos().y() - beginDrag.y();
        const double widthRatio = fabs(width / this->width());
        const double heightRatio = fabs(height / this->height());

        if(widthRatio > heightRatio)
          {
            height = this->height() * widthRatio;
            if(event->pos().y() < beginDrag.y()) {  height *= -1;  }  //make sure we keep the right sign
          }
        else
          {
            width = this->width() * heightRatio;
            if(event->pos().x() < beginDrag.x()) {  width *= -1;  }   //make sure we keep the right sign
          }


        zoomPainter.setPen(QPen(QColor(255, 255, 255), 1, Qt::DashLine));
        // Delete the old rectangle:
        zoomPainter.drawRect( beginDrag.x(), beginDrag.y(), sizeDrag.x(), sizeDrag.y());
        // Draw the new one:
        zoomPainter.drawRect( beginDrag.x(), beginDrag.y(), (int)width, (int)height);
        zoomPainter.end();

        //This is odd. Why use a QPoint when a QSize would be more appropriate?
        sizeDrag.setX((int)width);
        sizeDrag.setY((int)height);
    }

    //warning("dragzoomrect: %d, event->state: %d (leftbutton=%d)", dragZoomRect, event->state(), LeftButton);
    if ((!dragZoomRect) && (event->state() == Qt::LeftButton)){
      //start dragZoom
      setCursor(Qt::CrossCursor);
      isZoomRect = true;
      beginDrag = event->pos();
      sizeDrag = QPoint(0,0);
      dragZoomRect=true;
    }

}

/**
  * just a helper function
  * to keep the Waypoint class lightweight
  */
QString getInfoString (Waypoint* wp)
{
  extern TranslationList waypointTypes;

  QString path = _globalMapConfig->getIconPath();

  QString text = "<HTML><TABLE BORDER=0><TR>";

  // we don't have a pixmap for landmarks ?!
  if (wp->type != BaseMapElement::Landmark && !_globalMapConfig->getPixmapName(wp->type).isEmpty())
    text += QString ("<TD><IMG SRC= %1/%2></TD>").arg(path).arg(_globalMapConfig->getPixmapName(wp->type));
  else
    text += "<TD></TD>";

  text += "<TD>" + QObject::tr("Waypoint:") + " " + wp->name;
  if (!wp->icao.isEmpty())
    text += QString (" (%1)").arg(wp->icao);
  text += "</TD></TR>";

  if (wp->type >= 0)
    text += "<TR><TD></TD><TD>" + waypointTypes.itemById(wp->type)->text + "</TD></TR>";

  text += QString ("<TR><TD></TD><TD><FONT SIZE=-1> %1 m").arg(wp->elevation);

  if (wp->frequency > 0.0)
  {
    text += QString ("<BR>%1").arg (wp->frequency, 3);
  }
  text += "<BR>" + WGSPoint::printPos(wp->origP.lat());
  text += "<BR>" + WGSPoint::printPos(wp->origP.lon(), false);

  text += "</FONT></TD></TR></TABLE></HTML>";

  return text;
}

Waypoint* Map::findWaypoint (const QPoint& current)
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;
  Waypoint *wp;
  foreach(wp, *_globalMapContents->getWaypointList())
//  for (Q3PtrListIterator<Waypoint> it (*(_globalMapContents->getWaypointList())); it.current(); ++it)
  {
//    Waypoint* wp = it.current();
    QPoint sitePos (_globalMapMatrix->map(_globalMapMatrix->wgsToMap(wp->origP)));
    double dX = abs(sitePos.x() - current.x());
    double dY = abs(sitePos.y() - current.y());

    // Abstand entspricht der Icon-Gr��e.
    if ( (dX < 8.0) && (dY < 8.0) )
    {
      return wp;
    }
  }
  return 0;
}

void Map::__displayMapInfo(const QPoint& current, bool automatic)
{
  /*
   * Glider airfields first, if there exist any
   */
  extern MapContents *_globalMapContents;
  extern MapMatrix   *_globalMapMatrix;

  BaseFlightElement *baseFlight = _globalMapContents->getFlight();

  Airfield *hitElement;

  QPoint sitePos;
  // Radius for Mouse Snapping
  double delta(16.0);

  int timeout=60000;

  if (automatic)
    {
      timeout=20000;
    }

  QString text;

  bool show = false;

  for(int loop = 0; loop < _globalMapContents->getListLength(MapContents::GliderfieldList); loop++)
    {
      hitElement = (Airfield *) _globalMapContents->getElement(MapContents::GliderfieldList, loop);
      sitePos = hitElement->getMapPosition();

      double dX = abs (sitePos.x() - current.x());
      double dY = abs (sitePos.y() - current.y());

      // Abstand entspricht der Icon-Grösse.
      if ( ( dX < delta ) && ( dY < delta ) )
      {
        text += hitElement->getInfoString();
        // Text anzeigen
        WhatsThat* box = new WhatsThat(this, text, timeout, mapToGlobal(current));
        box->setVisible( true );
        return;
      }
    }

  for(int loop = 0; loop < _globalMapContents->getListLength(MapContents::AirfieldList); loop++)
    {
      hitElement = (Airfield *)_globalMapContents->getElement(MapContents::AirfieldList, loop);
      sitePos = hitElement->getMapPosition();

      double dX = abs (sitePos.x() - current.x());
      double dY = abs (sitePos.y() - current.y());

      // Abstand entspricht der Icon-Grösse.
      if ( ( dX < delta ) && ( dY < delta ) )
      {
        text += hitElement->getInfoString();
        // Text anzeigen
        WhatsThat* box = new WhatsThat(this, text, timeout, mapToGlobal(current));
        box->setVisible( true );
        return;
      }
    }

  for(int loop = 0; loop < _globalMapContents->getListLength(MapContents::OutLandingList); loop++)
    {
      hitElement = (Airfield *) _globalMapContents->getElement(MapContents::OutLandingList, loop);
      sitePos = hitElement->getMapPosition();

      double dX = abs (sitePos.x() - current.x());
      double dY = abs (sitePos.y() - current.y());

      // Abstand entspricht der Icon-Grösse.
      if ( ( dX < delta ) && ( dY < delta ) )
      {
        text += hitElement->getInfoString();
        // Text anzeigen
        WhatsThat* box = new WhatsThat(this, text, timeout, mapToGlobal(current));
        box->setVisible( true );
        return;
      }
    }

  if(baseFlight && baseFlight->getObjectType() == BaseMapElement::Flight)
    {
      QList<Waypoint*> wpList = baseFlight->getWPList();

      delta = 25;
      bool isWP = false;
      QString wpText;
      wpText = "<HTML><B>Waypoint:</B><UL>";

      for(int loop = 0; loop < wpList.count(); loop++)
        {
          sitePos = _globalMapMatrix->map(wpList.at(loop)->projP);

          double dX = abs (sitePos.x() - current.x());
          double dY = abs (sitePos.y() - current.y());

          // We do not search for the sector ...
          if ( ( dX < delta ) && ( dY < delta ) )
            {
              isWP = true;

              QString tmpText, timeText;

              if(wpList.at(loop)->sector1 != 0)
                {
                  timeText = printTime(wpList.at(loop)->sector1);
                  tmpText = QObject::tr("Sector 1");
                }
              else if(wpList.at(loop)->sector2 != 0)
                {
                  timeText = printTime(wpList.at(loop)->sector2);
                  tmpText = QObject::tr("Sector 2");
                }

              else if(wpList.at(loop)->sectorFAI != 0)
                {
                  timeText = printTime(wpList.at(loop)->sectorFAI);
                  tmpText = QObject::tr("FAI-Sector");
                }
              else
                {
                  timeText = "&nbsp;" + QObject::tr("not reached");
                }

              switch(wpList.at(loop)->type)
                {
                  case FlightTask::TakeOff:
                    tmpText = QObject::tr("Take Off");
                    break;
                  case FlightTask::Begin:
                    tmpText = QObject::tr("Begin of task");
                    break;
                  case FlightTask::End:
                    tmpText = QObject::tr("End of task");
                    break;
                  case FlightTask::Landing:
                    tmpText = QObject::tr("Landing");
                    break;
                }

              wpText = wpText + "<LI><B>" + wpList.at(loop)->name +
                  "</B>  " +
                  "&nbsp;" + timeText + " / " + tmpText + "<BR>" +
                  WGSPoint::printPos(wpList.at(loop)->origP.lat()) + " / " +
                  WGSPoint::printPos(wpList.at(loop)->origP.lon(), false) + "</LI>";
            }
        }

      if(isWP)
        {
          wpText += "</UL></HTML>";
          // Show text
          WhatsThat* box = new WhatsThat(this, wpText, timeout, mapToGlobal(current));
          box->setVisible( true );
          return;
        }
    }

  // let's show waypoints
  Waypoint* wp = findWaypoint (current);

  if( wp )
  {
    QString wpInfo = getInfoString(wp);

    WhatsThat* box = new WhatsThat(this, wpInfo, timeout, mapToGlobal(current));
    box->setVisible( true );
    return;
  }

  // At last serach for airspaces.
  text += "<html><table border=1><tr><th align=left>" +
          tr("Airspace&nbsp;Structure") +
          "</th></tr>";

  QList<QPair<QPainterPath, Airspace *> >& airspaceRegionList =
                                    _globalMapContents->getAirspaceRegionList();

  for( int loop = 0; loop < airspaceRegionList.count(); loop++)
    {
      QPair<QPainterPath, Airspace*> pair = airspaceRegionList.at(loop);

      if( pair.first.contains(current) )
        {
          text += "<tr><td align=left>" + pair.second->getInfoString() + "</td></tr>";
          show = true;
        }
    }

  text += "</table></html>";

  if(show)
    {
      // Show text
      WhatsThat* box = new WhatsThat(this, text, timeout, mapToGlobal(current));
      box->show();
    }
}

void Map::__graphicalPlanning(const QPoint& current, QMouseEvent* event)
{
  extern MapContents *_globalMapContents;

  BaseFlightElement *baseFlight = _globalMapContents->getFlight();
  if(baseFlight == NULL)  return;

  QList<Waypoint*> taskPointList = baseFlight->getWPList();
  QList<Waypoint*> tempTaskPointList = baseFlight->getWPList();
  QList<Waypoint*> * wpList = _globalMapContents->getWaypointList();
  Waypoint wp, *w;
  QString text;
  bool found;

  // is the point already in the flight?
  found  = __getTaskWaypoint(current, &wp, taskPointList);

  if (!found)         // try the wpcatalog
        found = __getTaskWaypoint(current, &wp, *wpList);

  if(!taskPointList.isEmpty() && event->state() == Qt::ControlModifier)
    {
      // gleicher Punkt --> löschen
      for(unsigned int n = taskPointList.count() - 1; n > 0; n--)
        {
          if(wp.projP == taskPointList.at(n)->projP)
            {
//              warning("lösche Punkt %d", n);
              taskPointList.removeAt(n);
            }
        }
     }

  if(event->button() == Qt::RightButton && event->state() == Qt::ControlModifier)
    {
      moveWPindex = -999;

      prePlanPos.setX(-999);
      prePlanPos.setY(-999);
      planning = 2;

      emit taskPlanningEnd();
      return;
    }
  else
    {
      if (found)
        {
          if(planning == 1)
            {
              // neuen Punkt an Task Liste anhängen
//              warning("hänge Punkt an");

              taskPointList.append(new Waypoint);
              w = taskPointList.last();

              w->name = wp.name;
              w->origP = wp.origP;
              w->elevation = wp.elevation;
              w->projP = wp.projP;
              w->description = wp.description;
              w->type = wp.type;
              w->elevation = wp.elevation;
              w->icao = wp.icao;
              w->frequency = wp.frequency;
              w->runway = wp.runway;
              w->length = wp.length;
              w->surface = wp.surface;
              w->comment = wp.comment;
              w->isLandable = wp.isLandable;
            }
          else if(planning == 2)
            {
              // Punkt wird verschoben
              for(int n = 0; n < taskPointList.count(); n++)
                {
                  if (wp.projP == taskPointList.at(n)->projP)
                    {
                      qWarning("verschiebe Punkt %d", n);
                      taskPointList.removeAt(n);
                      ((FlightTask *)baseFlight)->setWaypointList(taskPointList);
                      planning = 3;
                      moveWPindex = n;
                      prePlanPos.setX(-999);
                      prePlanPos.setY(-999);
                      break;
                    }
                }
            }
          else if(planning == 3)
            {
              planning = 2;
//              warning("insert - verschiebe Punkt?");
              taskPointList.insert(moveWPindex,new Waypoint);
              w = taskPointList.at(moveWPindex);

              w->name = wp.name;
              w->origP = wp.origP;
              w->elevation = wp.elevation;
              w->projP = wp.projP;
              w->description = wp.description;
              w->type = wp.type;
              w->elevation = wp.elevation;
              w->icao = wp.icao;
              w->frequency = wp.frequency;
              w->runway = wp.runway;
              w->length = wp.length;
              w->surface = wp.surface;
              w->comment = wp.comment;
              w->isLandable = wp.isLandable;
            } //planning == 3
        } // found

/*
      else if(planning != 2 && event->state() == QEvent::ShiftButton)
        {
          warning("No WP found !!!");
          // nothing found, try to create a free waypoint
          // only when shift is pressed!!!

          WaypointDialog *waypointDlg = new WaypointDialog(this);
          QPoint p = _globalMapMatrix->mapToWgs(current);

          // initialize dialg
          waypointDlg->setWaypointType(BaseMapElement::Landmark);
          waypointDlg->longitude->setText(WGSPoint::printPos(p.x(), false));
          waypointDlg->latitude->setText(WGSPoint::printPos(p.y(), true));
          waypointDlg->setSurface(-1);

          if (waypointDlg->exec() == QDialog::Accepted)
            {
              if (waypointDlg->name->text().isEmpty())
                {
                   // Perhaps we should add this feature to the Waypoint Dialog
                   w->name = QObject::tr("New WAYPOINT");
                }
              else
                {
                   w->name = waypointDlg->name->text().left(6).upper();
                }

              if(planning == 3)
                {
                  // insert a moved point
                  taskPointList.insert(moveWPindex,new wayPoint);
                  w = taskPointList.at(moveWPindex);
                  planning = 2;
                }
              else
                {
                  // append a new point
                  taskPointList.append(new wayPoint);
                  w = taskPointList.last();
                }

              w->description = waypointDlg->description->text();
              w->type = waypointDlg->getWaypointType();
              w->origP.setLat(WGSPoint::degreeToNum(waypointDlg->latitude->text()));
              w->origP.setLon(WGSPoint::degreeToNum(waypointDlg->longitude->text()));
              w->projP = _globalMapMatrix->wgsToMap(w->origP.lat(), w->origP.lon());
              w->elevation = waypointDlg->elevation->text().toInt();
              w->icao = waypointDlg->icao->text().upper();
              w->frequency = waypointDlg->frequency->text().toDouble();
              w->runway = waypointDlg->runway->text().toInt();
              w->length = waypointDlg->length->text().toInt();
              w->surface = waypointDlg->getSurface();
              w->comment = waypointDlg->comment->text();
              w->isLandable = waypointDlg->isLandable->isChecked();
              w->sector1 = 0;
              w->sector2 = 0;
              w->sectorFAI = 0;
            }
          delete waypointDlg;
        }
      */
    } // left button

  // Aufgabe zeichnen
  if(taskPointList.count() > 0)
    {
//      warning("zeichen");
      pixPlan.fill(Qt::white);
      ((FlightTask *)baseFlight)->setWaypointList(taskPointList);
      __drawPlannedTask();
      __showLayer();
    }

  if(planning == 2)
    {
      moveWPindex = -999;

      prePlanPos.setX(-999);
      prePlanPos.setY(-999);

      emit taskPlanningEnd();
    }
}

void Map::keyReleaseEvent(QKeyEvent* event)
{
qWarning("key Release");

  if(event->state() == Qt::ShiftModifier)
    {
      qWarning("key Release");
      __showLayer();
    }
}

void Map::mouseReleaseEvent(QMouseEvent* event)
{
  extern MapMatrix *_globalMapMatrix;
  if (isZoomRect)
    {
      dragZoomRect=false;
      isZoomRect=false;
      __setCursor();

      if(abs(beginDrag.x()-event->pos().x())>10 && // don't zoom if rect is too small
            abs(beginDrag.y()-event->pos().y())>10 && event->button()==Qt::LeftButton) // or other than left button was pressed
        {
          double width = event->pos().x() - beginDrag.x();
          double height = event->pos().y() - beginDrag.y();
          const double widthRatio = fabs(width / this->width());
          const double heightRatio = fabs(height / this->height());

          if(widthRatio > heightRatio)
            {
              height = this->height() * widthRatio;
              if(event->pos().y() < beginDrag.y()) {  height *= -1;  }
            }
          else
            {
              width = this->width() * heightRatio;
              if(event->pos().x() < beginDrag.x()) {  width *= -1;  }
            }


          if (width<0 && height<0) {  //work around for problem where mapmatrix calculates the wrong scale if both width and height are < 0
            _globalMapMatrix->centerToRect(QRect(QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height), beginDrag), QSize(0,0), false);
          } else {
            _globalMapMatrix->centerToRect(QRect(beginDrag, QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height)), QSize(0,0), false);
          }
          __redrawMap();
        } else {
          //we are not going to zoom, but we still need to clean up our mess!
          QPainter zoomPainter;
          zoomPainter.begin(this);
          zoomPainter.setCompositionMode(QPainter::CompositionMode_Xor);
          zoomPainter.setPen(QPen(QColor(255, 255, 255), 1, Qt::DashLine));
          // Delete the old rectangle:
          zoomPainter.drawRect( beginDrag.x(), beginDrag.y(), sizeDrag.x(), sizeDrag.y());
        }
    }
}

void Map::mousePressEvent(QMouseEvent* event)
{
  qDebug() << "Map::mousePressEvent";

  // First: delete the cursor, if visible:
  if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
          prePos.x() - 20, prePos.y() - 20, 40, 40);

  if( isZoomRect )
    { // Zooming
      beginDrag = event->pos();
      sizeDrag = QPoint( 0, 0 );
      dragZoomRect = true;
    }
  else
  {
    extern MapContents *_globalMapContents;
    extern MapMatrix *_globalMapMatrix;

    const QPoint current(event->pos());

    Airfield *hitElement;
    QString text;

    double dX, dY, delta(16.0);

    if(_globalMapMatrix->isSwitchScale()) delta = 8.0;

    if(event->button() == Qt::MidButton)
      {
        // Move Map
        _globalMapMatrix->centerToPoint(event->pos());
        __redrawMap();
      }

    else if(event->button() == Qt::LeftButton)
      {
        if(event->state() == Qt::ShiftModifier)
          {
            // select WayPoint
            QRegExp blank("[ ]");
            bool found = false;

            // add WPList !!!
            int searchList[] = { MapContents::GliderfieldList,
                                 MapContents::AirfieldList,
                                 MapContents::OutLandingList };

            for(int l = 0; l < 3; l++)
              {
               for(int loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++)
                 {
                  hitElement = (Airfield *)_globalMapContents->getElement(searchList[l], loop);
                  QPoint sitePos = hitElement->getMapPosition();

                  dX = abs(sitePos.x() - current.x());
                  dY = abs(sitePos.y() - current.y());

                  // Abstand entspricht der Icon-Grösse.
                  if (dX < delta && dY < delta)
                    {
                      qDebug() << "Found" << hitElement->getName();
                      Waypoint *w = new Waypoint;

                      QString name = hitElement->getName();
                      w->name = name.replace(blank, "").left(6).upper();
                      w->description = hitElement->getName();
                      w->type = hitElement->getObjectType();hitElement->getName();
                      w->origP = hitElement->getWGSPosition();
                      w->elevation = hitElement->getElevation();
                      w->icao = hitElement->getICAO();
                      w->frequency = hitElement->getFrequency().toDouble();
                      w->isLandable = true;

                      emit waypointSelected(w);
                      found = true;
                      break;
                  }
               }
               if(found)
                   break;
            }

            if(!found) {
                qWarning("new waypoint");

                WaypointDialog *waypointDlg = new WaypointDialog(this);
                emit regWaypointDialog(waypointDlg); //register the dialog and connect it's signals.

                waypointDlg->enableApplyButton(false);

                QPoint p = _globalMapMatrix->mapToWgs(current);

                // initialize dialog
                //waypointDlg->setWaypointType(BaseMapElement::Landmark); now set by default.
                waypointDlg->longitude->setText(WGSPoint::printPos(p.x(), false));
                waypointDlg->latitude->setText(WGSPoint::printPos(p.y(), true));
                waypointDlg->setSurface(-1);

                waypointDlg->exec(); //we only need to exec the dialog. The dialog can take care of itself now :-)

                delete waypointDlg;

            }
          }  else {
           // __displayMapInfo(current);
          }

        if(planning)  {
          __graphicalPlanning(current, event);
        }
    }


    else if(event->button() == Qt::RightButton && event->state() == Qt::ControlModifier)
      {
        moveWPindex = -999;

        prePlanPos.setX(-999);
        prePlanPos.setY(-999);
        planning = 2;

        emit taskPlanningEnd();
        return;
      }
    else if( event->button() == Qt::RightButton )
        {
          popupPos = event->pos();
          __showPopupMenu( event );
        }

  }
}

void Map::paintEvent( QPaintEvent* event )
{
  QPainter painter(this);

  painter.drawPixmap( event->rect().left(),
                      event->rect().top(),
                      pixBuffer,
                      0, 0, event->rect().width(),
                      event->rect().height() );

  // Set cursor position back to its original position
  prePos.setX(-50);
  prePos.setY(-50);
}

void Map::__drawGrid()
{
  const QRect mapBorder = _globalMapMatrix->getViewBorder();

  QPainter gridP;

  gridP.begin(&pixGrid);
  gridP.setBrush(Qt::NoBrush);
  gridP.setClipping(true);

  // die Kanten des Bereichs
  const int lon1 = mapBorder.left() / 600000 - 1;
  const int lon2 = mapBorder.right() / 600000 + 1;
  const int lat1 = mapBorder.top() / 600000 + 1;
  const int lat2 = mapBorder.bottom() / 600000 - 1;

  // Step between two degree-lines (in 1/60 degree)
  int step = 60;
  int gridStep = 1;
  int lineWidth = 1;

  switch( _globalMapMatrix->getScaleRange() )
    {
    case 0:
      step = 10;
      break;
    case 1:
      step = 30;
      break;
    case 2:
      gridStep = 2;
      break;
    default:
      gridStep = 4;
    }

  QPoint cP, cP2;

  // First the latitudes:
  for( int loop = 0; loop < (lat1 - lat2 + 1); loop += gridStep )
    {
      int size = (lon2 - lon1 + 1) * 10;

      QPolygon pointArray( size );

      for( int lonloop = 0; lonloop < size; lonloop++ )
        {
          cP = _globalMapMatrix->wgsToMap( (lat2 + loop) * 600000,
                                           (int) rint((lon1 + (lonloop * 0.1)) * 600000) );
          pointArray.setPoint( lonloop, cP );
        }

      // Draw the small lines between:
      int number = (int) (60.0 / step);

      for( int loop2 = 1; loop2 < number; loop2++ )
        {
          QPolygon pointArraySmall( size );

          for( int lonloop = 0; lonloop < size; lonloop++ )
            {
              cP = _globalMapMatrix->wgsToMap( (int) rint((lat2 + loop + (loop2 * (step / 60.0))) * 600000),
                                               (int) rint((lon1 + (lonloop * 0.1)) * 600000) );
              pointArraySmall.setPoint( lonloop, cP );
            }

          if( loop2 == (number / 2.0) )
            {
              gridP.setPen( QPen( QColor( 0, 0, 0 ), 1, Qt::DashLine ) );
              gridP.drawPolyline( _globalMapMatrix->map( pointArraySmall ) );
            }
          else
            {
              gridP.setPen( QPen( QColor( 0, 0, 0 ), 1, Qt::DotLine ) );
              gridP.drawPolyline( _globalMapMatrix->map( pointArraySmall ) );
            }
        }
      // Draw the main lines
      gridP.setPen( QPen( QColor( 0, 0, 0 ), 1 ) );
      gridP.drawPolyline( _globalMapMatrix->map( pointArray ) );
    }

  // Now the longitudes:
  for( int loop = lon1; loop <= lon2; loop += gridStep )
    {
      cP  = _globalMapMatrix->wgsToMap( lat1 * 600000, (loop * 600000) );
      cP2 = _globalMapMatrix->wgsToMap( lat2 * 600000, (loop * 600000) );

      // Draw the main longitudes:
      gridP.setPen( QPen( QColor( 0, 0, 0 ), 1 ) );
      gridP.drawLine( _globalMapMatrix->map( cP ), _globalMapMatrix->map( cP2 ) );

      // Draw the small lines between:
      int number = (int) (60.0 / step);

      for( int loop2 = 1; loop2 < number; loop2++ )
        {
          cP = _globalMapMatrix->wgsToMap( (lat1 * 600000),
                                           (int) rint((loop + (loop2 * step / 60.0)) * 600000) );

          cP2 = _globalMapMatrix->wgsToMap( (lat2 * 600000),
                                            (int) rint((loop + (loop2 * step / 60.0)) * 600000) );

          if( loop2 == (number / 2.0) )
            {
              gridP.setPen( QPen( QColor( 0, 0, 0 ), 1, Qt::DashLine ) );
            }
          else
            {
              gridP.setPen( QPen( QColor( 0, 0, 0 ), lineWidth, Qt::DotLine ) );
            }

          gridP.drawLine( _globalMapMatrix->map( cP ), _globalMapMatrix->map( cP2 ) );
        }
    }

  gridP.end();

  __drawScale( pixGrid );
}

void Map::__drawMap()
{
  qDebug() << "Map::__drawMap()";

  QPainter aeroP(&pixAero);
  QPainter uMapP(&pixUnderMap);
  QPainter isoMapP(&pixIsoMap);

  extern MapContents *_globalMapContents;

  // Take the color of the subterrain for filling
  pixIsoMap.fill( _globalMapConfig->getIsoColor(0) );

  _globalMapContents->drawIsoList( &isoMapP, rect() );

  emit setStatusBarProgress(10);

  _globalMapContents->drawList(&uMapP, 0, MapContents::TopoList);

  _globalMapContents->drawList(&uMapP, 0, MapContents::CityList);

  _globalMapContents->drawList(&uMapP, 0, MapContents::HydroList);

  emit setStatusBarProgress(15);

  _globalMapContents->drawList(&uMapP, 0, MapContents::RoadList);

  _globalMapContents->drawList(&uMapP, 0, MapContents::HighwayList);

  emit setStatusBarProgress(25);

  _globalMapContents->drawList(&uMapP, 0, MapContents::RailList);

  emit setStatusBarProgress(35);

  _globalMapContents->drawList(&uMapP, 0, MapContents::VillageList);

  emit setStatusBarProgress(45);

  _globalMapContents->drawList(&uMapP, 0, MapContents::LandmarkList);

  emit setStatusBarProgress(50);

  _globalMapContents->drawList(&uMapP, 0, MapContents::ObstacleList);

  emit setStatusBarProgress(55);

  _globalMapContents->drawList(&aeroP, 0, MapContents::ReportList);

  emit setStatusBarProgress(60);

  _globalMapContents->drawList(&aeroP, 0, MapContents::NavList);

  emit setStatusBarProgress(65);

  __drawAirspaces();

  emit setStatusBarProgress(70);

  emit setStatusBarProgress(75);

  _globalMapContents->drawList(&aeroP, 0, MapContents::AirfieldList);

  emit setStatusBarProgress(80);

  _globalMapContents->drawList(&aeroP, 0, MapContents::AddSitesList);

  emit setStatusBarProgress(85);

  _globalMapContents->drawList(&aeroP, 0, MapContents::GliderfieldList);

  emit setStatusBarProgress(90);

  _globalMapContents->drawList(&aeroP, 0, MapContents::OutLandingList);

  emit setStatusBarProgress(95);

  __drawGrid();
}


void Map::__drawAirspaces()
{
  qDebug() << "Map::__drawAirspaces()";

  QPainter cuAeroMapP;
  cuAeroMapP.begin(&pixAirspace);

  QTime t;
  t.start();

  QList<QPair<QPainterPath, Airspace *> >& airspaceRegionList =
                                    _globalMapContents->getAirspaceRegionList();
  airspaceRegionList.clear();

  SortableAirspaceList& airspaceList = _globalMapContents->getAirspaceList();

  // Decide, if airspaces are filled with a color or not
  bool fillAirspace = _settings.value( "/Airspaces/Filling", true ).toBool();

  // Get airspace opacity 0.0 is full transparency, default is 10%
  qreal airspaceOpacity = _settings.value( "/Airspaces/Opacity", 10 ).toDouble();

  if( fillAirspace == false )
    {
      airspaceOpacity = 100.0; // no transparency
    }

  for( int i = 0; i < airspaceList.size(); i++ )
    {
      Airspace& as = airspaceList[i];

      // airspaces we don't draw, we don't warn for either (and vice versa)
#warning "Draw only airspaces, desired by the user. Must be implemented?"
#if 0
      if( !settings->getAirspaceDrawingEnabled( as.getTypeID() ) )
        {
          continue;
        }
#endif

      if( ! as.isDrawable() )
        {
          // Not of interest, step away
          continue;
        }

      QPair<QPainterPath, Airspace *> pair( as.createRegion(), &as );
      airspaceRegionList.append( pair );

      as.drawRegion( &cuAeroMapP, this->rect(), airspaceOpacity );
    }

  cuAeroMapP.end();
  qDebug("Airspace, drawTime=%d ms", t.elapsed());
}

void Map::__drawFlight()
{
  QPainter flightP(&pixFlight);
  QPainter flightMaskP(&bitFlightMask);

  extern MapContents *_globalMapContents;

  _globalMapContents->drawList(&flightP, &flightMaskP, MapContents::FlightList);

  flightP.end();
  flightMaskP.end();
}

void Map::__drawPlannedTask(bool solid)
{
//warning("Map::__drawPlannedTask()");
//  extern const MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;

  QPainter planP;
  FlightTask* task;

  if(solid)
      task = (FlightTask*)_globalMapContents->getFlight();
  else {
    task = &tempTask;
    task->reProject();
  }

  if(task && task->getObjectType() == BaseMapElement::Task)
    {
      QList<Waypoint*> WPList = task->getWPList();

      // Strecke zeichnen
      if(solid)
          planP.begin(&pixPlan);
      else
          planP.begin(this);

      QPainter planPMask(&bitPlanMask);
      bitPlanMask.fill(Qt::color0);
      bitFlightMask.fill(Qt::color0);
      pixPlan.fill(Qt::white);
      pixFlight.fill(Qt::white);

      QPen drawP(QColor(170,0,0), 5);
      drawP.setJoinStyle(Qt::MiterJoin);
      planP.setBrush(Qt::NoBrush);
      planP.setPen(drawP);

      // Aufgabe mit Sektoren
      task->drawMapElement(&planP,&planPMask);
      emit showTaskText(task);
      planPMask.end();
      planP.end();
    }
}

void Map::resizeEvent(QResizeEvent* event)
{
  if( ! event->size().isEmpty() )
    {
      pixBuffer = QPixmap( event->size() );
      pixBuffer.fill(Qt::transparent);
      __redrawMap();
    }
}

void Map::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(Q3TextDrag::canDecode(event));
}

void Map::dropEvent(QDropEvent* event)
{
  QStringList dropList;

  if(Q3UriDrag::decodeToUnicodeUris(event, dropList))
    {
      for(QStringList::Iterator it = dropList.begin(); it != dropList.end(); it++)
          emit openFile(*it);
    }
}

void Map::__redrawMap()
{
  static QSize lastSize;

  qDebug() << "Map::__redrawMap()";

  if( isDrawing )
    {
      // Queue the redraw request
      redrawRequest = true;
      return;
    }

  isDrawing = true;
  redrawMapTimer->stop();

  if( ! lastSize.isValid() || lastSize != size() )
    {
      lastSize = size();
      pixAero = QPixmap( size() );
      pixAirspace = QPixmap( size() );
      pixFlight = QPixmap( size() );
      pixPlan = QPixmap( size() );
      pixGrid = QPixmap( size() );
      pixUnderMap = QPixmap( size() );
      pixIsoMap = QPixmap( size() );
      pixWaypoints = QPixmap( size() );

      bitPlanMask = QPixmap( size() );
      bitFlightMask = QPixmap( size() );
      bitWaypointsMask = QPixmap( size() );
    }

  extern MapMatrix *_globalMapMatrix;
  _globalMapMatrix->createMatrix( size() );
  emit changed( size() );

  // Status bar not set "geniously" so far...
  emit setStatusBarProgress(0);

  pixPlan.fill(Qt::transparent);

  pixAero.fill(Qt::transparent);
  pixAirspace.fill(Qt::transparent);
  pixGrid.fill(Qt::transparent);
  pixUnderMap.fill(Qt::transparent);
  pixIsoMap.fill(Qt::transparent);
  pixFlight.fill(Qt::transparent);
  pixWaypoints.fill(Qt::transparent);

  bitFlightMask.fill(Qt::color0);
  bitPlanMask.fill(Qt::color0);
  bitWaypointsMask.fill(Qt::color0);

  QPoint temp1(preCur1);
  QPoint temp2(preCur2);

  preCur1.setX(-50);
  preCur2.setX(-50);

  _globalMapContents->proofeSection();

  __drawMap();
  __drawFlight();
  __drawWaypoints();
  //__drawPlannedTask();
  // Linie zum aktuellen Punkt löschen
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);

  __showLayer();

  emit setStatusBarProgress(100);
  slotDrawCursor(temp1, temp2);

  if( redrawRequest == true )
    {
      qDebug( "Map::__redrawMap(): queued redraw event found, schedule Redraw" );

      redrawMapTimer->start(1000);
    }

  isDrawing = false;
}

/** Save Map to PNG-file with width,heigt. Use actual size if width=0 & height=0 */
void Map::slotSavePixmap(QUrl fUrl, int width, int height)
{
  extern MapContents *_globalMapContents;
  extern QSettings _settings;
  int w_orig,h_orig;

  if(fUrl.isValid())  return;

  QString fName;
  if(fUrl.scheme()=="file")
      fName = fUrl.path();
  else
      return;

  if (width && height)
  {
    w_orig=pixBuffer.width();
    h_orig=pixBuffer.height();
    resize(width,height);
    slotCenterToFlight();
  }
  else
      return;

  if (_settings.readBoolEntry("/CommentSettings/ShowComment"))
  {
    Flight* flight = (Flight*)_globalMapContents->getFlight();
    QPainter bufferP(&pixBuffer);
    bufferP.setPen( Qt::white );
    QFont font;
    int by=pixBuffer.height()-35;
    int bw=pixBuffer.width()-10;
    QString text=QObject::tr("%1 with %2 (%3) on %4").arg(flight->getPilot()).arg(flight->getType()).arg(flight->getID()).arg(flight->getDate().toString());
    bufferP.setFont(font);
          bufferP.drawText(10,by+15,bw,25,Qt::AlignLeft,QObject::tr("created by KFLog (www.kflog.org)"));
    font.setBold(true);
    font.setPointSize( 18 );
    bufferP.setFont(font);
          bufferP.drawText(10,by,bw,25,Qt::AlignLeft,text);
  }

        QImage image = QImage(pixBuffer.convertToImage());
        image.save(fName,"PNG");
        if (width && height){
          resize(w_orig,h_orig);
          slotCenterToFlight();
  }
}

void Map::slotSavePixmap()
{
  QFileDialog* dlg = new QFileDialog(this, QObject::tr("Select PNG-File"));
  dlg->setFilters(QStringList("*.png *.PNG"));
  dlg->setModal(true);
  dlg->exec();
  slotSavePixmap(QUrl::fromLocalFile(dlg->directory().canonicalPath()), 0, 0);
}

void Map::slotRedrawFlight()
{
  pixFlight.fill(Qt::white);
  bitFlightMask.fill(Qt::color0);
  __drawFlight();
  __showLayer();
}

void Map::slotRedrawMap()
{
  qDebug() << "Map::slotRedrawMap()";
  __redrawMap();
}

void Map::slotActivatePlanning()
{
  if(planning != 1)
    {
      planning = 1;

      pixPlan.fill(Qt::white);
      prePlanPos.setX(-999);
      prePlanPos.setY(-999);
//warning("start");
      emit setStatusBarMsg(QObject::tr("To finish the planing, press <CTRL> and the right mouse button!"));
    }
  else
    {
      // Planen "ausschalten"
      planning = 0;
      __showLayer();
      emit setStatusBarMsg("");
      emit taskPlanningEnd();
//warning("ende");
/*          pixPlan.fill(white);
          prePlanPos.setX(-999);
          prePlanPos.setY(-999);
*/
    }
}


void Map::__showLayer()
{
  pixAero.setMask(QBitmap(pixAero));
  pixFlight.setMask(bitFlightMask);
  pixPlan.setMask(bitPlanMask);
  pixWaypoints.setMask(bitWaypointsMask);

  pixBuffer = pixIsoMap;

  QPainter buffer(&pixBuffer);

  buffer.drawPixmap(pixUnderMap.rect(), pixUnderMap);
  buffer.drawPixmap(pixAero.rect(), pixAero);
  buffer.drawPixmap(pixAirspace.rect(), pixAirspace);
  buffer.drawPixmap(pixFlight.rect(), pixFlight);
  buffer.drawPixmap(pixPlan.rect(), pixPlan);
  buffer.drawPixmap(pixWaypoints.rect(), pixWaypoints);
  buffer.drawPixmap(pixGrid.rect(), pixGrid);

  update();
}

void Map::slotDrawCursor(const QPoint& p1, const QPoint& p2)
{
//FIXME: Qt gives the following runtime warning in this function:
//QPainter::begin: Widget painting can only begin as a result of a paintEvent

  QPoint pos1(_globalMapMatrix->map(p1)), pos2(_globalMapMatrix->map(p2));

  QPoint prePos1(_globalMapMatrix->map(preCur1)),
         prePos2(_globalMapMatrix->map(preCur2));

  if(preCur1.x() > -50)
    {
      bitBlt(&pixBuffer, prePos1.x() - 32, prePos1.y() - 32, &pixCursorBuffer1);
      bitBlt(this, prePos1.x() - 32, prePos1.y() - 32, &pixCursorBuffer1);
    }
  if(preCur2.x() > -50)
    {
      bitBlt(&pixBuffer, prePos2.x() - 0, prePos2.y() - 32, &pixCursorBuffer2);
      bitBlt(this, prePos2.x() - 0, prePos2.y() - 32, &pixCursorBuffer2);
    }
  //
  // copying the pixmaps can crash the x-server, if the coordinates
  // are out of range.
  //
  //                                                Fixed 2001-12-01
  //
  if(pos1.x() > -50 && pos1.x() < width() + 50 &&
          pos1.y() > -50 && pos1.y() < height() + 50)
      bitBlt(&pixCursorBuffer1, 0, 0, &pixBuffer,
          pos1.x() - 32, pos1.y() - 32, 32, 32);

  if(pos2.x() > -50 && pos2.x() < width() + 50 &&
          pos2.y() > -50 && pos2.y() < height() + 50)
      bitBlt(&pixCursorBuffer2, 0, 0, &pixBuffer,
          pos2.x() - 0, pos2.y() - 32, 32, 32);

  if(pos1.x() > -50 && pos1.x() < width() + 50 &&
          pos1.y() > -50 && pos1.y() < height() + 50)
    {
      bitBlt(this, pos1.x() - 32, pos1.y() - 32, &pixCursor1);
      bitBlt(&pixBuffer, pos1.x() - 32, pos1.y() - 32, &pixCursor1);
    }

  if(pos2.x() > -50 && pos2.x() < width() + 50 &&
          pos2.y() > -50 && pos2.y() < height() + 50)
    {
      bitBlt(this, pos2.x() - 0, pos2.y() - 32, &pixCursor2);
      bitBlt(&pixBuffer, pos2.x() - 0, pos2.y() - 32, &pixCursor2);
    }

  preCur1 = p1;
  preCur2 = p2;
}

void Map::slotZoomRect()
{
  setCursor(Qt::CrossCursor);
  isZoomRect=true;
}

void Map::slotCenterToWaypoint(const int id)
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;

  if(id >= _globalMapContents->getFlight()->getWPList().count())
    {
      qWarning("KFLog: Map::slotCenterToWaypoint: wrong Waypoint-ID");
      return;
    }

  _globalMapMatrix->centerToPoint(_globalMapMatrix->map(
      _globalMapContents->getFlight()->getWPList().at(id)->projP));
  _globalMapMatrix->slotSetScale(_globalMapMatrix->getScale(MapMatrix::LowerLimit));

  emit changed(this->size());
}

void Map::slotCenterToFlight()
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;

  Flight *f = (Flight *)_globalMapContents->getFlight();
  if (f) {
    QRect r;
    QRect r2;
    QList<Flight*> fl;

    switch (f->getObjectType()) {
      case BaseMapElement::Flight:
        r = f->getFlightRect();
        break;
      case BaseMapElement::FlightGroup:
        fl = ((FlightGroup *)f)->getFlightList();
        r = fl.at(0)->getFlightRect();
        for(int i = 1; i < fl.count(); i++) {
          r2 = fl.at(i)->getFlightRect();
          r.setLeft(QMIN(r.left(), r2.left()));
          r.setTop(QMIN(r.top(), r2.top()));
          r.setRight(QMAX(r.right(), r2.right()));
          r.setBottom(QMAX(r.bottom(), r2.bottom()));
        }
        break;
      default:
        return;
    }

    // check if the Rectangle is zero
    // is it necessary here?
    if (!r.isNull()) {
      _globalMapMatrix->centerToRect(r);
      __redrawMap();
    }

  emit changed(this->size());
  }
}

void Map::slotCenterToTask()
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;
  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      QRect r;
      QRect r2;
      QList<Flight*> fl;

      switch (f->getObjectType())
        {
          case BaseMapElement::Flight:
            r = ((Flight *)f)->getTaskRect();
            break;
          case BaseMapElement::Task:
            r = ((FlightTask *)f)->getRect();
            break;
          case BaseMapElement::FlightGroup:
            fl = ((FlightGroup *)f)->getFlightList();
            r = fl.at(0)->getTaskRect();
            for(int i = 1; i < fl.count(); i++) {
              r2 = fl.at(i)->getTaskRect();
              r.setLeft(QMIN(r.left(), r2.left()));
              r.setTop(QMIN(r.top(), r2.top()));
              r.setRight(QMAX(r.right(), r2.right()));
              r.setBottom(QMAX(r.bottom(), r2.bottom()));
            }
            break;
          default:
            return;
        }

      // check if the Rectangle is zero
      if(!r.isNull())
        {
          _globalMapMatrix->centerToRect(r);
          __redrawMap();
        }
    }
}

/**
 * Animation slot.
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStart()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  QPoint pos;
  QPixmap pix;
  flightPoint cP;

  Flight *f = (Flight *)_globalMapContents->getFlight();

  if(f)
    {
            // save this one to speed up timeout code
      flightToAnimate = f;

      switch(f->getObjectType())
        {
          case BaseMapElement::Flight:
            f->setAnimationIndex(0);
            f->setAnimationActive(true);
            break;

          case BaseMapElement::FlightGroup:
            // loop through all and set animation index to start
            {
              QList<Flight*> flightList = ((FlightGroup *)f)->getFlightList();
              foreach(f, flightList)
                {
                  f->setAnimationIndex(0);
                  f->setAnimationActive(true);
                }

              break;
            }

          default:
            break;
        }
        // force redraw
        // flights will not be visible as nAnimationIndex is zero for all flights to animate.
        slotRedrawFlight();

      switch(f->getObjectType())
        {
          case BaseMapElement::Flight:
            cP = f->getPoint(0);
            prePos = _globalMapMatrix->map(cP.projP);
            pos = _globalMapMatrix->map(cP.projP);
            pix = f->getLastAnimationPixmap();
            pix.paintEngine()->drawPixmap(QRect(QPoint(0, 0), pixBuffer.size()), pixBuffer, QRect(pos.x(), pos.y()-32, 32, 32));
//            bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
            f->setLastAnimationPos(pos);
            f->setLastAnimationPixmap(pix);
            // put flag
            bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2 );
           break;

          case BaseMapElement::FlightGroup:
            // loop through all and set animation index to start
              {
                QList<Flight*> flightList = ((FlightGroup *)f)->getFlightList();
                for(int loop = 0; loop < flightList.count(); loop++)
                  {
                    cP = f->getPoint(0);
                    pos = _globalMapMatrix->map(cP.projP);
                    pix = f->getLastAnimationPixmap();
                    pix.paintEngine()->drawPixmap(QRect(QPoint(0, 0), pixBuffer.size()), pixBuffer, QRect(pos.x(), pos.y()-32, 32, 32));
    //                bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
                    f->setLastAnimationPos(pos);
                    f->setLastAnimationPixmap(pix);
                    // put flag
                    bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2 );
                  }
              }
            break;

          default:

            break;
           }

      // 50ms multi-shot timer
      timerAnimate->start( 50, FALSE );
    }
}

/**
 * Animation slot.
 * Called for every timeout of the animation timer.
 */
void Map::slotAnimateFlightTimeout()
{
  extern MapMatrix *_globalMapMatrix;
  flightPoint cP; //, prevP;
  Flight *f  = this->flightToAnimate; // = (Flight *)_globalMapContents->getFlight();
  bool bDone = true;
  QPoint lastpos, pos;
  QPixmap pix;

  if(f)
    {
      switch(f->getObjectType())
        {
          case BaseMapElement::Flight:
            f->setAnimationNextIndex();
            if (f->isAnimationActive())
                      bDone = false;
            //write info from current point on statusbar
            cP = f->getPoint((f->getAnimationIndex()));
            pos = _globalMapMatrix->map(cP.projP);
            lastpos = f->getLastAnimationPos();
            pix = f->getLastAnimationPixmap();
            emit showFlightPoint(_globalMapMatrix->mapToWgs(pos), cP);
            // erase prev indicator-flag
            bitBlt(&pixBuffer, lastpos.x(), lastpos.y()-32, &pix);

//            bitBlt(&pixBuffer, lastpos.x()-100, lastpos.y()-32, &pix);
                  // redraw flight up to this point, blt the pixmap onto the already created pixmap
            __drawFlight();
            pixFlight.setMask(bitFlightMask);
            bitBlt(&pixBuffer, 0, 0, &pixFlight);
            //save for next timeout
            pix.paintEngine()->drawPixmap(QRect(QPoint(0, 0), pixBuffer.size()), pixBuffer, QRect(pos.x(), pos.y()-32, 32, 32));
            //bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
            f->setLastAnimationPixmap(pix);
            f->setLastAnimationPos(pos);
            // add indicator-flag
            bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2);
            break;
          case BaseMapElement::FlightGroup:
            // loop through all and set animation index to start
            {
              QList<Flight*> flightList = ((FlightGroup*)flightToAnimate)->getFlightList();

              for(int loop = 0; loop < flightList.count(); loop++)
                {
                              f = flightList.at(loop);
                  f->setAnimationNextIndex();
                  if (f->isAnimationActive())
                                   bDone = false;
                  //write info from current point on statusbar
                  cP = f->getPoint((f->getAnimationIndex()));
                  pos = _globalMapMatrix->map(cP.projP);
                  lastpos = f->getLastAnimationPos();
                  pix = f->getLastAnimationPixmap();
                  emit showFlightPoint(_globalMapMatrix->mapToWgs(pos), cP);
                  // erase prev indicator-flag
                  bitBlt(&pixBuffer, lastpos.x(), lastpos.y()-32, &pix);
                        // redraw flight up to this point, blt the pixmap onto the already created pixmap
                  __drawFlight();
                  pixFlight.setMask(bitFlightMask);
                  bitBlt(&pixBuffer, 0, 0, &pixFlight);
                  //save for next timeout
                  pix.paintEngine()->drawPixmap(QRect(QPoint(0, 0), pixBuffer.size()), pixBuffer, QRect(pos.x(), pos.y()-32, 32, 32));
  //                bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
                  f->setLastAnimationPixmap(pix);
                  f->setLastAnimationPos(pos);
                  // add indicator-flag
                  bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2);
                }

              break;
            }

          default:
            break;
         }
    }
  // force paint event
  repaint();

        // if one of the flights still is active, bDone will be false
  if (bDone)
    timerAnimate->stop();
}

/**
 * Animation slot.
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStop()
{
  extern MapContents *_globalMapContents;
  Flight *f = (Flight *)_globalMapContents->getFlight();
  QList<Flight*> flightList;

  if(!f) return;

  switch (f->getObjectType())
    {
      case BaseMapElement::Flight:
        flightList.append(f);
        break;
      case BaseMapElement::FlightGroup:
        flightList = ((FlightGroup *)f)->getFlightList();
        break;
      default:
        return;
    }

  // stop animation on user request.
  if(timerAnimate->isActive())
    {
      timerAnimate->stop();
            // loop through all and increment animation index
            for(int loop = 0; loop < flightList.count(); loop++)
          flightList.at(loop)->setAnimationActive(false);

      slotRedrawFlight();
    }
}

/**
 * Stepping slots.
 */
void Map::slotFlightNext()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if(prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
           bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
          // get the next point, preIndex now holds last point
          if ((index = f->searchGetNextPoint(preIndex, cP)) != -1)
            {
                          // if close to edge, recenter map to next point
                          prePos = _globalMapMatrix->map(cP.projP);
                          if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
                                     _globalMapMatrix->centerToPoint(prePos);
                                 __redrawMap();
              }
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightPrev()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
            prePos.x() - 20, prePos.y() - 20, 40, 40);
          // get the next point, preIndex now holds last point
          if ((index = f->searchGetPrevPoint(preIndex, cP)) != -1)
            {
                                // if close to edge, recenter map to next point
                          prePos = _globalMapMatrix->map(cP.projP);
                          if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
                                     _globalMapMatrix->centerToPoint(prePos);
              }
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightStepNext()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
          // get the next point, preIndex now holds last point
          if((index = f->searchStepNextPoint(preIndex, cP, 10)) != -1)
            {
                          // if close to edge, recenter map to next point
                          prePos = _globalMapMatrix->map(cP.projP);
                          if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
                                     _globalMapMatrix->centerToPoint(prePos);
              }
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightStepPrev()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
          // get the next point, preIndex now holds last point
          if ((index = f->searchStepPrevPoint(preIndex, cP,10)) != -1)
            {
                          // if close to edge, recenter map to next point
                          prePos = _globalMapMatrix->map(cP.projP);
                          if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
                                     _globalMapMatrix->centerToPoint(prePos);
                                 __redrawMap();
              }
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightHome()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
          if ((index = f->searchGetNextPoint(0, cP)) != -1)
            {
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightEnd()
{
  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  flightPoint cP;
  int index;
  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);

          // just a workaround !!!!!!!!!!!!!
          if(f->getObjectType() == BaseMapElement::Flight)
            {
              if((index = f->searchGetNextPoint(((Flight *)f)->getRouteLength()-1, cP)) != -1)
                {
                  emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
                  prePos = _globalMapMatrix->map(cP.projP);
                  preIndex = index;
                  bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
                }
            }
        }
    }
}

/** No descriptions */
void Map::slotShowCurrentFlight()
{
  extern MapContents *_globalMapContents;
  BaseFlightElement *f = _globalMapContents->getFlight();
  // just to make sure ...
  slotAnimateFlightStop();

  planning = 0;

  if (f && f->getObjectType() == BaseMapElement::Task) {
    if (((FlightTask *)f)->getWPList().count() < 1) {
      slotActivatePlanning();
    }
    else {
      planning = 2;
    }
  }

  // Hier wird der Flug 2x neu gezeichnet, denn erst beim
  // ersten Zeichnen werden die Rahmen von Flug und Aufgabe
  // bestimmt.
  slotRedrawFlight();

  if(f)
    {
      switch(f->getObjectType())
        {
          case BaseMapElement::Flight:
            // fall through
          case BaseMapElement::FlightGroup:
            slotCenterToFlight();
            break;
          case BaseMapElement::Task:
            slotCenterToTask();
            break;
          default:
            break;
        }
    }
}
/** append a waypoint to the current task */
void Map::slotAppendWaypoint2Task(Waypoint *p)
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;

  FlightTask *f = (FlightTask *)_globalMapContents->getFlight();
  if(f && f->getObjectType() == BaseMapElement::Task && planning)
    {
      QList<Waypoint*> taskPointList = f->getWPList();
      p->projP = _globalMapMatrix->wgsToMap(p->origP);
      taskPointList.append(p);
      f->setWaypointList(taskPointList);
      __drawPlannedTask(true);
      __showLayer();
    }
}

/** search for a waypoint
First look in task itself
Second look in map contents */
bool Map::__getTaskWaypoint(const QPoint& current, Waypoint *wp, QList<Waypoint*> &taskPointList)
{
  Waypoint *tmpPoint;
  QPoint sitePos;
  double dX, dY;
  // Radius for Mouse Snapping
  double delta(16.0);
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;
  bool found = false;
  Airfield *hitElement;

  for(int i = 0; i < taskPointList.count(); i++)
    {
      tmpPoint = taskPointList.at(i);
      sitePos = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(tmpPoint->origP));
      dX = abs(sitePos.x() - current.x());
      dY = abs(sitePos.y() - current.y());

      // Abstand entspricht der Icon-Größe.
      if (dX < delta && dY < delta)
        {
          *wp = *tmpPoint;
          found = true;
          break;
        }
    }

  if(!found)
    {
      /*
       *  Muss für alle Punktdaten durchgeführt werden
       */
      QVector<int> contentArray(3);
      contentArray[0] = MapContents::GliderfieldList;
      contentArray[1] = MapContents::AirfieldList;
      contentArray[3] = MapContents::OutLandingList;


      for(int n = 0; n < contentArray.count(); n++)
        {
          for(int loop = 0; loop < _globalMapContents->getListLength(contentArray.at(n)); loop++)
            {
              hitElement = (Airfield *)_globalMapContents->getElement(contentArray.at(n), loop);
              sitePos = hitElement->getMapPosition();
              dX = abs(sitePos.x() - current.x());
              dY = abs(sitePos.y() - current.y());

              if (dX < delta && dY < delta)
                {
                  wp->name = hitElement->getWPName();
                  wp->origP = hitElement->getWGSPosition();
                  wp->elevation = hitElement->getElevation();
                  wp->projP = hitElement->getPosition();
                  wp->description = hitElement->getName();
                  wp->type = hitElement->getObjectType();
                  wp->elevation = hitElement->getElevation();
                  wp->icao = hitElement->getICAO();
                  wp->frequency = hitElement->getFrequency().toDouble();
                  wp->runway.first = 0;
                  wp->runway.second = 0;
                  wp->length = -1;
                  wp->surface = Runway::Unknown;

                  if( hitElement->getRunwayNumber() )
                    {
                      wp->runway = hitElement->getRunway(0)->getRunwayDirection();
                      wp->length = hitElement->getRunway(0)->length;
                      wp->surface = hitElement->getRunway(0)->surface;
                    }

                  wp->comment = hitElement->getComment();
                  wp->isLandable = hitElement->isLandable();

                  found = true;
                  break;
                }
            }
        }
    }

  return found;
}

/** Puts the waypoints of the active waypoint catalog to the map */
void Map::__drawWaypoints()
{
  extern MapContents *_globalMapContents;
  extern MapMatrix *_globalMapMatrix;
  extern MapConfig *_globalMapConfig;

  QList<Waypoint*> *wpList;
  QPoint p;

  wpList = _globalMapContents->getWaypointList();

  QPainter wpPainter(&pixWaypoints);
  QPainter wpMaskPainter(&bitWaypointsMask);

  wpPainter.setBrush(Qt::NoBrush);
  wpPainter.setPen(QPen(QColor(0,0,0), 2, Qt::SolidLine));

  // now do complete list
  Waypoint *wp;
  foreach(wp, *wpList) {
    // make sure projection is ok, and map to screen
    wp->projP = _globalMapMatrix->wgsToMap(wp->origP.lat(), wp->origP.lon());
    p = _globalMapMatrix->map(wp->projP);

    // draw marker
    wpPainter.setBrush(Qt::NoBrush);
    wpPainter.setPen(QPen(QColor(0,0,0), 2, Qt::SolidLine));
    wpPainter.drawRect(p.x() - 4,p.y() - 4, 8, 8);
    wpMaskPainter.drawRect(p.x() - 4,p.y() - 4, 8, 8);

     // draw name of wp
     if(_globalMapConfig->drawWpLabels()) {
        int xOffset;
        int yOffset;
        QRect textbox;
        textbox=wpPainter.fontMetrics().boundingRect(wp->name);
        if (wp->origP.lon()<_globalMapMatrix->getMapCenter(false).y()) {
          //the wp is on the left side of the map, so draw the textlabel on the right side
          xOffset=14;
          yOffset=-2;
          if (_globalMapConfig->useSmallIcons()){
            xOffset=6;
            yOffset=0;
          }
        } else {
          //the wp is on the right side of the map, so draw the textlabel on the left side
          xOffset=-textbox.width()-14;
          yOffset=-2;
          if (_globalMapConfig->useSmallIcons()){
            xOffset=-textbox.width()-6;
            yOffset=0;
          }
         }

        wpPainter.setPen(QPen(Qt::black, 3, Qt::SolidLine));
        wpPainter.fillRect(p.x()+xOffset-1,p.y()+yOffset,textbox.width()+2,-textbox.height()-2,Qt::white);
        wpPainter.drawText(p.x()+xOffset, p.y()+yOffset, wp->name, -1);
        wpMaskPainter.drawText(p.x()+xOffset, p.y()+yOffset, wp->name, -1);
        wpMaskPainter.fillRect(p.x()+xOffset-1,p.y()+yOffset,textbox.width()+2,-textbox.height()-2,wpMaskPainter.pen().color());
     }
   }

  // clean up
  wpPainter.end();
  wpMaskPainter.end();
}

/** Slot signalled when user selects another waypointcatalog.  */
void Map::slotWaypointCatalogChanged(WaypointCatalog* c)
{
  extern MapContents *_globalMapContents;
  Waypoint *w;
  bool filterRadius, filterArea;
  QList<Waypoint*> *wpList;

  wpList = _globalMapContents->getWaypointList();
  wpList->clear();

  filterRadius = (c->radiusLat != 1  || c->radiusLong != 1);
  filterArea = (c->areaLat2 != 1 && c->areaLong2 != 1 && !filterRadius);

  foreach(w, c->wpList)
  {
    if(!c->showAll) {
      switch(w->type) {
      case BaseMapElement::IntAirport:
      case BaseMapElement::Airport:
      case BaseMapElement::MilAirport:
      case BaseMapElement::CivMilAirport:
      case BaseMapElement::Airfield:
        if (!c->showAirports) {
          continue;
        }
        break;
      case BaseMapElement::Gliderfield:
        if (!c->showGliderSites) {
          continue;
        }
        break;
      case BaseMapElement::UltraLight:
      case BaseMapElement::HangGlider:
      case BaseMapElement::Parachute:
      case BaseMapElement::Balloon:
        if (!c->showOtherSites) {
          continue;
        }
        break;
      }
    }

     if (filterArea) {
       if (w->origP.lat() < c->areaLat1 || w->origP.lat() > c->areaLat2 ||
           w->origP.lon() < c->areaLong1 || w->origP.lon() > c->areaLong2) {
           continue;
       }
     }
     else if (filterRadius) {
       if (dist(c->radiusLat, c->radiusLong, w->origP.lat(), w->origP.lon()) > c->radiusSize) {
         continue;
       }
     }
   // add the waypoint to the list
   wpList->append(new Waypoint(w));
  }
  // force a update
  emit changed(this->size());

  __redrawMap();
}

/**
 * Tries to locate the elevation for the given point
 * and emits a signal elevation if found.
 *
 * \param coordMap The map coordinates.
 */
void Map::__findElevation( const QPoint& coordMap )
{
  extern MapContents *_globalMapContents;

  int height = _globalMapContents->getElevation( coordMap, 0 );

  emit elevation(height);
}

void Map::__setCursor()
{
  static const unsigned char cross_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x00,
    0x00, 0x80, 0x01, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x98, 0x19, 0x00,
    0x00, 0x8c, 0x31, 0x00, 0x00, 0x86, 0x61, 0x00, 0x00, 0x83, 0xc1, 0x00,
    0x00, 0xc1, 0x83, 0x00, 0x80, 0x60, 0x06, 0x01, 0x80, 0x30, 0x0c, 0x01,
    0xe0, 0x9f, 0xf9, 0x07, 0xe0, 0x9f, 0xf9, 0x07, 0x80, 0x30, 0x0c, 0x01,
    0x80, 0x60, 0x06, 0x01, 0x00, 0xc1, 0x83, 0x00, 0x00, 0x83, 0xc1, 0x00,
    0x00, 0x86, 0x61, 0x00, 0x00, 0x8c, 0x31, 0x00, 0x00, 0x98, 0x19, 0x00,
    0x00, 0xe0, 0x07, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);
  setCursor(crossCursor);
}

/** Creates the popup menu for the map */
void Map::__createPopupMenu()
{
  extern MapMatrix *_globalMapMatrix;

  mapPopup=new Q3PopupMenu(this);

//  mapPopup->insertTitle(/*SmallIcon("task")*/ 0, QObject::tr("Map"), 0);
  idMpAddWaypoint  = mapPopup->insertItem(_mainWindow->getPixmap("kde_filenew_16.png"), QObject::tr("&New waypoint"), this, SLOT(slotMpNewWaypoint()));
  idMpEditWaypoint = mapPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), QObject::tr("&Edit waypoint"), this, SLOT(slotMpEditWaypoint()));
  idMpDeleteWaypoint = mapPopup->insertItem(_mainWindow->getPixmap("kde_editdelete_16.png"), QObject::tr("&Delete waypoint"), this, SLOT(slotMpDeleteWaypoint()));

  mapPopup->insertSeparator();

  idMpEndPlanning  = mapPopup->insertItem(QObject::tr("&End taskplanning"), this, SLOT(slotMpEndPlanning()));
  mapPopup->insertItem(_mainWindow->getPixmap("kde_info_16.png"), QObject::tr("&Show map info..."), this, SLOT(slotMpShowMapInfo()));

  mapPopup->insertSeparator();

  idMpCenterMap  = mapPopup->insertItem(_mainWindow->getPixmap("centerto_22.png"), QObject::tr("&Center map"), this, SLOT(slotMpCenterMap()));

  idMpZoomIn = mapPopup->insertItem(_mainWindow->getPixmap("kde_viewmag+_16.png"), QObject::tr("Zoom &In"), _globalMapMatrix, SLOT(slotZoomIn()));
  idMpZoomOut = mapPopup->insertItem(_mainWindow->getPixmap("kde_viewmag-_16.png"), QObject::tr("Zoom &Out"), _globalMapMatrix, SLOT(slotZoomOut()));
  /*
  idMpAddTaskPoint
 */
}

/** Selects the correct items to show from the menu and then shows it. */
void Map::__showPopupMenu(QMouseEvent * Event)
{
  if (findWaypoint (Event->pos()))
  {
    mapPopup->setItemEnabled(idMpAddWaypoint, false);
    mapPopup->setItemEnabled(idMpEditWaypoint, true);
    mapPopup->setItemEnabled(idMpDeleteWaypoint, true);
  }
  else
  {
    mapPopup->setItemEnabled(idMpAddWaypoint, true);
    mapPopup->setItemEnabled(idMpEditWaypoint, false);
    mapPopup->setItemEnabled(idMpDeleteWaypoint, false);
  }

  mapPopup->setItemEnabled(idMpEndPlanning, (planning == 1 || planning == 3));

  mapPopup->exec(mapToGlobal(Event->pos()));
}

/** called from the MapPopupmenu to add a new waypoint. */
void Map::slotMpNewWaypoint()
{
   extern MapContents *_globalMapContents;
   extern MapMatrix *_globalMapMatrix;

   RadioPoint *hitElement;
   QString text;

   QPoint sitePos;
   double dX, dY, delta(16.0);

   const QPoint current(popupPos);

   // select WayPoint
   QRegExp blank("[ ]");
   bool found = false;

    // add WPList !!!
    int searchList[] = {MapContents::GliderfieldList, MapContents::AirfieldList};

    for (int l = 0; l < 2; l++) {
      for(int loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++) {
        hitElement = (RadioPoint*)_globalMapContents->getElement(searchList[l], loop);
        sitePos = hitElement->getMapPosition();

        dX = abs(sitePos.x() - current.x());
        dY = abs(sitePos.y() - current.y());

        // Abstand entspricht der Icon-Größe.
        if (dX < delta && dY < delta) {
          Waypoint *w = new Waypoint;

          QString name = hitElement->getName();
          w->name = name.replace(blank, "").left(6).upper();
          w->description = hitElement->getName();
          w->type = hitElement->getObjectType();
          w->origP = hitElement->getWGSPosition();
          w->elevation = hitElement->getElevation();
          w->icao = hitElement->getICAO();
          w->frequency = hitElement->getFrequency().toDouble();
          w->isLandable = true;

          emit waypointSelected(w);
          found = true;
          break;
        }
      }
      if (found)  break;
    }

    if (!found) {
      //warning("new waypoint");

      WaypointDialog *waypointDlg = new WaypointDialog(this);
      emit regWaypointDialog(waypointDlg); //register the dialog and connect it's signals.

      waypointDlg->enableApplyButton(false);

      QPoint p = _globalMapMatrix->mapToWgs(current);

      // initialize dialog
      //waypointDlg->setWaypointType(BaseMapElement::Landmark); now set by default.
      waypointDlg->longitude->setText(WGSPoint::printPos(p.x(), false));
      waypointDlg->latitude->setText(WGSPoint::printPos(p.y(), true));
      waypointDlg->setSurface(-1);
      waypointDlg->exec(); //we only need to exec the dialog. The dialog can take care of itself now :-)

      delete waypointDlg;
   }
}

/** called from the MapPopupmenu to edit waypoint. */
void Map::slotMpEditWaypoint()
{
  Waypoint* wp = findWaypoint( popupPos );

  if( wp )
    {
      emit waypointEdited( wp );
    }
}

/** called from the MapPopupmenu to edit waypoint. */
void Map::slotMpDeleteWaypoint()
{
  Waypoint* wp = findWaypoint( popupPos );

  if( wp )
    {
      emit waypointDeleted( wp );
    }
}

/** Called from the context menu to center the map. */
void Map::slotMpCenterMap()
{
  extern MapMatrix *_globalMapMatrix;
   // Move Map
  _globalMapMatrix->centerToPoint(popupPos);
  __redrawMap();
}

void Map::slotMpEndPlanning()
{
  moveWPindex = -999;

  prePlanPos.setX(-999);
  prePlanPos.setY(-999);
  planning = 2;

  emit taskPlanningEnd();
}

void Map::slotMpShowMapInfo()
{
  leaveEvent(0);
  __displayMapInfo(popupPos, false);
}

void Map::leaveEvent( QEvent * )
{
  mapInfoTimer->stop();
  mapInfoTimerStartpoint = QPoint( -999, -999 );
}

void Map::slotMapInfoTimeout()
{
  __displayMapInfo( mapInfoTimerStartpoint, true );
}

/** Draws a scale indicator on the pixmap. */
void Map::__drawScale( QPixmap& scalePixmap )
{
  QPen pen;
  QBrush brush(Qt::white);

  QPainter scaleP( &scalePixmap );

  pen.setColor(Qt::black);
  pen.setWidth(3);
  pen.setCapStyle(Qt::RoundCap);
  scaleP.setPen(pen);
  QFont f = scaleP.font();

  f.setPointSize(12);
  scaleP.setFont(f);

  double scale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);
  Distance barLen;

  /** select appropriate length of bar. This needs to be done for each unit
   * separately, because else you'd get weird, broken bar length.
   * Note: not all possible units are taken into account (meters, feet),
   * because we are not going to use these for horizontal distances (at least
   * not externally.) */
  int len=1;

  switch (barLen.getUnit())
    {
    case Distance::kilometers:
      len = 100;
      if (scale<1000)
        len=50;
      if (scale<475)
        len=25;
      if (scale<240)
        len=10;
      if (scale<100)
        len=5;
      if (scale<50)
        len=3;
      if (scale<20)
        len=1;
      barLen.setKilometers(len);
      break;

    case Distance::miles:
      len=60;
      if (scale<1000)
        len=30;
      if (scale<475)
        len=12;
      if (scale<200)
        len=6;
      if (scale<95)
        len=4;
      if (scale<60)
        len=2;
      if (scale<30)
        len=1;
      barLen.setMiles(len);
      break;

    case Distance::nautmiles:
      len=50;
      if (scale<1000)
        len=25;
      if (scale<450)
        len=10;
      if (scale<175)
        len=5;
      if (scale<90)
        len=3;
      if (scale<55)
        len=1;
      barLen.setNautMiles(len);
      break;

    default: //should not happen, other units are not used for horizontal distances.
      len = 100;
      if (scale<1000)
        len=50;
      if (scale<475)
        len=25;
      if (scale<240)
        len=10;
      if (scale<100)
        len=5;
      if (scale<50)
        len=3;
      if (scale<20)
        len=1;
      barLen.setKilometers(len);
      break;
    };

  //determine how long the bar should be in pixels
  int drawLength = (int)rint(barLen.getMeters()/scale);
  //...and where to start drawing. Now at the left lower side ...
  scaleP.translate( QPoint( -this->width()+drawLength+10, 0) );

  int leftXPos=this->width()-drawLength-5;

  //Now, draw the bar
  scaleP.drawLine(leftXPos, this->height()-5, this->width()-5, this->height()-5); //main bar
  pen.setWidth(3);
  scaleP.setPen(pen);
  scaleP.drawLine(leftXPos, this->height()-9,leftXPos,this->height()-1);              //left endbar
  scaleP.drawLine(this->width()-5,this->height()-9,this->width()-5,this->height()-1);//right endbar

  //get the string to draw
  QString scaleText=barLen.getText(true,0);
  //get some metrics for this string
  QRect txtRect=scaleP.fontMetrics().boundingRect(scaleText);
  int leftTPos=this->width()+int((drawLength-txtRect.width())/2)-drawLength-5;

  //draw white box to draw text on
  scaleP.setBrush(brush);
  scaleP.setPen(Qt::NoPen);
  scaleP.drawRect( leftTPos, this->height()-txtRect.height()-8,
                   txtRect.width()+4, txtRect.height() );

  //draw text itself
  scaleP.setPen(pen);
  // scaleP.drawText( leftTPos, this->height()-10+txtRect.height()/2, scaleText );
  scaleP.drawText( leftTPos, this->height()-txtRect.height()-8,
                   txtRect.width()+4, txtRect.height(), Qt::AlignCenter,
                   scaleText );
}
