/***********************************************************************
**
**   mapcontents.h
**
**   This file is part of KFLog2.
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

#ifndef MAPCONTENTS_H
#define MAPCONTENTS_H

#include <map>
#include <qbitarray.h>
#include <qfile.h>
#include <qptrlist.h>
#include <qobject.h>
#include <qstrlist.h>
#include <kio/jobclasses.h>

#include "downloadlist.h"
#include "flighttask.h"

class Airport;
class Airspace;
class Flight;
class FlightGroup;
class GliderSite;
class Isohypse;
class LineElement;
class RadioPoint;
class SinglePoint;

/**
 * @short Entry in the isolist
 *
 * This class contains a @ref QRegion and a height. A list of entries
 * like this is created when the map is drawn, and is used to detect the
 * elevation at a given position, for instance under the mousecursor.
 *
 * @author André Somers
 * @version $Id$
 */

class isoListEntry {
  public:
    /**
     * Constructor.
     * @param region Region in coordinate system of the map-object, not in KFLog system
     * @param height the elevation of the isoline in meters
     */
    isoListEntry(QRegion* region=0, int height=0);
    /**
     * Destructor
     */
    ~isoListEntry();

    QRegion* region;
    int height;
};

/**
 * This class provides functions for accessing the contents of the map.
 * It takes control over loading all needed map-files.
 * The class contains several QPtrLists holding the mapelements.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */

class MapContents : public QObject
{
  Q_OBJECT

  public:
  /**
   * The index of Mapelement-Lists.
   */
  enum MapContentsListID {NotSet = 0, AirportList, GliderSiteList,
                          AddSitesList, OutList, NavList, AirspaceList,
                          ObstacleList, ReportList, CityList, VillageList,
                          LandmarkList, HighwayList, HighwayEntryList,
                          RoadList, RailList, StationList, HydroList,
                          TopoList, IsohypseList,
                          WaypointList, DigitList, FlightList};

  /**
   * Creates a new MapContents-object.
   */
  MapContents();

  /**
   * Destructor, deletes all lists.
   */
  ~MapContents();

  /**
   * @return the current length of the given list.
   *
   * @param  listIndex  the index of the list.
   */
  unsigned int getListLength(int listIndex) const;

  /**
   * Proofes, which mapsections are needed to draw the map and loads
   * the missing sections.
   *
   * @param  isPrint  "true", if the map should be printed.
   */
  void proofeSection(bool isPrint = false);
  /**
   * @return a pointer to the BaseMapElement of the given mapelement in
   *         the list.
   *
   * @param  listIndex  the index of the list containing the element
   * @param  index  the index of the element in the list
   */
  BaseMapElement* getElement(int listIndex, unsigned int index);

  /**
   * @return a pointer to the given airspace
   *
   * @param  index  the list-index of the airspace
   */
  Airspace* getAirspace(unsigned int index);

  /**
   * @returns a pointer to the given glidersite
   *
   * @param  index  the list-index of the glidersite
   */
  GliderSite* getGlidersite(unsigned int index);

  /**
   * @return a pointer to the given airport
   *
   * @param  index  the list-index of the airport
   */
  Airport* getAirport(unsigned int index);

  /**
   * @return a pointer to the SinglePoint of the given mapelement
   *
   * @param  listIndex  the index of the list containing the element
   * @param  index  the index of the element in the list
   */
  SinglePoint* getSinglePoint(int listIndex, unsigned int index);

  /**
   * Draws all elements of a list into the painter.
   *
   * @param  targetP  The painter to draw the elements into
   * @param  maskP  The maskpainter of targetP
   * @param  listID  The index of the list to be drawn
   */
  void drawList(QPainter* targetPainter, QPainter* maskPainter,
                unsigned int listID);

  /**
   * Draws all isohypses into the given painter
   *
   * @param  targetP  The painter to draw the elements into
   * @param  maskP  The maskpainter of targetP
   */
  void drawIsoList(QPainter* targetP, QPainter* maskP);

  /**
   * Prints the whole content of the map into the given painter.
   *
   * @param  targetP  The painter to draw the elements into
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  void printContents(QPainter* targetP, bool isText);

  /**
   * @returns the waypoint list
   */
  QPtrList<Waypoint>* getWaypointList()
  {
    return &wpList;
  };

  /**
   * @param flight adds a flight to flightList
   */
  void appendFlight(Flight* flight);
  /**
   * @return a pointer to the current flight
   */
  BaseFlightElement* getFlight();

  /**
   * @return a pointer to the current flight index
   */
  int getFlightIndex() { return flightList.at(); };

  /**
   * @returns the flightList
   */
  QPtrList<BaseFlightElement>* getFlightList();

  /**
   * Converts the longitute or latitute into the internal format.
   *
   * @param  degree  The position to be converted. The string must
   *                 have the format:<BR>
   *                 <TT>[g]gg.mm'ss"X</TT> where <TT>g</TT>, <TT>m</TT>,
   *                 <TT>s</TT> are any digits from 0 to 9 and <TT>X</TT>
   *                 is one of N, S, E, W.
   */
  static int degreeToNum(QString degree);
  /** read a task file and append all tasks to flight list switch to first task in file */
  bool loadTask(QFile& path);
  /** generate new task name */
  QString genTaskName();
  /** generate a task name, using the suggestion given. Prevents double task names */
  QString genTaskName(QString suggestion);
  inline QPtrList<isoListEntry>* getIsohypseRegions(){return &regIsoLines;};
  /**
   * find the terrain elevation for the given point
   * @returns the elevation in meters or -1 if the elevation could not be found.
   */
  int getElevation(QPoint);

  /**
   * this function serves as a substitute for the not existing
   * QDir::entryInfoList with complete path information
   */
  static void addDir(QStringList& list, const QString& path, const QString& filter);

  /**
   * @AP: Compares two projection objects for equality.
   * @returns true if equal; otherwise false
   */
  static bool compareProjections(ProjectionBase* p1, ProjectionBase* p2);

  public slots:
  /**
   * Close current flight
   */
  void closeFlight();
  /** No descriptions */
  void slotSetFlight(int id);
  /** No descriptions */
  void slotSetFlight(BaseFlightElement *);
  /** create a new, empty flight group */
  void slotNewFlightGroup();
  /** create a new, empty task */
  void slotNewTask();
  /** append task to flight list */
  void slotAppendTask(FlightTask *f);
  /** No descriptions */
  void slotEditFlightGroup();
  /** */
  void slotReloadMapData();
  /*
  *  automatic download has finished
  */
  void slotDownloadFinished();
  /** Re-projects any flights and tasks that may be loaded. */
  void reProject();

 signals:
  /**
   * emitted during map loading to display a message f.e. in the
   * splash-screen of the mainwindow.
   */
  void loadingMessage(QString message);
  /**
   * signal that a new flight/task/flight group become active
   */
  void currentFlightChanged();
  /**
   * signal that a new task has been created
   */
  void newTaskAdded(FlightTask *);
  /**
   * signal emitted when new task has been created
   */
  void taskHelp(QString);
  /**
   * Emitted, when no mapfiles are found, or the when the map-directories
   * do not exists.
   */
  void errorOnMapLoading();

  /**
   * Activates the graphical planning
   */
  void activatePlanning();

  /**
   *  Map contents have changed
   */
  void contentsChanged();

  /**
   * Emitted if a new flight was added to the flightlist.
   */
  void newFlightAdded(Flight *);

  /**
   * Emitted if a new flightgroup was added to the flightlist.
   */
  void newFlightGroupAdded(FlightGroup *);

  /**
   * Emitted if a flight (task/group) is about to be closed
   */
  void closingFlight(BaseFlightElement*);

 private:
  /**
   * Displays a messagebox and asks, wether the mapfiles shall be downloaded.
   */
  void __askForDownload();
  /**
   * Reads a binary map file containing airfields.
   *
   * @param  fileName  The path and name of the airfield-file.
   */
  bool __readAirfieldFile(const char* pathName);
  /**
   * Reads a new binary map file.
   *
   * @param  fileSecID  The sectionID of the mapfile
   * @param  fileTypeID  The typeID of the mapfile ("G" for ground-data,
   *                     "M" for additional mapdata and "T" for
   *                     terraindata)
   *
   * @return "true", when the file has successfully been loaded
   */
  bool __readBinaryFile(const int fileSecID, const char fileTypeID);

  /**
   * Reads a new binary terrain-map file.
   *
   * @param  fileSecID  The sectionID of the mapfile
   * @param  fileTypeID  The typeID of the mapfile ("G" for ground-data,
   *                     "M" for additional mapdata and "T" for
   *                     terraindata)
   *
   * @return "true", when the file has successfully been loaded
   */
  bool __readTerrainFile(const int fileSecID, const int fileTypeID);

  /**
   * airportList contains all airports.
   */
  QPtrList<Airport> airportList;

  /**
   * gliderSiteList contains all glider-sites.
   */
  QPtrList<GliderSite> gliderSiteList;

  /**
   * addSitesList contains all, ultra-light,
   * hang-glider-sites, free-balloon-sites, parachute-jumping-sites.
   * FIXME: Currently those sites are stored somewhere else?!?
   */
  QPtrList<SinglePoint> addSitesList;

  /**
   * outList contains all outlanding-fields.
   */
  QPtrList<SinglePoint> outList;

  /**
   * navList contains all radio navigation facilities.
   */
  QPtrList<RadioPoint> navList;

  /**
   * airspaceList contails all airspaces.

   */
  QPtrList<Airspace> airspaceList;

  /**
   * obstacleList contains all obstacles and -groups, as well
   * as the spots and passes.
   */
  QPtrList<SinglePoint> obstacleList;

  /**
   * reportList contains all reporting points.
   */
  QPtrList<SinglePoint> reportList;

  /**
   * cityList contails all cities (areas).
   */
  QPtrList<LineElement> cityList;

  /**
   * villageList contains all villages, towns & cities (points).
   */
    QPtrList<SinglePoint> villageList;

  /**
   * landmarkList contains all landmarks.
   */
  QPtrList<SinglePoint> landmarkList;

  /**
   * roadList contails all roads.
   */
  QPtrList<LineElement> roadList;
  /**
   * railList contains all railways and aerial railways.
   */
  QPtrList<LineElement> railList;
  /**
   * stationList contains all stations.
   */
//  QPtrList<SinglePoint> stationList;
  /**
   * hydroList contains all shorelines, rivers, lakes, ...
   */
  QPtrList<LineElement> hydroList;
  /**
   * topoList contains all topographical objects.
   */
  QPtrList<LineElement> topoList;
  /**
   * isohypseList contains all isohypses.
   */
  QPtrList< QPtrList<Isohypse> > isoList;
  /**
   * Contains list of all loaded Flight and FlightTask objects, wich are
   * both subclasses of BaseFlightElement.
   */
  QPtrList<BaseFlightElement> flightList;
  /**
   * This list is reset every time the current WaypointCatalog is changed.
   */
  QPtrList<Waypoint> wpList;
  /**
   * List of all map-section. Contains a "1" for all loaded section-files,
   * otherwise "0".
   */
  QBitArray sectionArray;
  /**
   * Array containing the evevations of all possible isohypses.
   */
  static const int isoLines[];
  /**
   * Hash isoLines index
   */
  std::map<int, int> isoHash;
  /** */
  QString mapDir;
  /**
   * Used to determine, if we must display messageboxes on missing
   * map-directories.
   */
  int isFirstLoad;
  /**
   * List of all drawn isohypses.
   * Used to find the elevation belonging to a point on the map.
   */
  QPtrList<isoListEntry> regIsoLines;

  /**
   * List of all loaded isohypses (in world coordinates).
   * Used to find the elevation belonging to a flightpoint.
   */
  QPtrList<isoListEntry> regIsoLinesWorld;
  /**
   * downloads File from www.kflog.org, optionally waits until finished (blocking operation)
   */
  void __downloadFile(QString fileName, QString destString, bool wait=false);
  /** */
  DownloadList* downloadList;
};

enum AutoDownloadType {ADT_NotSet = 0, Automatic, Inhibited};

#endif
