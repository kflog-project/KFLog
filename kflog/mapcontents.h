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

#include <qbitarray.h>
#include <qfile.h>
#include <qlist.h>
#include <qobject.h>
#include <qstrlist.h>
#include <kio/jobclasses.h>
#include <downloadlist.h>

#include <flighttask.h>

class Airport;
class Airspace;
class Flight;
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
 * The class contains several QLists holding the mapelements.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapContents : public QObject
{
  Q_OBJECT

  public:
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
     * Loads a new flight-file.
     *
     * @param  file  The path to the igc-file
     * @return "true", when the file has successfully been loaded
     */
    bool loadFlight(QFile&);
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
    QList<BaseFlightElement>* getFlightList();
    /**
     * @returns the wpList
     */
	QList<Waypoint>* getWaypointList();
    /**
     * Searches the first point of the current flight, which distance to the
     * mousecursor is less than 30 pixel. If no point is found, -1 is
     * returned.
     *
     * @see Flight::searchPoint
     *
     * @param  cPoint  The map-position of the mousecursor.
     * @param  searchPoint  A pointer to a flightpoint. Will be filled
     *                      with the flightpoint found.
     *
     * @return the index of the flightpoint or -1 if no point is found.
     */
    int searchFlightPoint(QPoint cPos, flightPoint& fP);
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
  /** No descriptions */
  bool importFlightGearFile(QFile& flightgearFile);
  /** Imports a file downloaded with Gardown in DOS  */
  bool importGardownFile(QFile& gardownFile);
  /** read a task file and append all tasks to flight list
switch to first task in file */
  bool loadTask(QFile& path);
  /** generate new task name */
  QString genTaskName();
    /**
		 * Get the contents of the next FlightPoint after number 'index'
	   */
//    int searchGetNextFlightPoint(int index, flightPoint & fP);
    /**
		 * Get the contents of the previous FlightPoint before number 'index'
	   */
//    int searchGetPrevFlightPoint(int index, flightPoint & fP);
		/**
		 * Get the contents of the next FlightPoint 'step' indexes after number 'index'
		 */
//  	int searchStepNextFlightPoint(int index, flightPoint & fP, int step);
  	/**
		 * Get the contents of the previous FlightPoint 'step' indexes before number 'index'
		 */
//  	int searchStepPrevFlightPoint(int index,  flightPoint & fP, int step);
    /**
     * The index of Mapelement-Lists.
     */
    enum MapContentsListID {NotSet = 0, AirportList, GliderList,
        AddSitesList, OutList, NavList, AirspaceList, ObstacleList, ReportList,
        CityList, PopulationList, LandmarkList, HighwayList, HighwayEntryList,
        RoadList, RailList, StationList, HydroList, TopoList, IsohypseList,
        WaypointList, DigitList, FlightList};
    inline QList<isoListEntry>* getIsohypseRegions(){return &regIsoLines;};

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
     * emitted during maploading to display a message f.e. in the
     * splash-screen of the mainwindow.
     */
    void loadingMessage(QString message);
    /** signal that a new flight/task/flight group become active */
    void currentFlightChanged();
    /** signal that a new task has been created */
    void newTaskAdded(FlightTask *);
    /** signal emitted when new task has been created */
    void taskHelp(QString);
    /**
     * Emitted, when no mapfiles are found, or the when the map-directories
     * do not exists.
     */
    void errorOnMapLoading();

    /*
      *
      * Activates the graphical planning
      */
    void activatePlanning();

    /*
    *  Map contents have changed
    */
    void contentsChanged();

  private:
    /**
     * Reads a binary map file containing airfields.
     *
     * @param  fileName  The path and name of the airfield-file.
     */
    bool __readAirfieldFile(const char* pathName);
    /**
     * Reads a binary map file containing airspaces.
     *
     * @param  fileName  The path and name of the airspace-file.
     */
    bool __readAirspaceFile(const char* pathName);
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
     * Reads a new ascii map file.
     *
     * NOTE: This function is obsolete and will be removed in further
     * releases!!!
     *
     * @param  fileName  The name of the mapfile
     *
     * @return "true", when the file has successfully been loaded
     */
    bool __readAsciiFile(const char* fileName);
    /**
     * airportList contains all airports.
     */
    QList<Airport> airportList;
    /**
     * gliderList contains all glider-sites.
     */
    QList<GliderSite> gliderList;
    /**
     * addSitesList contains all, ultra-light,
     * hang-glider-sites, free-ballon-sites, parachute-jumping-sites.
     */
    QList<SinglePoint> addSitesList;
    /**
     * outList contains all outlanding-fields.
     */
    QList<SinglePoint> outList;
    /**
     * navList contains all radio navigation facilities.
     */
    QList<RadioPoint> navList;
    /**
     * airspaceList contails all airspaces.
     */
    QList<Airspace> airspaceList;
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
     * cityList contails all cities (areas).
     */
    QList<LineElement> cityList;
    /**
     * populationList contains all villages, towns & cities (points).
     */
    QList<SinglePoint> populationList;
    /**
     * landmarkList contains all landmarks.
     */
    QList<SinglePoint> landmarkList;
    /**
     * roadList contails all roads.
     */
    QList<LineElement> roadList;
    /**
     * railList contains all railways and aerial railways.
     */
    QList<LineElement> railList;
    /**
     * stationList contains all stations.
     */
//    QList<SinglePoint> stationList;
    /**
     * hydroList contains all shorelines, rivers, lakes, ...
     */
    QList<LineElement> hydroList;
    /**
     * topoList contains all topographical objects.
     */
    QList<LineElement> topoList;
    /**
     * isohypseList contains all isohypses.
     */
    QList< QList<Isohypse> > isoList;
    /**
     * Contains list of all loaded Flight and FlightTask objects, wich are
     * both subclasses of BaseFlightElement.
     */
    QList<BaseFlightElement> flightList;
    /*
	   * This list is reset every time the current WaypointCatalog is changed.
	   */
    QList<Waypoint> wpList;
    /**
     * List of all map-section. Contains a "1" for all loaded section-files,
     * otherwise "0".
     */
    QBitArray sectionArray;
    /**
     * Array containing the evevations of all possible isohypses.
     */
    static const int isoLines[];
    /** */
    QString mapDir;

    /**
     * Depreciated. Should be deleted somtime ...
     */
    bool isFirst;


    /**
     * List of all drawn isohypses.
     */
    QList<isoListEntry> regIsoLines;

    /**
     * downloads File from www.kflog.org, optionally waits until finished (blocking operation)
     */
    void __downloadFile(QString fileName, QString destString, bool wait=false);
    DownloadList* downloadList;
};

enum AutoDownloadType {ADT_NotSet = 0, Automatic, Inhibited};

#endif
