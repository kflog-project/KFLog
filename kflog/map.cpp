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
#include <qfont.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qwhatsthis.h>

#include <kflog.h>
#include <map.h>
#include <mapcalc.h>
#include <mapcontents.h>
#include <mapmatrix.h>
#include <singlepoint.h>
#include <resource.h>

#define PROOF_LAYER(a,b,c) if(a){bitBlt(&b,0,0,&c,0,0,-1,-1,NotEraseROP);}
#define PROOF_BUTTON(a,b) if(a){mainApp->toolBar()->getButton(b)->toggle();}

#define MAP_X calc_X_Lambert(numToRad(mapCenterLat), 0)
#define MAP_Y calc_Y_Lambert(numToRad(mapCenterLat), 0)

#define DELTA_X ( ( this->width() / 2 ) - ( MAP_X / _currentScale * RADIUS ) )
#define DELTA_Y ( ( this->height() / 2 ) - ( MAP_Y / _currentScale * RADIUS ) )

#define DRAW_LOOP(a,b) for(unsigned int loop = 0; loop < \
    _globalMapContents.getListLength(a); loop++) { \
    _current = _globalMapContents.getElement(a, loop); \
    _current->drawMapElement(b, dX, dY, mapCenterLon, mapBorder); \
  }

#define SHOW_LAYER(a,b) a = mainApp->toolBar()->isButtonOn(b); \
      mainApp->menuBar()->setItemChecked(b, a);

#define SET_BORDER(a,b,c) _scaleBorder[BaseMapElement::a] = \
      config->readNumEntry(b,c);

#define SHOW_ELEMENT(a,b) _showElements[BaseMapElement::a] = \
      config->readBoolEntry(b, true);

#define MATRIX_MOVE(a) extern MapMatrix _globalMapMatrix; \
    _globalMapMatrix.moveMap(a); \
    _globalMapMatrix.createMatrix(this->size()); \
  __redrawMap();

// Festlegen der Größe der Pixmaps
#define PIX_WIDTH  1000
#define PIX_HEIGHT 1000

Map::Map(KFLogApp *m, QFrame* parent)
: QWidget(parent),
  mainApp(m), posNum(1), indexLength(0), preX(-50), preY(-50)
{
  extern double _scale[];
  extern int _scaleBorder[];
  extern bool _showElements[];

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
  cursor.setPen(QPen(QColor(255,0,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

//  pixTempA.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixTempB.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixTempC.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixTempD.resize( PIX_WIDTH, PIX_HEIGHT );

  pixAllSites.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixAirport.resize( PIX_WIDTH, PIX_HEIGHT );
  pixAirspace.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixCity.resize( PIX_WIDTH, PIX_HEIGHT );
  pixFlight.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGlider.resize( PIX_WIDTH, PIX_HEIGHT );
  pixGrid.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixHydro.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixIso.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixLand.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixNav.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixOut.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixRail.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixRoad.resize( PIX_WIDTH, PIX_HEIGHT );
//  pixTopo.resize( PIX_WIDTH, PIX_HEIGHT );
  pixWaypoints.resize( PIX_WIDTH, PIX_HEIGHT );
  pixUnderMap.resize( PIX_WIDTH, PIX_HEIGHT );

  indexList = new unsigned int[1];
  xPos = new int[1];
  yPos = new int[1];

  airspaceRegList = new QList<QRegion>;
  cityRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);
  cityRegList->setAutoDelete(true);
  displayFlights = false;

  KConfig *config = kapp->config();

  config->setGroup("MapLayer");
  showAddSites = config->readBoolEntry("AddSites", true);
  showAirport  = config->readBoolEntry("Airport",  true);
  showAirspace = config->readBoolEntry("Airspace", true);
  showCity     = config->readBoolEntry("City",     true);
  showFlight   = config->readBoolEntry("Flight",   true);
  showGlider   = config->readBoolEntry("Glider",   true);
  showHydro    = config->readBoolEntry("Hydro",    true);
  showLand     = config->readBoolEntry("Land",     true);
  showNav      = config->readBoolEntry("Nav",      true);
  showOut      = config->readBoolEntry("Out",      true);
  showRail     = config->readBoolEntry("Rail",     true);
  showRoad     = config->readBoolEntry("Road",     true);
  showTopo     = config->readBoolEntry("Topo",     true);
  showWaypoint = config->readBoolEntry("Waypoint", true);

  config->setGroup("MapScale");
  _scale[0] = config->readNumEntry("Scale0", ID_BORDER_L);
  _scale[1] = config->readNumEntry("Scale1", ID_BORDER_1);
  _scale[2] = config->readNumEntry("Scale2", ID_BORDER_2);
  _scale[3] = config->readNumEntry("Scale3", ID_BORDER_3);
  _scale[4] = config->readNumEntry("Scale4", ID_BORDER_4);
  _scale[5] = config->readNumEntry("Scale5", ID_BORDER_5);
  _scale[6] = config->readNumEntry("Scale6", ID_BORDER_6);
  _scale[7] = config->readNumEntry("Scale7", ID_BORDER_7);
  _scale[8] = config->readNumEntry("Scale8", ID_BORDER_8);
  _scale[9] = config->readNumEntry("Scale9", ID_BORDER_U);

  config->setGroup("MapBorder");
  SET_BORDER(Coast,              "ShoreLine", 6)
  SET_BORDER(BigRiver,           "BigHydro", 6)
  SET_BORDER(BigLake,            "BigHydro", 6)
  SET_BORDER(MidRiver,           "MidHydro", 5)
  SET_BORDER(MidLake,            "MidHydro", 5)
  SET_BORDER(SmallRiver,         "SmallHydro", 3)
  SET_BORDER(SmallLake,          "SmallHydro", 3)
  SET_BORDER(Dam,                "Dam",2)
  SET_BORDER(Railway,            "RailTrack",5)
  SET_BORDER(AerialRailway,      "AerialTrack",3)
  SET_BORDER(Station,            "Station",3)
  SET_BORDER(Obstacle,           "Obstacle",4)
  SET_BORDER(LightObstacle,      "Obstacle",4)
  SET_BORDER(ObstacleGroup,      "Obstacle",4)
  SET_BORDER(LightObstacleGroup, "Obstacle",4)
  SET_BORDER(Spot,               "Spot",2)
  SET_BORDER(Pass,               "Pass",2)
  SET_BORDER(Glacier,            "Glacier",4)
  SET_BORDER(Highway,            "Highway",6)
  SET_BORDER(HighwayEntry,       "HighwayEntry",2)
  SET_BORDER(MidRoad,            "MidRoad",4)
  SET_BORDER(SmallRoad,          "SmallRoad",3)
  SET_BORDER(Oiltank,            "Landmark",2)
  SET_BORDER(Factory,            "Landmark",2)
  SET_BORDER(Castle,             "Landmark",2)
  SET_BORDER(Church,             "Landmark",2)
  SET_BORDER(Tower,              "Landmark",2)
  SET_BORDER(HugeCity,           "HugeCity",6)
  SET_BORDER(BigCity,            "BigCity",5)
  SET_BORDER(MidCity,            "MidCity",4)
  SET_BORDER(SmallCity,          "SmallCity",3)
  SET_BORDER(Village,            "Village",3)
  SET_BORDER(AirC,               "Airspace",6)
  SET_BORDER(AirCtemp,           "Airspace",6)
  SET_BORDER(AirD,               "Airspace",6)
  SET_BORDER(AirDtemp,           "Airspace",6)
  SET_BORDER(ControlD,           "Airspace",6)
  SET_BORDER(AirElow,            "Airspace",6)
  SET_BORDER(AirEhigh,           "Airspace",6)
  SET_BORDER(AirF,               "Airspace",6)
  SET_BORDER(Restricted,         "Airspace",6)
  SET_BORDER(Danger,             "Airspace",6)
  SET_BORDER(LowFlight,          "Airspace",6)
  SET_BORDER(VOR,                "Radio",4)
  SET_BORDER(VORDME,             "Radio",4)
  SET_BORDER(VORTAC,             "Radio",4)
  SET_BORDER(NDB,                "Radio",4)
  SET_BORDER(CompPoint,          "Radio",4)
  SET_BORDER(IntAirport,         "Airport",4)
  SET_BORDER(Airport,            "Airport",4)
  SET_BORDER(MilAirport,         "Airport",4)
  SET_BORDER(CivMilAirport,      "Airport",4)
  SET_BORDER(Airfield,           "Airport",4)
  SET_BORDER(ClosedAirfield,     "Airport",4)
  SET_BORDER(CivHeliport,        "Airport",4)
  SET_BORDER(MilHeliport,        "Airport",4)
  SET_BORDER(AmbHeliport,        "Airport",4)
  SET_BORDER(Glidersite,         "GliderSites",4)
  SET_BORDER(Outlanding,         "Outlanding",3)
  SET_BORDER(WayPoint,           "Waypoints",3)
  SET_BORDER(UltraLight,         "AddSites",3)
  SET_BORDER(HangGlider,         "AddSites",3)
  SET_BORDER(Parachute,          "AddSites",3)
  SET_BORDER(Ballon,             "AddSites",3)

  config->setGroup("ShowElements");
  SHOW_ELEMENT(VOR,            "VOR")
  SHOW_ELEMENT(VORDME,         "VORDME")
  SHOW_ELEMENT(VORTAC,         "VORTAC")
  SHOW_ELEMENT(NDB,            "NDB")
  SHOW_ELEMENT(CompPoint,      "CompPoint")
  SHOW_ELEMENT(IntAirport,     "IntAirport")
  SHOW_ELEMENT(Airport,        "Airport")
  SHOW_ELEMENT(MilAirport,     "MilAirport")
  SHOW_ELEMENT(CivMilAirport,  "CivMilAirport")
  SHOW_ELEMENT(Airfield,       "Airfield")
  SHOW_ELEMENT(ClosedAirfield, "ClosedAirfield")
  SHOW_ELEMENT(CivHeliport,    "CivHeliport")
  SHOW_ELEMENT(MilHeliport,    "MilHeliport")
  SHOW_ELEMENT(AmbHeliport,    "AmbHeliport")
  SHOW_ELEMENT(UltraLight,     "UltraLight")
  SHOW_ELEMENT(HangGlider,     "HangGlider")
  SHOW_ELEMENT(Parachute,      "Parachute")
  SHOW_ELEMENT(Ballon,         "Ballon")

  const QBitmap cross(32, 32, cross_bits, true);
  const QCursor crossCursor(cross, cross);

  this->setMouseTracking(true);
  this->setBackgroundColor(QColor(255,255,255));
  this->setCursor(crossCursor);

  QWhatsThis::add(this, i18n("The map\n\n"
         "To move or scale the map, please use the buttons on the right. "
         "To zoom in or out, use the slider or the two buttons on the "
         "toolbar. You can also zoom with the two keys \"+\" (zoom in) "
         "and \"-\" (zoom out).\n\n"
         "With the menu-item \"Options\" -> \"Configure map\" you can "
         "configure, which map elements should be displayed at which "
         "scale."));
}

Map::~Map()
{
  extern const double _v1, _v2, _currentScale, _scale[];
  extern const int _scaleBorder[];
  extern const bool _showElements[];

  KConfig *config = kapp->config();

  config->setGroup("General");
  config->writeEntry("MapScale",   _currentScale);
  config->writeEntry("MapCenterX", mapCenterLon);
  config->writeEntry("MapCenterY", mapCenterLat);
  config->writeEntry("Parallel1",  radToNum(_v1));
  config->writeEntry("Parallel2",  radToNum(_v2));

  config->setGroup("MapLayer");
  config->writeEntry("AddSites", showAddSites);
  config->writeEntry("Airport",  showAirport);
  config->writeEntry("Airspace", showAirspace);
  config->writeEntry("City",     showCity);
  config->writeEntry("Flight",   showFlight);
  config->writeEntry("Glider",   showGlider);
  config->writeEntry("Hydro",    showHydro);
  config->writeEntry("Land",     showLand);
  config->writeEntry("Nav",      showNav);
  config->writeEntry("Out",      showOut);
  config->writeEntry("Rail",     showRail);
  config->writeEntry("Road",     showRoad);
  config->writeEntry("Topo",     showTopo);
  config->writeEntry("Waypoint", showWaypoint);

  config->setGroup("MapScale");
  config->writeEntry("Scale0", _scale[0]);
  config->writeEntry("Scale1", _scale[1]);
  config->writeEntry("Scale2", _scale[2]);
  config->writeEntry("Scale3", _scale[3]);
  config->writeEntry("Scale4", _scale[4]);
  config->writeEntry("Scale5", _scale[5]);
  config->writeEntry("Scale6", _scale[6]);
  config->writeEntry("Scale7", _scale[7]);
  config->writeEntry("Scale8", _scale[8]);
  config->writeEntry("Scale9", _scale[9]);

  config->setGroup("MapBorder");
  config->writeEntry("ShoreLine",       _scaleBorder[ID_SHORELINE]);
  config->writeEntry("BigHydro",        _scaleBorder[ID_BIGHYDRO]);
  config->writeEntry("MidHydro",        _scaleBorder[ID_MIDHYDRO]);
  config->writeEntry("SmallHydro",      _scaleBorder[ID_SMALLHYDRO]);
  config->writeEntry("Dam",             _scaleBorder[ID_DAM]);
  config->writeEntry("RailTrack",       _scaleBorder[ID_RAILTRACK]);
  config->writeEntry("AerialTrack",     _scaleBorder[ID_AERIALTRACK]);
  config->writeEntry("Station",         _scaleBorder[ID_STATION]);
  config->writeEntry("Obstacle",        _scaleBorder[ID_OBSTACLE]);
  config->writeEntry("Spot",            _scaleBorder[ID_SPOT]);
  config->writeEntry("Pass",            _scaleBorder[ID_PASS]);
  config->writeEntry("Glacier",         _scaleBorder[ID_GLACIER]);
  config->writeEntry("Highway",         _scaleBorder[ID_HIGHWAY]);
  config->writeEntry("HighwayEntry",    _scaleBorder[ID_HIGHWAY_E]);
  config->writeEntry("MidRoad",         _scaleBorder[ID_MIDROAD]);
  config->writeEntry("SmallRoad",       _scaleBorder[ID_SMALLROAD]);
  config->writeEntry("Landmark",        _scaleBorder[ID_LANDMARK]);
  config->writeEntry("HugeCity",        _scaleBorder[ID_HUGECITY]);
  config->writeEntry("BigCity",         _scaleBorder[ID_BIGCITY]);
  config->writeEntry("MidCity",         _scaleBorder[ID_MIDCITY]);
  config->writeEntry("SmallCity",       _scaleBorder[ID_SMALLCITY]);
  config->writeEntry("Village",         _scaleBorder[ID_VILLAGE]);
  config->writeEntry("Airspace",        _scaleBorder[ID_AIRSPACE]);
  config->writeEntry("Radio",           _scaleBorder[ID_RADIO]);
  config->writeEntry("Airport",         _scaleBorder[ID_AIRPORT]);
  config->writeEntry("GliderSites",     _scaleBorder[ID_GLIDERSITE]);
  config->writeEntry("Outlanding",      _scaleBorder[ID_OUTLANDING]);
  config->writeEntry("Waypoints",       _scaleBorder[ID_WAYPOINTS]);
  config->writeEntry("AddSites",        _scaleBorder[ID_ADDSITES]);

  config->setGroup("ShowElements");
  config->writeEntry("VOR",             _showElements[ID_VOR]);
  config->writeEntry("VORDME",          _showElements[ID_VORDME]);
  config->writeEntry("VORTAC",          _showElements[ID_VORTAC]);
  config->writeEntry("NDB",             _showElements[ID_NDB]);
  config->writeEntry("CompPoint",       _showElements[ID_COMPPOINT]);
  config->writeEntry("IntAirport",      _showElements[ID_INTAIRPORT]);
  config->writeEntry("Airport",         _showElements[ID_AIRPORTEL]);
  config->writeEntry("MilAirport",      _showElements[ID_MAIRPORT]);
  config->writeEntry("CivMilAirport",   _showElements[ID_CMAIRPORT]);
  config->writeEntry("Airfield",        _showElements[ID_AIRFIELD]);
  config->writeEntry("ClosedAirfield",  _showElements[ID_CLOSEDAIR]);
  config->writeEntry("CivHeliport",     _showElements[ID_CHELIPORT]);
  config->writeEntry("MilHeliport",     _showElements[ID_MHELIPORT]);
  config->writeEntry("AmbHeliport",     _showElements[ID_AHELIPORT]);
  config->writeEntry("UltraLight",      _showElements[ID_ULTRALIGHT]);
  config->writeEntry("HangGlider",      _showElements[ID_HANGGLIDER]);
  config->writeEntry("Parachute",       _showElements[ID_PARACHUTE]);
  config->writeEntry("Ballon",          _showElements[ID_BALLON]);

  config->setGroup(0);

  delete[] indexList;
}

void Map::mouseMoveEvent(QMouseEvent* event)
{
  extern double _currentScale;
/*
  // Wird zur Zeit noch nicht benötigt, daher erst mal
  // auskommentieren.
  // War nur ein erster Test ...

  QPainter window(this);
  window.setRasterOp(XorROP);
  window.setPen(QPen(QColor(0,255,0), 2));
  window.drawLine(_start, _current);
  window.drawLine(_start, event->pos());
  window.end();
*/
  // remember the current mouse position
  _currentPos = event->pos();

  struct point loc;
  map2Lambert((( event->pos().y() - DELTA_Y ) * _currentScale / RADIUS),
      (( event->pos().x() - DELTA_X ) * _currentScale / RADIUS), &loc);

//  mainApp->showCoords(loc.latitude, (loc.longitude + mapCenterLon));

  if(preX >= 0)
    bitBlt(this, preX - 20, preY - 20, &pixBuffer,
              preX - 20, preY - 20, 40, 40);

  struct flightPoint* cP = new struct flightPoint[1];
  const int index = 0;//view->getFlightDataView()->searchFlightPoint(_currentPos, cP);
  if(index != -1)
    {
//      mainApp->showPointInfo(cP);
      preX = cP->latitude;
      preY = cP->longitude;
      bitBlt(this, preX - 20, preY - 20, &pixCursor, 0, 0, -1, -1, NotEraseROP);
    }
  else
    {
//      mainApp->clearPointInfo();

      preX = -50;
      preY = -50;
    }
  delete cP;
}

void Map::mousePressEvent(QMouseEvent* event)
{
  extern double _currentScale, _scale[];
  extern MapContents _globalMapContents;

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

          if(!show)
            {
              /*
               * Es wurde keine Stadt gefunden.
               * Nun können die Dörfer durchsucht werden.
               */
              QPoint villagePos;
              double dX, dY;
              for(unsigned int loop = 0;
                    loop < _globalMapContents.getListLength(
                              MapContents::VillageList); loop++)
                {
                  hitElement = _globalMapContents.getElement(
                              MapContents::VillageList, loop);
                  villagePos = hitElement->getMapPosition();

                  dX = villagePos.x() - current.x();
                  dY = villagePos.y() - current.y();
                  // Abstand 1 Punkt größer als der Kreis
                  if( ( ( dX < 6.0 ) && ( dX > -6.0 ) ) &&
                      ( ( dY < 6.0 ) && ( dY > -6.0 ) ) )
                    {
                      text = i18n("Village:");
                      text = text + ' ' + hitElement->getName();
                      helpMenu->setTitle(text);
                      show = true;
                    }
                }
            }
        }

      if(showGlider)
        {
          /*
           * Segelflugplätze, soweit vorhanden, kommen als erster Eintrag
           */
          QPoint gliderPos;
          double dX, dY, delta;

          delta = 16.0;
          // Ist die Grenze noch aktuell ???
          if(_currentScale > _scale[3]) delta = 8.0;

          for(unsigned int loop = 0;
                loop < _globalMapContents.getListLength(
                        MapContents::GliderList); loop++)
            {
              hitElement = _globalMapContents.getElement(
                        MapContents::GliderList, loop);
              gliderPos = hitElement->getMapPosition();

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
        }

      if(showAirport)
        {
          QPoint sitePos;
          double dX, dY, delta;

          delta = 16.0;
            // Ist die Grenze noch aktuell ???
          if(_currentScale > _scale[3]) delta = 8.0;

          for(unsigned int loop = 0;
                loop < _globalMapContents.getListLength(
                      MapContents::AirportList); loop++)
            {
              hitElement = _globalMapContents.getElement(
                      MapContents::AirportList, loop);
              sitePos = hitElement->getMapPosition();

            dX = sitePos.x() - current.x();
            dY = sitePos.y() - current.y();

            // Abstand entspricht der Icon-Größe.
            if( ( ( dX < delta ) && ( dX > -delta ) ) &&
                ( ( dY < delta ) && ( dY > -delta ) ) )
              {
                text = i18n("Airport:");
                text = text + ' ' + hitElement->getName();
                /*
                 * Hier wird nicht nach Typ unterschieden.
                 */
//                helpMenu->insertItem(Icon(KApplication::kde_datadir() +
//                    "/kflog/map/small/airport.xpm"), text);
                helpMenu->connectItem(helpMenu->count() - 1, this,
                    SLOT(slotShowMapElement()));
                // helpMenu enthält auf jeden Fall den Titel und
                // zwei Linien, also 3 Einträge !!!
                indexLength = helpMenu->count() - 3;
                indexList = (unsigned int*) realloc(indexList,
                          (2 * indexLength * sizeof(int)));
                indexList[2 * indexLength - 2] =
                        (unsigned int)hitElement->getTypeID();
                indexList[2 * indexLength - 1] = loop;

                show = true;
              }
          }
      }

    if(showAirspace)
      {
        for(unsigned int loop = 0; loop < airspaceRegList->count(); loop++)
          {
            testReg = airspaceRegList->at(loop);
            if(testReg->contains(current))
              {
                hitElement = _globalMapContents.getElement(
                      MapContents::AirspaceList, loop);
              // Icon könnte "Aufgabe" der Klasse "Airspace" werden ...
                switch(hitElement->getTypeID())
                  {
                    case BaseMapElement::AirC:
                    case BaseMapElement::AirCtemp:
                      text = "C";
                      break;
                    case BaseMapElement::AirD:
                    case BaseMapElement::AirDtemp:
                      text = "D";
                      break;
                    case BaseMapElement::AirElow:
                    case BaseMapElement::AirEhigh:
                      text = "E";
                      break;
                    case BaseMapElement::AirF:
                      text = "F";
                      break;
                    case BaseMapElement::Restricted:
                    case BaseMapElement::Danger:
                      text = "Danger";
                      break;
                    case BaseMapElement::ControlD:
                      text = "Control";
                      break;
                    default:
                      text = "?";
                      break;
                  }
                text = text + ' ' + hitElement->getName();
//                helpMenu->insertItem(Icon(KApplication::kde_datadir() +
//                  "/kflog/map/small/airspace.xpm"), text);
                helpMenu->connectItem(helpMenu->count() - 1, this,
                    SLOT(slotShowMapElement()));
                indexLength = helpMenu->count() - 3;
                indexList = (unsigned int*) realloc(indexList,
                          (2 * indexLength * sizeof(int)));
                indexList[2 * indexLength - 2] = hitElement->getTypeID();
                indexList[2 * indexLength - 1] = loop;
                show = true;
              }
          }
      }

    if(show) helpMenu->exec(event->globalPos());
    else indexLength = 0;
  }
}

void Map::paintEvent(QPaintEvent* event = 0)
{
  bitBlt(this, 0, 0, &pixBuffer);
  /* Cursor-Position zurücksetzen! */
  preX = -50;
  preY = -50;
}

void Map::__drawGrid()
{
  extern const double _currentScale, _scale[];

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
  if(_currentScale < _scale[4])      step = 10;
  else if(_currentScale < _scale[6]) step = 30;
  else if(_currentScale < _scale[8]) gridStep = 2;
  else gridStep = 4;

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
  QPainter allSitesP(&pixAllSites);
  QPainter airSpaceP(&pixAirspace);
  QPainter gliderP(&pixGlider);
  QPainter waypointP(&pixWaypoints);
  QPainter underMapP(&pixUnderMap);

  delete airspaceRegList;
  delete cityRegList;
  airspaceRegList = new QList<QRegion>;
  cityRegList = new QList<QRegion>;
  airspaceRegList->setAutoDelete(true);
  cityRegList->setAutoDelete(true);

  extern double _currentScale, _scale[];
  extern int _scaleBorder[];
  extern bool _showElements[];
  extern MapContents _globalMapContents;

  BaseMapElement* _current;

  delete[] xPos;
  delete[] yPos;

  posNum = _globalMapContents.getListLength(MapContents::GliderList);
  xPos = new int[posNum];
  yPos = new int[posNum];

  // Statusbar noch nicht "genial" eingestellt ...
  mainApp->slotSetProgress(0);

  pixUnderMap.fill(QColor(96,128,248));

  _globalMapContents.drawIsoList(&underMapP);

  if(_currentScale <= _scale[_scaleBorder[ID_GLACIER]])
      _globalMapContents.drawList(&underMapP, MapContents::TopoList);

  mainApp->slotSetProgress(5);

  _globalMapContents.drawList(&underMapP, MapContents::HydroList);

  mainApp->slotSetProgress(10);

  _globalMapContents.drawList(&underMapP, MapContents::RoadList);

  mainApp->slotSetProgress(15);

  if(_currentScale <= _scale[_scaleBorder[ID_HIGHWAY]])
      _globalMapContents.drawList(&underMapP, MapContents::HighwayList);

  mainApp->slotSetProgress(20);

  mainApp->slotSetProgress(25);

  _globalMapContents.drawList(&underMapP, MapContents::RailList);

  mainApp->slotSetProgress(30);
/*
  // If needed, here are the stations
  // Hier stimmt die Liste noch nicht!
  if(isRail) DRAW_LOOP(MapContents::RailList, &railP)
*/
  mainApp->slotSetProgress(35);

  for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
          MapContents::CityList); loop++)
    {
      _current = _globalMapContents.getElement(MapContents::CityList, loop);
      cityRegList->append(_current->drawRegion(&underMapP));
    }

  mainApp->slotSetProgress(40);

  if(_currentScale <= _scale[_scaleBorder[ID_VILLAGE]])
      _globalMapContents.drawList(&underMapP, MapContents::VillageList);

  mainApp->slotSetProgress(45);

  if(_currentScale <= _scale[_scaleBorder[ID_LANDMARK]])
      _globalMapContents.drawList(&underMapP, MapContents::LandmarkList);

  mainApp->slotSetProgress(50);

  if(_currentScale <= _scale[_scaleBorder[ID_OBSTACLE]])
      _globalMapContents.drawList(&underMapP, MapContents::ObstacleList);

  mainApp->slotSetProgress(55);

  if(_currentScale <= _scale[_scaleBorder[ID_RADIO]])
      _globalMapContents.drawList(&allSitesP, MapContents::ReportList);

  mainApp->slotSetProgress(60);

  if(_currentScale <= _scale[_scaleBorder[ID_RADIO]])
      _globalMapContents.drawList(&allSitesP, MapContents::NavList);

  mainApp->slotSetProgress(65);

  if(_currentScale <= _scale[_scaleBorder[ID_AIRSPACE]])
    for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
              MapContents::AirspaceList); loop++)
      {
        _current = _globalMapContents.getElement(
            MapContents::AirspaceList, loop);
        airspaceRegList->append(_current->drawRegion(&airSpaceP));
      }

  mainApp->slotSetProgress(70);

  if((_currentScale <= _scale[_scaleBorder[ID_AIRPORT]]) && _showElements[5])
    for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
          MapContents::IntAirportList); loop++)
      {
        _current = _globalMapContents.getElement(
              MapContents::IntAirportList, loop);
        _current->drawRegion(&allSitesP);
      }

  mainApp->slotSetProgress(75);

  if(_currentScale <= _scale[_scaleBorder[ID_AIRPORT]])
      _globalMapContents.drawList(&allSitesP, MapContents::AirportList);

  mainApp->slotSetProgress(80);

  if(_currentScale <= _scale[_scaleBorder[ID_ADDSITES]])
      _globalMapContents.drawList(&allSitesP, MapContents::AddSitesList);

  mainApp->slotSetProgress(85);

  if(_currentScale <= _scale[_scaleBorder[ID_OUTLANDING]])
      _globalMapContents.drawList(&allSitesP, MapContents::OutList);

  mainApp->slotSetProgress(90);

  if(_currentScale <= _scale[_scaleBorder[ID_GLIDERSITE]])
      _globalMapContents.drawList(&gliderP, MapContents::GliderList);

  mainApp->slotSetProgress(95);

/*
  if(isWaypoint && (_currentScale <= _scale[_scaleBorder[ID_WAYPOINTS]]))
      DRAW_LOOP(MapContents::WaypointList, &mapP)
*/
  if(displayFlights) __drawFlight();

  mainApp->slotSetProgress(100);

  // Closing the painter ...
  allSitesP.end();
  airSpaceP.end();
  gliderP.end();
  waypointP.end();
  underMapP.end();
}

void Map::resizeEvent(QResizeEvent* event)
{
warning("Map::resizeEvent(%d / %d)",
    event->size().width(), event->size().height());

  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.createMatrix(event->size());

  pixBuffer.resize(event->size());
  pixBuffer.fill(white);

  if(event->size() == event->oldSize())
    warning("    Größe unverändert!");
  else
    warning("    Größe hat sich verändert!");

  // Beim Programmstart wird dies einmal zuviel aufgerufen ...
  if(!event->size().isEmpty())
      __redrawMap();
}

void Map::__redrawMap()
{
  pixAllSites.fill(white);
  pixAirspace.fill(white);
  pixGlider.fill(white);
  pixGrid.fill(white);
  pixUnderMap.fill(white);
  pixWaypoints.fill(white);

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();

  slotShowLayer();

//  view->slotShowMapData(this->width(), this->height());
}

void Map::slotRedrawMap() { __redrawMap(); }

void Map::slotShowLayer()
{
  SHOW_LAYER(showAddSites, ID_LAYER_ADDSITES)
  SHOW_LAYER(showAirport,  ID_LAYER_AIRPORT)
  SHOW_LAYER(showAirspace, ID_LAYER_AIRSPACE)
  SHOW_LAYER(showCity,     ID_LAYER_CITY)
  SHOW_LAYER(showFlight,   ID_LAYER_FLIGHT)
  SHOW_LAYER(showGlider,   ID_LAYER_GLIDER)
  SHOW_LAYER(showHydro,    ID_LAYER_HYDRO)
  SHOW_LAYER(showLand,     ID_LAYER_LAND)
  SHOW_LAYER(showNav,      ID_LAYER_NAV)
  SHOW_LAYER(showOut,      ID_LAYER_OUT)
  SHOW_LAYER(showRail,     ID_LAYER_RAIL)
  SHOW_LAYER(showRoad,     ID_LAYER_ROAD)
  SHOW_LAYER(showTopo,     ID_LAYER_TOPO)
  SHOW_LAYER(showWaypoint, ID_LAYER_WAYPOINT)

//  pixBuffer.fill(QColor(239,238,236));
  /* Wg. der Höhenlinien mal geändert ... */
  pixBuffer.fill(white);
//  pixTempA.fill(white);
//  pixTempB.fill(white);
//  pixTempC.fill(QColor(120,120,120));
//  pixTempD.fill(QColor(50,50,50));

  // deleting the current pixmap and displaying the grid:
//  bitBlt(&pixTempA, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);
/*
  PROOF_LAYER(showHydro,    pixTempA, pixHydro)
  PROOF_LAYER(showRoad,     pixTempA, pixRoad)
  PROOF_LAYER(showRail,     pixTempA, pixRail)
  PROOF_LAYER(showCity,     pixTempA, pixCity)
  PROOF_LAYER(showLand,     pixTempA, pixLand)
  PROOF_LAYER(showTopo,     pixTempA, pixTopo)
  PROOF_LAYER(showAirport,  pixTempB, pixAirport)
  PROOF_LAYER(showNav,      pixTempB, pixNav)
  PROOF_LAYER(showAddSites, pixTempB, pixAddSites)
  PROOF_LAYER(showGlider,   pixTempB, pixGlider)
  PROOF_LAYER(showOut,      pixTempB, pixOut)
  PROOF_LAYER(showWaypoint, pixTempB, pixWaypoints)
  PROOF_LAYER(showAirspace, pixTempB, pixAirspace)

  bitBlt(&pixTempC, 0, 0, &pixTempA, 0, 0, -1, -1, OrROP);
  bitBlt(&pixTempD, 0, 0, &pixTempB, 0, 0, -1, -1, OrROP);

  bitBlt(&pixBuffer, 0, 0, &pixIso, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixTempC, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixTempD, 0, 0, -1, -1, NotEraseROP);

//  bitBlt(&pixBuffer, 0, 0, &pixTempC, 0, 0, -1, -1, NotEraseROP);
//  bitBlt(&pixBuffer, 0, 0, &pixTempD, 0, 0, -1, -1, NotEraseROP);

  PROOF_LAYER(showFlight,   pixBuffer, pixFlight)
*/

  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixGrid, 0, 0, -1, -1, NotEraseROP);

  paintEvent();
}

void Map::slotZoomIn()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.scaleAdd(this->size());

  __redrawMap();
}

void Map::slotZoomOut()
{
  extern MapMatrix _globalMapMatrix;
  _globalMapMatrix.scaleSub(this->size());

  __redrawMap();
}

void Map::slotDrawFlight()
{
//  if(view->getFlightDataView()->getFlightList()->count() == 0) return;

//  displayFlights = true;

//  __drawFlight();
//  slotShowLayer();
}

void Map::slotConfigureMap()
{
//  MapConfig* confD = new MapConfig(mainApp);
//  confD->exec();
//  if(confD->result()) __redrawMap();
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
//  PrintDialog pD(mainApp, true);
//  pD.openMapPrintDialog(mapCenterLat, mapCenterLon);
}

void Map::slotCenterToItem(int listIndex, int elementIndex)
{
  extern MapContents _globalMapContents;
  SinglePoint* hitEl = _globalMapContents.getSinglePoint(listIndex,
            elementIndex);

  mapCenterLat = hitEl->getPosition().x();
  mapCenterLon = hitEl->getPosition().y();

  __redrawMap();
}

void Map::slotCenterToFlight()
{
/*
  if(!view->getFlightDataView()->getFlightList()->count()) return;

  extern double _currentScale;
  extern const double _scale[];
  const struct point* center = view->getFlightDataView()->getFlightCenter();

  int newCenterLat = ( center[1].latitude + center[0].latitude ) / 2;
  int newCenterLon = ( center[1].longitude + center[0].longitude ) / 2;

  double latScale = dist(center[0].latitude, center[0].longitude,
          center[1].latitude, center[0].longitude) * 1000.0 / this->height();

  double lonScale =  dist(newCenterLat, center[0].longitude,
          newCenterLat, center[1].longitude) * 1000.0 / this->width();

  double newScale = ( MAX(latScale, lonScale) + 10.0 ) * 1.05;

  // Wenn Abstand der Punkte zu klein, und Maßstab ähnlich ist,
  // wird nicht neu gezeichnet ...
  if( ( ( dist(newCenterLat, newCenterLon, mapCenterLat, mapCenterLon)
          * 1000.0 / _currentScale ) < 20 ) &&
      ((newScale / _currentScale) < 1.1 && (newScale / _currentScale) > 0.75))
    return;

  mapCenterLat = newCenterLat;
  mapCenterLon = newCenterLon;

  if(newScale < _scale[0])      newScale = _scale[0];
  else if(newScale > _scale[9]) newScale = _scale[9];

  _currentScale = newScale;
  __redrawMap();
*/
}

void Map::slotCenterToTask()
{
/*
  if(!view->getFlightDataView()->getFlightList()->count()) return;

  extern double _currentScale;
  extern const double _scale[];
  const struct point* center = view->getFlightDataView()->getTaskCenter();

  int newCenterLat = ( center[1].latitude + center[0].latitude ) / 2;
  int newCenterLon = ( center[1].longitude + center[0].longitude ) / 2;

  double latScale = dist(center[0].latitude, center[0].longitude,
          center[1].latitude, center[0].longitude) * 1000.0 / this->height();

  double lonScale =  dist(newCenterLat, center[0].longitude,
          newCenterLat, center[1].longitude) * 1000.0 / this->width();

  double newScale = ( MAX(latScale, lonScale) + 10.0 ) * 1.1;

  // Wenn Abstand der Punkte zu klein, und Maßstab ähnlich ist,
  // wird nicht neu gezeichnet ...
  if( ( ( dist(newCenterLat, newCenterLon, mapCenterLat, mapCenterLon)
          * 1000.0 / _currentScale ) < 20 ) &&
      ((newScale / _currentScale) < 1.1 && (newScale / _currentScale) > 0.75))
    return;

  mapCenterLat = newCenterLat;
  mapCenterLon = newCenterLon;

  if(newScale < _scale[0])      newScale = _scale[0];
  else if(newScale > _scale[9]) newScale = _scale[9];

  _currentScale = newScale;
  __redrawMap();
*/
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
      __drawFlight();
    }

  if(!showFlight)
    mainApp->toolBar()->getButton(ID_LAYER_FLIGHT)->toggle();

  slotShowLayer();
}

void Map::__drawFlight()
{
/*
  QPainter flightP(&pixFlight);

  extern const double _currentScale;

  QList<Flight>* flightList = view->getFlightDataView()->getFlightList();
  for(unsigned int loop = 0; loop < flightList->count(); loop++)
    {
      flightList->at(loop)->drawMapElement(&flightP, DELTA_X, DELTA_Y,
          mapCenterLon, mapBorder);
    }
  flightP.end();
*/
}

void Map::loadWaypoint(QString waypointFileName)
{
  QFile* waypointFile = new QFile(waypointFileName);
  QFileInfo file(waypointFileName);
  if(file.extension() == "wgs")
    {
//      importWGS(waypointFile);
    }
  else
    {
      if(waypointFile->open(IO_ReadOnly))
        {
          QTextStream t( waypointFile );
          // Typen Namen
          QString s, height, typ, name, frequency, direction,
                  length, surface, alias;
          QList<QString> runwayList;
          bool isObject = false;
          runwayList.setAutoDelete(true);
          int latitude = 0, longitude = 0;
          int list = 1, runway = 0;
          while(!t.eof())
            {
              s = t.readLine().simplifyWhiteSpace();
              if(s == "")
                {
                  if(isObject && (typ != "") && (name != ""))
                    {
                      //  write into file;
                      QFile mapFile("/tmp/kflog_wend.out");
                      if(!mapFile.open(IO_ReadWrite))
                        {
                          KMessageBox::error(parentWidget(),
                              i18n("Cannot access the mapfile!"));
                          return;
                        }
                      mapFile.at(mapFile.size());
                      QTextStream mapOut(&mapFile);

                      name = name.replace(QRegExp("\""), "");
                      mapOut << "[NEW]\n"
                             << "TYPE=" << typ << endl
                             << "NAME=" << name << endl
                             << "LIST=" << list << endl
                             << "ELEV=" << height << endl;
                      if(frequency != "")
                          mapOut << "FREQUENCY=" << frequency << endl;
                      else if(alias != "")
                          mapOut << "ALIAS=" << alias << endl;

                      unsigned int length = runwayList.count();
                      for(unsigned int loop = 0; loop < length; loop++)
                        {
                          QString temp = *runwayList.take(0);
                          mapOut << "RUNWAY=" << temp << endl;
                        }
                      mapOut << "AT=" << latitude << " " << longitude << endl
                             << "[END]" << endl;
                     // löschen der Variablen
                      isObject = false;
                      typ = "";
                      list = 1;
                      name = "";
                      height = "";
                      frequency = "";
                      alias = "";
                      latitude = 0;
                      longitude = 0;
                   }
                }
              else if(s.mid(0,2) == "at")
                {
                  isObject = true;
                  runway = 0;
                  runwayList.~QList();
                  // New Object
                  unsigned int loop = 0;
                  // Koordinaten
                  if((s.mid(11,1) == "N") || (s.mid(11,1) == "S"))
                    {
                      latitude = degreeToNum(s.mid(3,9));
                      loop = 13;
                    }
                  else if((s.mid(12,1) == "N") || (s.mid(12,1) == "S"))
                    {
                      latitude = degreeToNum(s.mid(3,10));
                      loop = 14;
                    }

                  if((s.mid(loop + 9,1) == "E") || (s.mid(loop + 9,1) == "W"))
                    {
                      longitude = degreeToNum(s.mid(loop,9));
                      loop += 9;
                    }
                  else if((s.mid(loop + 10,1) == "E") ||
                          (s.mid(loop + 10,1) == "W"))
                    {
                      longitude = degreeToNum(s.mid(loop,10));
                      loop += 10;
                    }
                  else if((s.mid(loop + 11,1) == "E") ||
                          (s.mid(loop + 11,1) == "W"))
                    {
                      longitude = degreeToNum(s.mid(loop,11));
                      loop += 11;
                    }
                  else
                    {
                    }
                  //Hoehe
                  height = s.mid(loop + 6,10);
                  // Typen
                }
              else if(s.mid(0,11) == "GLIDER_SITE")
                {
                  typ = "glider";
                  list = 3;
                  name = s.mid(12,100);
                }
              else if(s.mid(0,15) == "AIRPORT_CIV_MIL")
                {
                  typ = "civ-mil-air";
                  name = s.mid(16,100);
                }
              else if(s.mid(0,8) == "AIRFIELD")
                {
                  typ = "airfield";
                  name = s.mid(9,100);
                }
              else if(s.mid(0,16) == "SPECIAL_AIRFIELD")
                {
                  // ???????????
                }
              else if(s.mid(0,11) == "AIRPORT_MIL")
                {
                  typ = "mil-air";
                  name = s.mid(12,25);
                }
              else if(s.mid(0,11) == "INTLAIRPORT")
                {
                  typ = "int-airport";
                  name = s.mid(12,25);
                }
              else if(s.mid(0,7) == "AIRPORT")
                {
                  typ = "airport";
                  name = s.mid(8,25);
                }
              else if(s.mid(0,9) == "frequency")
                  frequency = s.mid(10,7);
              else if(s.mid(0,6) == "runway")
                {
                  runway++;
                  QString* temp = 0;
                  // search for space (hopefully, there will only be one!)
                  unsigned int loop = 0;
                  for(loop = 11; loop < strlen(s); loop++)
                      if(s.mid(loop, 1) == " ") break;

                  direction = s.mid(7,3);
                  if(direction.mid(0,2) > "36")
                      direction = "0" + direction.mid(0,1);
                  else if(direction.mid(0,2) == "00")
                      direction = "36";
                  else
                      direction = direction.mid(0,2);

                  length = s.mid(11,loop - 11);

                  if(s.mid(loop + 1,8) == "CONCRETE")
                      temp = new QString("C " + direction + " " + length);
                  else if(s.mid(loop + 1,4) == "GRAS")
                      temp = new QString("G " + direction + " " + length);
                  else if(s.mid(loop + 1,7) == "ASPHALT")
                      temp = new QString("A " + direction + " " + length);
                  runwayList.append(temp);
                }
              else if(s.mid(0,5) == "alias")
                  alias = s.mid(5,4);
              else
                  warning("KFLog: unknown entry found in ascii-map-file");
            }
        }
    }
}
