/***********************************************************************
**
**   flight.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 Heiner Lamprecht, Florian Ehinger, Jan Max Walter Krueger
**                :  2008 Constantijn Neeteson
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHT_H
#define FLIGHT_H

#include <time.h>
#include "baseflightelement.h"
#include "flighttask.h"
#include "optimization.h"
#include "map.h"

#include <qdatetime.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qptrlist.h>

struct statePoint
{
  int f_state;
  time_t start_time;
  time_t end_time;
  int duration;
  float distance;
  float speed;
  float L_D;
  float circles;
  float vario;
  int dH_pos;
  int dH_neg;
};

/*
 * Contains the logged flight-data.
 */
class Flight : public BaseFlightElement
{
  public:
	  /**
	   * Creates a new flight-object.
	   * @param  fileName  The name of the igc-file
	   * @param  route  The logged flight-points
	   * @param  pName  The name of the pilot
	   * @param  gType  The type of the glider
	   * @param  gIG  The id of the glider
	   * @param  wpL  The list of waypoints of the task
	   * @param  date  The date of the flight
	   */
    Flight(const QString& fileName, const QString& recID,
  	    const QPtrList<flightPoint>& route, const QString& pName, const QString& gType,
        const QString& gID, int cClass, const QPtrList<Waypoint>& wpL, const QDate& date);
	  /**
	   * Destroys the flight-object.
	   */
  	~Flight();
	  /**
	   * @return the name of the pilot.
	   */
  	QString getPilot() const;
	  /**
	   * @return the type of the glider.
	   */
  	QString getType() const;
	  /**
	   * @return the id of the glider.
	   */
  	QString getID() const;
	  /**
	   * @param  isOrig  "true", if the original-task should be returned.
	   *                 The default is "false". If the flight has not been
	   *                 optimized, the original-task will be returned in any
	   *                 case.
	   * @return the route-type of the flight
	   */
  	QString getTaskTypeString(bool isOrig = false) const;
	  /**
	   * @param  isOrig  "true", if the original-task should be used.
	   *                 The default is "false". If the flight has not been
	   *                 optimized, the original-task will be used in any case.
	   * @return the distance between all reached waypoints
	   */
    QString getDistance(bool isOrig = false);
    /** */
    FlightTask getTask(bool isOrig = false) const;
	  /**
	   * @param  isOrig  "true", if the original-task should be used.
	   *                 The default is "false". If the flight has not been
	   *                 optimized, the original-task will be used in any case.
	   * @return the distance of the task
	   */
    QString getTaskDistance(bool isOrig = false);
    /**
	   * @param  isOrig  "true", if the original-task should be used.
	   *                 The default is "false". If the flight has not been
	   *                 optimized, the original-task will be used in any case.
	   * @return the dmst-points of the flight.
     */
    QString getPoints(bool isOrig = false);
	  /**
	   * @return the starttime.
	   */
  	time_t getStartTime() const;
	  /**
	   * @return the start-site.
	   */
    QString getStartSite() const;
    /**
     * @return the landing time.
     */
    time_t getLandTime() const;
    /**
     * @return the index to the landing point.
     */
    int getLandIndex() const;
    /**
     * @return the index to the take-off point.
     */
    int getStartIndex() const;
   /**
    * @return the route
    */
    QPtrList<flightPoint> getRoute() const;
    /**
     * @return the number of logged points.
     */
    unsigned int getRouteLength() const;
    /**
     * @return the site, where the glider has landed.
     */
    QString getLandSite() const;
    /**
     * Creates a stringlist, that contains several info about the part
     * between the two given points.
     * @return the info-string
     * @param  start  the index of the first point of the section. If the
     *                index is 0, the first point of the flight will be
     *                used.
     * @param  end  the index of the last point of the section. If the index
     *              is 0, the last point of the flight will be used.
     */
    QStrList getFlightValues(unsigned int start = 0, unsigned int end = 0);
    /**
     * Creates a list with info about every state.
     * @return the info-statelist
     * @param  start  the index of the first point of the section. If the
     *                index is 0, the first point of the flight will be
     *                used.
     * @param  end  the index of the last point of the section. If the index
     *              is 0, the last point of the flight will be used.
     */
    QPtrList<statePoint> getFlightStates(unsigned int start = 0, unsigned int end = 0);
    /**
     * @return the date of the flight.
     */
    QDate getDate() const;
    /**
     * Searches the point of the flight, which time is the nearest
     * to the given time.
     * @return the point
     */
    flightPoint getPointByTime(time_t time);
    /**
     * Searches the point of the flight, which time is the nearest
     * to the given time.
     * @return the index of the point
     */
    int getPointIndexByTime(time_t time);
    /**
     * Draws the flight an the task into the given painter. Reimplemented
     * from BaseMapElement.
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /** */
    virtual void printMapElement(QPainter* printP, bool isText);
    /**
     * @return the point with the index "n"
     */
    flightPoint getPoint(int n);
    /**
	   * @return the list of optimized waypoints if task is optimized else then orig waypoints
     */
    virtual QPtrList<Waypoint> getWPList();
    /**
	   * @return the original list of waypoints
     */
    QPtrList<Waypoint> getOriginalWPList();
    /**
     * @return the type of the task
     */
    unsigned int getTaskType() const;
    /**
     * Optimizes the task.
     * @return  "true", if the user wants to use the optimized task.
     */
    bool optimizeTask();
    /**
     * Optimizes the task for OLC.
     * @return  "true", if the user wants to use the optimized task.
     */
    bool optimizeTaskOLC(Map* map);
    /**
     * Searches the first point of the flight, which distance to the
     * mousecursor is less than 30 pixel. If no point is found, -1 is
     * returned.
     * @param  cPoint  The map-position of the mousecursor.
     * @param  searchPoint  A pointer to a flightpoint. Will be filled
     *                      with the flightpoint found.
     * @return the index of the flightpoint or -1 if no point is found.
     */
    int searchPoint(const QPoint& cPoint, flightPoint& searchPoint);
    /**
		 * Get the previous FlightPoint before number 'index'
		 */
    int searchGetPrevPoint(int index, flightPoint& searchPoint);
    /**
		 * Get the next FlightPoint after number 'index'
		 */
    int searchGetNextPoint(int index, flightPoint& searchPoint);
  	/**
  	 * Get the contents of the next FlightPoint 'step' indexes after number 'index'
  	 */
  	int searchStepNextPoint(int index, flightPoint & fP, int step);
  	/**
  	 * Get the contents of the previous FlightPoint 'step' indexes before number 'index'
  	 */
  	int searchStepPrevPoint(int index,  flightPoint & fP, int step);
    /**
     * @return "true" if the flight has been optimized.
     */
    bool isOptimized() const;
    /**
     * @return the bounding-box of the flight.
     */
    QRect getFlightRect() const;
    /**
     * @return the bounding-box of the task.
     */
    QRect getTaskRect() const;
    /**
     * @return the header-info of the igc-file (date, pilot-name, ...)
     */
    QStrList getHeader();
    /** @return the competition-class */
    int getCompetitionClass() const;
    /**
		 * Increments the nAnimationIndex member
     */
    void setAnimationNextIndex(void);
    /** sets the bAnimationActive flag */
    void setAnimationActive(bool b);
    /**
		 * Sets the nAnimationIndex member to 'n'
		 */
    void setAnimationIndex(int n);
    /** returns the bAnimationActive flag */
    bool isAnimationActive(void);
    /** No descriptions */
    int getAnimationIndex();
    /** No descriptions */
    void setLastAnimationPixmap(QPixmap pix);
    /** No descriptions */
    QPixmap getLastAnimationPixmap(void);
    /** No descriptions */
    QPoint getLastAnimationPos(void);
    /** No descriptions */
    void setLastAnimationPos(QPoint pos);
    /** Sets task begin and end time */
    void setTaskByTimes(int timeBegin,int timeEnd);
    /** Re-calculates all projections for this flight. */
    void reProject();
    /**
     * The flight-types.
     */
//    enum FlightType {ZielS = 1, ZielR = 2, FAI = 3, Dreieck = 4, FAI_S = 5,
//                   Dreieck_S = 6, Abgebrochen = 7, Unknown = 8, FAI_2 = 9,
//                   FAI_S2 = 10, FAI_3 = 11, FAI_S3 = 12, Vieleck = 13};
    /**
     * Return values for the Min/Max Points
     */
    enum MaxPoints {V_MAX = -1, H_MAX = -2, VA_MAX=-3, VA_MIN = -4};
    /**
     *  Flight State
     */
    enum FlightState {Straight = 0, LeftTurn = 1, RightTurn = 2, MixedTurn = 3};
    /**
     * "Unknown" is used, when there is an unrecognized competitionclass in
     * the igc-file. If there is no class given, "NotSet" is used.
     */
    enum CompetitionClass {Unknown = -1, NotSet = 0,  PW5 = 1, Club = 2,
        Standard = 3, FifteenMeter = 4, EightteenMeter = 5, DoubleSitter = 6,
        OpenClass = 7, HGFlexWing = 8, HGRigidWing = 9, ParaGlider = 10,
        ParaOpen = 11, ParaSport = 12, ParaTandem = 13};

  private:
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
    double __calculateOptimizePoints(flightPoint* fp1,
        flightPoint* fp2, flightPoint* fp3);
	  /** */
  	void __checkMaxMin();
  	/** */
  	virtual bool __isVisible() const;
  	/** Kreisflug?? */
  	void __flightState();
    /** calculate the basic en-route information, like dT, dH, dS, dBearing and bearing */
    void __calculateBasicInformation();
    /** calculates the smallest difference of two angles */
    float __diffAngle(float firstAngle, float secondAngle);

    QString recorderID;
    QString pilotName;
    QString gliderType;
    QString gliderID;
    QDate date;
    int competitionClass;

    flightPoint* drawRoute;
    unsigned int drawLength;
    unsigned int v_max;
    unsigned int h_max;
    unsigned int va_min;
    unsigned int va_max;

    QPtrList<flightPoint> route;

    QRect bBoxFlight;
    time_t startTime;
    time_t landTime;
    unsigned int startIndex; // index to take-off point
    unsigned int landIndex;  // index to landing point
    int taskBegin; //index to begin of task
    int taskEnd;   //index to end of task

    FlightTask origTask;
    FlightTask optimizedTask;
//    Optimization *optim;
    bool optimized;

    /**
		 * Index into flight used for animation
		 */
    int nAnimationIndex;
    /**  */
    bool bAnimationActive;
    bool taskTimesSet;
    QPoint preAnimationPos;
    QPixmap pixAnimate;
    /** */
    QStrList header;
};

#endif
