/***********************************************************************
**
**   map.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  1999-2000 by Heiner Lamprecht, Florian Ehinger
**                   2010-2013 by Axel Pauli
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
#include "target.h"

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
  preStepIndex(-1),
  drawFlightStepCursor(false),
  animationPaused(false),
  planning(0),
  tempTask(""),
  startDragZoom(false),
  isDragZoomActive(false),
  isMapMoveActive(false),
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
  pixGliders = _mainWindow->getPixmap("gliders.png");

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
     "<P>To move or scale the map, you can use the buttons in the "
     "<B>Map control area</B>.</P>"
     "<P>To zoom in or out, use the slider or the two buttons on the "
     "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" or \"+\" (zoom in) "
     "and \"&lt;Ctrl&gt;&lt;-&gt;\" or \"-\"(zoom out).</P>"
     "<P>The cursor keys and the keys on the NumPad can also pan the map, if "
     "NumLock is switched on.</P>"
     "<P>With the menu item <b>Settings-&gt;KFLog Setup-&gt;Map Elements</b> you can "
     "configure, which map elements should be displayed at which scale.</P>"
     "<b>Supported mouse actions at the map:</b>"
     "<ul>"
     "<li>Moving the mouse pointer at a map item will display the item data after a short delay, "
     "if the <i>Show map data touched by Mouse</i> option is enabled."
     "<li>Pressing the right mouse button opens the map menu with different possible actions."
     "<li>Pressing the middle mouse button centers the map to the mouse point."
     "<li>Pressing the left mouse button opens the task menu, if graphical task planning is activated."
     " Otherwise you can move the map so long the left mouse button is pressed during move."
     "<li>Pressing the shift key and the left mouse button over a map item"
     " creates a new waypoint by using the item data. If no item is touched by the mouse"
     " the waypoint dialog is opened and the user can enter the waypoint data."
     "<li>Pressing the control key and the left mouse button "
     "and hold both down during drag will draw a frame at the map. Release the mouse "
     "button to zoom the map into the frame."
     "<li>Turning the mouse wheel will zoom into or out the map."
     "</ul>"
     "</html>"
      ) );

  __createPopupMenu();

  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), SLOT(slotAnimateFlightTimeout()) );

  mapInfoTimer = new QTimer(this);
  mapInfoTimer->setSingleShot( true );
  connect (mapInfoTimer, SIGNAL(timeout()), SLOT(slotMapInfoTimeout()));

  /** Create a timer for queuing draw events. */
  redrawMapTimer = new QTimer(this);
  redrawMapTimer->setSingleShot(true);
  connect( redrawMapTimer, SIGNAL(timeout()), this, SLOT(slotRedrawMap()));

  /** Create a timer for map move redraw control. */
  timerMapMove = new QTimer(this);
  timerMapMove->setSingleShot(true);
  connect( timerMapMove, SIGNAL(timeout()), this, SLOT(slotMapMoveTimeout()));
}

Map::~Map()
{
}

/**
 * \return List with all current active flights.
 */
QList<Flight *> Map::getFlightList()
{
  QList<Flight *> fl;

  BaseFlightElement* bfe = _globalMapContents->getFlight();

  if( ! bfe )
    {
      return fl;
    }

  switch( bfe->getObjectType() )
    {
      case BaseMapElement::Flight:

        fl.append( dynamic_cast<Flight *>(bfe) );
        break;

      case BaseMapElement::FlightGroup:
        {
          FlightGroup* fg = dynamic_cast<FlightGroup *>(bfe);
          fl = fg->getFlightList();
          break;
        }

      default:
        break;
    }

  return fl;
}

void Map::mouseMoveEvent( QMouseEvent* event )
{
  if( isMapMoveActive )
    {
      // Activate supervision timer for map redrawing
      timerMapMove->start(500);
      event->accept();
      return;
    }

  const QPoint current = event->pos();

  QPoint vector = event->pos() - mapInfoTimerStartpoint;

  if( vector.manhattanLength() > 4 )
    {
      // don't start the timer when in planning or drag zoom mode or the user has
      // switched off the info display via the menu option.
      if( planning != 1 && planning != 3 && startDragZoom == false &&
          _settings.value( "/MapData/ViewDataUnderMouseCursor", false ).toBool() == true )
        {
          mapInfoTimer->start( MAP_INFO_DELAY );
          mapInfoTimerStartpoint = event->pos();
        }
      else
        {
          mapInfoTimer->stop();
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

  // Show flight data, if the mouse is in the near of a flight.
  if( ! timerAnimate->isActive() )
    {
      QList<Flight *> flightList = getFlightList();

      bool match = false;

      if( flightList.size() > 0 )
        {
          for( int i = 0; i < flightList.size(); i++ )
            {
              FlightPoint fP;
              int index;

              if( (index = flightList.at(i)->searchPoint( event->pos(), fP )) != -1 )
                {
                  emit showFlightPoint( fP.origP, fP );
                  match = true;

                  // Note the first matched flight is the winner in a group!
                  break;
                }
            }
        }

      if( ! match )
        {
          emit showPoint( _globalMapMatrix->mapToWgs( event->pos() ) );
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

  event->accept();
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

  QString text;

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

  // let's show city names
  if( m_drawnCityList.size() > 0 )
    {
      for( int i = 0; i <  m_drawnCityList.size(); i++ )
        {
          LineElement *city = static_cast<LineElement *> ( m_drawnCityList[i] );

          // The mouse position must be converted to map coordinates.
          const QPoint mapPos = _globalMapMatrix->invertToMap(current);

          if( city->getPolygon().containsPoint( mapPos, Qt::OddEvenFill ) )
            {
              text = city->getInfoString();
              WhatsThat* box = new WhatsThat( this, text, timeout, mapToGlobal( current ) );
              box->setVisible( true );
              return;
            }
        }
    }

  // At last look for airspace to be displayed.
  __displayAirspaceInfo(current, automatic);
}

void Map::__displayAirspaceInfo(const QPoint& current, bool automatic)
{
  int timeout = 60000;

  if( automatic )
    {
      timeout = 20000;
    }

  // At last search for airspaces.
  QString text = QString("<html><table border=1><tr><th align=left>") +
                 tr("Airspace&nbsp;Structure") +
                 "</th></tr>";

  QList<QPair<QPainterPath, Airspace *> >& airspaceRegionList = _globalMapContents->getAirspaceRegionList();

  bool show = false;

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

        break;

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

void Map::keyReleaseEvent( QKeyEvent* event )
{
  qDebug() << "Map::keyReleaseEvent()";

  if( event->modifiers() == Qt::ShiftModifier )
    {
      qDebug() << "Map::keyReleaseEvent()";
      __showLayer();
    }
  else
    {
      QWidget::keyPressEvent( event );
    }
}

void Map::mouseReleaseEvent( QMouseEvent* event )
{
  if( isDragZoomActive )
    {
      emit setStatusBarMsg( tr( "Ready." ) );
      QApplication::restoreOverrideCursor();
      startDragZoom = false;
      isDragZoomActive = false;
      pixZoomRect = QPixmap();

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
  else if( isMapMoveActive )
    {
      // Map move by mouse is active and finished. We compute the new map center point.
      timerMapMove->stop();
      emit setStatusBarMsg( tr( "Ready." ) );
      QApplication::restoreOverrideCursor();
      isMapMoveActive = false;

      QPoint dist = beginMapMove - event->pos();

      if( dist.manhattanLength() > 10 )
        {
          QPoint center( width()/2, height()/2 );

          center += dist;

          // Center Map to new center point
          _globalMapMatrix->centerToPoint( center );
          __redrawMap();
        }
    }

  event->accept();
}

void Map::mousePressEvent(QMouseEvent* event)
{
  // Stop map info timer
  mapInfoTimer->stop();

  if( startDragZoom )
    {
      // User has pressed key zero before. No other actions are possible now.
      emit setStatusBarMsg( tr( "Drag zooming on" ) );
      isDragZoomActive = true;
      beginDrag = event->pos();
      sizeDrag = QSize( 0, 0 );
      event->accept();
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
      event->accept();
      return;
    }

  if( event->button() == Qt::LeftButton )
    {
      // Check if task planning is active. In this case allow only planning actions.
      if( planning == 1 )
        {
          __graphicalPlanning( current, event );
          event->accept();
          return;
        }

      if( event->modifiers() == Qt::ControlModifier )
        {
          // User wants to make a drag zoom action.
          emit setStatusBarMsg( tr( "Drag zooming on" ) );
          startDragZoom = true;
          isDragZoomActive = true;
          beginDrag = event->pos();
          sizeDrag = QSize( 0, 0 );
          QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
          event->accept();
          return;
        }

      if( event->modifiers() == Qt::NoModifier )
        {
          // User wants to make a map move. Save the start position.
          emit setStatusBarMsg( tr( "Map move on" ) );
          isMapMoveActive = true;
          beginMapMove = event->pos();
          QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));
          event->accept();
          return;
        }

      if( event->modifiers() == Qt::ShiftModifier )
        {
          bool found = false;

          // Adds the map item to the Waypoint list, when found.
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
                    w->rwyList = hitElement->getRunwayList();

                    // That adds the found item to the current waypoint list.
                    emit waypointSelected(w);
                    found = true;
                    break;
                  }
               }

             if( found )
                {
                  break;
                }
          }

          if( !found )
              {
                // No map item found, we open the waypoint editor, that the user
                // can create a new waypoint at the current position.
                WaypointDialog* waypointDlg = __openWaypointDialog( current );
                waypointDlg->exec();
                delete waypointDlg;
              }
          }
    }
  else if( event->button() == Qt::RightButton )
    {
      // We display the map popup menu that the user can select an certain action.
      popupPos = event->pos();
      __showPopupMenu( event );
    }

  event->accept();
}

/**
 * Please note! Qt4 allows only to draw at the widget's paint engine inside
 * a paint event. Otherwise you will get displayed warnings.
 */
void Map::paintEvent( QPaintEvent* event )
{
  if( pixBuffer.isNull() )
    {
      qWarning() << "Map::paintEvent: Reject paintEvent pixBuffer is Null!";
      return;
    }

  QPainter painter(this);

  painter.drawPixmap( event->rect().left(),
                      event->rect().top(),
                      pixBuffer,
                      0, 0, event->rect().width(),
                      event->rect().height() );

  // Redraw the flight cursors on request.
  if( drawFlightCursors == true && pixFlightCursors.isNull() == false )
    {
      painter.drawPixmap( event->rect().left(),
                          event->rect().top(),
                          pixFlightCursors,
                          0, 0, event->rect().width(),
                          event->rect().height() );
    }

  // Draw the flight step cursor at the map on request.
  if( drawFlightStepCursor == true && pixCursor.isNull() == false )
    {
      painter.drawPixmap( preStepPos.x() - 20, preStepPos.y() - 20, pixCursor );
      drawFlightStepCursor = false;
    }

  if( isDragZoomActive == true && pixZoomRect.isNull() == false )
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
  for( int loop = lat2; loop <= lat1; loop += gridStep )
    {
      int size = (lon2 - lon1 + 1) * 10;

      QPolygon pointArray;

      for( int lonloop = 0; lonloop < size; lonloop++ )
        {
          QPoint p0 = _globalMapMatrix->wgsToMap( loop * 600000,
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

          gridP.setFont(QFont("Helvetica", 8, QFont::Normal));

          int lat = loop;

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
              QPoint p1 = _globalMapMatrix->wgsToMap( (int) rint((loop + (loop2 * (step / 60.0))) * 600000),
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

                  int lat = loop;

                  QString text = QString("%1%2.5%3")
                                 .arg( lat >= 0 ? "N" : "S" )
                                 .arg( lat >= 0 ? lat : -lat - 1)
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
  // qDebug() << "Map::__drawMap()";

  QPainter aeroP(&pixAero);
  QPainter uMapP(&pixUnderMap);
  QPainter isoMapP(&pixIsoMap);

  m_drawnCityList.clear();
  QList<BaseMapElement *> drawnElements;

  // Take the color of the subterrain for filling
  pixIsoMap.fill( _globalMapConfig->getIsoColor(0) );

  _globalMapContents->drawIsoList( &isoMapP, rect() );

  emit setStatusBarProgress(10);

  _globalMapContents->drawList(&uMapP, MapContents::TopoList, drawnElements);

  _globalMapContents->drawList(&uMapP, MapContents::CityList, m_drawnCityList);

  _globalMapContents->drawList(&uMapP, MapContents::HydroList, drawnElements);

  _globalMapContents->drawList(&uMapP, MapContents::LakeList, drawnElements);

  emit setStatusBarProgress(15);

  _globalMapContents->drawList(&uMapP, MapContents::RoadList, drawnElements);

  _globalMapContents->drawList(&uMapP, MapContents::HighwayList, drawnElements);

  emit setStatusBarProgress(25);

  _globalMapContents->drawList(&uMapP, MapContents::RailList, drawnElements);

  emit setStatusBarProgress(35);

  _globalMapContents->drawList(&uMapP, MapContents::VillageList, drawnElements);

  emit setStatusBarProgress(45);

  _globalMapContents->drawList(&uMapP, MapContents::LandmarkList, drawnElements);

  emit setStatusBarProgress(50);

  _globalMapContents->drawList(&uMapP, MapContents::ObstacleList, drawnElements);

  emit setStatusBarProgress(55);

  _globalMapContents->drawList(&aeroP, MapContents::ReportList, drawnElements);

  emit setStatusBarProgress(60);

  _globalMapContents->drawList(&aeroP, MapContents::NavList, drawnElements);

  uMapP.end();

  if( _globalMapMatrix->getScale( MapMatrix::CurrentScale ) <= 100.0 )
    {
      __drawCityLabels( pixUnderMap );
    }

  emit setStatusBarProgress(65);

  __drawAirspaces();

  emit setStatusBarProgress(70);

  emit setStatusBarProgress(75);

  _globalMapContents->drawList(&aeroP, MapContents::AirfieldList, drawnElements);

  emit setStatusBarProgress(80);

  _globalMapContents->drawList(&aeroP, MapContents::AddSitesList, drawnElements);

  emit setStatusBarProgress(85);

  _globalMapContents->drawList(&aeroP, MapContents::GliderfieldList, drawnElements);

  emit setStatusBarProgress(90);

  _globalMapContents->drawList(&aeroP, MapContents::OutLandingList, drawnElements);

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
  pixFlight.fill(Qt::transparent);
  QPainter flightP(&pixFlight);
  QList<BaseMapElement *> drawnElements;

  _globalMapContents->drawList( &flightP, MapContents::FlightList, drawnElements );
}

void Map::__drawPlannedTask( bool solid )
{
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
}

void Map::resizeEvent(QResizeEvent* event)
{
  if( ! event->size().isEmpty() )
    {
      __redrawMap();
    }
}

void Map::dragEnterEvent( QDragEnterEvent* event )
{
  if( event->mimeData()->hasUrls() )
    {
      event->acceptProposedAction();
    }
}

void Map::dropEvent( QDropEvent* event )
{
  QList<QUrl> urlList = event->mimeData()->urls();

  for( int i = 0; i < urlList.size(); i++ )
    {
      emit openFile( urlList.at(i) );
    }
}

void Map::__redrawMap()
{
  static QSize lastSize;

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
      pixBuffer = QPixmap( size() );
      pixBuffer.fill(Qt::transparent);
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
      redrawMapTimer->start(500);
      redrawRequest = false;
    }

  isDrawing = false;
}

/** Save Map to PNG-file with width, height. Use actual size if width=0 & height=0 */
void Map::slotSavePixmap(QUrl fUrl, int width, int height)
{
  int w_orig, h_orig;

  w_orig = h_orig = 0;

  if( !fUrl.isValid() || fUrl.scheme() != "file" )
    {
      return;
    }

  QString fName = fUrl.path();

  if( width && height )
    {
      w_orig = pixBuffer.width();
      h_orig = pixBuffer.height();
      resize( width, height );
    }

  slotCenterToFlight();

  // ensures that all is up to date
  repaint();

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  // Take over map content into new pixmap to avoid modifications at the map.
  QPixmap pixmap(pixBuffer);

  QPainter bufferP( &pixmap );
  bufferP.setPen( Qt::magenta );

  QFont font;
  font.setBold( true );
  font.setPointSize( 10 );
  font.setStyle( QFont::StyleItalic );
  font.setStyleHint( QFont::SansSerif );
  bufferP.setFont( font );

  QString msg = QString("%1created by KFLog %2 (www.kflog.org)")
      .arg( QChar(Qt::Key_copyright) )
      .arg( KFLOG_VERSION );

  bufferP.drawText( 10, 30, msg );

  if( flight != 0 )
    {
      QFontMetrics fm( font );
      int strWidth = fm.width( msg );

      QString text = tr( "%1 with %2 (%3) on %4" )
                    .arg( flight->getPilot() )
                    .arg( flight->getType() )
                    .arg( flight->getID() )
                    .arg( flight->getDate().toString() );

      bufferP.drawText( 10 + strWidth + 20, 30, text );
    }

  QImage image = QImage( pixmap.toImage() );

  image.save( fName, "png" );

  if( width && height )
    {
      resize( w_orig, h_orig );
      slotCenterToFlight();
    }
}

void Map::slotRedrawFlight()
{
  __drawFlight();
  __showLayer();
}

void Map::slotRedrawMap()
{
  // qDebug() << "Map::slotRedrawMap()";
  __redrawMap();
}

void Map::slotScheduleRedrawMap()
{
  redrawMapTimer->start(500);
}

void Map::slotActivatePlanning()
{
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
}

void Map::__showLayer()
{
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
}

void Map::__showFlightData( const QPoint& mapPos )
{
  // Show flight data, if position is in the near of a flight.
  QList<Flight *> flightList = getFlightList();

  if( flightList.size() > 0 )
    {
      for( int i = 0; i < flightList.size(); i++ )
        {
          FlightPoint fP;

          if( flightList.at(i)->searchPoint( mapPos, fP ) != -1 )
            {
              emit showFlightPoint( fP.origP, fP );

              // Note the first matched flight is the winner in a group!
              break;
            }
        }
    }
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

  QPoint pos1( _globalMapMatrix->map( p1 ) );
  QPoint pos2( _globalMapMatrix->map( p2 ) );

  if( p1 != lastCur1Pos )
    {
      __showFlightData( pos1 );
    }
  else if( p2 != lastCur1Pos )
    {
      __showFlightData( pos2 );
    }

  lastCur1Pos = p1;
  lastCur2Pos = p2;

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
  QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

  // That is needed by the mouse event handlers to recognize that a drag action
  // has to be started or handled respectively.
  startDragZoom = true;

  emit setStatusBarMsg( tr( "Drag zooming activated" ) );
}

void Map::slotMapMoveTimeout()
{
  // User has paused with mouse moving. We do redraw the map in the meantime.
  if( isMapMoveActive )
    {
      QPoint curPos = QWidget::mapFromGlobal( QCursor::pos() );

      QPoint dist = beginMapMove - curPos;

      if( dist.manhattanLength() > 20 )
        {
          beginMapMove = curPos;
          QPoint center( width()/2, height()/2 );

          center += dist;

          // Center Map to new center point
          _globalMapMatrix->centerToPoint( center );
          __redrawMap();
        }
    }
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

  emit changed( size() );
}

void Map::slotCenterToFlight()
{
  BaseFlightElement *bfe = _globalMapContents->getFlight();

  if( ! bfe )
    {
      return;
    }

  QRect r0, r1;

  switch( bfe->getObjectType() )
    {
      case BaseMapElement::Flight:

        r0 = dynamic_cast<Flight *>(bfe)->getFlightRect();
        break;

      case BaseMapElement::FlightGroup:
        {
          QList<Flight *> fl = dynamic_cast<FlightGroup *>(bfe)->getFlightList();
          r0 = fl.at( 0 )->getFlightRect();

          for( int i = 1; i < fl.count(); i++ )
            {
              r1 = fl.at( i )->getFlightRect();
              r0 = r0.united( r1 );
            }

          break;
        }

      default:

        return;
    }

  // check if the Rectangle is not zero
  if( !r0.isNull() )
    {
      _globalMapMatrix->centerToRect( r0 );
      __redrawMap();
    }

  emit changed( size() );
}

void Map::slotCenterToTask()
{
  BaseFlightElement *bfe = _globalMapContents->getFlight();

  if( ! bfe )
    {
      return;
    }

  QRect r0, r1;

  switch( bfe->getObjectType() )
    {
      case BaseMapElement::Flight:

        r0 = dynamic_cast<Flight *>(bfe)->getTaskRect();
        break;

      case BaseMapElement::Task:

        r0 = dynamic_cast<FlightTask *>(bfe)->getRect();
        break;

      case BaseMapElement::FlightGroup:
        {
          QList<Flight *> fl = dynamic_cast<FlightGroup *>(bfe)->getFlightList();
          r0 = fl.at( 0 )->getTaskRect();

          for( int i = 1; i < fl.count(); i++ )
            {
              r1 = fl.at( i )->getTaskRect();
              r0 = r0.united( r1 );
            }

          break;
        }

      default:

        return;
    }

  // check if the Rectangle is not zero
  if( !r0.isNull() )
    {
      _globalMapMatrix->centerToRect( r0 );
      __redrawMap();
    }

  emit changed( size() );
}

/**
 * Called to start the animation timer.
 */
void Map::slotAnimateFlightStart()
{
  QList<Flight *> flightList = getFlightList();

  if( flightList.size() == 0 )
    {
      return;
    }

  if( animationPaused )
    {
      // Animation was paused, continue animation.
      animationPaused = false;
      timerAnimate->start( 50 );
      return;
    }

  // Loop through the flight list and reset animation flag.
  for( int i = 0; i < flightList.size(); i++ )
    {
      Flight *flight = flightList.at(i);

      flight->setAnimationIndex(0);
      flight->setAnimationActive(true);
    }

  // flights will not be visible as nAnimationIndex is zero for all flights to animate.
  slotRedrawFlight();

  for( int i = 0; i < flightList.size(); i++ )
    {
      Flight *flight = flightList.at(i);
      FlightPoint cP = flight->getPoint(0);
      QPoint prePos  = _globalMapMatrix->map(cP.projP);
      QPoint pos     = prePos;

      // Save map part, which will be overwritten by glider symbol.
      QPixmap pix = pixBuffer.copy( pos.x() - 20, pos.y() - 20, 40, 40 );

      flight->setLastAnimationPos(pos);
      flight->setLastAnimationPixmap(pix);

      int bearing = (int) rint(cP.bearing * 180.0 / M_PI);

      // We only rotate in steps of 15 degrees.
      int rot = (( bearing + 7 ) / 15 ) % 24;

      // Draw the right glider symbol at the map.
      QPainter p( &pixBuffer );
      p.drawPixmap( pos.x() - 20, pos.y() - 20, pixGliders, rot*40, 0, 40, 40 );
    }

  // Force an immediate redraw of the map to see the animation.
  repaint();

  // start 50ms timer
  timerAnimate->start( 50 );
}

/**
 * Called to pause the animation timer.
 */
void Map::slotAnimateFlightPause()
{
  animationPaused = true;
  timerAnimate->stop();
}

/**
 * Called for every timeout of the animation timer.
 */
void Map::slotAnimateFlightTimeout()
{
  bool bDone = true;

  QList<Flight *> flightList = getFlightList();

  if( flightList.size() == 0 )
    {
      animationPaused = false;
      timerAnimate->stop();
      return;
    }

  // Loop through the flight list and reset animation flag.
  for( int i = 0; i < flightList.size(); i++ )
    {
      Flight *flight = flightList.at(i);

      flight->setAnimationNextIndex();

      if( flight->isAnimationActive() )
        {
          bDone = false;
        }

      // Write info from current point on statusbar. The last flight in the list
      // is always the winner.
      FlightPoint cP = flight->getPoint( (flight->getAnimationIndex()) );
      QPoint pos = _globalMapMatrix->map( cP.projP );
      QPoint lastpos = flight->getLastAnimationPos();
      QPixmap& pix = flight->getLastAnimationPixmap();
      emit showFlightPoint( cP.origP, cP );

      // Erase previous glider symbol
      QPainter p;
      p.begin( &pixBuffer );
      p.drawPixmap( lastpos.x() - 20, lastpos.y() - 20, pix );

      // Draw flight up to this point in pixFlight.
      __drawFlight();

      // draw flight at the map.
      p.drawPixmap( 0, 0, pixFlight );

      // save map part for next timeout
      pix = pixBuffer.copy( pos.x() - 20, pos.y() - 20, 40, 40 );
      flight->setLastAnimationPos(pos);

      int bearing = (int) rint(cP.bearing * 180.0 / M_PI);

      // We only rotate in steps of 15 degrees.
      int rot = (( bearing + 7 ) / 15 ) % 24;

      // draw the right glider symbol at the map
      p.drawPixmap( pos.x() - 20, pos.y() - 20, pixGliders, rot*40, 0, 40, 40 );
      p.end();
    }

  if( bDone )
    {
      // if one of the flights still is active, bDone will be false
      timerAnimate->stop();
    }
  else
    {
      // force a paint event of the map
      repaint();
    }
}

/**
 * Called to stop the animation timer.
 */
void Map::slotAnimateFlightStop()
{
  if( ! timerAnimate->isActive() && animationPaused == false )
    {
      return;
    }

  // stop animation timer on user request.
  timerAnimate->stop();

  // Reset animation pause flag
  animationPaused = false;

  QList<Flight *> flightList = getFlightList();

  if( flightList.size() == 0 )
    {
      return;
    }

  // Loop through the flight list and reset animation flag.
  for( int i = 0; i < flightList.size(); i++ )
    {
      flightList.at(i)->setAnimationIndex(0);
      flightList.at(i)->setAnimationActive(false);
    }

  slotRedrawFlight();
}

/**
 * Stepping slots.
 */
void Map::slotFlightNext()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight || preStepIndex == -1 )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  // get the next point, preStepIndex now holds last point
  if( (index = flight->searchGetNextPoint( preStepIndex, cP )) != -1 )
    {
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;

      if( preStepPos.x() < MIN_X_TO_PAN || preStepPos.x() > MAX_X_TO_PAN ||
          preStepPos.y() < MIN_Y_TO_PAN || preStepPos.y() > MAX_Y_TO_PAN )
        {
          _globalMapMatrix->centerToPoint( preStepPos );
          preStepPos = _globalMapMatrix->map( cP.projP );
        }

      emit showFlightPoint( cP.origP, cP );
      drawFlightStepCursor = true;
      repaint();
    }
}

void Map::slotFlightPrev()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight || preStepIndex == -1 )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  // get the next point, preStepIndex now holds last point
  if( (index = flight->searchGetPrevPoint(preStepIndex, cP)) != -1 )
    {
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;

      if( preStepPos.x() < MIN_X_TO_PAN || preStepPos.x() > MAX_X_TO_PAN ||
          preStepPos.y() < MIN_Y_TO_PAN || preStepPos.y() > MAX_Y_TO_PAN )
        {
          _globalMapMatrix->centerToPoint( preStepPos );
          preStepPos = _globalMapMatrix->map( cP.projP );
        }

      emit showFlightPoint( cP.origP, cP );
      drawFlightStepCursor = true;
      repaint();
    }
}

void Map::slotFlightStepNext()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight || preStepIndex == -1 )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  // get the next point, preStepIndex now holds last point
  if( (index = flight->searchStepNextPoint(preStepIndex, cP, 10)) != -1 )
    {
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;

      if( preStepPos.x() < MIN_X_TO_PAN || preStepPos.x() > MAX_X_TO_PAN ||
          preStepPos.y() < MIN_Y_TO_PAN || preStepPos.y() > MAX_Y_TO_PAN )
        {
          _globalMapMatrix->centerToPoint( preStepPos );
          preStepPos = _globalMapMatrix->map( cP.projP );
        }

      emit showFlightPoint( cP.origP, cP );
      drawFlightStepCursor = true;
      repaint();
    }
}

void Map::slotFlightStepPrev()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight || preStepIndex == -1 )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  // get the next point, preStepIndex now holds last point
  if( (index = flight->searchStepPrevPoint( preStepIndex, cP, 10 )) != -1 )
    {
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;

      if( preStepPos.x() < MIN_X_TO_PAN || preStepPos.x() > MAX_X_TO_PAN ||
          preStepPos.y() < MIN_Y_TO_PAN || preStepPos.y() > MAX_Y_TO_PAN )
        {
          _globalMapMatrix->centerToPoint( preStepPos );
          preStepPos = _globalMapMatrix->map( cP.projP );
        }

      emit showFlightPoint( cP.origP, cP );
      drawFlightStepCursor = true;
      repaint();
    }
}

void Map::slotFlightHome()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  if( (index = flight->searchGetNextPoint( 0, cP )) != -1 )
    {
      emit showFlightPoint( cP.origP, cP );
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;
      drawFlightStepCursor = true;
      repaint();
    }
}

void Map::slotFlightEnd()
{
  if( timerAnimate->isActive() )
    {
      // Animation is running, do nothing.
      return;
    }

  Flight* flight = dynamic_cast<Flight *> ( _globalMapContents->getFlight() );

  if( ! flight )
    {
      // The action is only supported for single flight.
      return;
    }

  FlightPoint cP;
  int index;

  if( (index = flight->searchGetNextPoint(flight->getRouteLength() - 1, cP )) != -1 )
    {
      emit showFlightPoint( cP.origP, cP );
      preStepPos = _globalMapMatrix->map( cP.projP );
      preStepIndex = index;
      drawFlightStepCursor = true;
      repaint();
    }
}

/** No descriptions */
void Map::slotShowCurrentFlight()
{
  // Reset pre-step items.
  preStepIndex = -1;
  drawFlightStepCursor = false;

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
                  wp->rwyList = hitElement->getRunwayList();
                  wp->comment = hitElement->getComment();

                  found = true;
                  break;
                }
            }
        }
    }

  return found;
}

/** Draws the waypoints of the active waypoint catalog to the map */
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

void Map::__drawCityLabels( QPixmap& pixmap )
{
  if( m_drawnCityList.size() == 0 )
    {
      return;
    }

  QString labelText;

  QPainter painter(&pixmap);
  QFont font = painter.font();
  font.setPointSize( 6 );
  painter.setFont( font );
  painter.setBrush(Qt::NoBrush);
  painter.setPen(QPen(Qt::black, 2, Qt::SolidLine));

  QSet<QString> set;

  for( int i = 0; i < m_drawnCityList.size(); i++ )
    {
      LineElement* city = static_cast<LineElement *> (m_drawnCityList.at(i));

      // A city can consist of several segments at a border edge but we want to
      // draw the name only once.
      if( ! set.contains( city->getName() ) )
        {
          // map polygon to the screen
          QPolygon mP( _globalMapMatrix->map( city->getPolygon() ) );

          QRect bRect = mP.boundingRect();

          painter.drawText( bRect.x() + bRect.width() / 2,
                            bRect.y() + bRect.height() / 2 + 3,
                            city->getName() );

          set.insert( city->getName() );
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

  if( c == 0 )
    {
      emit changed(this->size());
      __redrawMap();
      return;
    }

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

  miShowMapInfoAction = mapPopup->addAction( _mainWindow->getPixmap("kde_info_16.png"),
                                             QObject::tr("&Show item info"),
                                             this,
                                             SLOT(slotMpShowMapItemInfo()) );

  miShowMapAirspaceInfoAction = mapPopup->addAction( _mainWindow->getPixmap("kde_info_16.png"),
                                                     QObject::tr("&Show airspace info"),
                                                     this,
                                                     SLOT(slotMpShowAirspaceInfo()) );
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
              w->rwyList = hitElement->getRunwayList();
              w->comment = hitElement->getComment();

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

void Map::slotMpShowMapItemInfo()
{
  mapInfoTimer->stop();
  __displayMapInfo(popupPos, false);
}

void Map::slotMpShowAirspaceInfo()
{
  mapInfoTimer->stop();
  __displayAirspaceInfo(popupPos, false);
}

void Map::leaveEvent( QEvent *event )
{
  mapInfoTimer->stop();
  mapInfoTimerStartpoint = QPoint( -999, -999 );
  event->accept();
}

void Map::wheelEvent(QWheelEvent *event)
{
  int numDegrees = event->delta() / 8;
  int numSteps = numDegrees / 15;

  if( abs(numSteps) > 10 || numSteps == 0 )
    {
      event->ignore();
      return;
    }

  if( numSteps > 0 )
    {
      _globalMapMatrix->slotZoomIn( numSteps );
    }
  else
    {
      _globalMapMatrix->slotZoomOut( -numSteps );
    }

  event->accept();
}

void Map::slotMapInfoTimeout()
{
  if( _settings.value( "/MapData/ViewDataUnderMouseCursor", false ).toBool() == true )
    {
      __displayMapInfo( mapInfoTimerStartpoint, true );
    }
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
      waypointDlg->setElevation( height );
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
