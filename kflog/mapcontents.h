/***********************************************************************
**
**   mapcontents.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2010-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

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
 * \date 2000-2014
 *
 * \version $Id$
 */

#ifndef MAP_CONTENTS_H
#define MAP_CONTENTS_H

#include <QBitArray>
#include <QFile>
#include <QList>
#include <QObject>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPair>
#include <QPoint>
#include <QRect>
#include <QSet>

#include "airspace.h"
#include "downloadmanager.h"
#include "flighttask.h"
#include "isolist.h"

class Airfield;
class Distance;
class Flight;
class FlightGroup;
class GliderSite;
class Isohypse;
class LineElement;
class RadioPoint;
class SinglePoint;

// number of isoline levels
#define ISO_LINE_LEVELS 51


class MapContents : public QObject
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( MapContents )

  public:
  /**
   * The index of Map element-Lists.
   */
  enum MapContentsListID { NotSet = 0, AirfieldList, GliderfieldList,
                           AddSitesList, OutLandingList, NavList, AirspaceList,
                           ObstacleList, ReportList, CityList, VillageList,
                           LandmarkList, HighwayList, HighwayEntryList,
                           RoadList, RailList, StationList, HydroList,
                           LakeList, TopoList, IsohypseList,
                           WaypointList, DigitList, FlightList };

  /**
   * Types used by automatic download actions.
   */
  enum AutoDownloadType { ADT_NotSet = 0, Automatic, Inhibited };

  /**
   * Creates a new MapContents-object.
   */
  MapContents( QObject* object = 0 );

  /**
   * Destructor, deletes all lists.
   */
  virtual ~MapContents();

  static MapContents* instance();

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
   * \return The airspace list.
   */

  SortableAirspaceList& getAirspaceList()
  {
    return airspaceList;
  };

  /**
   * \return The airspace region list.
   */

  QList<QPair<QPainterPath, Airspace *> > &getAirspaceRegionList()
    {
      return airspaceRegionList;
    };

  /**
   * @return a pointer to the given airfield
   *
   * @param  index  the list index of the airfield
   */
  Airfield* getAirfield(unsigned int index);

  /**
   * @returns a pointer to the given gliderfield
   *
   * @param  index  the list-index of the gliderfield
   */
  Airfield* getGliderfield(unsigned int index);

  /**
   * @return a pointer to the given outlanding
   *
   * @param  index  the list-index of the outlanding
   */
  Airfield* getOutlanding(uint index);

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
   * @param  listID  The index of the list to be drawn
   * @param  drawnElements A list of drawn elements
   */
  void drawList( QPainter* targetPainter,
                 unsigned int listID,
                 QList<BaseMapElement *>& drawnElements );

  /**
   * Draws all isohypses into the given painter
   *
   * @param  targetP  The painter to draw the elements into
   * @param  windowRect Internal geometry of the drawing window.
   */
  void drawIsoList( QPainter* targetP, QRect windowRect );

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
   * @returns the waypoint list.
   */
  QList<Waypoint*>& getWaypointList()
  {
    return wpList;
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
  int getFlightIndex() { return m_currentFlightListIndex; };

  /**
   * @returns the flightList
   */
  QList<BaseFlightElement*> *getFlightList();

  /** read a task file and append all tasks to flight list switch to first task in file */
  bool loadTask(QFile& path);
  /** generate new task name */
  QString genTaskName();
  /** generate a task name, using the suggestion given. Prevents double task names */
  QString genTaskName(QString suggestion);
  /** Checks if a task name is already in use or not. */
  bool taskNameInUse( QString name );

  /** \return The list of IsoHypse Regions. */
  IsoList* getIsohypseRegions()
  {
    return &pathIsoLines;
  };
  /**
   * Find the terrain elevation for the given point.
   *
   * \param coordMap The map coordinates of the point.
   *
   * \param errorDist Distance error value in meters
   *
   * \returns The elevation in meters or -1 if the elevation could not be found.
   */
  int getElevation(const QPoint& coordMap, Distance* errorDist);

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
  void slotCloseFlight();
  /** No descriptions */
  void slotSetFlight( QAction *action );
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

  /** Update airspace intersections in all flight. */
  void updateFlightAirspaceIntersections();

  /**
   * This slot is called to download the Welt2000 file from the Internet.
   */
  void slotDownloadWelt2000(bool askUser);

  /**
   * Reload airfield data. Can be called after a configuration change or
   * after a download update.
   */
  void slotReloadAirfieldData();

  /*
   * Called, if openAIP airspace files shall be downloaded.
   */
  void slotDownloadOpenAipAirspaceFiles(bool askUser);

  /**
   * Reload airspace data. Can be called after a configuration change.
   */
  void slotReloadAirspaceData();

  /*
   * Called, if openAIP airfield files shall be downloaded.
   */
  void slotDownloadOpenAipAirfieldFiles(bool askUser);

  /**
   * Reload airfield data. Can be called after a configuration change.
   */
  //void slotReloadOpenAipAirfieldData();


 private slots:

  /** Called, if all downloads are finished. */
  void slotDownloadsFinished( int requests, int errors );

  /** Called, if a network error occurred during the downloads. */
  void slotNetworkError();

  /** Called, if a WELT2000.TXT download is finished. */
  void slotWelt2000DownloadFinished( int requests, int errors );

  /** Called, if all openAIP airspace file downloads are finished. */
  void slotOpenAipAsDownloadsFinished( int requests, int errors );

  /**
   * Called, if a network error occurred during the openAIP airspace file
   * downloads.
   */
  void slotOpenAipAsNetworkError();

  /** Called, if all openAIP airfield file downloads are finished. */
  void slotOpenAipAfDownloadsFinished( int requests, int errors );

  /**
   * Called, if a network error occurred during the openAIP airfield file
   * downloads.
   */
  void slotOpenAipAfNetworkError();

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
  void taskHelp(QString& text);
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
   * Emitted if a new flight was added to the flight list.
   */
  void newFlightAdded(Flight *);

  /**
   * Emitted if a new flight group was added to the flight list.
   */
  void newFlightGroupAdded(FlightGroup *);

  /**
   * Emitted if a flight (task/group) is about to be closed
   */
  void closingFlight(BaseFlightElement*);

  /**
   * Emitted to the map, if the current flight is closed.
   */
  void clearFlightCursor();

  /**
   * Emitted, if airspaces have been loaded.
   */
  void airspacesLoaded();

 private:

  /**
   * Reads a binary map file.
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
   * Reads a binary ground or terrain file.
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
   * airfieldList contains airports, airfields, ultralight sites
   */
  QList<Airfield> airfieldList;

  /**
   * gliderfieldList contains all glider sites.
   */
  QList<Airfield> gliderfieldList;

  /**
   * outLandingList contains all outlanding fields.
   */
  QList<Airfield> outLandingList;

  /**
   * addSitesList contains all
   * hang-glider-sites, free-balloon-sites, parachute-jumping-sites.
   */
  QList<SinglePoint> addSitesList;

  /**
   * navList contains all radio navigation facilities.
   */
  QList<RadioPoint> navList;

  /**
   * airspaceList contains all airspaces. The sort function on this
   * list will sort the airspaces from top to bottom. This list must be stay
   * a pointer list because the cross reference to the airspace region.
   */
  SortableAirspaceList airspaceList;

  /**
   * Contains the regions of all visible airspaces. The list is needed to
   * find the airspace data when the user selects an airspace in the map.
   */
  QList<QPair<QPainterPath, Airspace*> > airspaceRegionList;

  //================ MAP ITEM LISTS =====================================

  /**
   * obstacleList contains all obstacles and -groups, as well
   * as the spots and passes.
   */
  QList<SinglePoint> obstacleList;

  /**
   * reportList contains all reporting points.
   */
  QList<SinglePoint> reportList;

  /**
   * cityList contains all cities (areas).
   */
  QList<LineElement> cityList;

  /**
   * villageList contains all villages, towns & cities (points).
   */
  QList<SinglePoint> villageList;

  /**
   * landmarkList contains all landmarks.
   */
  QList<SinglePoint> landmarkList;

  /**
   * highwayList contains all highways.
   */
  QList<LineElement> highwayList;

  /**
   * roadList contains all roads.
   */
  QList<LineElement> roadList;

  /**
   * railList contains all railways and aerial railways.
   */
  QList<LineElement> railList;

  /**
   * hydroList contains all shore lines, rivers, lakes, ...
   */
  QList<LineElement> hydroList;

  /**
   * lakeList contains all lakes, ...
   */
  QList<LineElement> lakeList;

  /**
   * topoList contains all topographical objects.
   */
  QList<LineElement> topoList;

  /**
   * Isohypse map contains all isohypses above the ground of a tile in a list.
   */
  QMap<int, QList<Isohypse> > terrainMap;
  /**
   * Isohypse map contains all ground isohypses of a tile in a list.
   */
  QMap<int, QList<Isohypse> > groundMap;

  /**
   * Contains list of all loaded Flight and FlightTask objects, which are
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
   * Set over map tiles. Contains the sectionId for all fully loaded
   * section files otherwise nothing.
   */
  QSet<int> tileSectionSet;

  /**
   * QMap of all partially loaded map tiles. Partially
   * loaded tiles (each tile currently consists of a maximum of
   * three files) can occur when KFLog if flies are missing. The proofeSection
   * routine uses this QMap to determine if it really needs to load a specific
   * file for that tile.
   */
  typedef QMap<int, char> TilePartMap;
  TilePartMap tilePartMap;

  /** */
  QString mapDir;

  /**
   * Used to determine, if we must display message boxes on missing
   * map-directories.
   */
  bool askUser;

  /**
   * Flag to indicate, if airfield load is needed.
   */
  bool loadAirfields;

  /**
   * Flag to indicate, if airspace load is needed.
   */
  bool loadAirspaces;

  /**
   * List of all drawn isohypses.
   */
  IsoList pathIsoLines;

  /**
   * Elevation where the next search for the current elevation will start.
   * Set to one level higher than the current level by findElevation().
   */
  int _nextIsoLevel;
  int _lastIsoLevel;
  bool _isoLevelReset;
  const IsoListEntry* _lastIsoEntry;

  /**
   * Array containing the used elevation levels in meters. Is used as help
   * for reverse mapping elevation to array index.
   */
  static const short isoLevels[ISO_LINE_LEVELS];

  /**
   * Hash table with elevation in meters as key and related elevation
   * index as value
   */
  QHash<short, uchar> isoHash;

  /**
   * Displays a message box and asks, weather the map files shall be downloaded.
   */
  int __askUserForDownload();

  /**
   * Try to download a missing ground/terrain file.
   *
   * \param file The name of the file without any path prefixes.
   * \param directory The destination directory.
   * \return true on success otherwise false
   */
  bool __downloadMapFile( QString &file, QString &directory );

  /** Manager to handle downloads of missing map file. */
  DownloadManager *m_downloadManger;

  /** Manager to handle download of WELT2000.TXT file. */
  DownloadManager *m_downloadMangerW2000;

  /** Manager to handle downloads of requested openAIP airspace files. */
  DownloadManager *m_downloadOpenAipAsManger;

  /** Manager to handle downloads of requested openAIP airfield files. */
  DownloadManager *m_downloadOpenAipAfManger;

  /**
   * index of the current flight in flightList
   */
  int m_currentFlightListIndex;
};

#endif
