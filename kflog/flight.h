/***********************************************************************
**
**   flight.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHT_H
#define FLIGHT_H

#include <mapcontents.h>
#include <flighttask.h>
#include <basemapelement.h>
#include <wp.h>

#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qstrlist.h>

/**
 * Contains the logged flight-data.
 *
 * @author Heiner Lamprecht, Florian Ehinger, Jan Max Walter Krueger
 * @version $Id$
 */
class Flight : public BaseMapElement
{
  public:
	  /**
	   * Creates a new flight-object.
	   */
    Flight(QString fileName,
  	    QList<flightPoint> route, QString pName, QString gType,
        QString gID, QList<struct wayPoint> wpL, QString d);
	  /**
	   * Destroys the flight-object.
	   */
  	~Flight();
	  /**
	   * Returns the name of the pilot.
	   */
  	QString getPilot() const;
	  /**
	   * Returns the type of the glider.
	   */
  	QString getType() const;
	  /**
	   * Returns the id of the glider.
	   */
  	QString getID() const;
	  /**
	   * Returns the route-type of the flight
	   */
  	QString getRouteType(bool isOrig = false) const;
	  /**
	   * Returns the distance between all reached waypoints
	   */
    QString getDistance(bool isOrig = false) const;
	  /**
	   * Returns the distance of the task
	   */
    QString getTaskDistance(bool isOrig = false) const;
    /**
     * Returns the dmst-points of the flight.
     */
    QString getPoints(bool isOrig = false) const;
	  /**
	   * Returns the starttime.
	   */
  	int getStartTime() const;
	  /**
	   * Returns the start-site.
	   */
    QString getStartSite() const;
    /**
     * Returns the landing time.
     */
    int getLandTime() const;
    /** */
    unsigned int getRouteLength() const;
    /**
     * Returns the site, where the glider has landed.
     */
    QString getLandSite() const;
    /**
      * Gibt das mittlere Steigen zwischen Zwei Punkten zurück
      */
    QStrList getFlightValues(unsigned int start = 0, unsigned int end = 0);
    /**
     *
     */
    QString getDate() const;
    /**
     * Returns the Point with the next time
     */
    struct flightPoint getPointByTime(int time);
    int getPointByTime_i(int time);
    /**
     * Draws the element into the given painter.
     */
    virtual void drawMapElement(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    struct flightPoint getPoint(int n);
    /**
     * Contains the list of waypoints
     */
    QList<struct wayPoint>* getWPList(bool isOrig = false);
    /** */
    const char* getFileName() const;
    /** */
    unsigned int getTaskType() const;
    /** */
    bool optimizeTask();
    /** */
    int searchPoint(QPoint cPoint, struct flightPoint& searchPoint);
    /** */
    bool isOptimized() const;
    /** */
    QRect getFlightRect() const;
    /** */
    QRect getTaskRect() const;
    /** */
    QStrList getHeader();
    /**
     * The waypoint-types.
     */
    enum WaypointType {NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                     End = 8, FreeP = 16, Landing = 32};
    /**
     * The flight-types.
     */
    enum FlightType {ZielS = 1, ZielR = 2, FAI = 3, Dreieck = 4, FAI_S = 5,
                   Dreieck_S = 6, Abgebrochen = 7, Unknown = 8, FAI_2 = 9,
                   FAI_S2 = 10, FAI_3 = 11, FAI_S3 = 12, Vieleck = 13};
    /**
     * Return Values for the Min/Max Points
     */
    enum MaxPoints {V_MAX = -1, H_MAX = -2, VA_MAX=-3, VA_MIN = -4};

    /**
      *  Flight State
      */
    enum FlightState {Strecke = 0, Links = 1, Rechts = 2, Vermischt = 3};

  private:
    /**
      *  Setzt den Status der WendePunkte
      *
      */
    void __setWaypointType();

    /**
     * Prueft, ob Dreieck FAI ist.
     */
    bool __isFAI(double distance_wp, double dist1, double dist2, double dist3);
    /**
     * Calculates the sector.
     */
    double __sectorangle(int n, bool isDraw = true);
    /** */
    unsigned int __calculateBestTask(unsigned int start[], unsigned int stop[],
        unsigned int step, unsigned int idList[],
        double taskValue[], bool isTotal);
    /** */
    void __moveOptimizePoint(unsigned int idList[], double taskValue[],
        unsigned int id);
    /** */
    void __setOptimizeRange(unsigned int start[], unsigned int stop[],
        unsigned int idList[], unsigned int id, unsigned int step);
    /** */
    double __calculateOptimizePoints(struct flightPoint* fp1,
        struct flightPoint* fp2, struct flightPoint* fp3);
    /**
     * Findet die Art der Strecke heraus
     */
    void __checkType();
    /** */
  	void __checkWaypoints();
	  /** */
  	void __checkMaxMin();
  	/** */
  	virtual bool __isVisible() const;
  	/** */
  	double __polar(double x, double y);
  	/** Kreisflug?? */
  	void __flightState();

	  QString pilotName;
    QString gliderType;
    QString gliderID;
    QString startSite;
    QString landSite;

    QString date;

    QList<struct wayPoint> wpList;
    QList<struct wayPoint> origList;

    double distance_tot;
    double distance_wp;

    unsigned int flightType;
    unsigned int origType;
    double origDistanceWP;
    double origDistanceTot;
    double origPoints;

    double taskPoints;
    QString sourceFileName;
    struct flightPoint* drawRoute;
    unsigned int drawLength;
    unsigned int v_max;
    unsigned int h_max;
    unsigned int va_min;
    unsigned int va_max;

    QList<flightPoint> route;
    FlightTask origTask;

    QRect bBoxFlight;
    QRect bBoxTask;
    int landTime;
    int startTime;
};

#endif
