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

#include <airspace.h>
#include <flight.h>
#include <kflog.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>
#include <singlepoint.h>

// Festlegen der Größe der Pixmaps auf Desktop-Grösse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

Map::Map(KFLogApp *m, QFrame* parent, const char* name)
  : QWidget(parent, name),
    mainApp(m), prePos(-50, -50), preCur1(-50, -50), preCur2(-50, -50)
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
  pixGrid.resize( PIX_WIDTH, PIX_HEIGHT );
  pixUnderMap.resize( PIX_WIDTH, PIX_HEIGHT );
  pixIsoMap.resize( PIX_WIDTH, PIX_HEIGHT );
  bitMapMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitAirspaceMask.resize( PIX_WIDTH, PIX_HEIGHT );
  bitFlightMask.resize( PIX_WIDTH, PIX_HEIGHT );

  airspaceRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);

  setMouseTracking(true);
  setBackgroundColor(QColor(255,255,255));
  setCursor(crossCursor);
  setAcceptDrops(true);

	setFocusPolicy( QWidget::StrongFocus );
	grabKeyboard();

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

Map::~Map()
{

}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  int index;

  if(prePos.x() >= 0)
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
          prePos.x() - 20, prePos.y() - 20, 40, 40);

  struct flightPoint cP;

  if((index = _globalMapContents.searchFlightPoint(event->pos(), cP)) != -1)
    {
      emit showFlightPoint(_globalMapMatrix.mapToWgs(event->pos()), cP);
      prePos = _globalMapMatrix.map(cP.projP);
      bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
    }
  else
    {
      emit showPoint(_globalMapMatrix.mapToWgs(event->pos()));
      prePos.setX(-50);
      prePos.setY(-50);
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

  if(event->button() == MidButton)
    {
      _globalMapMatrix.centerToPoint(event->pos());
      _globalMapMatrix.createMatrix(this->size());
      __redrawMap();
    }
  else if(event->button() == LeftButton)
    {
//    _start = event->pos();
    }
  else if(event->button() == RightButton)
    {
      bool show = false, isAirport = false;

      BaseMapElement* hitElement;
      QString text;

      QPoint sitePos;
      double dX, dY, delta(16.0);

      if(_globalMapMatrix.isSwitchScale()) delta = 8.0;

      /*
       * Segelflugplätze, soweit vorhanden, kommen als erster Eintrag
       */
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
              text = text + ((SinglePoint*)hitElement)->getInfoString();
              // Text anzeigen
              QWhatsThis::enterWhatsThisMode();
              QWhatsThis::leaveWhatsThisMode(text);
              isAirport = true;
            }
        }

//      text = "";    // Wir wollen _nur_ Flugplätze anzeigen!

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
              // Text anzeigen
              QWhatsThis::enterWhatsThisMode();
              QWhatsThis::leaveWhatsThisMode(text);
              isAirport = true;
            }
        }

      if(_globalMapContents.getFlightList()->count() > 0)
        {
          QList<struct wayPoint>* wpList =
              _globalMapContents.getFlight()->getWPList();

          delta = 25;
          bool isWP = false;
          QString wpText;
          wpText = "<B>Waypoint:</B><UL>";

          for(unsigned int loop = 0; loop < wpList->count(); loop++)
            {
              sitePos = _globalMapMatrix.map(wpList->at(loop)->projP);

              dX = sitePos.x() - current.x();
              dY = sitePos.y() - current.y();

              // We do not search for the sector ...
              if( ( ( dX < delta ) && ( dX > -delta ) ) &&
                  ( ( dY < delta ) && ( dY > -delta ) ) )
                {
                  isWP = true;

                  QString tmpText, timeText;

                  if(wpList->at(loop)->sector1 != 0)
                    {
                      timeText = printTime(wpList->at(loop)->sector1);
                      tmpText = i18n("Sector 1");
                    }
                  else if(wpList->at(loop)->sector2 != 0)
                    {
                      timeText = printTime(wpList->at(loop)->sector2);
                      tmpText = i18n("Sector 2");
                    }
                  else if(wpList->at(loop)->sectorFAI != 0)
                    {
                      timeText = printTime(wpList->at(loop)->sectorFAI);
                      tmpText = i18n("FAI-Sector");
                    }
                  else
                    {
                      timeText = "&nbsp;" + i18n("not reached");
                    }

                  switch(wpList->at(loop)->type)
                    {
                      case Flight::TakeOff:
                        tmpText = i18n("Take Off");
                        break;
                      case Flight::Begin:
                        tmpText = i18n("Begin of task");
                        break;
                      case Flight::End:
                        tmpText = i18n("End of task");
                        break;
                      case Flight::Landing:
                        tmpText = i18n("Landing");
                        break;
                    }

                  wpText = wpText + "<LI><B>" + wpList->at(loop)->name +
                      "</B>  " +
                      "&nbsp;" + timeText + " / " + tmpText + "<BR>" +
                      printPos(wpList->at(loop)->origP.x()) + " / " +
                      printPos(wpList->at(loop)->origP.y(), false) + "</LI>";
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
              hitElement = _globalMapContents.getElement(
                    MapContents::AirspaceList, loop);

              text = text + "<LI>" + ((Airspace*)hitElement)->getInfoString()
                  + "</LI>";
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

  BaseMapElement* _current;

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

  pixAero.fill(white);
  pixAirspace.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(black);
  pixIsoMap.fill(white);
  pixFlight.fill(white);

  bitMapMask.fill(Qt::color0);
  bitFlightMask.fill(Qt::color0);
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

//  emit changed(this->size());

  __redrawMap();
}

void Map::__showLayer()
{
  pixUnderMap.setMask(bitMapMask);
  pixFlight.setMask(bitFlightMask);

  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAero, 0, 0, -1, -1, NotEraseROP);

  pixAirspace.setMask(bitAirspaceMask);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace);

  bitBlt(&pixBuffer, 0, 0, &pixFlight);
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

  if(id >= _globalMapContents.getFlight()->getWPList()->count())
    {
      warning("KFLog: Map::slotCenterToWaypoint: wrong Waypoint-ID");
      return;
    }

  _globalMapMatrix.centerToPoint(_globalMapMatrix.map(
      _globalMapContents.getFlight()->getWPList()->at(id)->projP));
  _globalMapMatrix.slotSetScale(_globalMapMatrix.getScale(MapMatrix::LowerLimit));

  emit changed(this->size());
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

/** Löscht den Fluglayer */
void Map::slotDeleteFlightLayer()
{
  extern MapContents _globalMapContents;
  _globalMapContents.closeFlight();
  pixFlight.fill(white);
  bitFlightMask.fill(black);
  __showLayer();
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

/** Processes the keyboard input to the Map class */
void Map::keyPressEvent( QKeyEvent* event)
{
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  struct flightPoint cP;
  int index;
  int preindex;

  // is log loaded and point selected with mouse?
  if (prePos.x() >= 0){
    preindex = _globalMapContents.searchFlightPoint(prePos, cP);
  	switch ( event->key() ){
			/**
			 * Single-step
			 */
   		case Key_Up:
       	bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
 	   		  					 prePos.x() - 20, prePos.y() - 20, 40, 40);
		    // get the next point, preIndex now holds last index
				if ((index = _globalMapContents.searchGetNextFlightPoint(preindex, cP)) != -1){
          emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
          prePos = _globalMapMatrix.map(cP.projP);
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
				}
				event->accept();	// set by default really
      	break;
   		case Key_Down:
  	   	bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
    								 prePos.x() - 20, prePos.y() - 20, 40, 40);
		    // get the next point, preIndex now holds last index
				if ((index = _globalMapContents.searchGetPrevFlightPoint(preindex, cP)) != -1){
          emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
          prePos = _globalMapMatrix.map(cP.projP);
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
				}
				event->accept();	// set by default really
      	break;
			/**
			 * Multi-step
			 */
   		case Key_PageUp:
  	   	bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
    								 prePos.x() - 20, prePos.y() - 20, 40, 40);
		    // get the next point, preIndex now holds last index
				if ((index = _globalMapContents.searchStepNextFlightPoint(preindex, cP, 20)) != -1){
          emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
          prePos = _globalMapMatrix.map(cP.projP);
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
				}
				event->accept();	// set by default really
      	break;
   		case Key_PageDown:
  	   	bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixBuffer,
    								 prePos.x() - 20, prePos.y() - 20, 40, 40);
		    // get the next point, preIndex now holds last index
				if ((index = _globalMapContents.searchStepPrevFlightPoint(preindex, cP, 20)) != -1){
          emit showFlightPoint(_globalMapMatrix.wgsToMap(cP.origP), cP);
          prePos = _globalMapMatrix.map(cP.projP);
          bitBlt(this, prePos.x() - 20, prePos.y() - 20, &pixCursor);
				}
				event->accept();	// set by default really
      	break;
			default:
	 	  	event->ignore();
      	break;
   	}
	}
}
