/***********************************************************************
**
**   map.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <ctype.h>
#include <iostream>
#include <stdlib.h>

#include <kapp.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qwhatsthis.h>

#include "airspace.h"
#include "flight.h"
#include "flightgroup.h"
#include "kflog.h"
#include "map.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "mapmatrix.h"
#include "singlepoint.h"
#include "radiopoint.h"
#include "waypoints.h"
#include "waypointdialog.h"
#include "resource.h"
#include "whatsthat.h"


// Festlegen der Größe der Pixmaps auf Desktop-Grösse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

// These values control the borders at which to pan the map
// NOTE: These values are only for testing, and need revision.
#define MIN_X_TO_PAN 30
#define MAX_X_TO_PAN QApplication::desktop()->width()-30
#define MIN_Y_TO_PAN 30
#define MAX_Y_TO_PAN QApplication::desktop()->height()-30
#define MAP_INFO_DELAY 1000


Map::Map(KFLogApp *m, QFrame* parent, const char* name)
  : QWidget(parent, name),
    mainApp(m), prePos(-50, -50), preCur1(-50, -50), preCur2(-50, -50),
    planning(-1), tempTask(""), preSnapPoint(-999, -999)
{
  QBitmap bitCursorMask;
  bitCursorMask.resize(40,40);
  bitCursorMask.fill(Qt::color0);
  pixCursor.resize(40,40);
  pixCursor.fill(white);

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

  pixCursor1 = QPixmap(KGlobal::dirs()->findResource("appdata",
						     "pics/flag_green.png"));
  pixCursor2 = QPixmap(KGlobal::dirs()->findResource("appdata",
						     "pics/flag_red.png"));

  pixCursorBuffer1.resize(32,32);
  pixCursorBuffer1.fill(white);

  pixCursorBuffer2.resize(32,32);
  pixCursorBuffer2.fill(white);

  pixAero.resize( PIX_WIDTH, PIX_HEIGHT );
  pixAirspace.resize( PIX_WIDTH, PIX_HEIGHT );
  pixFlight.resize( PIX_WIDTH, PIX_HEIGHT );
  pixPlan.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGrid.resize( PIX_WIDTH, PIX_HEIGHT );
  pixUnderMap.resize( PIX_WIDTH, PIX_HEIGHT );
  pixIsoMap.resize( PIX_WIDTH, PIX_HEIGHT );
  pixWaypoints.resize( PIX_WIDTH, PIX_HEIGHT );
  bitMapMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitAirspaceMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitPlanMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitFlightMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitWaypointsMask.resize( PIX_WIDTH, PIX_HEIGHT );

//  pixAnimate.resize(32,32);
//  pixAnimate.fill(white);

  airspaceRegList = new QPtrList<QRegion>;
  airspaceRegList->setAutoDelete(true);

  __setCursor();
  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setAcceptDrops(true);

  // For Planning
  pixPlan.fill(white);
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);  

  QWhatsThis::add(this, i18n("<B>The map</B>"
			     "<P>To move or scale the map, please use the buttons in the "
			     "<B>Map-control</B>-area. Or center the map to the current "
			     "cursor-positon by using the right mouse-button.</P>"
			     "<P>To zoom in or out, use the slider or the two buttons on the "
			     "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" or \"+\" (zoom in) "
			     "and \"&lt;Ctrl&gt;&lt;-&gt;\" or \"-\"(zoom out).</P>"
           "<P>The cursor keys and the keys on the NumPad can also pan the map, if"
           "NumLock is switched on.</P>"
			     "<P>With the menu-item \"Options\" -> \"Configure KFLog\" you can "
			     "configure, which map elements should be displayed at which "
			     "scale.</P>"));


  __createPopupMenu();
                       
  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), this,
	   SLOT(slotAnimateFlightTimeout()) );

  mapInfoTimer = new QTimer(this);
  connect (mapInfoTimer, SIGNAL(timeout()), this, SLOT(slotMapInfoTimeout()));
  
  isZoomRect=false;
}

Map::~Map()
{
}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern MapContents _globalMapContents;
  extern const MapMatrix _globalMapMatrix;
  const QPoint current = event->pos();
  Waypoint *w;

  QPoint vector = event->pos()-mapInfoTimerStartpoint;
  if(vector.manhattanLength()>4) {
    mapInfoTimer->stop();
    mapInfoTimer->start(MAP_INFO_DELAY, true);
    mapInfoTimerStartpoint=event->pos();
  }
  
  if(planning == 1 || planning == 3)
    {
      double delta(8.0);
      if(_globalMapMatrix.isSwitchScale()) delta = 16.0;

      // If we are near to the last temp-point of the task,
      // and SHIFT is not pressed, we can exit ...
      if( (abs(current.x() - preSnapPoint.x()) <= delta) &&
          (abs(current.y() - preSnapPoint.y()) <= delta) &&
          event->state() != QEvent::ShiftButton )  return;

      BaseFlightElement *f = _globalMapContents.getFlight();

      if(!f)  return;

      QPtrList<Waypoint> taskPointList = f->getWPList();
      QPtrList<Waypoint> tempTaskPointList = f->getWPList();
      QPtrList<Waypoint> *wpList = _globalMapContents.getWaypointList();

      // 3: Task beendet verschieben eines Punktes

      QPoint preSitePos, nextSitePos;
      QPoint point(current);

      if (!taskPointList.isEmpty())
        {
          if(planning == 1)
              preSitePos = _globalMapMatrix.map(taskPointList.getLast()->projP);
          else if(planning == 3)
            {
              if(moveWPindex > 0)
                  preSitePos = _globalMapMatrix.map(taskPointList.at(moveWPindex - 1)->projP);

              if(moveWPindex + 1 < (int)taskPointList.count())
                  nextSitePos = _globalMapMatrix.map(taskPointList.at(moveWPindex + 1)->projP);
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

                  point = _globalMapMatrix.map(_globalMapMatrix.wgsToMap(wp.origP));

                  // Eigentlich könnte man dann auf point verzichten?
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
                      //anhängen
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
                      // außerhalb der Karte
                      prePlanPos.setX(-999);
                      prePlanPos.setY(-999);
                    }

                  warning(wp.name);
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

                  if(event->state() == QEvent::ShiftButton)
                    {
//                      bitPlanMask.fill(Qt::color0);
//                      pixPlan.fill(white);
//                      __showLayer();
                      // temporärer Weg
                      if(planning == 3)
                        {
                          //verschieben
                          tempTaskPointList.insert(moveWPindex,new Waypoint);
                          tempTaskPointList.at(moveWPindex)->name = "";
                          QPoint tmp(_globalMapMatrix.mapToWgs(event->pos()));
                          tempTaskPointList.at(moveWPindex)->origP = WGSPoint(tmp.y(), tmp.x());
                          tempTaskPointList.at(moveWPindex)->projP =
                                _globalMapMatrix.wgsToMap(tempTaskPointList.at(moveWPindex)->origP);
                        }
                      else
                        {
                          //anhängen
                          tempTaskPointList.append(new Waypoint);
                          tempTaskPointList.last()->name = "test";
                          QPoint tmp(_globalMapMatrix.mapToWgs(event->pos()));
                          tempTaskPointList.last()->origP = WGSPoint(tmp.y(), tmp.x());
                          tempTaskPointList.last()->projP =
                                _globalMapMatrix.wgsToMap(tempTaskPointList.last()->origP);
                        }


                    tempTask.setWaypointList(tempTaskPointList);
//                    __drawPlannedTask(false);
                    emit showTaskText(&tempTask);

                  }
                prePlanPos = point;
              }
            else
              {
                // außerhalb der Karte
                prePlanPos.setX(-999);
                prePlanPos.setY(-999);
              }
          }
      }

    //      emit showTaskText(task,_globalMapMatrix.mapToWgs(point));

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

      BaseFlightElement *f = _globalMapContents.getFlight();

      if(f)
        {
          flightPoint cP;
          int index;
          if ((index = f->searchPoint(event->pos(), cP)) != -1)
            {
              emit showFlightPoint(_globalMapMatrix.mapToWgs(event->pos()), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
          else
            {
              emit showPoint(_globalMapMatrix.mapToWgs(event->pos()));
            } 
        }
      else
        {
          emit showPoint(_globalMapMatrix.mapToWgs(event->pos()));
          prePos.setX(-50);
          prePos.setY(-50);
        }

    }

    __findElevation(current);

    if (dragZoomRect){
        QPainter zoomPainter;
        zoomPainter.begin(this);
        zoomPainter.setRasterOp( Qt::XorROP );

        double width = event->pos().x() - beginDrag.x();
        double height = event->pos().y() - beginDrag.y();
        const double widthRatio = ABS(width / this->width());
        const double heightRatio = ABS(height / this->height());

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
          
       
        zoomPainter.setPen(QPen(QColor(255, 255, 255), 1, DashLine));
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
    if ((!dragZoomRect) && (event->state() == LeftButton)){
      //start dragZoom
      setCursor(CrossCursor);
      isZoomRect = true;
      beginDrag = event->pos();
      sizeDrag = QPoint(0,0);
      dragZoomRect=true;
    }
    
}

void Map::__displayMapInfo(const QPoint& current, bool automatic)
{
  /*
   * Glider airfields first, if there exist any
   */
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  BaseFlightElement *baseFlight = _globalMapContents.getFlight();

  SinglePoint *hitElement;

  QPoint sitePos;
  // Radius for Mouse Snapping
  double delta(16.0);

  int timeout=60000;
  if (automatic) timeout=3500;

  QString text;
  
  bool show = false, isAirport = false;

  for(unsigned int loop = 0;
      loop < _globalMapContents.getListLength(
		      MapContents::GliderList); loop++)
    {
      hitElement = (SinglePoint*)_globalMapContents.getElement(
          MapContents::GliderList, loop);
      sitePos = hitElement->getMapPosition();

      double dX = abs (sitePos.x() - current.x());
      double dY = abs (sitePos.y() - current.y());

      // Abstand entspricht der Icon-Größe.
      if ( ( dX < delta ) && ( dY < delta ) )
      {
        text += hitElement->getInfoString();
        // Text anzeigen
        WhatsThat * box=new WhatsThat(this, text, this, "", timeout, &current);
        box->show();
        isAirport = true;
      }
    }

//          text = "";    // Wir wollen _nur_ Flugplätze anzeigen!

  if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

  for(unsigned int loop = 0;
      loop < _globalMapContents.getListLength(
		          MapContents::AirportList); loop++)
    {
      hitElement = (SinglePoint*)_globalMapContents.getElement(
          MapContents::AirportList, loop);
      sitePos = hitElement->getMapPosition();

      double dX = abs (sitePos.x() - current.x());
      double dY = abs (sitePos.y() - current.y());

      // Abstand entspricht der Icon-Größe.
      if ( ( dX < delta ) && ( dY < delta ) )
      {
        text += hitElement->getInfoString();
        // Text anzeigen
        WhatsThat * box=new WhatsThat(this, text, this, "", timeout, &current);
        box->show();

        isAirport = true;
      }
    }

  // let's show waypoints
  for (QPtrListIterator<Waypoint> it (*(_globalMapContents.getWaypointList())); it.current(); ++it)
  {
    Waypoint* wp = it.current();
    QPoint sitePos (_globalMapMatrix.map(_globalMapMatrix.wgsToMap(wp->origP)));
    double dX = abs(sitePos.x() - current.x());
    double dY = abs(sitePos.y() - current.y());

    // Abstand entspricht der Icon-Größe.
    if ( (dX < delta) && (dY < delta) )
    {
      QString wpText = "<B>"+i18n("Waypoint:")+"</B><UL>";
      wpText += "<B>" + wp->name +
                  "</B><BR>" +
                  printPos(wp->origP.lat()) + "<BR>" +
                  printPos(wp->origP.lon(), false);
      wpText += "</UL>";
      text += wpText;
      // Text anzeigen
      WhatsThat * box=new WhatsThat(this, wpText, this, "", timeout, &current);
      box->show();

      isAirport = true;
    }
  }

  if(baseFlight && baseFlight->getTypeID() == BaseMapElement::Flight)
    {
      QPtrList<Waypoint> wpList = baseFlight->getWPList();

      delta = 25;
      bool isWP = false;
      QString wpText;
      wpText = "<B>Waypoint:</B><UL>";

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          sitePos = _globalMapMatrix.map(wpList.at(loop)->projP);

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
                  tmpText = i18n("Sector 1");
                }
              else if(wpList.at(loop)->sector2 != 0)
                {
                  timeText = printTime(wpList.at(loop)->sector2);
                  tmpText = i18n("Sector 2");
                }

              else if(wpList.at(loop)->sectorFAI != 0)
                {
                  timeText = printTime(wpList.at(loop)->sectorFAI);
                  tmpText = i18n("FAI-Sector");
                }
              else
                {
                  timeText = "&nbsp;" + i18n("not reached");
                }

              switch(wpList.at(loop)->type)
                {
                  case FlightTask::TakeOff:
                    tmpText = i18n("Take Off");
                    break;
                  case FlightTask::Begin:
                    tmpText = i18n("Begin of task");
                    break;
                  case FlightTask::End:
                    tmpText = i18n("End of task");
                    break;
                  case FlightTask::Landing:
                    tmpText = i18n("Landing");
                    break;
                }

              wpText = wpText + "<LI><B>" + wpList.at(loop)->name +
                  "</B>  " +
                  "&nbsp;" + timeText + " / " + tmpText + "<BR>" +
                  printPos(wpList.at(loop)->origP.lat()) + " / " +
                  printPos(wpList.at(loop)->origP.lon(), false) + "</LI>";
            }
        }

      if(isWP)
        {
          wpText += "</UL>";
          // Show text
          WhatsThat * box=new WhatsThat(this, wpText, this, "", timeout, &current);
          box->show();
          isAirport = true;
        }
    }

  if(isAirport)  return;

  text += "<B>" + i18n("Airspace-Structure") + ":</B><UL>";

  for(unsigned int loop = 0; loop < airspaceRegList->count(); loop++)
    {
      if(airspaceRegList->at(loop)->contains(current))
        {
          text += "<LI>" + ((Airspace*)_globalMapContents.getElement(
              MapContents::AirspaceList, loop))->getInfoString() + "</LI>";
          show = true;
        }
    }
  text += "</UL>";

  if(show)
    {
      //  Show text
      WhatsThat * box=new WhatsThat(this, text, this, "", timeout, &current);
      box->show();
    }
}

void Map::__graphicalPlanning(const QPoint& current, QMouseEvent* event)
{
  extern MapContents _globalMapContents;

  BaseFlightElement *baseFlight = _globalMapContents.getFlight();
  if(baseFlight == NULL)  return;

  QPtrList<Waypoint> taskPointList = baseFlight->getWPList();
  QPtrList<Waypoint> tempTaskPointList = baseFlight->getWPList();
  QPtrList<Waypoint> * wpList = _globalMapContents.getWaypointList();
  Waypoint wp, *w;
  QString text;
  bool found;

  // is the point already in the flight?
  found  = __getTaskWaypoint(current, &wp, taskPointList);

  if (!found) 	// try the wpcatalog
	found = __getTaskWaypoint(current, &wp, *wpList);

  if(!taskPointList.isEmpty() && event->state() == QEvent::ControlButton)
    {
      // gleicher Punkt --> löschen
      for(unsigned int n = taskPointList.count() - 1; n > 0; n--)
        {
          if(wp.projP == taskPointList.at(n)->projP)
            {
//              warning("lösche Punkt %d", n);
              taskPointList.remove(n);
            }
        }
     }

  if(event->button() == RightButton && event->state() == ControlButton)
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
              for(unsigned int n = 0; n < taskPointList.count(); n++)
                {
                  if (wp.projP == taskPointList.at(n)->projP)
                    {
                      warning("verschiebe Punkt %d", n);
                      taskPointList.remove(n);
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
          QPoint p = _globalMapMatrix.mapToWgs(current);

          // initialize dialg
          waypointDlg->setWaypointType(BaseMapElement::Landmark);
          waypointDlg->longitude->setText(printPos(p.x(), false));
          waypointDlg->latitude->setText(printPos(p.y(), true));
          waypointDlg->setSurface(-1);

          if (waypointDlg->exec() == QDialog::Accepted)
            {
              if (waypointDlg->name->text().isEmpty())
                {
                   // Perhaps we should add this feature to the Waypoint Dialog
                   w->name = i18n("New WAYPOINT");
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
              w->origP.setLat(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
              w->origP.setLon(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
              w->projP = _globalMapMatrix.wgsToMap(w->origP.lat(), w->origP.lon());
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
      pixPlan.fill(white);
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
warning("key Release");

  if(event->state() == QEvent::ShiftButton)
    {
      warning("key Release");
      __showLayer();
    }
}

void Map::mouseReleaseEvent(QMouseEvent* event)
{
  extern MapMatrix _globalMapMatrix;
  if (isZoomRect)
    {
      dragZoomRect=false;
      isZoomRect=false; 
      __setCursor();

      if(abs(beginDrag.x()-event->pos().x())>10 && // don't zoom if rect is too small
            abs(beginDrag.y()-event->pos().y())>10 && event->button()==LeftButton) // or other than left button was pressed
        {
          double width = event->pos().x() - beginDrag.x();
          double height = event->pos().y() - beginDrag.y();
          const double widthRatio = ABS(width / this->width());
          const double heightRatio = ABS(height / this->height());

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
            _globalMapMatrix.centerToRect(QRect(QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height), beginDrag), QSize(0,0), false);
          } else {
            _globalMapMatrix.centerToRect(QRect(beginDrag, QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height)), QSize(0,0), false);
          }
          _globalMapMatrix.createMatrix(this->size());
          __redrawMap();
          emit changed(this->size());
        } else {
          //we are not going to zoom, but we still need to clean up our mess!
          QPainter zoomPainter;
          zoomPainter.begin(this);
          zoomPainter.setRasterOp( Qt::XorROP );
          zoomPainter.setPen(QPen(QColor(255, 255, 255), 1, DashLine));
          // Delete the old rectangle:
          zoomPainter.drawRect( beginDrag.x(), beginDrag.y(), sizeDrag.x(), sizeDrag.y());
        }   
    }
}

void Map::mousePressEvent(QMouseEvent* event)
{
  // First: delete the cursor, if visible:
  if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
          prePos.x() - 20, prePos.y() - 20, 40, 40);

  if (isZoomRect){ // Zooming
    beginDrag = event->pos();
    sizeDrag = QPoint(0,0);
    dragZoomRect=true;
  }
  else
  {
    extern MapContents _globalMapContents;
    extern MapMatrix _globalMapMatrix;

    const QPoint current(event->pos());

    RadioPoint *hitElement;
    QString text;

    QPoint sitePos;
    double dX, dY, delta(16.0);

    if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

    if(event->button() == MidButton)
      {
        // Move Map
        _globalMapMatrix.centerToPoint(event->pos());
        _globalMapMatrix.createMatrix(this->size());

        __redrawMap();
      }

    else if(event->button() == LeftButton)
      {
        if(event->state() == QEvent::ShiftButton)
          {
            // select WayPoint
            QRegExp blank("[ ]");
            bool found = false;

            // add WPList !!!
            int searchList[] = {MapContents::GliderList, MapContents::AirportList};
            for (int l = 0; l < 2; l++)
              {
             	  for(unsigned int loop = 0;
             	      loop < _globalMapContents.getListLength(searchList[l]); loop++)
             	    {
          	        hitElement = (RadioPoint*)_globalMapContents.getElement(
          	            searchList[l], loop);
            	      sitePos = hitElement->getMapPosition();

                    dX = abs(sitePos.x() - current.x());
                    dY = abs(sitePos.y() - current.y());

    	              // Abstand entspricht der Icon-Größe.
          	        if (dX < delta && dY < delta)
          	          {
            	          Waypoint *w = new Waypoint;
    	                  w->name = hitElement->getName().replace(blank, QString::null).left(6).upper();
                        w->description = hitElement->getName();
            	          w->type = hitElement->getTypeID();
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

    	      if (!found)
    	        {
                warning("new waypoint");

                WaypointDialog *waypointDlg = new WaypointDialog(this);
                emit regWaypointDialog(waypointDlg); //register the dialog and connect it's signals.
                
                waypointDlg->enableApplyButton(false);

                QPoint p = _globalMapMatrix.mapToWgs(current);

                // initialize dialog
                //waypointDlg->setWaypointType(BaseMapElement::Landmark); now set by default.
                waypointDlg->longitude->setText(printPos(p.x(), false));
                waypointDlg->latitude->setText(printPos(p.y(), true));
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


    else if(event->button() == RightButton && event->state() == ControlButton)
      {
        moveWPindex = -999;

        prePlanPos.setX(-999);
        prePlanPos.setY(-999);
        planning = 2;

        emit taskPlanningEnd();
        return;
      }
    else if(event->button() == RightButton)  {
        popupPos=event->pos();
        __showPopupMenu(event);
    }
      
  }
}

void Map::paintEvent(QPaintEvent* event = 0)
{
  if(event == 0)
      bitBlt(this, 0, 0, &pixBuffer);
  else
      bitBlt(this, event->rect().topLeft(), &pixBuffer, event->rect());

  /* Cursor-Position zurücksetzen! */
  prePos.setX(-50);
  prePos.setY(-50);
}

void Map::__drawGrid()
{
  extern const MapMatrix _globalMapMatrix;
  const QRect mapBorder = _globalMapMatrix.getViewBorder();

  QPainter gridP(&pixGrid);
  gridP.setBrush(NoBrush);

  // die Kanten des Bereichs
  const int lon1 = mapBorder.left() / 600000 - 1;
  const int lon2 = mapBorder.right() / 600000 + 1;
  const int lat1 = mapBorder.top() / 600000 + 1;
  const int lat2 = mapBorder.bottom() / 600000 - 1;

  // Step between two degree-lines (in 1/60 degree)
  int step = 60;
  int gridStep = 1;
  int lineWidth = 1;

  switch(_globalMapMatrix.getScaleRange())
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
  for(int loop = 0; loop < (lat1 - lat2 + 1) ; loop += gridStep)
	  {
	    int size = (lon2 - lon1 + 1) * 10;
  	  QPointArray pointArray(size);

	    for(int lonloop = 0; lonloop < size; lonloop++)
	      {
	        cP = _globalMapMatrix.wgsToMap( ( lat2 + loop ) * 600000,
              (int)( lon1 + ( lonloop * 0.1 ) ) * 600000);
	        pointArray.setPoint(lonloop, cP);
  	    }

	    // Draw the small lines between:
	    int number = (int) (60.0 / step);
  	  for(int loop2 = 1; loop2 < number; loop2++)
	      {
	        QPointArray pointArraySmall(size);

  	      for(int lonloop = 0; lonloop < size; lonloop++)
            {
              cP = _globalMapMatrix.wgsToMap(
                  (int)( lat2 + loop + ( loop2 * ( step / 60.0 ) ) ) * 600000,
                  (int)(lon1 + (lonloop * 0.1)) * 600000);
              pointArraySmall.setPoint(lonloop, cP);
            }
  	
          if(loop2 == (number / 2.0))
    		    {
        		  gridP.setPen(QPen(QColor(0,0,0), 1, DashLine));
        		  gridP.drawPolyline(_globalMapMatrix.map(pointArraySmall));
        		}
	        else
        		{
        		  gridP.setPen(QPen(QColor(0,0,0), 1, DotLine));
        		  gridP.drawPolyline(_globalMapMatrix.map(pointArraySmall));
        		}
  	    }
	    // Draw the main lines
  	  gridP.setPen(QPen(QColor(0,0,0), 1));
	    gridP.drawPolyline(_globalMapMatrix.map(pointArray));
  	}

  // Now the longitudes:
  for(int loop = lon1; loop <= lon2; loop += gridStep)
	  {
  	  cP = _globalMapMatrix.wgsToMap(lat1 * 600000, (loop * 600000));
	    cP2 = _globalMapMatrix.wgsToMap(lat2 * 600000, (loop * 600000));

  	  // Draw the main longitudes:
	    gridP.setPen(QPen(QColor(0,0,0), 1));
	    gridP.drawLine(_globalMapMatrix.map(cP), _globalMapMatrix.map(cP2));

  	  // Draw the small lines between:
	    int number = (int) (60.0 / step);

  	  for(int loop2 = 1; loop2 < number; loop2++)
	      {
	        cP = _globalMapMatrix.wgsToMap((lat1 * 600000),
			  	    (int)((loop + (loop2 * step / 60.0)) * 600000));

  	      cP2 = _globalMapMatrix.wgsToMap((lat2 * 600000),
					      (int)((loop + (loop2 * step / 60.0)) * 600000));

  	      if(loop2 == (number / 2.0))
              gridP.setPen(QPen(QColor(0,0,0), 1, DashLine));
	        else
              gridP.setPen(QPen(QColor(0,0,0), lineWidth, DotLine));
	
          gridP.drawLine(_globalMapMatrix.map(cP), _globalMapMatrix.map(cP2));
  	    }
	  }
  gridP.end();
}

void Map::__drawMap()
{
  QPainter aeroP(&pixAero);
  QPainter airSpaceP(&pixAirspace);
  QPainter uMapP(&pixUnderMap);
  QPainter isoMapP(&pixIsoMap);
  QPainter mapMaskP(&bitMapMask);
  QPainter airspaceMaskP(&bitAirspaceMask);

  airspaceRegList->clear();

  extern MapContents _globalMapContents;

  pixIsoMap.fill(QColor(96,128,248));

  _globalMapContents.drawIsoList(&isoMapP, &mapMaskP);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::TopoList);

  mainApp->slotSetProgress(5);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::CityList);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::HydroList);

  mainApp->slotSetProgress(10);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::RoadList);

  mainApp->slotSetProgress(15);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::HighwayList);

  mainApp->slotSetProgress(20);

  mainApp->slotSetProgress(25);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::RailList);

  mainApp->slotSetProgress(30);

  mainApp->slotSetProgress(35);

  mainApp->slotSetProgress(40);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::PopulationList);

  mainApp->slotSetProgress(45);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::LandmarkList);

  mainApp->slotSetProgress(50);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::ObstacleList);

  mainApp->slotSetProgress(55);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::ReportList);

  mainApp->slotSetProgress(60);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::NavList);

  mainApp->slotSetProgress(65);

  Airspace* currentAirS;
  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
              MapContents::AirspaceList); loop++)
    {
      currentAirS = (Airspace*)_globalMapContents.getElement(
          MapContents::AirspaceList, loop);
        // We should only add airspaces to the list that really have been
        // drawn. Searching will be faster.
      airspaceRegList->append(currentAirS->drawRegion(&airSpaceP,
            &airspaceMaskP));
    }

  mainApp->slotSetProgress(70);

  mainApp->slotSetProgress(75);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::AirportList);

  mainApp->slotSetProgress(80);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::AddSitesList);

  mainApp->slotSetProgress(85);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::OutList);

  mainApp->slotSetProgress(90);

  _globalMapContents.drawList(&aeroP, &mapMaskP, MapContents::GliderList);

  mainApp->slotSetProgress(95);

  // Closing the painter ...
  aeroP.end();
  airSpaceP.end();
  uMapP.end();
  airspaceMaskP.end();
}

void Map::__drawFlight()
{
  QPainter flightP(&pixFlight);
  QPainter flightMaskP(&bitFlightMask);

  extern MapContents _globalMapContents;

  _globalMapContents.drawList(&flightP, &flightMaskP, MapContents::FlightList);

  flightP.end();
  flightMaskP.end();
}

void Map::__drawPlannedTask(bool solid)
{
//warning("Map::__drawPlannedTask()");
//  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;

  QPainter planP;
  FlightTask* task;

  if(solid)
      task = (FlightTask*)_globalMapContents.getFlight();
  else {
    task = &tempTask;
    task->reProject();
  }   

  if(task && task->getTypeID() == BaseMapElement::Task)
    {
      QPtrList<Waypoint> WPList = task->getWPList();

      // Strecke zeichnen
      if(solid)
          planP.begin(&pixPlan);
      else
          planP.begin(this);

      QPainter planPMask(&bitPlanMask);
      bitPlanMask.fill(Qt::color0);
      bitFlightMask.fill(Qt::color0);
      pixPlan.fill(white);
      pixFlight.fill(white);

      QPen drawP(QColor(170,0,0), 5);
      drawP.setJoinStyle(Qt::MiterJoin);
      planP.setBrush(NoBrush);
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
  if(!event->size().isEmpty())
    {
    
      extern MapMatrix _globalMapMatrix;
      _globalMapMatrix.createMatrix(event->size());
      pixBuffer.resize(event->size());
      pixBuffer.fill(white);

      __redrawMap();
    }
  emit changed(event->size());
}

void Map::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QTextDrag::canDecode(event));
}

void Map::dropEvent(QDropEvent* event)
{
  QStringList dropList;

  if(QUriDrag::decodeToUnicodeUris(event, dropList))
    {
      for(QStringList::Iterator it = dropList.begin();
              it != dropList.end(); it++)
          mainApp->slotFileOpenRecent((*it).latin1());
    }
}

void Map::__redrawMap()
{
//  qWarning("__redrawMap()");
  // Statusbar not set "geniously" so far...
  mainApp->slotSetProgress(0);

  pixPlan.fill(white);

  pixAero.fill(white);
  pixAirspace.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(black);
  pixIsoMap.fill(white);
  pixFlight.fill(white);
  pixWaypoints.fill(white);

  bitMapMask.fill(Qt::color0);
  bitFlightMask.fill(Qt::color0);
  bitPlanMask.fill(Qt::color0);
  bitAirspaceMask.fill(Qt::color0);
  bitWaypointsMask.fill(Qt::color0);

  QPoint temp1(preCur1);
  QPoint temp2(preCur2);

  preCur1.setX(-50);
  preCur2.setX(-50);

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();
  __drawFlight();
  __drawWaypoints();
//  __drawPlannedTask();
  // Linie zum aktuellen Punkt löschen
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);  

  __showLayer();

  mainApp->slotSetProgress(100);

  slotDrawCursor(temp1,temp2);
}

/** Save Map to PNG-file with width,heigt. Use actual size if width=0 & height=0 */
void Map::slotSavePixmap(KURL fUrl, int width, int height){

  int w_orig,h_orig;
  extern MapContents _globalMapContents;

  if(fUrl.isEmpty())  return;

  QString fName;
  if(fUrl.isLocalFile())
      fName = fUrl.path();
  else
    {
      KNotifyClient::event(i18n("Can not save to file %1").arg(fUrl.url()));
      return;
    }
	if (width && height){
	  w_orig=pixBuffer.width();
	  h_orig=pixBuffer.height();
	  resize(width,height);
  	slotCenterToFlight(); 	
  }
	
  KConfig* config = KGlobal::config();
  config->setGroup("CommentSettings");
  if (config->readBoolEntry("ShowComment"))
  {
  	Flight* flight = (Flight*)_globalMapContents.getFlight();
    QPainter bufferP(&pixBuffer);
    bufferP.setPen( Qt::white );
    QFont font;
    int by=pixBuffer.height()-35;
    int bw=pixBuffer.width()-10;
    QString text=i18n("%1 with %2 (%3) on %4").arg(flight->getPilot()).arg(flight->getType()).arg(flight->getID()).arg(flight->getDate().toString());
    bufferP.setFont(font);
  	bufferP.drawText(10,by+15,bw,25,AlignLeft,i18n("created by KFLog (www.kflog.org)"));
    font.setBold(true);
    font.setPointSize( 18 );
    bufferP.setFont(font);
  	bufferP.drawText(10,by,bw,25,AlignLeft,text);
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
  KFileDialog* dlg = new KFileDialog(0, "*.png *.PNG", this,
      i18n("Select PNG-File"), true);
  dlg->exec();
  slotSavePixmap(dlg->selectedURL(),0,0);
}

void Map::slotRedrawFlight()
{
  pixFlight.fill(white);
  bitFlightMask.fill(Qt::color0);
  __drawFlight();
  __showLayer();
}

void Map::slotRedrawMap()
{
  qWarning("slotRedrawMap()");
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.createMatrix(this->size());

  emit changed(this->size());

  __redrawMap();
}

void Map::slotActivatePlanning()
{
  if(planning != 1)
    {
      planning = 1;

      pixPlan.fill(white);
      prePlanPos.setX(-999);
      prePlanPos.setY(-999);
warning("start");
      mainApp->slotStatusMsg(i18n("To finish the planing, press <STRG> and the right mouse button!"));
    }
  else
    {
      // Planen "ausschalten"
      planning = 0;
      __showLayer();
      mainApp->slotStatusMsg("");
      emit taskPlanningEnd();
warning("ende");
/*	  pixPlan.fill(white);
  	prePlanPos.setX(-999);
	  prePlanPos.setY(-999);
*/
    }
}


void Map::__showLayer()
{
  pixUnderMap.setMask(bitMapMask);
  pixFlight.setMask(bitFlightMask);
  pixPlan.setMask(bitPlanMask);
  pixWaypoints.setMask(bitWaypointsMask);

  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAero, 0, 0, -1, -1, NotEraseROP);

  pixAirspace.setMask(bitAirspaceMask);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace);

  bitBlt(&pixBuffer, 0, 0, &pixFlight);
  bitBlt(&pixBuffer, 0, 0, &pixPlan);
  bitBlt(&pixBuffer, 0, 0, &pixWaypoints);
  bitBlt(&pixBuffer, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);

  paintEvent();
}

void Map::slotDrawCursor(const QPoint& p1, const QPoint& p2)
{

//warning("Map::slotDrawCursor");

  extern const MapMatrix _globalMapMatrix;

  QPoint pos1(_globalMapMatrix.map(p1)), pos2(_globalMapMatrix.map(p2));

  QPoint prePos1(_globalMapMatrix.map(preCur1)),
         prePos2(_globalMapMatrix.map(preCur2));

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
  setCursor(CrossCursor);
  isZoomRect=true;
}

void Map::slotCenterToWaypoint(const unsigned int id)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  if(id >= _globalMapContents.getFlight()->getWPList().count())
    {
      warning("KFLog: Map::slotCenterToWaypoint: wrong Waypoint-ID");
      return;
    }

  _globalMapMatrix.centerToPoint(_globalMapMatrix.map(
      _globalMapContents.getFlight()->getWPList().at(id)->projP));
  _globalMapMatrix.slotSetScale(_globalMapMatrix.getScale(MapMatrix::LowerLimit));

  emit changed(this->size());
}

void Map::slotCenterToFlight()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  unsigned int i;

  Flight *f = (Flight *)_globalMapContents.getFlight();
  if (f) {
    QRect r;
    QRect r2;
    QPtrList<Flight> fl;

    switch (f->getTypeID()) {
      case BaseMapElement::Flight:
        r = f->getFlightRect();
        break;
      case BaseMapElement::FlightGroup:
        fl = ((FlightGroup *)f)->getFlightList();
        r = fl.at(0)->getFlightRect();
        for (i = 1; i < fl.count(); i++) {
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
      _globalMapMatrix.centerToRect(r);
      _globalMapMatrix.createMatrix(this->size());

      __redrawMap();
    }

  emit changed(this->size());
  }
}

void Map::slotCenterToTask()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  unsigned int i;
  BaseFlightElement *f = _globalMapContents.getFlight();

  if(f)
    {
      QRect r;
      QRect r2;
      QPtrList<Flight> fl;

      switch (f->getTypeID())
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
            for (i = 1; i < fl.count(); i++) {
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
          _globalMapMatrix.centerToRect(r);
          _globalMapMatrix.createMatrix(this->size());

          __redrawMap();

          emit changed(this->size());
        }
    }
}

/**
 * Animation slot.
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStart()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  QPoint pos;
  QPixmap pix;
  flightPoint cP;

  Flight *f = (Flight *)_globalMapContents.getFlight();
  if(f)
    {
  	  // save this one to speed up timeout code
      flightToAnimate = f;

      switch(f->getTypeID())
        {
          case BaseMapElement::Flight:
            f->setAnimationIndex(0);
            f->setAnimationActive(true);
            break;
          case BaseMapElement::FlightGroup:
		        // loop through all and set animation index to start
      		  QPtrList<Flight> flightList = ((FlightGroup *)f)->getFlightList();
  		      for(unsigned int loop = 0; loop < flightList.count(); loop++)
              {
          		  f = flightList.at(loop);
                f->setAnimationIndex(0);
                f->setAnimationActive(true);
              }
            break;
      	}
    	// force redraw
    	// flights will not be visible as nAnimationIndex is zero for all flights to animate.
    	slotRedrawFlight();

    	// prepare the pixmap for next timeout
//	    __drawFlight();
//      __showLayer();

    	// save what will be under the flag
      switch(f->getTypeID())
        {
          case BaseMapElement::Flight:
            cP = f->getPoint(0);
           	prePos = _globalMapMatrix.map(cP.projP);
           	pos = _globalMapMatrix.map(cP.projP);
            pix = f->getLastAnimationPixmap();
       	   	bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
            f->setLastAnimationPos(pos);
            f->setLastAnimationPixmap(pix);
   	      	// put flag
         		bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2 );
           break;
          case BaseMapElement::FlightGroup:
		        // loop through all and set animation index to start
      		  QPtrList<Flight> flightList = ((FlightGroup *)f)->getFlightList();
    	    	for(unsigned int loop = 0; loop < flightList.count(); loop++)
              {
                cP = f->getPoint(0);
                pos = _globalMapMatrix.map(cP.projP);
                pix = f->getLastAnimationPixmap();
                bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
                f->setLastAnimationPos(pos);
                f->setLastAnimationPixmap(pix);
                // put flag
                bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2 );
          		}
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
  extern MapMatrix _globalMapMatrix;
  flightPoint cP; //, prevP;
  Flight *f  = this->flightToAnimate; // = (Flight *)_globalMapContents.getFlight();
  bool bDone = true;
  QPoint lastpos, pos;
  QPixmap pix;

  if(f)
    {
      switch(f->getTypeID())
        {
          case BaseMapElement::Flight:
            f->setAnimationNextIndex();
            if (f->isAnimationActive())
	              bDone = false;
            //write info from current point on statusbar
            cP = f->getPoint((f->getAnimationIndex()));
            pos = _globalMapMatrix.map(cP.projP);
            lastpos = f->getLastAnimationPos();
            pix = f->getLastAnimationPixmap();
            emit showFlightPoint(_globalMapMatrix.mapToWgs(pos), cP);
            // erase prev indicator-flag
            bitBlt(&pixBuffer, lastpos.x(), lastpos.y()-32, &pix);

//            bitBlt(&pixBuffer, lastpos.x()-100, lastpos.y()-32, &pix);
          	// redraw flight up to this point, blt the pixmap onto the already created pixmap
            __drawFlight();
            pixFlight.setMask(bitFlightMask);
            bitBlt(&pixBuffer, 0, 0, &pixFlight);
            //save for next timeout
            bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
            f->setLastAnimationPixmap(pix);
            f->setLastAnimationPos(pos);
            // add indicator-flag
            bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2);
            break;
          case BaseMapElement::FlightGroup:
		        // loop through all and set animation index to start
      		  QPtrList<Flight> flightList = ((FlightGroup*)flightToAnimate)->getFlightList();
        		for(unsigned int loop = 0; loop < flightList.count(); loop++)
              {
          		  f = flightList.at(loop);
                f->setAnimationNextIndex();
                if (f->isAnimationActive())
 		                bDone = false;
                //write info from current point on statusbar
                cP = f->getPoint((f->getAnimationIndex()));
                pos = _globalMapMatrix.map(cP.projP);
                lastpos = f->getLastAnimationPos();
                pix = f->getLastAnimationPixmap();
                emit showFlightPoint(_globalMapMatrix.mapToWgs(pos), cP);
                // erase prev indicator-flag
                bitBlt(&pixBuffer, lastpos.x(), lastpos.y()-32, &pix);
              	// redraw flight up to this point, blt the pixmap onto the already created pixmap
                __drawFlight();
                pixFlight.setMask(bitFlightMask);
                bitBlt(&pixBuffer, 0, 0, &pixFlight);
                //save for next timeout
                bitBlt(&pix, 0, 0, &pixBuffer, pos.x(), pos.y()-32, 32, 32, CopyROP);
                f->setLastAnimationPixmap(pix);
                f->setLastAnimationPos(pos);
                // add indicator-flag
                bitBlt(&pixBuffer, pos.x(), pos.y()-32, &pixCursor2);
              }
            break;
    		}
    }
  // force paint event
  paintEvent();

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
  extern MapContents _globalMapContents;
  Flight *f = (Flight *)_globalMapContents.getFlight();
  QPtrList<Flight> flightList;

  if(!f) return;

  switch (f->getTypeID())
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
	    for(unsigned int loop = 0; loop < flightList.count(); loop++)
          flightList.at(loop)->setAnimationActive(false);

      slotRedrawFlight();
    }
}

/**
 * Stepping slots.
 */
void Map::slotFlightNext()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents.getFlight();

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
			  prePos = _globalMapMatrix.map(cP.projP);
			  if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
				  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
     				_globalMapMatrix.centerToPoint(prePos);
     	            _globalMapMatrix.createMatrix(this->size());

     			    __redrawMap();
              }			 		
              emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightPrev()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents.getFlight();

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
			  prePos = _globalMapMatrix.map(cP.projP);
			  if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
				  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
     				_globalMapMatrix.centerToPoint(prePos);
     	            _globalMapMatrix.createMatrix(this->size());

     			    __redrawMap();
              }		
              emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}
void Map::slotFlightStepNext()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents.getFlight();

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
			  prePos = _globalMapMatrix.map(cP.projP);
			  if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
				  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
     				_globalMapMatrix.centerToPoint(prePos);
     	            _globalMapMatrix.createMatrix(this->size());

     			    __redrawMap();
              }			 		
              emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightStepPrev()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents.getFlight();

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
			  prePos = _globalMapMatrix.map(cP.projP);
			  if ((prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
				  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) ){
     				_globalMapMatrix.centerToPoint(prePos);
     	            _globalMapMatrix.createMatrix(this->size());

     			    __redrawMap();
              }			 		
              emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightHome()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents.getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
          if ((index = f->searchGetNextPoint(0, cP)) != -1)
            {
              emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix.map(cP.projP);
              preIndex = index;
              bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
            }
        }
    }
}

void Map::slotFlightEnd()
{
  extern MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  flightPoint cP;
  int index;
  BaseFlightElement *f = _globalMapContents.getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if crosshair is shown in map.
        {
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);

          // just a workaround !!!!!!!!!!!!!
          if(f->getTypeID() == BaseMapElement::Flight)
            {
              if((index = f->searchGetNextPoint(((Flight *)f)->getRouteLength()-1, cP)) != -1)
                {
                  emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
                  prePos = _globalMapMatrix.map(cP.projP);
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
  extern MapContents _globalMapContents;
  BaseFlightElement *f = _globalMapContents.getFlight();
  // just to make sure ...
  slotAnimateFlightStop();

  planning = 0;

  if (f && f->getTypeID() == BaseMapElement::Task) {
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
      switch(f->getTypeID())
        {
          case BaseMapElement::Flight:
            // fall through
          case BaseMapElement::FlightGroup:
            slotCenterToFlight();
            break;
          case BaseMapElement::Task:
            slotCenterToTask();
            break;
        }
    }
}
/** append a waypoint to the current task */
void Map::slotAppendWaypoint2Task(Waypoint *p)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  FlightTask *f = (FlightTask *)_globalMapContents.getFlight();
  if(f && f->getTypeID() == BaseMapElement::Task && planning)
    {
      QPtrList<Waypoint> taskPointList = f->getWPList();
      p->projP = _globalMapMatrix.wgsToMap(p->origP);
      taskPointList.append(p);
      f->setWaypointList(taskPointList);
      __drawPlannedTask(true);
      __showLayer();
    }
}

/** search for a waypoint
First look in task itself
Second look in map contents */
bool Map::__getTaskWaypoint(const QPoint& current, Waypoint *wp, QPtrList<Waypoint> &taskPointList)
{
  unsigned int i;
  Waypoint *tmpPoint;
  QPoint sitePos;
  double dX, dY;
  // Radius for Mouse Snapping
  double delta(16.0);
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  bool found = false;
  RadioPoint *hitElement;

  for(i = 0; i < taskPointList.count(); i++)
    {
      tmpPoint = taskPointList.at(i);
      sitePos = _globalMapMatrix.map(_globalMapMatrix.wgsToMap(tmpPoint->origP));
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
      QArray<int> contentArray(2);
      contentArray.at(0) = MapContents::GliderList;
      contentArray.at(1) = MapContents::AirportList;

      for(unsigned int n = 0; n < contentArray.count(); n++)
        {
          for(unsigned int loop = 0; loop < _globalMapContents.getListLength(contentArray.at(n)); loop++)
            {
              hitElement = (RadioPoint*)_globalMapContents.getElement(contentArray.at(n), loop);			
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
                  wp->type = hitElement->getTypeID();
                  wp->elevation = hitElement->getElevation();
                  wp->icao = hitElement->getICAO();
                  wp->frequency = hitElement->getFrequency().toDouble();
                  wp->runway = -1;
                  wp->length = -1;
                  wp->surface = 0;
                  wp->comment = "";
                  wp->isLandable = true;

                  found = true;
                  break;
                }
            }
        }
    }
  return found;
}
/** Puts the waypoints of the active waypoint catalog to the map */
void Map::__drawWaypoints(){
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  extern MapConfig _globalMapConfig;

  QPtrList<Waypoint> * wpList;
  QPoint p;

  wpList = _globalMapContents.getWaypointList();

  QPainter wpPainter(&pixWaypoints);
  QPainter wpMaskPainter(&bitWaypointsMask);

  wpPainter.setBrush(NoBrush);
  wpPainter.setPen(QPen(QColor(0,0,0), 2, SolidLine));

  // now do complete list
  for (Waypoint* wp = wpList->first(); wp; wp = wpList->next()){
    // make sure projection is ok, and map to screen
    wp->projP = _globalMapMatrix.wgsToMap(wp->origP.lat(), wp->origP.lon());
    p = _globalMapMatrix.map(wp->projP);

    // draw marker
    wpPainter.setBrush(NoBrush);
    wpPainter.setPen(QPen(QColor(0,0,0), 2, SolidLine));
    wpPainter.drawRect(p.x() - 4,p.y() - 4, 8, 8);
    wpMaskPainter.drawRect(p.x() - 4,p.y() - 4, 8, 8);

     // draw name of wp
     if(_globalMapConfig.drawWpLabels()) {
        int xOffset;
        int yOffset;
        QRect textbox;
        textbox=wpPainter.fontMetrics().boundingRect(wp->name);
        if (wp->origP.lon()<_globalMapMatrix.getMapCenter(false).y()) {
          //the wp is on the left side of the map, so draw the textlabel on the right side
          xOffset=14;
          yOffset=-2;
          if (_globalMapConfig.useSmallIcons()){
            xOffset=6;
            yOffset=0;
          }
        } else {
          //the wp is on the right side of the map, so draw the textlabel on the left side
          xOffset=-textbox.width()-14;
          yOffset=-2;
          if (_globalMapConfig.useSmallIcons()){
            xOffset=-textbox.width()-6;
            yOffset=0;
          }
         }

        wpPainter.setPen(QPen(Qt::black, 3, SolidLine));
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
void Map::slotWaypointCatalogChanged(WaypointCatalog* c){
  extern MapContents _globalMapContents;
  Waypoint *w;
  QDictIterator<Waypoint> it(c->wpList);
  bool filterRadius, filterArea;
  QPtrList<Waypoint> * wpList;

  wpList = _globalMapContents.getWaypointList();
  wpList->clear();

  filterRadius = (c->radiusLat != 1  || c->radiusLong != 1);
  filterArea = (c->areaLat2 != 1 && c->areaLong2 != 1 && !filterRadius);

  for (w = it.toFirst(); w != 0; w = ++it) {
    if (!c->showAll) {
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
      case BaseMapElement::Glidersite:
        if (!c->showGliderSites) {
          continue;
        }
        break;
      case BaseMapElement::UltraLight:
      case BaseMapElement::HangGlider:
      case BaseMapElement::Parachute:
      case BaseMapElement::Ballon:
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

/** Tries to locate the elevation for the given point, and emits a signal elevation if found. */
void Map::__findElevation(const QPoint& coord){
  extern MapContents _globalMapContents;
  isoListEntry* entry;
  int height=0;
  
  QPtrList<isoListEntry>* list=_globalMapContents.getIsohypseRegions();

  for(int i=0; i<list->count();i++) {
    entry=list->at(i);
    if (entry->region->contains(coord)) 
      height=MAX(height,entry->height);
  }

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

/** Creates the popupmenu for the map */
void Map::__createPopupMenu(){
  extern MapMatrix _globalMapMatrix;
  
  mapPopup=new KPopupMenu(this);
  
  mapPopup->insertTitle(/*SmallIcon("task")*/ 0, i18n("Map"), 0);
  idMpAddWaypoint  = mapPopup->insertItem(SmallIcon("waypoint"), i18n("&New waypoint"), this, SLOT(slotMpNewWaypoint()));
  idMpEndPlanning = mapPopup->insertItem(i18n("&End taskplanning"), this, SLOT(slotMpEndPlanning()));
  mapPopup->insertItem(SmallIcon("info"), i18n("&Show map info..."), this, SLOT(slotMpShowMapInfo()));
  
  mapPopup->insertSeparator();
  idMpCenterMap  = mapPopup->insertItem(SmallIcon("centerto"), i18n("&Center map"), this, SLOT(slotMpCenterMap()));

  idMpZoomIn = mapPopup->insertItem(SmallIcon("viewmag+"), i18n("Zoom &In"), &_globalMapMatrix, SLOT(slotZoomIn()));
  idMpZoomOut = mapPopup->insertItem(SmallIcon("viewmag-"), i18n("Zoom &Out"), &_globalMapMatrix, SLOT(slotZoomOut()));
  /*
  idMpAddTaskPoint
 */  
}

/** Selects the correct items to show from the menu and then shows it. */
void Map::__showPopupMenu(QMouseEvent * Event){
  mapPopup->setItemEnabled(idMpEndPlanning, (planning == 1 || planning == 3));
  
  mapPopup->exec(mapToGlobal(Event->pos()));

}

/** called from the MapPopupmenu to add a new waypoint. */
void Map::slotMpNewWaypoint(){
   extern MapContents _globalMapContents;
   extern MapMatrix _globalMapMatrix;

   RadioPoint *hitElement;
   QString text;

   QPoint sitePos;
   double dX, dY, delta(16.0);
    
   const QPoint current(popupPos);

   // select WayPoint
   QRegExp blank("[ ]");
   bool found = false;

    // add WPList !!!
    int searchList[] = {MapContents::GliderList, MapContents::AirportList};
    for (int l = 0; l < 2; l++) {
      for(unsigned int loop = 0; loop < _globalMapContents.getListLength(searchList[l]); loop++) {
        hitElement = (RadioPoint*)_globalMapContents.getElement(searchList[l], loop);
        sitePos = hitElement->getMapPosition();

        dX = abs(sitePos.x() - current.x());
        dY = abs(sitePos.y() - current.y());

        // Abstand entspricht der Icon-Größe.
        if (dX < delta && dY < delta) {
          Waypoint *w = new Waypoint;
          w->name = hitElement->getName().replace(blank, QString::null).left(6).upper();
          w->description = hitElement->getName();
          w->type = hitElement->getTypeID();
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

      QPoint p = _globalMapMatrix.mapToWgs(current);

      // initialize dialog
      //waypointDlg->setWaypointType(BaseMapElement::Landmark); now set by default.
      waypointDlg->longitude->setText(printPos(p.x(), false));
      waypointDlg->latitude->setText(printPos(p.y(), true));
      waypointDlg->setSurface(-1);
      waypointDlg->exec(); //we only need to exec the dialog. The dialog can take care of itself now :-)

      delete waypointDlg;
   }
}

/** Called from the contextmenu to center the map. */
void Map::slotMpCenterMap(){
  extern MapMatrix _globalMapMatrix;
   // Move Map
  _globalMapMatrix.centerToPoint(popupPos);
  _globalMapMatrix.createMatrix(this->size());

  __redrawMap();
}

void Map::slotMpEndPlanning(){
  moveWPindex = -999;

  prePlanPos.setX(-999);
  prePlanPos.setY(-999);
  planning = 2;

  emit taskPlanningEnd();
}

void Map::slotMpShowMapInfo(){
  leaveEvent(0);
  __displayMapInfo(popupPos, false);
}

void Map::leaveEvent ( QEvent * ){
  mapInfoTimer->stop();
  mapInfoTimerStartpoint=QPoint(-999,-999);
}

void Map::slotMapInfoTimeout() {
  __displayMapInfo(mapInfoTimerStartpoint, true);

}
