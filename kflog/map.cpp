/***********************************************************************
**
**   map.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2010-2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QtGui>

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
#include "waypointdialog.h"
#include "wgspoint.h"
#include "waypointtreeview.h"
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
  QWidget(parent),
  drawFlightCursors(false),
  lastCur1Pos(-100,-100),
  lastCur2Pos(-100,-100),
  prePos(-5000, -5000),
  planning(0),
  tempTask(""),
  isDrawing(false),
  redrawRequest(false),
  preSnapPoint(-999, -999)
{
  pixCursor = QPixmap(40,40);
  pixCursor.fill(Qt::transparent);

  QPainter cursor;
  cursor.begin(&pixCursor);
  cursor.setPen(QPen(QColor(255,0,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  pixCursor1 = _mainWindow->getPixmap("flag_green.png");
  pixCursor2 = _mainWindow->getPixmap("flag_red.png");

//  pixAnimate.resize(32,32);
//  pixAnimate.fill(white);

  __setCursor();
  setMouseTracking(true);

  QPalette p = palette();
  p.setColor(backgroundRole(), QColor(255,255,255));
  setPalette(p);

  setAcceptDrops(true);

  // For Planning
  pixPlan.fill(Qt::transparent);
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);

  setWhatsThis( tr(
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
     "scale.</P>"
     "<b>Supported mouse actions at the map:</b>"
     "<ul>"
     "<li>Moving the mouse pointer at a map item will display the item data after a short delay."
     "<li>Pressing the right mouse button opens the map menu."
     "<li>Pressing the middle mouse button centers the map to the mouse point."
     "<li>Pressing the left mouse button opens the task menu, if graphical task planning is activated."
     "<li>Pressing the shift key and the left mouse button shows the data of a "
     "map item, if it lays under the mouse pointer. Otherwise the waypoint dialog "
     "is opened to create a new waypoint."
     "<li>Pressing key 0 shows a cross at the map. Press the left mouse button "
     "and hold it down during drag. A frame is drawn at the map. Release the mouse "
     "button to zoom the map into the frame."
     "</ul>"
     "</html>"
      ) );

  __createPopupMenu();

  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), this,
           SLOT(slotAnimateFlightTimeout()) );

  mapInfoTimer = new QTimer(this);
  mapInfoTimer->setSingleShot( true );
  connect (mapInfoTimer, SIGNAL(timeout()), this, SLOT(slotMapInfoTimeout()));

  /** Create a timer for queuing draw events. */
  redrawMapTimer = new QTimer(this);
  redrawMapTimer->setSingleShot(true);

  connect( redrawMapTimer, SIGNAL(timeout()), this, SLOT(slotRedrawMap()));

  isZoomActive=false;
  isDragZoomActive=false;
}

Map::~Map()
{
  qDebug() << "~Map()";
}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  const QPoint current = event->pos();

  QPoint vector = event->pos() - mapInfoTimerStartpoint;

  if( vector.manhattanLength() > 4 )
    {
      mapInfoTimer->stop();

      // don't start the timer when in planning or zoom mode
      if( planning != 1 && planning != 3 && isZoomActive == false )
        {
          mapInfoTimer->start( MAP_INFO_DELAY );
          mapInfoTimerStartpoint = event->pos();
        }
    }

#if 0

  // @AP: This code of graphical task planning is currently not used.
  // Too much effort for me to get it running.
  if( planning == 1 || planning == 3 )
    {
      qDebug() << "mouseMoveEvent: planning=" << planning;

      double delta(8.0);

      if( _globalMapMatrix->isSwitchScale() )
        {
          delta = 16.0;
        }

      // If we are near to the last temp-point of the task,
      // and SHIFT is not pressed, we can exit ...
      if( (abs(current.x() - preSnapPoint.x()) <= delta) &&
          (abs(current.y() - preSnapPoint.y()) <= delta) &&
          event->modifiers() != Qt::ShiftModifier )
        {
          qDebug() << "mouseMoveEvent: planning is left here";
          return;
        }

      BaseFlightElement *bfe = _globalMapContents->getFlight();

      // bfe can contain different classes. We take only into account a
      // flight task.
      FlightTask* ft = dynamic_cast<FlightTask*>(bfe);

      if( ! ft )
        {
          return;
        }

      QList<Waypoint*> taskPointList = ft->getWPList();
      QList<Waypoint*> tempTaskPointList = ft->getWPList();
      QList<Waypoint*> &wpList = _globalMapContents->getWaypointList();

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
              found = __getTaskWaypoint(current, &wp, wpList);

              if(found)
                {
                  isSnapping = true;

                  point = _globalMapMatrix->map(_globalMapMatrix->wgsToMap(wp.origP));

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
                      tempTaskPointList.insert(moveWPindex, new Waypoint);
                      w = tempTaskPointList.at(moveWPindex);
                    }
                  else
                    {
                      //anhängen
                      tempTaskPointList.append( new Waypoint );
                      w = tempTaskPointList.last();
                    }

                  *w = wp;

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

                  if(event->modifiers() == Qt::ShiftModifier)
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
                          QPoint tmp(_globalMapMatrix->mapToWgs(event->pos()));
                          tempTaskPointList.at(moveWPindex)->origP = WGSPoint(tmp.y(), tmp.x());
                          tempTaskPointList.at(moveWPindex)->projP =
                                _globalMapMatrix->wgsToMap(tempTaskPointList.at(moveWPindex)->origP);
                        }
                      else
                        {
                          //anhängen
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
                // außerhalb der Karte
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

#endif

  if( ! timerAnimate->isActive() )
    {
      BaseFlightElement *bfe = _globalMapContents->getFlight();

      if( bfe )
        {
          FlightPoint fP;
          int index;

          if( (index = bfe->searchPoint( event->pos(), fP )) != -1 )
            {
              emit showFlightPoint( _globalMapMatrix->mapToWgs( event->pos() ), fP );

              prePos = _globalMapMatrix->map( fP.projP );
              preIndex = index;
            }
          else
            {
              emit showPoint( _globalMapMatrix->mapToWgs( event->pos() ) );
            }
        }
      else
        {
          emit showPoint( _globalMapMatrix->mapToWgs( event->pos() ) );
          prePos.setX( -5000 );
          prePos.setY( -5000 );
        }
    }

  // Assume, that mouse position must be converted to map coordinates.
  __findElevation( _globalMapMatrix->invertToMap(current) );

  if( isDragZoomActive )
    {
      // We don't want to popup any info, when mouse is moved for zoom
      mapInfoTimer->stop();

      if( pixZoomRect.isNull() || pixZoomRect.size() != size() )
        {
          pixZoomRect = QPixmap( size() );
        }

      pixZoomRect.fill( Qt::transparent );

      QPainter zoomPainter;
      zoomPainter.begin( &pixZoomRect );

      double width = event->pos().x() - beginDrag.x();
      double height = event->pos().y() - beginDrag.y();
      const double widthRatio = fabs( width / this->width() );
      const double heightRatio = fabs( height / this->height() );

      if( widthRatio > heightRatio )
        {
          height = this->height() * widthRatio;

          if( event->pos().y() < beginDrag.y() )
            {
              // make sure we keep the right sign
              height *= -1;
            }
        }
      else
        {
          width = this->width() * heightRatio;

          if( event->pos().x() < beginDrag.x() )
            {
              // make sure we keep the right sign
              width *= -1;
            }
        }

      zoomPainter.setPen(QPen(QColor(100, 100, 100), 2, Qt::DashLine));
      // Draw the new one:
      zoomPainter.drawRect( beginDrag.x(), beginDrag.y(), (int)width, (int)height);
      zoomPainter.end();

      repaint();

      sizeDrag.setWidth((int)width);
      sizeDrag.setHeight((int)height);
  }

  if( ! isDragZoomActive && event->buttons() == Qt::LeftButton )
    {
      // We don't want to popup any info, when mouse is moved for zoom
      mapInfoTimer->stop();

      // start dragZoom
      setCursor( Qt::CrossCursor );
      isZoomActive = true;
      beginDrag = event->pos();
      sizeDrag = QSize( 0, 0 );
      isDragZoomActive = true;
    }
}

/**
  * just a helper function
  * to keep the Waypoint class light weight
  */
QString getInfoString (Waypoint* wp)
{
  QString path = _globalMapConfig->getIconPath();

  QString text = "<HTML><TABLE BORDER=0><TR>";

  // we don't have a pixmap for landmarks ?!
  if (wp->type != BaseMapElement::Landmark && !_globalMapConfig->getPixmapName(wp->type).isEmpty())
    text += QString ("<TD><IMG SRC= %1/%2></TD>").arg(path).arg(_globalMapConfig->getPixmapName(wp->type));
  else
    text += "<TD></TD>";

  text += "<TD>" + QObject::tr("Waypoint:") + " " + wp->name;

  if (!wp->icao.isEmpty())
    {
      text += QString (" (%1)").arg(wp->icao);
    }

  text += "</TD></TR>";

  if (wp->type >= 0)
    text += "<TR><TD></TD><TD>" +
            BaseMapElement::item2Text( wp->type, QObject::tr("unknown") ) +
            "</TD></TR>";

  text += QString ("<TR><TD></TD><TD><FONT SIZE=-1> %1 m").arg(wp->elevation);

  if (wp->frequency > 0.0)
  {
    text += QString ("<BR>%1").arg(wp->frequency, 0, 'f', 3);
  }

  text += "<BR>" + WGSPoint::printPos(wp->origP.lat());
  text += "<BR>" + WGSPoint::printPos(wp->origP.lon(), false);

  text += "</FONT></TD></TR></TABLE></HTML>";

  return text;
}

Waypoint* Map::findWaypoint (const QPoint& current)
{
  Waypoint *wp;

  foreach( wp, _globalMapContents->getWaypointList() )
  {
    QPoint sitePos (_globalMapMatrix->map(_globalMapMatrix->wgsToMap(wp->origP)));
    double dX = abs(sitePos.x() - current.x());
    double dY = abs(sitePos.y() - current.y());

    // Abstand entspricht der Icon-Grösse.
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
  QPoint sitePos;
  // Radius for Mouse Snapping
  double delta(16.0);

  int timeout = 60000;

  if( automatic )
    {
      timeout = 20000;
    }

  QString text = "";

  bool show = false;

  int searchList[] = { MapContents::AirfieldList,
                       MapContents::GliderfieldList,
                       MapContents::OutLandingList };

  // At first we look for single airfields, ... objects
  for( int l = 0; l < 3; l++ )
    {
     for( int loop = 0; loop < _globalMapContents->getListLength( searchList[l] ); loop++ )
       {
          Airfield *hitElement =
              static_cast<Airfield *> ( _globalMapContents->getElement( searchList[l], loop ) );

          sitePos = hitElement->getMapPosition();

          double dX = abs( sitePos.x() - current.x() );
          double dY = abs( sitePos.y() - current.y() );

          // Abstand entspricht der Icon-Grösse.
          if( (dX < delta) && (dY < delta) )
            {
              text += hitElement->getInfoString();
              // Text anzeigen
              WhatsThat* box = new WhatsThat( this, text, timeout, mapToGlobal( current ) );
              box->setVisible( true );
              return;
            }
        }
    }

  BaseFlightElement *baseFlight = _globalMapContents->getFlight();

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

  // At last search for airspaces.
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
  BaseFlightElement *bfe = _globalMapContents->getFlight();

  // bfe can contain different classes. We take only into account a
  // flight task.
  FlightTask* ft = dynamic_cast<FlightTask*>(bfe);

  if( ! ft )
    {
      return;
    }

  emit setStatusBarMsg( tr( "Task planning on" ) );

  QList<Waypoint*> taskPointList = ft->getWPList();
  QList<Waypoint*> tempTaskPointList = ft->getWPList();
  QList<Waypoint*> & wpList = _globalMapContents->getWaypointList();

  // Save initial size of task point list.
  int tplStartSize = taskPointList.size();

  Waypoint wp;
  QString text;

  // Is the point already in the flight task?
  bool found = __getTaskWaypoint(current, &wp, taskPointList);

  if( !found )
    {
      // Try the waypoint catalog to find it.
      found = __getTaskWaypoint( current, &wp, wpList );
    }

  // Open task action menu for further processing. That seems for me the best
  // way to be user friendly.
  QAction* action;
  QMenu menu(this);

  menu.setTitle( QObject::tr("Task Menu") );

  if( ! found )
    {
      action = menu.addAction( tr("Task Menu") );
      action->setData( -1 );

      menu.addSeparator();

      action = menu.addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                               tr("Create waypoint here") );
      action->setData( 4 );

      menu.addSeparator();

      action = menu.addAction( tr("End task planning") );
      action->setData( 5 );
    }
  else
    {
      action = menu.addAction( tr("Task Menu") );
      action->setData( -1 );

      menu.addSeparator();

      action = menu.addAction( wp.description.isEmpty() ? wp.name : wp.description );
      action->setData( -1 );

      menu.addSeparator();

      action = menu.addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                tr("Add waypoint 1x") );
      action->setData( 0 );

      action = menu.addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                tr("Add waypoint 2x") );
      action->setData( 1 );

      action = menu.addAction( _mainWindow->getPixmap("kde_editdelete_16.png"),
                                tr("Delete waypoint 1x") );
      action->setData( 2 );
      action->setEnabled( taskPointList.count() > 0 );

      action = menu.addAction( _mainWindow->getPixmap("kde_editdelete_16.png"),
                                tr("Delete all waypoints") );
      action->setData( 3 );
      action->setEnabled( taskPointList.count() > 0 );

      menu.addSeparator();
      action = menu.addAction( tr("End task planning") );
      action->setData( 5 );
    }

  // Popup the menu at mouse position.
  action = menu.exec( mapToGlobal( event->pos() ) );

  if( ! action )
    {
      return;
    }

  switch( action->data().toInt() )
    {
      case 0:
        // add waypoint 1x
        taskPointList.append( new Waypoint( &wp ));
        break;

      case 1:

        // add waypoint 2x
        taskPointList.append( new Waypoint( &wp ));
        taskPointList.append( new Waypoint( &wp ));
        break;

      case 2:
        // remove first found waypoint
        for( int i = 0; i < taskPointList.count(); i++ )
          {
            if( wp == *taskPointList.at( i ) )
              {
                delete taskPointList.takeAt( i );
                break;
              }
          }

        break;

      case 3:
        // remove all waypoints. start from the end of the list to get all
        // of them.
        for( int i = taskPointList.count() - 1; i >= 0; i-- )
          {
            if( wp == *taskPointList.at( i ) )
              {
                delete taskPointList.takeAt( i );
              }
          }

        break;

      case 4:
        {
          // Create a new waypoint at the current map position.
          WaypointDialog* waypointDlg = __openWaypointDialog( current );
          int result = waypointDlg->exec();

          delete waypointDlg;

          if( result == QDialog::Accepted )
            {
              // That is the trick, the last waypoint must be the new one.
              Waypoint* newWp = wpList.last();
              taskPointList.append( new Waypoint( newWp ));
            }

          break;
        }

      case 5:
        // End of task planning
        planning = 2;
        emit taskPlanningEnd();
        emit setStatusBarMsg( tr( "Ready." ) );

      default:
        // Should never happen
        return;
    }

  if( tplStartSize != taskPointList.size() )
    {
      ft->setWaypointList(taskPointList);
      __drawPlannedTask();
      __showLayer();
    }

  emit flightTaskModified();
}

void Map::keyReleaseEvent(QKeyEvent* event)
{
  qDebug() << " Map::keyReleaseEvent";

  if(event->modifiers() == Qt::ShiftModifier)
    {
      qDebug() << "Map::keyReleaseEvent()";
      __showLayer();
    }
}

void Map::mouseReleaseEvent( QMouseEvent* event )
{
  if( isZoomActive )
    {
      isDragZoomActive = false;
      isZoomActive = false;
      pixZoomRect = QPixmap();
      __setCursor();

      if( abs(beginDrag.x()-event->pos().x()) > 10 && // don't zoom if rect is too small
          abs(beginDrag.y()-event->pos().y()) > 10 &&
          event->button() == Qt::LeftButton) // or other than left button was pressed
        {
          double width = event->pos().x() - beginDrag.x();
          double height = event->pos().y() - beginDrag.y();
          const double widthRatio = fabs(width / this->width());
          const double heightRatio = fabs(height / this->height());

          if( widthRatio > heightRatio )
            {
              height = this->height() * widthRatio;

              if( event->pos().y() < beginDrag.y() )
                {
                  height *= -1;
                }
            }
          else
            {
              width = this->width() * heightRatio;

              if( event->pos().x() < beginDrag.x() )
                {
                  width *= -1;
                }
            }


          if( width < 0 && height < 0 )
            {  // work around for problem where mapmatrix calculates the wrong
               // scale if both width and height are < 0
              _globalMapMatrix->centerToRect(QRect(QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height), beginDrag), QSize(0,0), false);
            }
          else
            {
              _globalMapMatrix->centerToRect(QRect(beginDrag, QPoint(beginDrag.x() + (int)width, beginDrag.y() + (int)height)), QSize(0,0), false);
            }

          __redrawMap();
        }
      else
        {
          // we are not going to zoom, but we still need to clean up our mess!
          update();
        }
    }
}

void Map::mousePressEvent(QMouseEvent* event)
{
  qDebug() << "Map::mousePressEvent()";

  // First: delete the cursor, if visible:
  if( prePos.x() >= 0 )
    {
      drawFlightCursors = false;
      repaint();
    }

  if( isZoomActive )
    {
      // User has pressed key zero
      beginDrag = event->pos();
      sizeDrag = QSize( 0, 0 );
      isDragZoomActive = true;
      return;
    }

  const QPoint current( event->pos() );

  Airfield *hitElement;
  QString text;

  double dX, dY, delta( 16.0 );

  if( _globalMapMatrix->isSwitchScale() )
    {
      delta = 8.0;
    }

  if( event->button() == Qt::MidButton )
    {
      // Center Map
      _globalMapMatrix->centerToPoint( event->pos() );
      __redrawMap();
      return;
    }

  if(event->button() == Qt::LeftButton)
    {
      // Check if task planning is active. In this case allow only
      // planning actions.
      if( planning == 1 )
        {
          __graphicalPlanning( current, event );
          return;
        }

      if(event->modifiers() == Qt::ShiftModifier)
        {
          // select WayPoint
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
                    // qDebug() << "Found" << hitElement->getName();
                    Waypoint *w = new Waypoint;

                    QString name = hitElement->getName();
                    w->name = name.left(8).toUpper();
                    w->description = hitElement->getName();
                    w->type = hitElement->getObjectType();hitElement->getName();
                    w->origP = hitElement->getWGSPosition();
                    w->elevation = hitElement->getElevation();
                    w->icao = hitElement->getICAO();
                    w->frequency = hitElement->getFrequency();
                    w->country = hitElement->getCountry();
                    w->isLandable = true;

                    emit waypointSelected(w);
                    found = true;
                    break;
                }
             }

             if(found)
               {
                 break;
               }
          }

          if( !found )
              {
                WaypointDialog* waypointDlg = __openWaypointDialog( current );
                waypointDlg->exec();
                delete waypointDlg;
              }
          }
  }
  else if( event->button() == Qt::RightButton )
    {
      popupPos = event->pos();
      __showPopupMenu( event );
    }
}

/**
 * Please note! Qt4 allows only to draw at the widget's paint engine inside
 * a paint event. Otherwise you will get displayed warnings.
 */
void Map::paintEvent( QPaintEvent* event )
{
  QPainter painter(this);

  painter.drawPixmap( event->rect().left(),
                      event->rect().top(),
                      pixBuffer,
                      0, 0, event->rect().width(),
                      event->rect().height() );

  // Redraw the flight cursors on request.
  if( drawFlightCursors == true )
    {
      painter.drawPixmap( event->rect().left(),
                          event->rect().top(),
                          pixFlightCursors,
                          0, 0, event->rect().width(),
                          event->rect().height() );
    }

  if( ! pixZoomRect.isNull() && isDragZoomActive == true )
    {
      // Draws the zooming rectangle
      painter.drawPixmap( event->rect().left(),
                          event->rect().top(),
                          pixZoomRect );
    }

}

void Map::__drawGrid()
{
  const QRect mapBorder = _globalMapMatrix->getViewBorder();

  QPainter gridP;

  gridP.begin( &pixGrid );
  gridP.setBrush( Qt::NoBrush );
  gridP.setClipping( true );

  // WGS coordinates of the map in degree
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

  // First the latitudes:
  for( int loop = 0; loop < (lat1 - lat2 + 1); loop += gridStep )
    {
      int size = (lon2 - lon1 + 1) * 10;

      QPolygon pointArray;

      for( int lonloop = 0; lonloop < size; lonloop++ )
        {
          QPoint p0 = _globalMapMatrix->wgsToMap( (lat2 + loop) * 600000,
                                                  (int) rint((lon1 + (lonloop * 0.1)) * 600000) );

          if( _globalMapMatrix->isVisible(p0) )
            {
              pointArray.append( _globalMapMatrix->map(p0) );
            }
        }

      if( pointArray.size() >= 2 )
        {
          QPoint p = pointArray.first();
          p.setX(0);
          pointArray.insert( 0, p );

          p = pointArray.last();
          p.setX( width() );
          pointArray.append( p );

          // Draw the main lines
          gridP.setPen( QPen( QColor( Qt::black ), 1 ) );
          gridP.drawPolyline( pointArray );

          gridP.setFont(QFont("Helvetica", 8, QFont::Bold));

          int lat = lat2 + loop;

          QString text = QString("%1%2%3")
                         .arg( lat >= 0 ? "N" : "S" )
                         .arg( lat >= 0 ? lat : -lat )
                         .arg(QChar(Qt::Key_degree));

          gridP.drawText( 5, pointArray.first().y() - 5, text );
        }

      // Draw the small lines between:
      int number = (int) (60.0 / step);

      for( int loop2 = 1; loop2 < number; loop2++ )
        {
          QPolygon pointArraySmall;

          for( int lonloop = 0; lonloop < size; lonloop++ )
            {
              QPoint p1 = _globalMapMatrix->wgsToMap( (int) rint((lat2 + loop + (loop2 * (step / 60.0))) * 600000),
                                                      (int) rint((lon1 + (lonloop * 0.1)) * 600000) );

              if( _globalMapMatrix->isVisible(p1) )
                {
                  pointArraySmall.append( _globalMapMatrix->map(p1) );
                }
            }

          if( pointArraySmall.size() >= 2 )
            {
              QPoint p = pointArraySmall.first();
              p.setX(0);
              pointArraySmall.insert( 0, p );

              p = pointArraySmall.last();
              p.setX( width() );
              pointArraySmall.append( p );


              if( loop2 == (number / 2.0) )
                {
                  gridP.setPen( QPen( QColor( Qt::black ), 1, Qt::DashLine ) );
                  gridP.drawPolyline( pointArraySmall );

                  gridP.setFont(QFont("Helvetica", 8, QFont::Normal));

                  int lat = lat2 + loop;

                  QString text = QString("%1%2.5%3")
                                 .arg( lat >= 0 ? "N" : "S" )
                                 .arg( lat >= 0 ? lat : -lat )
                                 .arg(QChar(Qt::Key_degree));

                  gridP.drawText( 5, pointArraySmall.first().y() - 5, text );
                }
              else
                {
                  gridP.setPen( QPen( QColor( Qt::black ), 1, Qt::DotLine ) );
                  gridP.drawPolyline( pointArraySmall );
                }
              }
        }
    }

  // Now the longitudes:
  for( int loop = lon1; loop <= lon2; loop += gridStep )
    {
      QPoint cP1 = _globalMapMatrix->wgsToMap( mapBorder.top(), (loop * 600000) );
      QPoint cP2 = _globalMapMatrix->wgsToMap( mapBorder.bottom(), (loop * 600000) );

      QPoint m1 = _globalMapMatrix->map( cP1 );
      QPoint m2 = _globalMapMatrix->map( cP2 );

      // Draw the main longitudes:
      gridP.setPen( QPen( QColor( Qt::black ), 1 ) );
      gridP.drawLine( m1, m2 );

      QString text = QString("%1%2%3")
                     .arg( loop >= 0 ? "E" : "W" )
                     .arg( loop >= 0 ? loop : -loop )
                     .arg(QChar(Qt::Key_degree));

      gridP.drawText( m1.x() + 5, 15, text );

      // Draw the small lines between:
      int number = (int) (60.0 / step);

      for( int loop2 = 1; loop2 < number; loop2++ )
        {
          cP1 = _globalMapMatrix->wgsToMap( mapBorder.top(),
                                           (int) rint((loop + (loop2 * step / 60.0)) * 600000) );

          cP2 = _globalMapMatrix->wgsToMap( mapBorder.bottom(),
                                            (int) rint((loop + (loop2 * step / 60.0)) * 600000) );

          if( loop2 == (number / 2.0) )
            {
              gridP.setPen( QPen( QColor( Qt::black ), 1, Qt::DashLine ) );
            }
          else
            {
              gridP.setPen( QPen( QColor( Qt::black ), lineWidth, Qt::DotLine ) );
            }

          gridP.drawLine( _globalMapMatrix->map( cP1 ), _globalMapMatrix->map( cP2 ) );
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

  // Take the color of the subterrain for filling
  pixIsoMap.fill( _globalMapConfig->getIsoColor(0) );

  _globalMapContents->drawIsoList( &isoMapP, rect() );

  emit setStatusBarProgress(10);

  _globalMapContents->drawList(&uMapP, MapContents::TopoList);

  _globalMapContents->drawList(&uMapP, MapContents::CityList);

  _globalMapContents->drawList(&uMapP, MapContents::HydroList);

  emit setStatusBarProgress(15);

  _globalMapContents->drawList(&uMapP, MapContents::RoadList);

  _globalMapContents->drawList(&uMapP, MapContents::HighwayList);

  emit setStatusBarProgress(25);

  _globalMapContents->drawList(&uMapP, MapContents::RailList);

  emit setStatusBarProgress(35);

  _globalMapContents->drawList(&uMapP, MapContents::VillageList);

  emit setStatusBarProgress(45);

  _globalMapContents->drawList(&uMapP, MapContents::LandmarkList);

  emit setStatusBarProgress(50);

  _globalMapContents->drawList(&uMapP, MapContents::ObstacleList);

  emit setStatusBarProgress(55);

  _globalMapContents->drawList(&aeroP, MapContents::ReportList);

  emit setStatusBarProgress(60);

  _globalMapContents->drawList(&aeroP, MapContents::NavList);

  emit setStatusBarProgress(65);

  __drawAirspaces();

  emit setStatusBarProgress(70);

  emit setStatusBarProgress(75);

  _globalMapContents->drawList(&aeroP, MapContents::AirfieldList);

  emit setStatusBarProgress(80);

  _globalMapContents->drawList(&aeroP, MapContents::AddSitesList);

  emit setStatusBarProgress(85);

  _globalMapContents->drawList(&aeroP, MapContents::GliderfieldList);

  emit setStatusBarProgress(90);

  _globalMapContents->drawList(&aeroP, MapContents::OutLandingList);

  emit setStatusBarProgress(95);

  __drawGrid();
}


void Map::__drawAirspaces()
{
  QPainter cuAeroMapP;
  cuAeroMapP.begin(&pixAirspace);

  QTime t;
  t.start();

  QList<QPair<QPainterPath, Airspace *> >& airspaceRegionList =
                                    _globalMapContents->getAirspaceRegionList();
  airspaceRegionList.clear();

  SortableAirspaceList& airspaceList = _globalMapContents->getAirspaceList();

  for( int i = 0; i < airspaceList.size(); i++ )
    {
      Airspace& as = airspaceList[i];

      if( ! as.isDrawable() )
        {
          // Not of interest, step away
          continue;
        }

      QPair<QPainterPath, Airspace *> pair( as.createRegion(), &as );
      airspaceRegionList.append( pair );

      as.drawRegion( &cuAeroMapP, this->rect() );
    }

  cuAeroMapP.end();
  // qDebug("Airspace, drawTime=%d ms", t.elapsed());
}

void Map::__drawFlight()
{
  qDebug() << "Map::__drawFlight() Ein";

  pixFlight.fill(Qt::transparent);
  QPainter flightP(&pixFlight);

  _globalMapContents->drawList( &flightP, MapContents::FlightList );

  qDebug() << "Map::__drawFlight() Aus";
}

void Map::__drawPlannedTask( bool solid )
{
  qDebug() << "Map::__drawPlannedTask Ein";

  FlightTask* task;

  if( solid )
    {
      task = dynamic_cast<FlightTask *> (_globalMapContents->getFlight());
    }
  else
    {
      task = &tempTask;
      task->reProject();
    }

  if( task && task->getObjectType() == BaseMapElement::Task )
    {
      QList<Waypoint*> WPList = task->getWPList();

      QPainter planP;

      // Strecke zeichnen
      pixPlan.fill(Qt::transparent);
      planP.begin(&pixPlan);

      QPen drawP(QColor(170,0,0), 5);
      drawP.setJoinStyle(Qt::MiterJoin);
      planP.setBrush(Qt::NoBrush);
      planP.setPen(drawP);

      // Aufgabe mit Sektoren
      task->drawMapElement(&planP);
      emit showTaskText(task);
      planP.end();
    }

  qDebug() << "Map::__drawPlannedTask Aus";
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

void Map::dragEnterEvent( QDragEnterEvent* event )
{
  qDebug() << "Map::dragEnterEvent()";

  if( event->mimeData()->hasUrls() )
    {
      event->acceptProposedAction();
    }
}

void Map::dropEvent( QDropEvent* event )
{
  qDebug() << "Map::dropEvent";

  QList<QUrl> urlList = event->mimeData()->urls();

  for( int i = 0; i < urlList.size(); i++ )
    {
      emit openFile( urlList.at(i) );
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
      pixFlightCursors = QPixmap( size() );
    }

  _globalMapMatrix->createMatrix( size() );
  emit changed( size() );

  // Status bar not set "geniously" so far...
  emit setStatusBarProgress(0);

  pixAero.fill(Qt::transparent);
  pixAirspace.fill(Qt::transparent);
  pixGrid.fill(Qt::transparent);
  pixUnderMap.fill(Qt::transparent);
  pixIsoMap.fill(Qt::transparent);
  pixFlight.fill(Qt::transparent);
  pixPlan.fill(Qt::transparent);
  pixWaypoints.fill(Qt::transparent);

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

  if (_settings.value("/CommentSettings/ShowComment").toBool())
  {

#warning Flight can be NULL!!!

    Flight* flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

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

  QImage image = QImage( pixBuffer.toImage() );

  image.save( fName, "PNG" );

  if( width && height )
    {
      resize( w_orig, h_orig );
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
  qDebug() << "Map::slotActivatePlanning(): Rein Planning=" << planning;

  if( planning != 1 )
    {
      mapInfoTimer->stop();
      planning = 1;
      prePlanPos.setX( -999 );
      prePlanPos.setY( -999 );
      emit setStatusBarMsg( tr( "Task planning on" ) );
    }
  else
    {
      // switch off planning
      planning = 0;
      __showLayer();
      emit setStatusBarMsg( "" );
      emit taskPlanningEnd();
    }

  qDebug() << "Map::slotActivatePlanning(): Raus Planning=" << planning;
}

void Map::__showLayer()
{
  //qDebug() << "Map::__showLayer() Ein";
  pixBuffer = pixIsoMap;

  QPainter buffer(&pixBuffer);

  buffer.drawPixmap(pixUnderMap.rect(), pixUnderMap);
  buffer.drawPixmap(pixAero.rect(), pixAero);
  buffer.drawPixmap(pixAirspace.rect(), pixAirspace);
  buffer.drawPixmap(pixFlight.rect(), pixFlight);
  buffer.drawPixmap(pixPlan.rect(), pixPlan);
  buffer.drawPixmap(pixWaypoints.rect(), pixWaypoints);
  buffer.drawPixmap(pixGrid.rect(), pixGrid);

  slotDrawCursor( lastCur1Pos, lastCur2Pos );
  update();

  //qDebug() << "Map::__showLayer() Aus";
}

void Map::slotDrawCursor( const QPoint& p1, const QPoint& p2 )
{
  if( p1 == QPoint(-100,-100) && p2 == QPoint(-100,-100) )
    {
      // Cursor is not set, ignore request.
      pixFlightCursors.fill( Qt::transparent );
      drawFlightCursors = false;
      return;
    }

  lastCur1Pos = p1;
  lastCur2Pos = p2;

  QPoint pos1( _globalMapMatrix->map( p1 ) );
  QPoint pos2( _globalMapMatrix->map( p2 ) );

  if( pixFlightCursors.isNull() )
    {
      pixFlightCursors = QPixmap( size() );
    }

  pixFlightCursors.fill( Qt::transparent );

  QPainter painter;

  painter.begin( &pixFlightCursors );

  if( pos1.x() > -50 && pos1.x() < width() + 50 &&
      pos1.y() > -50 && pos1.y() < height() + 50 )
    {
      painter.drawPixmap( pos1.x() - 32, pos1.y() - 32, pixCursor1 );
    }

  if( pos2.x() > -50 && pos2.x() < width() + 50 &&
      pos2.y() > -50 && pos2.y() < height() + 50 )
    {
      painter.drawPixmap( pos2.x() - 0, pos2.y() - 32, pixCursor2 );
    }

  painter.end();

  drawFlightCursors = true;
  update();
}

void Map::slotClearCursor()
{
  lastCur1Pos = QPoint(-100,-100);
  lastCur2Pos = QPoint(-100,-100);
  drawFlightCursors = false;
}

void Map::slotZoomRect()
{
  setCursor(Qt::CrossCursor);
  isZoomActive = true;
}

void Map::slotCenterToWaypoint(const int id)
{
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
  Flight *bfe = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( bfe )
    {
      QRect r;
      QRect r2;
      QList<Flight*> fl;

      switch( bfe->getObjectType() )
        {
          case BaseMapElement::Flight:
            r = bfe->getFlightRect();
            break;

          case BaseMapElement::FlightGroup:
            fl = ((FlightGroup *) bfe)->getFlightList();
            r = fl.at( 0 )->getFlightRect();

            for( int i = 1; i < fl.count(); i++ )
              {
                r2 = fl.at( i )->getFlightRect();
                r.setLeft( qMin( r.left(), r2.left() ) );
                r.setTop( qMin( r.top(), r2.top() ) );
                r.setRight( qMax( r.right(), r2.right() ) );
                r.setBottom( qMax( r.bottom(), r2.bottom() ) );
              }

            break;

          default:
            return;
        }

      // check if the Rectangle is zero
      // is it necessary here?
      if( !r.isNull() )
        {
          _globalMapMatrix->centerToRect( r );
          __redrawMap();
        }

      emit changed( this->size() );
    }
}

void Map::slotCenterToTask()
{
  BaseFlightElement  *f = _globalMapContents->getFlight();

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
              r.setLeft(qMin(r.left(), r2.left()));
              r.setTop(qMin(r.top(), r2.top()));
              r.setRight(qMax(r.right(), r2.right()));
              r.setBottom(qMax(r.bottom(), r2.bottom()));
            }
            break;
          default:
            return;
        }

      // check if the Rectangle is zero
      if( !r.isNull() )
        {
          _globalMapMatrix->centerToRect( r );
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
  QPoint pos;
  QPixmap pix;
  FlightPoint cP;

  Flight *f = dynamic_cast<Flight *> (_globalMapContents->getFlight());

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
            {
              cP = f->getPoint(0);
              prePos = _globalMapMatrix->map(cP.projP);
              pos = _globalMapMatrix->map(cP.projP);
              pix = f->getLastAnimationPixmap();

              QPainter p1( &pix );
              p1.drawPixmap( 0, 0, pixBuffer, pos.x(), pos.y()-32, 32, 32 );

              f->setLastAnimationPos(pos);
              f->setLastAnimationPixmap(pix);

              // put flag
              QPainter p2( &pixBuffer );
              p2.drawPixmap( pos.x(), pos.y()-32, pixCursor2 );
            }
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

                    QPainter p1( &pix );
                    p1.drawPixmap( 0, 0, pixBuffer, pos.x(), pos.y()-32, 32, 32 );

                    f->setLastAnimationPos(pos);
                    f->setLastAnimationPixmap(pix);

                    // put flag
                    QPainter p2( &pixBuffer );
                    p2.drawPixmap( pos.x(), pos.y()-32, pixCursor2 );

                  }
              }
            break;

          default:

            break;
           }

      // 50ms multi-shot timer
      timerAnimate->start( 50 );
    }
}

/**
 * Animation slot.
 * Called for every timeout of the animation timer.
 */
void Map::slotAnimateFlightTimeout()
{
  qDebug() << "Map::slotAnimateFlightTimeout()";

  FlightPoint cP; //, prevP;
  Flight *f  = this->flightToAnimate;
  bool bDone = true;
  QPoint lastpos, pos;
  QPixmap pix;

  if(f)
    {
      switch(f->getObjectType())
        {
          case BaseMapElement::Flight:
            {
              f->setAnimationNextIndex();

              if( f->isAnimationActive() )
              {
                bDone = false;
              }

              //write info from current point on statusbar
              cP = f->getPoint((f->getAnimationIndex()));
              pos = _globalMapMatrix->map(cP.projP);
              lastpos = f->getLastAnimationPos();
              pix = f->getLastAnimationPixmap();
              emit showFlightPoint(_globalMapMatrix->mapToWgs(pos), cP);

              // erase prev indicator-flag
              QPainter p1( &pixBuffer );
              p1.drawPixmap( lastpos.x(), lastpos.y()-32, pix );

              // redraw flight up to this point, blt the pixmap onto the already created pixmap
              __drawFlight();
              QPainter p2( &pixBuffer );
              p2.drawPixmap( 0, 0, pixFlight );

              //save for next timeout
              QPainter p3( &pix );
              p3.drawPixmap( 0, 0, pixBuffer, pos.x(), pos.y()-32, 32, 32 );

              f->setLastAnimationPixmap(pix);
              f->setLastAnimationPos(pos);

              // add indicator-flag
              QPainter p4( &pixBuffer );
              p4.drawPixmap( pos.x(), pos.y()-32,  pixCursor2 );
            }

            break;

          case BaseMapElement::FlightGroup:
            // loop through all and set animation index to start
            {
              QList<Flight*> flightList = ((FlightGroup *) flightToAnimate)->getFlightList();

              for(int loop = 0; loop < flightList.count(); loop++)
                {
                  f = flightList.at(loop);
                  f->setAnimationNextIndex();

                  if (f->isAnimationActive())
                    {
                      bDone = false;
                    }

                  //write info from current point on statusbar
                  cP = f->getPoint((f->getAnimationIndex()));
                  pos = _globalMapMatrix->map(cP.projP);
                  lastpos = f->getLastAnimationPos();
                  pix = f->getLastAnimationPixmap();
                  emit showFlightPoint(_globalMapMatrix->mapToWgs(pos), cP);

                  // erase prev indicator-flag
                  QPainter p1( &pixBuffer );
                  p1.drawPixmap( lastpos.x(), lastpos.y()-32, pix );

                  // redraw flight up to this point, blt the pixmap onto the already created pixmap
                  __drawFlight();
                  QPainter p2( &pixBuffer );
                  p2.drawPixmap( 0, 0, pixFlight );

                  //save for next timeout
                  QPainter p3( &pix );
                  p3.drawPixmap( 0, 0, pixBuffer, pos.x(), pos.y()-32, 32, 32 );


                  f->setLastAnimationPixmap(pix);
                  f->setLastAnimationPos(pos);

                  // add indicator-flag
                  QPainter p4( &pixBuffer );
                  p4.drawPixmap( pos.x(), pos.y()-32,  pixCursor2 );
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
  if( bDone )
    {
      timerAnimate->stop();
    }
}

/**
 * Animation slot.
 * Called to stop the animation timer.
 */
void Map::slotAnimateFlightStop()
{
  if( ! timerAnimate->isActive() )
    {
      return;
    }

  // stop animation on user request.
  timerAnimate->stop();

  BaseFlightElement* bfe = _globalMapContents->getFlight();

  if( ! bfe )
    {
      return;
    }

  QList<Flight *> flightList;

  switch( bfe->getObjectType() )
    {
      case BaseMapElement::Flight:

        flightList.append( dynamic_cast<Flight *>(bfe) );
        break;

      case BaseMapElement::FlightGroup:
        {
          FlightGroup* fg = dynamic_cast<FlightGroup *>(bfe);
          flightList = fg->getFlightList();
          break;
        }

      default:
        return;
    }

  // Loop through the flight list and reset animation flag.
  for( int loop = 0; loop < flightList.size(); loop++ )
    {
      flightList.at(loop)->setAnimationActive(false);
    }

  slotRedrawFlight();
}

/**
 * Stepping slots.
 */
void Map::slotFlightNext()
{
  FlightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if(prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
           QPainter p1( this );
           p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                          pixBuffer,
                          prePos.x() - 20, prePos.y() - 20, 40, 40 );

          // get the next point, preIndex now holds last point
          if( (index = f->searchGetNextPoint( preIndex, cP )) != -1 )
            {
              // if close to edge, recenter map to next point
              prePos = _globalMapMatrix->map( cP.projP );

              if( (prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) )
                {
                  _globalMapMatrix->centerToPoint( prePos );
                  __redrawMap();
                }

              emit
              showFlightPoint( _globalMapMatrix->wgsToMap( cP.origP ), cP );
              prePos = _globalMapMatrix->map( cP.projP );
              preIndex = index;

              QPainter p1( this );
              p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
            }
        }
    }
}

void Map::slotFlightPrev()
{
  FlightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
          QPainter p1( this );
          p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                         pixBuffer,
                         prePos.x() - 20, prePos.y() - 20, 40, 40 );

          // get the next point, preIndex now holds last point
          if ((index = f->searchGetPrevPoint(preIndex, cP)) != -1)
            {
              // if close to edge, recenter map to next point
              prePos = _globalMapMatrix->map( cP.projP );

              if( (prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) )
                {
                  _globalMapMatrix->centerToPoint( prePos );
                }

              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;

              QPainter p1( this );
              p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
            }
        }
    }
}

void Map::slotFlightStepNext()
{
  FlightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
          QPainter p1( this );
          p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                         pixBuffer,
                         prePos.x() - 20, prePos.y() - 20, 40, 40 );

          // get the next point, preIndex now holds last point
          if((index = f->searchStepNextPoint(preIndex, cP, 10)) != -1)
            {
               // if close to edge, recenter map to next point
              prePos = _globalMapMatrix->map( cP.projP );

              if( (prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) )
                {
                  _globalMapMatrix->centerToPoint( prePos );
                }

              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;

              QPainter p1( this );
              p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
            }
        }
    }
}

void Map::slotFlightStepPrev()
{
  FlightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
          QPainter p1( this );
          p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                         pixBuffer,
                         prePos.x() - 20, prePos.y() - 20, 40, 40 );

          // get the next point, preIndex now holds last point
          if( (index = f->searchStepPrevPoint( preIndex, cP, 10 )) != -1 )
            {
              // if close to edge, recenter map to next point
              prePos = _globalMapMatrix->map( cP.projP );

              if( (prePos.x() < MIN_X_TO_PAN) || (prePos.x() > MAX_X_TO_PAN) ||
                  (prePos.y() < MIN_Y_TO_PAN) || (prePos.y() > MAX_Y_TO_PAN) )
                {
                  _globalMapMatrix->centerToPoint( prePos );
                  __redrawMap();
                }

              emit showFlightPoint( _globalMapMatrix->wgsToMap( cP.origP ), cP );
              prePos = _globalMapMatrix->map( cP.projP );
              preIndex = index;

              QPainter p1( this );
              p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
            }
        }
    }
}

void Map::slotFlightHome()
{
  FlightPoint cP;
  int index;

  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
          QPainter p1( this );
          p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                         pixBuffer,
                         prePos.x() - 20, prePos.y() - 20, 40, 40 );

          if ((index = f->searchGetNextPoint(0, cP)) != -1)
            {
              emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
              prePos = _globalMapMatrix->map(cP.projP);
              preIndex = index;

              QPainter p1( this );
              p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
            }
        }
    }
}

void Map::slotFlightEnd()
{
  FlightPoint cP;
  int index;
  BaseFlightElement *f = _globalMapContents->getFlight();

  if(f)
    {
      if (prePos.x() >= 0)   // only step if cross hair is shown in map.
        {
          QPainter p1( this );
          p1.drawPixmap( prePos.x() - 20, prePos.y() - 20,
                         pixBuffer,
                         prePos.x() - 20, prePos.y() - 20, 40, 40 );

          // just a workaround !!!!!!!!!!!!!
          if(f->getObjectType() == BaseMapElement::Flight)
            {
              if((index = f->searchGetNextPoint(((Flight *)f)->getRouteLength()-1, cP)) != -1)
                {
                  emit showFlightPoint(_globalMapMatrix->wgsToMap(cP.origP), cP);
                  prePos = _globalMapMatrix->map(cP.projP);
                  preIndex = index;

                  QPainter p1( this );
                  p1.drawPixmap( prePos.x() - 20, prePos.y() - 20, pixCursor );
                }
            }
        }
    }
}

/** No descriptions */
void Map::slotShowCurrentFlight()
{
  BaseFlightElement *bfe = _globalMapContents->getFlight();

  // just to make sure ...
  slotAnimateFlightStop();

  planning = 0;

  if( bfe && bfe->getObjectType() == BaseMapElement::Task )
    {
      if( bfe->getWPList().count() == 0 )
        {
          slotActivatePlanning();
        }
      else
        {
          planning = 2;
          emit setStatusBarMsg( "" );
        }
    }

  slotRedrawFlight();

  if( bfe )
    {
      switch( bfe->getObjectType() )
        {
          case BaseMapElement::Flight:
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
void Map::slotAppendWaypoint2Task( Waypoint *p )
{
  if( planning != 1 )
    {
      return;
    }

  FlightTask *ft = dynamic_cast<FlightTask *> ( _globalMapContents->getFlight() );

  if( ft && ft->getObjectType() == BaseMapElement::Task && planning )
    {
      QList<Waypoint*> taskPointList = ft->getWPList();

      p->projP = _globalMapMatrix->wgsToMap( p->origP );

      // Make a deep copy of waypoint
      taskPointList.append( new Waypoint(p) );

      ft->setWaypointList( taskPointList );

      __drawPlannedTask();
      __showLayer();

      // We have to send an update trigger to the object tree.
      emit flightTaskModified();
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
      contentArray[2] = MapContents::OutLandingList;


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
                  wp->frequency = hitElement->getFrequency();
                  wp->country = hitElement->getCountry();
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
  // get map screen size
  int w = size().width();
  int h = size().height();

  QRect testRect(-10, -10, w + 20, h + 20);
  QString labelText;

  QList<Waypoint*> &wpList = _globalMapContents->getWaypointList();

  QPainter painter(&pixWaypoints);
  QFont font = painter.font();
  font.setPointSize( 10 );
  painter.setFont( font );
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));

  // step through the list
  for( int i = 0; i < wpList.size(); i++ )
  {
    Waypoint *wp = wpList.at(i);

    // make sure projection is ok
    wp->projP = _globalMapMatrix->wgsToMap(wp->origP.lat(), wp->origP.lon());

    // map the projected point to the screen
    QPoint mp = _globalMapMatrix->map(wp->projP);

    // Check, if point lays in the visible screen area
    if( ! testRect.contains(mp) )
      {
        // qDebug("Not in Rec wp=%s", wp.name.toLatin1().data());
        continue;
      }

    // draw marker
    painter.drawRect( mp.x() - 4, mp.y() - 4, 8, 8 );

    // draw name of wp
    if( _globalMapConfig->drawWpLabels() )
      {
        // save the current painter, must be restored at the end!!!
        painter.save();

        QString labelText = wp->name;

        // calculate text bounding box
        QRect dRec( 0, 0, 400, 400 );
        QRect textBox;

        textBox = painter.fontMetrics().boundingRect( dRec, Qt::AlignCenter, labelText );

        // add a little bit more space in the width
        textBox.setRect( 0, 0, textBox.width() + 8, textBox.height() );

        int xOffset = 0;
        int yOffset = 0;

        int xShift = 18;

        if( _globalMapConfig->useSmallIcons() )
          {
            xShift = 9;
          }

        if( wp->origP.lon() < _globalMapMatrix->getMapCenter(false).y() )
          {
            // The point is on the left side of the map,
            // so draw the text label on the right side.
            xOffset = xShift;
            yOffset = -textBox.height() / 2;
          }
        else
          {
            // The point is on the right side of the map,
            // so draw the text label on the left side.
            xOffset = -textBox.width() - xShift;
            yOffset = -textBox.height() / 2;
          }

        // move the textbox at the right position on the display
        textBox.setRect( mp.x() + xOffset,
                         mp.y() + yOffset,
                         textBox.width(), textBox.height() );

        painter.setBrush( Qt::white );
        painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));
        painter.drawRect( textBox );
        painter.drawText( textBox, Qt::AlignCenter, labelText );
        painter.restore();
     }
   }
}

/** Slot signaled when user selects another waypoint catalog.  */
void Map::slotWaypointCatalogChanged(WaypointCatalog* c)
{
  Waypoint *w;
  bool filterRadius, filterArea;

  QList<Waypoint*> &wpList = _globalMapContents->getWaypointList();

  qDeleteAll( wpList );
  wpList.clear();

  filterRadius = (c->getCenterPoint().lat() != 0 || c->getCenterPoint().lon() != 0);

  filterArea = (c->areaLat2 != 0 && c->areaLong2 != 0 && !filterRadius);

  foreach(w, c->wpList)
  {
        if( !c->showAll )
          {
            switch( w->type )
              {
              case BaseMapElement::IntAirport:
              case BaseMapElement::Airport:
              case BaseMapElement::MilAirport:
              case BaseMapElement::CivMilAirport:
              case BaseMapElement::Airfield:

                if( !c->showAirfields )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Gliderfield:

                if( !c->showGliderfields )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::UltraLight:
              case BaseMapElement::HangGlider:
              case BaseMapElement::Parachute:
              case BaseMapElement::Balloon:

                if( !c->showOtherSites )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Outlanding:

                if( !c->showOutlandings )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Obstacle:

                if( !c->showObstacles )
                  {
                    continue;
                  }
                break;

              case BaseMapElement::Landmark:

                if( !c->showLandmarks )
                  {
                    continue;
                  }
                break;
              }
          }

        if( filterArea )
          {
            if( w->origP.lat() < c->areaLat1 || w->origP.lat() > c->areaLat2 ||
                w->origP.lon() < c->areaLong1 || w->origP.lon()
                > c->areaLong2 )
              {
                continue;
              }
          }
        else if( filterRadius )
          {
            // We have to consider the user chosen distance unit.
            double catalogDist = Distance::convertToMeters( c->radiusSize ) / 1000.;

            // This distance is calculated in kilometers.
            double radiusDist = dist( c->getCenterPoint().lat(),
                                      c->getCenterPoint().lon(),
                                      w->origP.lat(),
                                      w->origP.lon() );

            if ( radiusDist > catalogDist )
              {
                continue;
              }
          }

        // add the waypoint to the list
        wpList.append( new Waypoint( w ) );
      }

  // force an update
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

  static const QBitmap cross = QBitmap::fromData( QSize(32,32), cross_bits );

  static const QCursor crossCursor(cross, cross);

  setCursor(crossCursor);
}

/** Creates the popup menu for the map */
void Map::__createPopupMenu()
{
  mapPopup = new QMenu(this);

  mapPopup->setTitle( QObject::tr("Map Menu") );

  QAction* action = mapPopup->addAction( tr("Map Menu") );
  action->setData( -1 );

  mapPopup->addSeparator();

  miAddWaypointAction = mapPopup->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                                             QObject::tr("&New waypoint"),
                                             this,
                                             SLOT(slotMpNewWaypoint()) );

  miEditWaypointAction = mapPopup->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                              QObject::tr("&Edit waypoint"),
                                              this,
                                              SLOT(slotMpEditWaypoint()) );

  miDeleteWaypointAction = mapPopup->addAction( _mainWindow->getPixmap("kde_editdelete_16.png"),
                                                QObject::tr("&Delete waypoint"),
                                                this,
                                                SLOT(slotMpDeleteWaypoint()));
  mapPopup->addSeparator();

  miShowMapInfoAction = mapPopup->addAction( _mainWindow->getPixmap("kde_info_16.png"),
                                             QObject::tr("&Show map info..."),
                                             this,
                                             SLOT(slotMpShowMapInfo()) );
  mapPopup->addSeparator();

  miCenterMapAction = mapPopup->addAction( _mainWindow->getPixmap("centerto_22.png"),
                                           QObject::tr("&Center map"),
                                           this,
                                           SLOT(slotMpCenterMap()) );

  miZoomInAction = mapPopup->addAction( _mainWindow->getPixmap("kde_viewmag+_16.png"),
                                        QObject::tr("Zoom &In"),
                                        _globalMapMatrix,
                                        SLOT(slotZoomIn()) );

  miZoomOutAction = mapPopup->addAction( _mainWindow->getPixmap("kde_viewmag-_16.png"),
                                         QObject::tr("Zoom &Out"),
                                         _globalMapMatrix,
                                         SLOT(slotZoomOut()) );
  /*
  miAddTaskPointAction
 */
}

/** Enable/disable the correct items for the map menu and then shows it. */
void Map::__showPopupMenu(QMouseEvent* event)
{
  if( findWaypoint( event->pos() ) )
    {
      miAddWaypointAction->setEnabled( false );
      miEditWaypointAction->setEnabled( true );
      miDeleteWaypointAction->setEnabled( true );
    }
  else
    {
      miAddWaypointAction->setEnabled( true );
      miEditWaypointAction->setEnabled( false );
      miDeleteWaypointAction->setEnabled( false );
    }

  mapPopup->exec( mapToGlobal( event->pos() ) );
}

/** called from the Map popup menu to add a new waypoint. */
void Map::slotMpNewWaypoint()
{
  QString text;

  QPoint sitePos;
  double dX, dY, delta( 16.0 );

  const QPoint current( popupPos );

  // select WayPoint
  QRegExp blank( "[ ]" );

  // add WPList !!!
  int searchList[] = { MapContents::AirfieldList,
                       MapContents::GliderfieldList,
                       MapContents::OutLandingList };

  for( int l = 0; l < 3; l++ )
    {
      for( int loop = 0; loop < _globalMapContents->getListLength( searchList[l] ); loop++ )
        {
          Airfield *hitElement =
              static_cast<Airfield *> ( _globalMapContents->getElement( searchList[l], loop ) );
          sitePos = hitElement->getMapPosition();

          dX = abs( sitePos.x() - current.x() );
          dY = abs( sitePos.y() - current.y() );

          // Abstand entspricht der Icon-Größe.
          if( dX < delta && dY < delta )
            {
              Waypoint *w = new Waypoint;

              QString name = hitElement->getName();
              w->name = name.replace( blank, "" ).left( 8 ).toUpper();
              w->description = hitElement->getName();
              w->type = hitElement->getObjectType();
              w->origP = hitElement->getWGSPosition();
              w->elevation = hitElement->getElevation();
              w->icao = hitElement->getICAO();
              w->frequency = hitElement->getFrequency();
              w->country = hitElement->getCountry();
              w->runway.first = 0;
              w->runway.second = 0;
              w->length = 0;
              w->surface = Runway::Unknown;

              if( hitElement->getRunwayNumber() )
                {
                  w->runway = hitElement->getRunway( 0 )->getRunwayDirection();
                  w->length = hitElement->getRunway( 0 )->length;
                  w->surface = hitElement->getRunway( 0 )->surface;
                }

              w->comment = hitElement->getComment();
              w->isLandable = hitElement->isLandable();

              emit waypointSelected( w );
              return;
            }
        }
    }

  // create a new waypoint
  WaypointDialog* waypointDlg = __openWaypointDialog( current );
  waypointDlg->exec();
  delete waypointDlg;
}


/** called from the Map Popup menu to edit waypoint. */
void Map::slotMpEditWaypoint()
{
  Waypoint* wp = findWaypoint( popupPos );

  if( wp )
    {
      emit waypointEdited( wp );
    }
}

/** called from the Map Popup menu to delete waypoint. */
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
   // Move Map
  _globalMapMatrix->centerToPoint(popupPos);
  __redrawMap();
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

WaypointDialog* Map::__openWaypointDialog( const QPoint &position )
{
  WaypointDialog *waypointDlg = new WaypointDialog( this );
  waypointDlg->enableApplyButton( false );

  // register the dialog and connect it's signals.
  emit regWaypointDialog( waypointDlg );

  QPoint p = _globalMapMatrix->mapToWgs( position );

  // initialize dialog coordinates
  waypointDlg->setWindowTitle( tr( "Create a new Waypoint" ) );
  waypointDlg->longitude->setKFLogDegree( p.x() );
  waypointDlg->latitude->setKFLogDegree( p.y() );

  QPoint mapPos = _globalMapMatrix->invertToMap( position );

  Distance errorDist;

  int height = _globalMapContents->getElevation( mapPos, &errorDist );

  if( height != -1 )
    {
      // Elevation plus error distance is used.
      waypointDlg->elevation->setText( QString::number(height) );
    }

  return waypointDlg;
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
