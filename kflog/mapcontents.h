/***********************************************************************
**
**   mapcontents.h
**
**   This file is part of KFLog4.
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

#ifndef MAP_CONTENTS_H
#define MAP_CONTENTS_H

#include <map>

#include <QBitArray>
#include <QFile>
#include <QList>
#include <QObject>

#include "downloadmanager.h"

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
 * elevation at a given position, for instance under the mouse cursor.
 *
 * @author Andr� Somers
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
 * \class MapContents
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map content handler.
 *
 * This class provides functions for accessing the contents of the map.
 * It takes control over loading all needed map-files.
 * The class contains several QPtrLists holding the map elements.
 *
 * \version $Id$
 */
class MapContents : public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( MapContents )

  public:
  /**
   * The index of Map element-Lists.
   */
  enum MapContentsListID {NotSet = 0, AirportList, GliderfieldList,
                          AddSitesList, OutLandingList, NavList, AirspaceList,
                          ObstacleList, ReportList, CityList, VillageList,
                          LandmarkList, HighwayList, HighwayEntryList,
                          RoadList, RailList, StationList, HydroList,
                          TopoList, IsohypseList,
                          WaypointList, DigitList, FlightList};

  /**
   * Types used by automatic download actions.
   */
  enum AutoDownloadType {ADT_NotSet = 0, Automatic, Inhibited};

  /**
   * Creates a new MapContents-object.
   */
  MapContents( QObject* object = 0 );

  /**
   * Destructor, deletes all lists.
   */
  ~MapContents();

  /**
   * @return the current length of the given list.
   *
   * @param  listIndex  the index of the list.
   */
  int getListLength(int listIndex) const;

  /**
   * Proofs, which map sections are needed to draw the map and loads
   * the missing sections.
   *
   * @param  isPrint  "true", if the map should be printed.
   */
  void proofeSection(bool isPrint = false);
  /**
   * @return a pointer to the BaseMapElement of the given map element in
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
   * @return a pointer to the SinglePoint of the given map element
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
  void drawList(QPainter* targetPainter, QPainter* maskPainter, unsigned int listID);

  /**
   * Draws all isohypses into the given painter
   *
   * @param  targetP  The painter to draw the elements into
   * @param  maskP  The mask painter of targetP
   */
  void drawIsoList(QPainter* targetP, QPainter* maskP);

  /**
   * Prints the whole content of the map into the given painter.
   *
   * @param  targetP  The painter to draw the elements into
   *
   * @param  isText  Shows, if the text of some map elements should
   *                 be printed.
   */
  void printContents(QPainter* targetP, bool isText);

  /**
   * @returns the waypoint list
   */
  QList<Waypoint*>* getWaypointList()
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
  int getFlightIndex() { return currentFlightListIndex; };

  /**
   * @returns the flightList
   */
  QList<BaseFlightElement*> *getFlightList();

  /**
   * Converts the longitude or latitude into the internal format.
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
  inline QList<isoListEntry*>* getIsohypseRegions(){return &regIsoLines;};
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

  /**
   * Checks the existence of the three required map directories.
   *
   * \return True if all is okay otherwise false.
   */
  bool checkMapDirectories();

  /**
   * Creates all required map directories.
   *
   * \return True if all is okay otherwise false.
   */
  bool createMapDirectories();

  /**
   * Returns the map root directory.
   *
   * \return The map root directory.
   */
  QString getMapRootDirectory();

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
   /** Re-projects any flights and tasks that may be loaded. */
  void reProject();

  /**
   * This slot is called to download the Welt2000 file from the internet.
   */
  void slotDownloadWelt2000();

  /**
   * Reload Welt2000 data file. Can be called after a configuration change or
   * a dowonload.
   */
  void slotReloadWelt2000Data();

  private slots:

  /** Called, if all downloads are finished. */
  void slotDownloadsFinished( int requests, int errors );

  /** Called, if a network error occurred during the downloads. */
  void slotNetworkError();

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
   * Emitted, when no map files are found, or the when the map-directories
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
  QList<Airport*> airportList;

  /**
   * gliderfieldList contains all glider-sites.
   */
  QList<GliderSite*> gliderfieldList;

  /**
   * addSitesList contains all, ultralight,
   * hang-glider-sites, free-balloon-sites, parachute-jumping-sites.
   * FIXME: Currently those sites are stored somewhere else?!?
   */
  QList<SinglePoint*> addSitesList;

  /**
   * outLandingList contains all outlanding-fields.
   */
  QList<SinglePoint*> outLandingList;

  /**
   * navList contains all radio navigation facilities.
   */
  QList<RadioPoint*> navList;

  /**
   * airspaceList contains all airspaces.

   */
  QList<Airspace*> airspaceList;

  /**
   * obstacleList contains all obstacles and -groups, as well
   * as the spots and passes.
   */
  QList<SinglePoint*> obstacleList;

  /**
   * reportList contains all reporting points.
   */
  QList<SinglePoint*> reportList;

  /**
   * cityList contails all cities (areas).
   */
  QList<LineElement*> cityList;

  /**
   * villageList contains all villages, towns & cities (points).
   */
  QList<SinglePoint*> villageList;

  /**
   * landmarkList contains all landmarks.
   */
  QList<SinglePoint*> landmarkList;

  /**
   * roadList contails all roads.
   */
  QList<LineElement*> roadList;
  /**
   * railList contains all railways and aerial railways.
   */
  QList<LineElement*> railList;
  /**
   * stationList contains all stations.
   */
//  QList<SinglePoint*> stationList;
  /**
   * hydroList contains all shorelines, rivers, lakes, ...
   */
  QList<LineElement*> hydroList;
  /**
   * topoList contains all topographical objects.
   */
  QList<LineElement*> topoList;
  /**
   * isohypseList contains all isohypses.
   */
  QList< QList<Isohypse*>* > isoList;
  /**
   * Contains list of all loaded Flight and FlightTask objects, wich are
   * both subclasses of BaseFlightElement.
   */
  QList<BaseFlightElement*> flightList;
  /**
   * Pointer to the current flight visible on the map and in the rest of the widgets
   */
  BaseFlightElement *currentFlight;
  /**
   * This list is reset every time the current WaypointCatalog is changed.
   */
  QList<Waypoint*> wpList;
  /**
   * List of all map-section. Contains a "1" for all loaded section-files,
   * otherwise "0".
   */
  QBitArray sectionArray;
  /**
   * Array containing the elevations of all possible isohypses.
   */
  static const int isoLines[];
  /**
   * Hash isoLines index
   */
  std::map<int, int> isoHash;
  /** */
  QString mapDir;
  /**
   * Used to determine, if we must display message boxes on missing
   * map-directories.
   */
  bool isFirstLoad;
  /**
   * List of all drawn isohypses.
   * Used to find the elevation belonging to a point on the map.
   */
  QList<isoListEntry*> regIsoLines;

  /**
   * List of all loaded isohypses (in world coordinates).
   * Used to find the elevation belonging to a flightpoint.
   */
  QList<isoListEntry*> regIsoLinesWorld;

  /**
   * Displays a message box and asks, weather the map files shall be downloaded.
   */
  int __askUserForDownload();

  /**
   * Try to download a missing ground/terrain file.
   *
   * @param file The name of the file without any path prefixes.
   * @param directory The destination directory.
   *
   */
  bool __downloadMapFile( QString &file, QString &directory );

  /** Manager to handle downloads of missing map file. */
  DownloadManager *downloadManger;

  /**
   * index of the current flight in flightList
   */
  int currentFlightListIndex;
};

#endif
