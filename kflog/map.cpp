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

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kstddirs.h>

#include <qdragobject.h>
#include <qpainter.h>
#include <qwhatsthis.h>
#include <ctype.h>
#include <stdlib.h>

#include <airspace.h>
#include <flight.h>
#include <flightgroup.h>
#include <kflog.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>
#include <singlepoint.h>
#include <radiopoint.h>
#include "wp.h"
#include "waypointdialog.h"

#include <iostream.h>

// Festlegen der Größe der Pixmaps auf Desktop-Grösse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

Map::Map(KFLogApp *m, QFrame* parent, const char* name)
  : QWidget(parent, name),
    mainApp(m), prePos(-50, -50), preCur1(-50, -50), preCur2(-50, -50),
    tempTask("")
{
  // defining the cursor for the map:
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
  bitMapMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitAirspaceMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitPlanMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitFlightMask.resize( PIX_WIDTH, PIX_HEIGHT );

//  pixAnimate.resize(32,32);
//  pixAnimate.fill(white);

  airspaceRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);

  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setCursor(crossCursor);
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
			     "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" (zoom in) "
			     "and \"&lt;Ctrl&gt;&lt;-&gt;\" (zoom out).</P>"
			     "<P>With the menu-item \"Options\" -> \"Configure KFLog\" you can "
			     "configure, which map elements should be displayed at which "
			     "scale.</P>"));

  // create the animation timer
  timerAnimate = new QTimer( this );
  connect( timerAnimate, SIGNAL(timeout()), this,
	   SLOT(slotAnimateFlightTimeout()) );      
}

Map::~Map()
{

}


void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  int index;
  BaseFlightElement *f = _globalMapContents.getFlight();
  const QPoint current = event->pos();
  struct wayPoint wp;
  if (planning == 1 || planning == 3) {
    QList<wayPoint> taskPointList = f->getWPList();
    QList<wayPoint> tempTaskPointList = f->getWPList();
    bool found = __getTaskWaypoint(current, &wp, taskPointList);
    // 3: Task beendet verschieben eines Punktes
//    SinglePoint* hitElement;

//    QPoint sitePos, preSitePos, nextSitePos;
    QPoint preSitePos, nextSitePos;
    QPoint point = current;
    double dX, dY, delta(8.0);

    bool drawWP = false;

    if (_globalMapMatrix.isSwitchScale()) delta = 16.0;


    if (!taskPointList.isEmpty()) {
      double dX_old = delta + 10.0;
      double dY_old = delta + 10.0;
      /*
       *  Muss für alle Punktdaten durchgeführt werden
       */
      if(planning == 1) {
        preSitePos = _globalMapMatrix.map(taskPointList.getLast()->projP);
      }
      else if(planning == 3) {
        if(moveWPindex > 0) {
          preSitePos = _globalMapMatrix.map(taskPointList.at(moveWPindex - 1)->projP);
        }
        if(moveWPindex + 1 < (int)taskPointList.count()) {
          nextSitePos = _globalMapMatrix.map(taskPointList.at(moveWPindex + 1)->projP);
        }
      }

      point.setX(current.x());
      point.setY(current.y());

      QPainter planP(this);
      planP.setRasterOp(XorROP);
      planP.setBrush(NoBrush);
      planP.setPen(QPen(QColor(255,0,0), 5));




      if(isSnapping) {
        // Snapping Bereich verlassen?
        if( (abs(prePlanPos.x() - current.x()) > delta) ||
            (abs(prePlanPos.y() - current.y()) > delta) ) {
          isSnapping = false;

          // delete temp WP
          __drawPlannedTask();
          __showLayer();
        }
      }

      if(!isSnapping) {
        if (found) {
          isSnapping = true;

          //                      dX_old = dX;
          //                      dY_old = dY;
        point = _globalMapMatrix.map(_globalMapMatrix.wgsToMap(wp.origP.y(), wp.origP.x()));
//          point.setX(sitePos.x());
//          point.setY(sitePos.y());
        }

        if(isSnapping) {
          // temporärer Weg
          // add temp WP

          if(planning == 3) {
            //verschieben
            tempTaskPointList.insert(moveWPindex,new wayPoint);
            tempTaskPointList.at(moveWPindex)->name = wp.name;
            tempTaskPointList.at(moveWPindex)->origP = wp.origP;
            tempTaskPointList.at(moveWPindex)->elevation = wp.elevation;
            tempTaskPointList.at(moveWPindex)->projP = wp.projP;
            // hier müssen noch mehr Sachen übergeben werden
          }
          else {
            //anhängen
            tempTaskPointList.append(new wayPoint);
            tempTaskPointList.last()->name = wp.name;
            tempTaskPointList.last()->origP = wp.origP;
            tempTaskPointList.last()->elevation = wp.elevation;
            tempTaskPointList.last()->projP = wp.projP;
            // hier müssen noch mehr Sachen übergeben werden

          }
          tempTask.setWaypointList(tempTaskPointList);
          __drawPlannedTask(false);

          if((current.x() > 0 && current.x() < this->width()) &&
             (current.y() > 0 && current.y() < this->height())) {
            prePlanPos = point;
          }
          else {
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
              __drawPlannedTask();
              __showLayer();

              if(event->state() == QEvent::ShiftButton)
                {
                  // temporärer Weg
                  if(planning == 3)
                    {
                      //verschieben
                      tempTaskPointList.insert(moveWPindex,new wayPoint);
                      tempTaskPointList.at(moveWPindex)->name = "";
                      tempTaskPointList.at(moveWPindex)->origP = _globalMapMatrix.mapToWgs(event->pos());
                      tempTaskPointList.at(moveWPindex)->projP =
                            _globalMapMatrix.wgsToMap(tempTaskPointList.at(moveWPindex)->origP.y(),
                                                      tempTaskPointList.at(moveWPindex)->origP.x());
                      // hier müssen noch mehr Sachen übergeben werden
                    }
                  else
                    {
                      //anhängen
                      tempTaskPointList.append(new wayPoint);
                      tempTaskPointList.last()->name = "test";
                      tempTaskPointList.last()->origP = _globalMapMatrix.mapToWgs(event->pos());
                      tempTaskPointList.last()->projP =
                            _globalMapMatrix.wgsToMap(tempTaskPointList.last()->origP.y(),
                                                      tempTaskPointList.last()->origP.x());
                      // hier müssen noch mehr Sachen übergeben werden
                    }
                tempTask.setWaypointList(tempTaskPointList);
                __drawPlannedTask(false);
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
      planP.end();
    }

    //      emit showTaskText(task,_globalMapMatrix.mapToWgs(point));

    //
    // Planen ende
    //
    //////////////
  }

  if (!timerAnimate->isActive()) {
    if(prePos.x() >= 0) {
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
             prePos.x() - 20, prePos.y() - 20, 40, 40);
    }
    flightPoint cP;

    if (f) {
      if ((index = f->searchPoint(event->pos(), cP)) != -1) {
        emit showFlightPoint(_globalMapMatrix.mapToWgs(event->pos()), cP);
        prePos = _globalMapMatrix.map(cP.projP);
        preIndex = index;
        bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
      }
      else {
        emit showPoint(_globalMapMatrix.mapToWgs(event->pos()));
        prePos.setX(-50);
        prePos.setY(-50);
      }
    }
  }
}

void Map::__displayMapInfo(QPoint current)
{
  /*
   * Segelflugplätze, soweit vorhanden, kommen als erster Eintrag
   */
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  BaseFlightElement *baseFlight = _globalMapContents.getFlight();

  SinglePoint *hitElement;

  QPoint sitePos;
  double dX, dY;
  // Radius for Mouse Snapping
  double delta(16.0);

  QString text;

  bool show = false, isAirport = false;

  for(unsigned int loop = 0;
      loop < _globalMapContents.getListLength(
		      MapContents::GliderList); loop++)
    {
      hitElement = (SinglePoint*)_globalMapContents.getElement(
          MapContents::GliderList, loop);
      sitePos = hitElement->getMapPosition();

      dX = sitePos.x() - current.x();
      dY = sitePos.y() - current.y();

      // Abstand entspricht der Icon-Größe.
      if( ( ( dX < delta ) && ( dX > -delta ) ) &&
          ( ( dY < delta ) && ( dY > -delta ) ) )
        {
          text = text + ((SinglePoint*)hitElement)->getInfoString();
          // Text anzeigen
          QWhatsThis::enterWhatsThisMode();
          QWhatsThis::leaveWhatsThisMode(text);
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

      dX = sitePos.x() - current.x();
      dY = sitePos.y() - current.y();

      // Abstand entspricht der Icon-Größe.
      if( ( ( dX < delta ) && ( dX > -delta ) ) &&
          ( ( dY < delta ) && ( dY > -delta ) ) )
        {
          text = text + hitElement->getInfoString();
          // Text anzeigen
          QWhatsThis::enterWhatsThisMode();
          QWhatsThis::leaveWhatsThisMode(text);
          isAirport = true;
        }
    }

  if(baseFlight && baseFlight->getTypeID() == BaseMapElement::Flight)
    {
      QList<wayPoint> wpList = baseFlight->getWPList();

      delta = 25;
      bool isWP = false;
      QString wpText;
      wpText = "<B>Waypoint:</B><UL>";

      for(unsigned int loop = 0; loop < wpList.count(); loop++)
        {
          sitePos = _globalMapMatrix.map(wpList.at(loop)->projP);

          dX = sitePos.x() - current.x();
          dY = sitePos.y() - current.y();

          // We do not search for the sector ...
          if( ( ( dX < delta ) && ( dX > -delta ) ) &&
              ( ( dY < delta ) && ( dY > -delta ) ) )
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
                  printPos(wpList.at(loop)->origP.x()) + " / " +
                  printPos(wpList.at(loop)->origP.y(), false) + "</LI>";
            }
        }

      if(isWP)
        {
          wpText = wpText + "</UL>";
          text = text + wpText;
          // Text anzeigen
          QWhatsThis::enterWhatsThisMode();
          QWhatsThis::leaveWhatsThisMode(text);
          isAirport = true;
        }
    }

  if(isAirport)  return;

  text = text + "<B>" + i18n("Airspace-Structure") + ":</B><UL>";

  for(unsigned int loop = 0; loop < airspaceRegList->count(); loop++)
    {
      if(airspaceRegList->at(loop)->contains(current))
        {
          text = text + "<LI>" + ((Airspace*)_globalMapContents.getElement(
              MapContents::AirspaceList, loop))->getInfoString() + "</LI>";
          show = true;
        }
    }
  text = text + "</UL>";

  if(show)
    {
      //  Text anzeigen
      QWhatsThis::enterWhatsThisMode();
      QWhatsThis::leaveWhatsThisMode(text);
    }
}

void Map::__graphicalPlanning(QPoint current, QMouseEvent* event)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  BaseFlightElement *baseFlight = _globalMapContents.getFlight();
  QList<wayPoint> taskPointList = baseFlight->getWPList();
  QList<wayPoint> tempTaskPointList = baseFlight->getWPList();
  struct wayPoint wp;
  QString text;

  bool found = __getTaskWaypoint(current, &wp, taskPointList);


  if(!taskPointList.isEmpty() && event->state() == QEvent::ControlButton)
    {
      // gleicher Punkt --> löschen
      for(unsigned int n = taskPointList.count() - 1; n > 0; n--)
        {
          if(wp.projP == taskPointList.at(n)->projP)
            {
              warning("lösche Punkt %d", n);
              taskPointList.remove(n);
            }
        }
     }



  if(event->button() == LeftButton)
    {
      if (found)
        {
          if(planning == 1)
            {
              // neuen Punkt an Task Liste anhängen
              warning("hänge Punkt an");

              taskPointList.append(new wayPoint);
              taskPointList.last()->name = wp.name;
              taskPointList.last()->origP = wp.origP;
              taskPointList.last()->elevation = wp.elevation;
              taskPointList.last()->projP = wp.projP;
              // hier müssen noch mehr Sachen übergeben werden
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
              warning("insert - verschiebe Punkt?");
              taskPointList.insert(moveWPindex,new wayPoint);

              taskPointList.at(moveWPindex)->name = wp.name;
              taskPointList.at(moveWPindex)->origP = wp.origP;
              taskPointList.at(moveWPindex)->elevation = wp.elevation;
              taskPointList.at(moveWPindex)->projP = wp.projP;
              // hier müssen noch mehr Sachen übergeben werden
            } //planning == 3
        } // found
      else
        {
          // nothing found, try to create a free waypoint
          WaypointDialog *waypointDlg = new WaypointDialog(this);
          QPoint p = _globalMapMatrix.mapToWgs(current);

          // initialize dialg
          waypointDlg->setWaypointType(BaseMapElement::Landmark);
          waypointDlg->longitude->setText(printPos(p.x(), false));
          waypointDlg->latitude->setText(printPos(p.y(), true));
          waypointDlg->setSurface(-1);

          if (waypointDlg->exec() == QDialog::Accepted)
            {
              if (!waypointDlg->name->text().isEmpty())
                {
                  struct wayPoint *w = new wayPoint;

                  w->name = waypointDlg->name->text().left(6).upper();
                  w->description = waypointDlg->description->text();
                  w->type = waypointDlg->getWaypointType();
                  w->origP.setX(_globalMapContents.degreeToNum(waypointDlg->longitude->text()));
                  w->origP.setY(_globalMapContents.degreeToNum(waypointDlg->latitude->text()));
                  w->projP = _globalMapMatrix.wgsToMap(w->origP.y(), w->origP.x());
                  w->elevation = waypointDlg->elevation->text().toInt();
                  w->icao = waypointDlg->icao->text().upper();
                  w->frequency = waypointDlg->frequency->text().toDouble();
                  w->runway = waypointDlg->runway->text().toInt();
                  w->length = waypointDlg->length->text().toInt();
                  w->surface = waypointDlg->getSurface();
                  w->comment = waypointDlg->comment->text();
                  w->isLandable = waypointDlg->isLandable->isChecked();

                  taskPointList.append(w);
                }
            }
          delete waypointDlg;
        }
    } // left button
  else if(event->button() == RightButton)
    {
      moveWPindex = -999;

      prePlanPos.setX(-999);
      prePlanPos.setY(-999);
      planning = 2;
    }

  // Aufgabe zeichnen
  if(taskPointList.count() > 0)
    {
      warning("zeichen");
      pixPlan.fill(white);
      ((FlightTask *)baseFlight)->setWaypointList(taskPointList);
      __drawPlannedTask();
      __showLayer();
    }


}


void Map::mousePressEvent(QMouseEvent* event)
{
  // First: delete the cursor, if visible:
  if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
          prePos.x() - 20, prePos.y() - 20, 40, 40);


  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  const QPoint current = event->pos();

//  bool shiftButton = event->state() & ShiftButton;


  SinglePoint *hitElement;
  QString text;

  QPoint sitePos;
  double dX, dY, delta(16.0);

  if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

  BaseFlightElement *f = _globalMapContents.getFlight();
  if (f != 0) {
    QList<wayPoint> taskPointList = f->getWPList();
  }

  if(event->button() == MidButton)
    {
      // Move Map
      _globalMapMatrix.centerToPoint(event->pos());
      _globalMapMatrix.createMatrix(this->size());
      __redrawMap();
    }

  if(planning)

    {
      // graphical Planning
      __graphicalPlanning(current, event);
    }
  else if(event->button() == LeftButton)
    {
      if (event->state() == QEvent::ShiftButton)
        {
          // select WayPoint
          QRegExp blank("[ ]");
          bool found = false;
          int searchList[] = {MapContents::GliderList, MapContents::AirportList};
          for (int l = 0; l < 2; l++)
            {
           	  for(unsigned int loop = 0;
           	      loop < _globalMapContents.getListLength(searchList[l]); loop++)
           	    {
        	        hitElement = (SinglePoint*)_globalMapContents.getElement(
        	            searchList[l], loop);
          	      sitePos = hitElement->getMapPosition();

        	        dX = abs(sitePos.x() - current.x());
  	              dY = abs(sitePos.y() - current.y());
  	
  	              // Abstand entspricht der Icon-Größe.
        	        if (dX < delta && dY < delta)
        	          {
          	          wayPoint *w = new wayPoint;
  	                  w->name = hitElement->getName().replace(blank, QString::null).left(6).upper();
                      w->description = hitElement->getName();
          	          w->type = hitElement->getTypeID();
//          	          w->origP = _globalMapMatrix.mapToWgs(_globalMapMatrix.map(hitElement->getPosition()));
          	          w->origP = hitElement->getWGSPosition();
                      w->elevation = hitElement->getElevation();
                      w->icao = ((RadioPoint *)hitElement)->getICAO();
                      w->frequency = ((RadioPoint *)hitElement)->getFrequency().toDouble();
                      w->isLandable = true;
                      w->surface = -1;
                      w->runway = 0;
                      w->length = 0;

                      emit waypointSelected(w);
                      found = true;
                      break;
        	          }
      	        }
      	      if (found)  break;
      	    }

  	      if (!found)
  	        {
      	      // add an 'free' waypoint
              wayPoint *w = new wayPoint;
              // leave name empty, this will generate an syntetic name
              w->type = BaseMapElement::Landmark;
              w->origP = _globalMapMatrix.mapToWgs(current);
              w->isLandable = false;
              w->elevation = 0;
              w->frequency = 0.0;
              w->surface = -1;
              w->runway = 0;
              w->length = 0;

              emit waypointSelected(w);
  	        }
        }
    }
  else if(event->button() == RightButton)
    {
        // Display Information about Map Items
        __displayMapInfo(current);
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
					      ( lon1 + ( lonloop * 0.1 ) ) * 600000);
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
						 ( lat2 + loop + ( loop2 * ( step / 60.0 ) ) ) * 600000,
						 (lon1 + (lonloop * 0.1)) * 600000);
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
					     ((loop + (loop2 * step / 60.0)) * 600000));

	      cP2 = _globalMapMatrix.wgsToMap((lat2 * 600000),
					      ((loop + (loop2 * step / 60.0)) * 600000));

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

//  _globalMapContents.drawList(&uMapP, MapContents::VillageList);

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
        // wir sollten nur die Lufträume in der Liste speichern, die
        // tatsächlich gezeichnet werden. Dann geht die Suche schneller.
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
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;

  QPainter planP;
  FlightTask* task;

  if(solid)
    {
      task = (FlightTask *)_globalMapContents.getFlight();
    }
  else
    {
       task = &tempTask;
    }

  if (task && task->getTypeID() == BaseMapElement::Task) {
    QList<wayPoint> WPList = task->getWPList();

    // Strecke zeichnen


    if(solid)
      {
        planP.begin(&pixPlan);
      }
    else
      {
        planP.begin(this);
      }
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
  // Statusbar noch nicht "genial" eingestellt ...
  mainApp->slotSetProgress(0);

  pixPlan.fill(white);

  pixAero.fill(white);
  pixAirspace.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(black);
  pixIsoMap.fill(white);
  pixFlight.fill(white);

  bitMapMask.fill(Qt::color0);
  bitFlightMask.fill(Qt::color0);
  bitPlanMask.fill(Qt::color0);
  bitAirspaceMask.fill(Qt::color0);

  QPoint temp1(preCur1);
  QPoint temp2(preCur2);

  preCur1.setX(-50);
  preCur2.setX(-50);

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();
  __drawFlight();
//  __drawPlannedTask();
  // Linie zum aktuellen Punkt löschen
  prePlanPos.setX(-999);
  prePlanPos.setY(-999);  

  __showLayer();

  mainApp->slotSetProgress(100);

  slotDrawCursor(temp1,temp2);
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
    }
  else
    {
      // Planen "ausschalten"
      planning = 0;
      __showLayer();
      emit taskPlanningEnd();
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

  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAero, 0, 0, -1, -1, NotEraseROP);

  pixAirspace.setMask(bitAirspaceMask);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace);

  bitBlt(&pixBuffer, 0, 0, &pixFlight);
  bitBlt(&pixBuffer, 0, 0, &pixPlan);
  bitBlt(&pixBuffer, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);

  paintEvent();
}

void Map::slotDrawCursor(QPoint p1, QPoint p2)
{
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

 Flight *f = (Flight *)_globalMapContents.getFlight();
 if (f && f->getTypeID() == BaseMapElement::Flight) {
   // check if the Rectangle is zero
   // is it necessary here?
   if (!f->getFlightRect().isNull())
    {
      _globalMapMatrix.centerToRect(f->getFlightRect());
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

  BaseFlightElement *f = _globalMapContents.getFlight();

  if (f) {
    QRect r;
    switch (f->getTypeID()) {
    case BaseMapElement::Flight:
      r = ((Flight *)f)->getTaskRect();
      break;
    case BaseMapElement::Task:
      r = ((FlightTask *)f)->getRect();
      break;
    default:
      return;
    }

    // check if the Rectangle is zero
    if (!r.isNull())
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
  if (f){
  	// save this one to speed up timeout code
    this->flightToAnimate = f;
    switch(f->getTypeID()) {
    case BaseMapElement::Flight:
      f->setAnimationIndex(0);
      f->setAnimationActive(true);
      break;
    case BaseMapElement::FlightGroup:
		  // loop through all and set animation index to start
		  QList<Flight> flightList = ((FlightGroup *)f)->getFlightList();
  		for(unsigned int loop = 0; loop < flightList.count(); loop++){
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
//	__drawFlight();
//     __showLayer();

	// save what will be under the flag
    switch(f->getTypeID()) {
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
		  QList<Flight> flightList = ((FlightGroup *)f)->getFlightList();
  		for(unsigned int loop = 0; loop < flightList.count(); loop++){
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

  if (f){
    switch(f->getTypeID()) {
    case BaseMapElement::Flight:
      f->setAnimationNextIndex();
      if (f->isAnimationActive())
	      bDone = false;
      //write info from current point on statusbar
      cP = f->getPoint((f->getAnimationIndex()));
      pos = _globalMapMatrix.map(cP.projP);
      lastpos = f->getLastAnimationPos();
      pix = f->getLastAnimationPixmap();
      emit showFlightPoint(pos, cP);
      // erase prev indicator-flag
      bitBlt(&pixBuffer, lastpos.x(), lastpos.y()-32, &pix);

//  bitBlt(&pixBuffer, lastpos.x()-100, lastpos.y()-32, &pix);
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
		  QList<Flight> flightList = ((FlightGroup*)flightToAnimate)->getFlightList();
  		for(unsigned int loop = 0; loop < flightList.count(); loop++){
  		  f = flightList.at(loop);
          f->setAnimationNextIndex();
          if (f->isAnimationActive())
 		      bDone = false;
          //write info from current point on statusbar
          cP = f->getPoint((f->getAnimationIndex()));
          pos = _globalMapMatrix.map(cP.projP);
          lastpos = f->getLastAnimationPos();
          pix = f->getLastAnimationPixmap();
          emit showFlightPoint(pos, cP);
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
  QList<Flight> flightList;

  if (!f) {
    return;
  }

  switch (f->getTypeID()) {
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
  if (timerAnimate->isActive()){
    timerAnimate->stop();
	  // loop through all and increment animation index
	  for(unsigned int loop = 0; loop < flightList.count(); loop++){
      flightList.at(loop)->setAnimationActive(false);
    }
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      // get the next point, preIndex now holds last point
      if ((index = f->searchGetNextPoint(preIndex, cP)) != -1){
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      // get the next point, preIndex now holds last point
      if ((index = f->searchGetPrevPoint(preIndex, cP)) != -1){
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      // get the next point, preIndex now holds last point
      if ((index = f->searchStepNextPoint(preIndex, cP, 10)) != -1){
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      // get the next point, preIndex now holds last point
      if ((index = f->searchStepPrevPoint(preIndex, cP,10)) != -1){
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);
      if ((index = f->searchGetNextPoint(0, cP)) != -1){
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

  if (f) {
    if (prePos.x() >= 0){   // only step if crosshair is shown in map.
       bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
                   prePos.x() - 20, prePos.y() - 20, 40, 40);

      // just a workaround !!!!!!!!!!!!!
      if (f->getTypeID() == BaseMapElement::Flight) {
        if ((index = f->searchGetNextPoint(((Flight *)f)->getRouteLength()-1, cP)) != -1){
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
  // Hier wird der Flug 2x neu gezeichnet, denn erst beim
  // ersten Zeichnen werden die Rahmen von Flug und Aufgabe
  // bestimmt.
  slotRedrawFlight();
  if (f) {
    switch(f->getTypeID()) {
    case BaseMapElement::Flight:
      slotCenterToFlight();
      break;
    case BaseMapElement::Task:
      slotCenterToTask();
      break;
    }
  }
}
/** append a waypoint to the current task */
void Map::slotAppendWaypoint2Task(wayPoint *p)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  FlightTask *f = (FlightTask *)_globalMapContents.getFlight();
  if (f && f->getTypeID() == BaseMapElement::Task && planning) {
    QList<wayPoint> taskPointList = f->getWPList();
    p->projP = _globalMapMatrix.wgsToMap(p->origP.y(), p->origP.x());
    taskPointList.append(p);
    f->setWaypointList(taskPointList);
    __drawPlannedTask(true);
    __showLayer();
  }
}

/** search for a waypoint
First look in task itself
Second look in map contents */
bool Map::__getTaskWaypoint(QPoint current, struct wayPoint *wp, QList<wayPoint> &taskPointList)
{
  unsigned int i;
  struct wayPoint *tmpPoint;
  QPoint sitePos;
  double dX, dY;
  // Radius for Mouse Snapping
  double delta(16.0);
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;
  bool found = false;
  SinglePoint *hitElement;

  for (i = 0; i < taskPointList.count(); i++) {
    tmpPoint = taskPointList.at(i);
    sitePos = _globalMapMatrix.map(_globalMapMatrix.wgsToMap(tmpPoint->origP.y(), tmpPoint->origP.x()));
    dX = abs(sitePos.x() - current.x());
    dY = abs(sitePos.y() - current.y());

    // Abstand entspricht der Icon-Größe.
    if (dX < delta && dY < delta) {
      *wp = *tmpPoint;
      found = true;
      break;
    }
  }

  if (!found) {
    /*
    *  Muss für alle Punktdaten durchgeführt werden
    */
    QArray<int> contentArray(2);
    contentArray.at(0) = MapContents::GliderList;
    contentArray.at(1) = MapContents::AirportList;

    for(unsigned int n = 0; n < contentArray.count(); n++) {
      for(unsigned int loop = 0; loop < _globalMapContents.getListLength(contentArray.at(n)); loop++) {
        hitElement = (SinglePoint*)_globalMapContents.getElement(contentArray.at(n), loop);			
        sitePos = hitElement->getMapPosition();
        dX = abs(sitePos.x() - current.x());
        dY = abs(sitePos.y() - current.y());

        if (dX < delta && dY < delta) {
          wp->name = hitElement->getWPName();
          wp->origP = hitElement->getWGSPosition();
          wp->elevation = hitElement->getElevation();
          wp->projP = hitElement->getPosition();
          found = true;
          break;
        }
      }
    }
  }
  return found;
}
