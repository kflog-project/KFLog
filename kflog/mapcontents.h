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
#include <qstrlist.h>

#include <basemapelement.h>

class Airport;
class Airspace;
class ElevPoint;
class Flight;
class GliderSite;
class Isohypse;
class LineElement;
class RadioPoint;
class SinglePoint;

/**
 * This class provides functions for accessing the contents of the map.
 * It takes control over loading all needed map-files.
 * The class contains several QLists holding the mapelements.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class MapContents
{
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
     */
    void printContents(QPainter* targetP);
    /**
     * Loads a new flight-file.
     *
     * @param  file  The path to the igc-file
     * @return "true", when the file has successfully been loaded
     */
    bool loadFlight(QFile);
    /**
     * @return a pointer to the current flight
     */
    Flight* getFlight();
    /**
     * @returns the flightList
     */
    QList<Flight>* getFlightList();
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
    int searchFlightPoint(QPoint cPos, struct flightPoint& fP);
    /**
     * Closes all loaded flights
     */
    void closeFlight();
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
    /**
     * The listid.
     */
    enum listID {NotSet = 0, AirportList, GliderList,
        AddSitesList, OutList, NavList, AirspaceList, ObstacleList, ReportList,
        CityList, VillageList, LandmarkList, HighwayList, HighwayEntryList,
        RoadList, RailList, StationList, HydroList, TopoList, IsohypseList,
        WaypointList, DigitList, FlightList};

  private:
    /**
     * Reads a binary map file containing airfields.
     *
     * @param  fileName  The path and name of the airfield-file.
     */
    bool __readAirfieldFile(const char* fileName);
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
    QList<ElevPoint> outList;
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
    QList<ElevPoint> obstacleList;
    /**
     * reportList contains all reporting points.
     */
    QList<SinglePoint> reportList;
    /**
     * cityList contails all cities.
     */
    QList<LineElement> cityList;
    /**
     * villageList contains all villages.
     */
//    QList<SinglePoint> villageList;
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
    QList<SinglePoint> stationList;
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
    /** */
    QList<Flight> flightList;
    /**
     * List of all map-section. Contains a "1" for all loaded section-files,
     * otherwise "0".
     */
    QBitArray sectionArray;
    /**
     * Array containing the evevations of all possible isohypses.
     */
    int* isoLines;
    /** Should be deleted somtime ... */
    bool isFirst;
};

#endif
