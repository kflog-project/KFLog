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

#include <stdlib.h>
#include <unistd.h>
#include <cmath>

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <ktoolbarbutton.h>

#include <qbitmap.h>
#include <qdatetime.h>
#include <qdragobject.h>
#include <qfont.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qwhatsthis.h>

#include <airspace.h>
#include <flight.h>
#include <kflog.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>
#include <printdialog.h>
#include <singlepoint.h>
#include <resource.h>

#define MATRIX_MOVE(a) extern MapMatrix _globalMapMatrix; \
    _globalMapMatrix.moveMap(a); \
    _globalMapMatrix.createMatrix(this->size()); \
  __redrawMap();

// Festlegen der Größe der Pixmaps auf Desktop-Grösse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

Map::Map(KFLogApp *m, QFrame* parent, const char* name)
: QWidget(parent, name),
  mainApp(m), prePos(-50, -50), preCur1(-50, -50), preCur2(-50, -50), posNum(1),
    indexLength(0)
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

  pixCursor.resize(40,40);
  pixCursor.fill(white);

  QPainter cursor(&pixCursor);
  cursor.setPen(QPen(QColor(255,100,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

  pixCursor1.resize(40,40);
  pixCursor1.fill(white);
  QPainter cursor1(&pixCursor1);
  cursor1.setPen(QPen(QColor(0,255,0), 3));
  cursor1.drawLine(0,0,40,40);
  cursor1.drawLine(0,40,40,0);
//  cursor1.setPen(QPen(QColor(255,0,255), 3));
//  cursor.drawEllipse(10, 10, 20, 20);
  cursor1.end();

  pixCursor2.resize(40,40);
  pixCursor2.fill(white);
  QPainter cursor2(&pixCursor2);
  cursor2.setPen(QPen(QColor(255,0,0), 3));
  cursor2.drawLine(0,0,40,40);
  cursor2.drawLine(0,40,40,0);
//  cursor2.setPen(QPen(QColor(255,0,255), 3));
//  cursor.drawEllipse(10, 10, 20, 20);
  cursor2.end();

  pixCursorBuffer1.resize(40,40);
  pixCursorBuffer1.fill(white);
  pixCursorBuffer2.resize(40,40);
  pixCursorBuffer2.fill(white);

  pixAllSites.resize( PIX_WIDTH, PIX_HEIGHT );
  pixAirspace.resize( PIX_WIDTH, PIX_HEIGHT );
  pixFlight.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGlider.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGrid.resize( PIX_WIDTH, PIX_HEIGHT );
  pixWaypoints.resize( PIX_WIDTH, PIX_HEIGHT );
  pixUnderMap.resize( PIX_WIDTH, PIX_HEIGHT );
  pixIsoMap.resize( PIX_WIDTH, PIX_HEIGHT );
  bitMapMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitAirspaceMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitFlightMask.resize( PIX_WIDTH, PIX_HEIGHT );

  indexList = new unsigned int[1];
  xPos = new int[1];
  yPos = new int[1];

  airspaceRegList = new QList<QRegion>;
  cityRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);
  cityRegList->setAutoDelete(true);
  displayFlights = false;

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);

  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setCursor(crossCursor);
  setAcceptDrops(true);

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
}

Map::~Map()  {  delete[] indexList;  }

void Map::mouseMoveEvent(QMouseEvent* event)
{
  // keep the current mouse position
  _currentPos = event->pos();

  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;

  if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
          prePos.x() - 20, prePos.y() - 20, 40, 40);

  struct flightPoint* cP = new struct flightPoint[1];

  if(_globalMapContents.searchFlightPoint(_currentPos, cP) != -1)
    {
      mainApp->showPointInfo(_globalMapMatrix.mapToWgs(event->pos()), &cP[0]);
      prePos = _globalMapMatrix.map(cP[0].projP);
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor,
          0, 0, -1, -1, NotEraseROP);
    }
  else
    {
      mainApp->clearPointInfo(_globalMapMatrix.mapToWgs(event->pos()));

      prePos.setX(-50);
      prePos.setY(-50);
    }
}

void Map::mousePressEvent(QMouseEvent* event)
{
  extern MapContents _globalMapContents;
  extern const MapMatrix _globalMapMatrix;

  const QPoint current = event->pos();

  if(event->button() == MidButton)
    {
      extern MapMatrix _globalMapMatrix;
      _globalMapMatrix.centerToMouse(event->pos());
      _globalMapMatrix.createMatrix(this->size());
      __redrawMap();
    }
  else if(event->button() == LeftButton)
    {
//    _start = event->pos();
    }
  else if(event->button() == RightButton)
    {
      delete[] indexList;
      indexList = new unsigned int[1];

  	  KPopupMenu* helpMenu = new KPopupMenu("Info");
	    bool show = false;
	    bool isAirport = false;

      QRegion* testReg;
      BaseMapElement* hitElement;
      QString text;

      if(showCity)
        {
          /*
           * zunächst nach Städten suchen
           */
          for(unsigned int loop = 0; loop < cityRegList->count(); loop++)
            {
              testReg = cityRegList->at(loop);
              if(testReg->contains(current))
                {
                  hitElement = _globalMapContents.getElement(
                          MapContents::CityList, loop);
//          text.sprintf("%s %d", i18n("City:"), loop);
                  text = i18n("City:");
                  text = text + ' ' + hitElement->getName();
                  helpMenu->setTitle(text);
                  show = true;
                }
            }
        }

//      if(showGlider)
        {
          /*
           * Segelflugplätze, soweit vorhanden, kommen als erster Eintrag
           */
          QPoint sitePos;
          double dX, dY, delta;

          delta = 16.0;
          // Ist die Grenze noch aktuell ???
          if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

          for(unsigned int loop = 0;
                loop < _globalMapContents.getListLength(
                        MapContents::GliderList); loop++)
            {
              hitElement = _globalMapContents.getElement(
                      MapContents::GliderList, loop);
              sitePos = ((SinglePoint*)hitElement)->getMapPosition();

              dX = sitePos.x() - current.x();
              dY = sitePos.y() - current.y();

              // Abstand entspricht der Icon-Größe.
              if( ( ( dX < delta ) && ( dX > -delta ) ) &&
                  ( ( dY < delta ) && ( dY > -delta ) ) )
                {
//                  warning("Zeige Infos an ...");
                  text = text + ((SinglePoint*)hitElement)->getInfoString();
                  /*
                   * Text anzeigen
                   */
                  QWhatsThis::enterWhatsThisMode();
                  QWhatsThis::leaveWhatsThisMode(text);
                  isAirport = true;
                }
            }
            /*
              hitElement = _globalMapContents.getElement(
                        MapContents::GliderList, loop);
              gliderPos = ((SinglePoint*)hitElement)->getMapPosition();

              dX = gliderPos.x() - current.x();
              dY = gliderPos.y() - current.y();

              // Abstand entspricht der Icon-Größe.
              if( ( ( dX < delta ) && ( dX > -delta ) ) &&
                  ( ( dY < delta ) && ( dY > -delta ) ) )
                {
                  text = i18n("Glidersite:");
                  text = text + ' ' + hitElement->getName();
//                  helpMenu->insertItem(Icon(KApplication::kde_datadir() +
//                      "/kflog/map/small/glider.xpm"), text);
                  helpMenu->connectItem(helpMenu->count() - 1, this,
                      SLOT(slotShowMapElement()));
                  // helpMenu enthält auf jeden Fall den Titel und
                  // zwei Linien, also 3 Einträge !!!
                  indexLength = helpMenu->count() - 3;
                  indexList = (unsigned int*) realloc(indexList,
                            (2 * indexLength * sizeof(int)));
                  indexList[2 * indexLength - 2] =
                          (unsigned int)BaseMapElement::Glidersite;
                  indexList[2 * indexLength - 1] = loop;
                  show = true;
                }
            }
            */
        }

//      if(showAirport)
        {
          QPoint sitePos;
          double dX, dY, delta;

          delta = 16.0; // Ist die Grenze noch aktuell ???
          text = "";    // Wir wollen _nur_ Flugplätze anzeigen!
          if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

          for(unsigned int loop = 0;
                loop < _globalMapContents.getListLength(
                      MapContents::AirportList); loop++)
            {
              hitElement = _globalMapContents.getElement(
                      MapContents::AirportList, loop);
              sitePos = ((SinglePoint*)hitElement)->getMapPosition();

              dX = sitePos.x() - current.x();
              dY = sitePos.y() - current.y();

              // Abstand entspricht der Icon-Größe.
              if( ( ( dX < delta ) && ( dX > -delta ) ) &&
                  ( ( dY < delta ) && ( dY > -delta ) ) )
                {
                  text = text + ((SinglePoint*)hitElement)->getInfoString();
                  /*
                   * Text anzeigen
                   */
                  QWhatsThis::enterWhatsThisMode();
                  QWhatsThis::leaveWhatsThisMode(text);
                  isAirport = true;
                }
            }
        }

        if(isAirport)  return;
//      if(showAirspace)
        {
          Airspace* tempAir;

          text = text + "<B>" + i18n("Airspace-Structure") + ":</B><UL>";
          for(unsigned int loop = 0; loop < airspaceRegList->count(); loop++)
            {
              testReg = airspaceRegList->at(loop);
              if(testReg->contains(current))
                {
                  hitElement = _globalMapContents.getElement(
                        MapContents::AirspaceList, loop);
                  tempAir = (Airspace*)hitElement;
                  text = text + "<LI>" + tempAir->getInfoString() + "</LI>";
                  show = true;
                }
            }
          text = text + "</UL>";
        }

      if(show)
        {
          /*
           * Text anzeigen
           */
          QWhatsThis::enterWhatsThisMode();
          QWhatsThis::leaveWhatsThisMode(text);
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
  QPainter aSitesP(&pixAllSites);
  QPainter airSpaceP(&pixAirspace);
  QPainter flightP(&pixFlight);
  QPainter gliderP(&pixGlider);
  QPainter waypointP(&pixWaypoints);
  QPainter uMapP(&pixUnderMap);
  QPainter isoMapP(&pixIsoMap);
  QPainter mapMaskP(&bitMapMask);
  QPainter flightMaskP(&bitFlightMask);
  QPainter airspaceMaskP(&bitAirspaceMask);

  airspaceRegList->clear();
  cityRegList->clear();

  extern MapContents _globalMapContents;

  BaseMapElement* _current;

  delete[] xPos;
  delete[] yPos;

  posNum = _globalMapContents.getListLength(MapContents::GliderList);
  xPos = new int[posNum];
  yPos = new int[posNum];

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

//  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::CityList);
//  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
//          MapContents::CityList); loop++)
//    {
//      _current = _globalMapContents.getElement(MapContents::CityList, loop);
//      cityRegList->append(_current->drawRegion(&uMapP, &mapMaskP));
//    }

  mainApp->slotSetProgress(40);

//  _globalMapContents.drawList(&uMapP, MapContents::VillageList);

  mainApp->slotSetProgress(45);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::LandmarkList);

  mainApp->slotSetProgress(50);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::ObstacleList);

  mainApp->slotSetProgress(55);

  _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::ReportList);

  mainApp->slotSetProgress(60);

  _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::NavList);

  mainApp->slotSetProgress(65);

  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
              MapContents::AirspaceList); loop++)
    {
      _current = _globalMapContents.getElement(
          MapContents::AirspaceList, loop);
        // wir sollten nur die Lufträume in der Liste speichern, die
        // tatsächlich gezeichnet werden. Dann geht die Suche schneller.
      airspaceRegList->append(_current->drawRegion(&airSpaceP,
            &airspaceMaskP));
    }

  mainApp->slotSetProgress(70);

  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
        MapContents::IntAirportList); loop++)
    {
      _current = _globalMapContents.getElement(
            MapContents::IntAirportList, loop);
      _current->drawMapElement(&aSitesP, &mapMaskP);
    }

  mainApp->slotSetProgress(75);

  _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::AirportList);

  mainApp->slotSetProgress(80);

  _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::AddSitesList);

  mainApp->slotSetProgress(85);

  _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::OutList);

  mainApp->slotSetProgress(90);

  _globalMapContents.drawList(&gliderP, &mapMaskP, MapContents::GliderList);

  mainApp->slotSetProgress(95);

  _globalMapContents.drawList(&flightP, &flightMaskP, MapContents::FlightList);

  // Closing the painter ...
  aSitesP.end();
  airSpaceP.end();
  flightP.end();
  gliderP.end();
  waypointP.end();
  uMapP.end();
  airspaceMaskP.end();
  flightMaskP.end();
}

void Map::resizeEvent(QResizeEvent* event)
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.createMatrix(event->size());

  pixBuffer.resize(event->size());
  pixBuffer.fill(white);

  if(!event->size().isEmpty()) __redrawMap();

  emit changed(event->size());
}

void Map::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QTextDrag::canDecode(event));
}

void Map::dropEvent(QDropEvent* event)
{
//  QString text;
//  QStrList tempList;

//  if(QUriDrag::decode(event, tempList))
//    {
//      warning("Anzahl: %d", tempList.count());
//      for(unsigned int loop = 0; loop < tempList.count(); loop++)
//        {
//          warning("  %s", tempList.at(loop));
//        }
//    }
}

void Map::__redrawMap()
{
  // Statusbar noch nicht "genial" eingestellt ...
  mainApp->slotSetProgress(0);

  pixAllSites.fill(white);
  pixAirspace.fill(white);
  pixGlider.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(black);
  pixIsoMap.fill(white);
  pixWaypoints.fill(white);
//  pixBuffer.fill(white);
  pixFlight.fill(white);

  bitMapMask.fill(Qt::color0);
  bitFlightMask.fill(Qt::color0);
  bitAirspaceMask.fill(Qt::color0);

  //
  QPoint temp1(preCur1);
  QPoint temp2(preCur2);

  preCur1.setX(-50);
  preCur2.setX(-50);

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();

  slotShowLayer();

  mainApp->slotSetProgress(100);

  slotDrawCursor(temp1,temp2);
}

void Map::slotRedrawMap()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.createMatrix(this->size());

  emit changed(this->size());

  __redrawMap();
}

void Map::slotShowLayer()
{
//  pixBuffer.fill(QColor(239,238,236));
  /* Wg. der Höhenlinien mal geändert ... */
//  pixBuffer.fill(white);

  pixUnderMap.setMask(bitMapMask);
  pixFlight.setMask(bitFlightMask);
  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
//  bitBlt(&pixBuffer, 0, 0, &bitMapMask, 0, 0, -1, -1, NotEraseROP);
//  bitBlt(&pixBuffer, 0, 0, &pixUnderMap, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAllSites, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixGlider, 0, 0, -1, -1, NotEraseROP);
//  bitBlt(&pixBuffer, 0, 0, &pixFlight, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixFlight);
  bitBlt(&pixBuffer, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);

  paintEvent();
}

void Map::slotZoomIn()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.scaleAdd(this->size());

  __redrawMap();
  emit changed(this->size());
}

void Map::slotZoomOut()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.scaleSub(this->size());

  __redrawMap();
  emit changed(this->size());
}

void Map::slotDrawFlight()
{
//  if(view->getFlightDataView()->getFlightList()->count() == 0) return;

//  displayFlights = true;

//  __drawFlight();
//  slotShowLayer();
}


void Map::slotDrawCursor(QPoint p1, QPoint p2)
{
  extern const MapMatrix _globalMapMatrix;

  QPoint pos1(_globalMapMatrix.map(p1)), pos2(_globalMapMatrix.map(p2));

  QPoint prePos1(_globalMapMatrix.map(preCur1)),
         prePos2(_globalMapMatrix.map(preCur2));
  if(preCur1.x() >= 0)
    {
      bitBlt(&pixBuffer, prePos1.x() - 20, prePos1.y() - 20, &pixCursorBuffer1);
      bitBlt(this, prePos1.x() - 20, prePos1.y() - 20, &pixCursorBuffer1);
    }
  if(preCur2.x() >= 0)
    {
      bitBlt(&pixBuffer, prePos2.x() - 20, prePos2.y() - 20, &pixCursorBuffer2);
      bitBlt(this, prePos2.x() - 20, prePos2.y() - 20, &pixCursorBuffer2);
    }

  // erstmal Karte sichern
  bitBlt(&pixCursorBuffer1, 0, 0, &pixBuffer,
          pos1.x() - 20, pos1.y() - 20, 40, 40);
  bitBlt(&pixCursorBuffer2, 0, 0, &pixBuffer,
          pos2.x() - 20, pos2.y() - 20, 40, 40);

  bitBlt(this, pos1.x() - 20, pos1.y() - 20, &pixCursor1,
          0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, pos1.x() - 20, pos1.y() - 20, &pixCursor1,
          0, 0, -1, -1, NotEraseROP);

  bitBlt(this, pos2.x() - 20, pos2.y() - 20, &pixCursor2,
          0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, pos2.x() - 20, pos2.y() - 20, &pixCursor2,
          0, 0, -1, -1, NotEraseROP);

  preCur1 = p1;
  preCur2 = p2;

}

void Map::slotShowMapElement()
{
/*
  if(!indexLength) return;

  MapElementView* elementView = view->getMapElementView();

  elementView->beginHTML();

  for(unsigned int loop = 1; loop <= indexLength; loop++)
    {
      switch (indexList[loop * 2 - 2])
        {
          case BaseMapElement::Glidersite:
            elementView->addGlidersite(indexList[loop * 2 - 1]);
            break;
          case BaseMapElement::Airport:
          case BaseMapElement::MilAirport:
          case BaseMapElement::CivMilAirport:
          case BaseMapElement::Airfield:
          case BaseMapElement::ClosedAirfield:
          case BaseMapElement::CivHeliport:
          case BaseMapElement::MilHeliport:
          case BaseMapElement::AmbHeliport:
            elementView->addAirport(indexList[loop * 2 - 1]);
            break;
          default:
            elementView->addAirspace(indexList[loop * 2 - 1],
                  indexList[loop * 2 - 2]);
            break;
        }
    }

  elementView->showHTML(true);
*/
}

void Map::slotPrintMap()
{
  PrintDialog pD(mainApp, true);
  pD.openMapPrintDialog();
}

void Map::slotCenterToItem(int listIndex, int elementIndex)
{
  extern MapContents _globalMapContents;
  SinglePoint* hitEl = _globalMapContents.getSinglePoint(listIndex,
            elementIndex);

  mapCenterLat = hitEl->getPosition().x();
  mapCenterLon = hitEl->getPosition().y();

  slotRedrawMap();
}

void Map::slotCenterToFlight()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  _globalMapMatrix.centerToRect(
          _globalMapContents.getFlight()->getFlightRect());
  _globalMapMatrix.createMatrix(this->size());
  __redrawMap();

  emit changed(this->size());
}

void Map::slotCenterToTask()
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  _globalMapMatrix.centerToRect(
          _globalMapContents.getFlight()->getTaskRect());
  _globalMapMatrix.createMatrix(this->size());
  __redrawMap();

  emit changed(this->size());
}

void Map::slotCenterToWaypoint(struct wayPoint* centerP)
{
  mapCenterLat = centerP->latitude;
  mapCenterLon = centerP->longitude;
  __redrawMap();
}

void Map::slotCenterToHome()  { MATRIX_MOVE( MapMatrix::Home ) }

void Map::slotMoveMapNW() { MATRIX_MOVE( MapMatrix::North | MapMatrix::West ) }

void Map::slotMoveMapN()  { MATRIX_MOVE( MapMatrix::North ) }

void Map::slotMoveMapNE() { MATRIX_MOVE( MapMatrix::North | MapMatrix::East ) }

void Map::slotMoveMapW()  { MATRIX_MOVE( MapMatrix::West ) }

void Map::slotMoveMapE()  { MATRIX_MOVE( MapMatrix::East ) }

void Map::slotMoveMapSW() { MATRIX_MOVE( MapMatrix::South | MapMatrix::West ) }

void Map::slotMoveMapS()  { MATRIX_MOVE( MapMatrix::South ) }

void Map::slotMoveMapSE() { MATRIX_MOVE( MapMatrix::South | MapMatrix::East ) }

void Map::showFlightLayer(bool redrawFlight)
{
  if(redrawFlight)
    {
      pixFlight.fill(white);
//      __drawFlight();
    }

  if(!showFlight)
    mainApp->toolBar()->getButton(ID_LAYER_FLIGHT)->toggle();

  slotShowLayer();
}

/** Löscht den Fluglayer */
void Map::slotDeleteFlightLayer()
{
  extern MapContents _globalMapContents;
  _globalMapContents.closeFlight();
  pixFlight.fill(white);
  bitFlightMask.fill(black);
  slotShowLayer();
}

void Map::slotOptimzeFlight()
{
// if(!flightList.count()) return;
//extern MapContents _globalMapContents;
//  if(_globalMapContents.getFlight()->optimizeTask()) {
//    showFlightData(flightList.current());
  //  mainApp->getMap()->showFlightLayer(true);
//  }
}
