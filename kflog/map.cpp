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

#define PROOF_LAYER(a,b,c) if(a){bitBlt(&b,0,0,&c,0,0,-1,-1,NotEraseROP);}
#define PROOF_BUTTON(a,b) if(a){mainApp->toolBar()->getButton(b)->toggle();}

//#define NUM_TO_RAD(a) ( PI * a ) / 108000000.0 )
//
//#define MAP_X calc_X_Lambert(NUM_TO_RAD(mapCenterLat), 0)
//#define MAP_Y calc_Y_Lambert(NUM_TO_RAD(mapCenterLat), 0)
//
//#define DELTA_X ( ( this->width() / 2 ) - ( MAP_X / _currentScale * RADIUS ) )
//#define DELTA_Y ( ( this->height() / 2 ) - ( MAP_Y / _currentScale * RADIUS ) )

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

// Festlegen der Größe der Pixmaps auf Desktop-Grösse
#define PIX_WIDTH  QApplication::desktop()->width()
#define PIX_HEIGHT QApplication::desktop()->height()

Map::Map(KFLogApp *m, QFrame* parent)
: QWidget(parent),
  mainApp(m), posNum(1), indexLength(0), prePos(-50, -50)
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
  cursor.setPen(QPen(QColor(255,100,255), 2));
  cursor.drawLine(0,0,40,40);
  cursor.drawLine(0,40,40,0);
  cursor.setPen(QPen(QColor(255,0,255), 3));
  cursor.drawEllipse(10, 10, 20, 20);
  cursor.end();

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
  SET_BORDER(AirC,               "Airspace",9)
  SET_BORDER(AirCtemp,           "Airspace",9)
  SET_BORDER(AirD,               "Airspace",9)
  SET_BORDER(AirDtemp,           "Airspace",9)
  SET_BORDER(ControlD,           "Airspace",9)
  SET_BORDER(AirElow,            "Airspace",9)
  SET_BORDER(AirEhigh,           "Airspace",9)
  SET_BORDER(AirF,               "Airspace",9)
  SET_BORDER(Restricted,         "Airspace",9)
  SET_BORDER(Danger,             "Airspace",9)
  SET_BORDER(LowFlight,          "Airspace",9)
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

  QWhatsThis::add(this, i18n("<B>The map</B>"
         "<P>To move or scale the map, please use the buttons in the "
         "<B>Map-control</B>-area. Or center the map to the current "
         "cursor-positon by using the right mouse-button.</P>"
         "<P>To zoom in or out, use the slider or the two buttons on the "
         "toolbar. You can also zoom with \"&lt;Ctrl&gt;&lt;+&gt;\" (zoom in) "
         "and \"&lt;Ctrl&gt;&lt;-&gt;\" (zoom out).</P>"
         "<P>With the menu-item \"Options\" -> \"Configure map\" you can "
         "configure, which map elements should be displayed at which "
         "scale.</P>"));
}

Map::~Map()
{
  extern const double _currentScale, _scale[];
  extern const int _scaleBorder[];
  extern const bool _showElements[];

  KConfig *config = kapp->config();

  config->setGroup("General");
  config->writeEntry("MapScale",   _currentScale);
  config->writeEntry("MapCenterX", mapCenterLon);
  config->writeEntry("MapCenterY", mapCenterLat);

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
        }

//      if(showAirport)
        {
          QPoint sitePos;
          double dX, dY, delta;

          delta = 16.0; // Ist die Grenze noch aktuell ???
          text = "";    // Wir wollen _nur_ Flugplätze anzeigen!
          if(_currentScale > _scale[3]) delta = 8.0;

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

  pixIsoMap.fill(QColor(96,128,248));

  _globalMapContents.drawIsoList(&isoMapP, &mapMaskP);

  if(_currentScale <= _scale[_scaleBorder[ID_GLACIER]])
      _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::TopoList);

  mainApp->slotSetProgress(5);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::CityList);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::HydroList);

  mainApp->slotSetProgress(10);

  _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::RoadList);

  mainApp->slotSetProgress(15);

  if(_currentScale <= _scale[_scaleBorder[ID_HIGHWAY]])
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

//  if(_currentScale <= _scale[_scaleBorder[ID_VILLAGE]])
//      _globalMapContents.drawList(&uMapP, MapContents::VillageList);

  mainApp->slotSetProgress(45);

  if(_currentScale <= _scale[_scaleBorder[ID_LANDMARK]])
      _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::LandmarkList);

  mainApp->slotSetProgress(50);

  if(_currentScale <= _scale[_scaleBorder[ID_OBSTACLE]])
      _globalMapContents.drawList(&uMapP, &mapMaskP, MapContents::ObstacleList);

  mainApp->slotSetProgress(55);

  if(_currentScale <= _scale[_scaleBorder[ID_RADIO]])
      _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::ReportList);

  mainApp->slotSetProgress(60);

  if(_currentScale <= _scale[_scaleBorder[ID_RADIO]])
      _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::NavList);

  mainApp->slotSetProgress(65);

//  if(_currentScale <= _scale[_scaleBorder[ID_AIRSPACE]])
    unsigned int max = _globalMapContents.getListLength(
              MapContents::AirspaceList);

    for(unsigned int loop = 0; loop < max; loop++)
      {
        _current = _globalMapContents.getElement(
            MapContents::AirspaceList, loop);
        // wir sollten nur die Lufträume in der Liste speichern, die
        // tatsächlich gezeichnet werden. Dann geht die Suche schneller.
//        airspaceRegList->append(_current->drawRegion(&airSpaceP, &mapMaskP));
        airspaceRegList->append(_current->drawRegion(&airSpaceP,
            &airspaceMaskP));
      }

  mainApp->slotSetProgress(70);

  if((_currentScale <= _scale[_scaleBorder[ID_AIRPORT]]) && _showElements[5])
    for(unsigned int loop = 0; loop < _globalMapContents.getListLength(
          MapContents::IntAirportList); loop++)
      {
        _current = _globalMapContents.getElement(
              MapContents::IntAirportList, loop);
        _current->drawRegion(&aSitesP, &mapMaskP);
      }

  mainApp->slotSetProgress(75);

  if(_currentScale <= _scale[_scaleBorder[ID_AIRPORT]])
      _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::AirportList);

  mainApp->slotSetProgress(80);

  if(_currentScale <= _scale[_scaleBorder[ID_ADDSITES]])
      _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::AddSitesList);

  mainApp->slotSetProgress(85);

  if(_currentScale <= _scale[_scaleBorder[ID_OUTLANDING]])
      _globalMapContents.drawList(&aSitesP, &mapMaskP, MapContents::OutList);

  mainApp->slotSetProgress(90);

  if(_currentScale <= _scale[_scaleBorder[ID_GLIDERSITE]])
      _globalMapContents.drawList(&gliderP, &mapMaskP, MapContents::GliderList);

  mainApp->slotSetProgress(95);

  _globalMapContents.drawList(&flightP, &flightMaskP, MapContents::FlightList);

  mainApp->slotSetProgress(100);

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

void Map::__redrawMap()
{
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

  extern MapContents _globalMapContents;
  _globalMapContents.proofeSection();

  __drawGrid();
  __drawMap();

  slotShowLayer();
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
//  pixBuffer.fill(white);

  pixUnderMap.setMask(bitMapMask);
  pixFlight.setMask(bitFlightMask);
  bitBlt(&pixBuffer, 0, 0, &pixIsoMap);
//  bitBlt(&pixBuffer, 0, 0, &bitMapMask, 0, 0, -1, -1, NotEraseROP);
//  bitBlt(&pixBuffer, 0, 0, &pixUnderMap, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixUnderMap);
  bitBlt(&pixBuffer, 0, 0, &pixAllSites, 0, 0, -1, -1, NotEraseROP);
  bitBlt(&pixBuffer, 0, 0, &pixAirspace, 0, 0, -1, -1, NotEraseROP);
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
