/***********************************************************************
**
**   flight.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 Heiner Lamprecht, Florian Ehinger, Jan Max Walter Krueger
**                :  2008 Constantijn Neeteson
**                :  2011-2023 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#pragma once

#include <ctime>

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

#include "baseflightelement.h"
#include "flighttask.h"
#include "map.h"
#include "optimization.h"
#include "airspace.h"

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

/**
 * \class Flight
 *
 * \author Heiner Lamprecht, Florian Ehinger, Jan Max Walter Krueger, Constantijn Neeteson, Axel Pauli
 *
 * \brief Contains the logged flight data.
 *
 * Contains the logged flight data.
 *
 * \date 2001-2014
 *
 * \version 1.1
 */
class Flight : public BaseFlightElement
{
  public:

  class AirSpaceIntersection
    {
    public:
      AirSpaceIntersection( Airspace* AirSpace,
                            const int First,
                            const int Last,
                            Airspace::ConflictType Type=Airspace::Inside ) :
        m_AirSpace(AirSpace),
        m_TypeOfIntersection(Type),
        m_FirstPointIndexinRoute(First),
        m_LastPointIndexinRoute(Last)
      {};

      AirSpaceIntersection(const AirSpaceIntersection& other) :
        m_AirSpace(other.m_AirSpace),
        m_TypeOfIntersection(other.m_TypeOfIntersection),
        m_FirstPointIndexinRoute(other.m_FirstPointIndexinRoute),
        m_LastPointIndexinRoute(other.m_LastPointIndexinRoute)
      {};

      AirSpaceIntersection& operator = ( const Flight::AirSpaceIntersection& asi) = default;

      Airspace* AirSpace()
        { return m_AirSpace; };

      Airspace::ConflictType Type()
        { return m_TypeOfIntersection; };

      int FirstIndexPointinRoute() const
        { return m_FirstPointIndexinRoute; };

      void setFirstIndexPointinRoute( const int idx )
        { m_FirstPointIndexinRoute = idx; };

      int LastIndexPointinRoute() const
        { return m_LastPointIndexinRoute; };

      void SetLastIndexPointinRoute( const int idx )
        { m_LastPointIndexinRoute = idx; };

    protected:

      Airspace* m_AirSpace;
      Airspace::ConflictType m_TypeOfIntersection;
      int m_FirstPointIndexinRoute;
      int m_LastPointIndexinRoute;
    };

  class FlightStaticData
  {
    public:

    FlightStaticData() :
      competitionClass(-1),
      qnh(1013)
      {
      };

    QString date;
    QString pilot;
    QString copilot;
    QString gliderType;
    QString gliderRegistration;
    QString competitionClass;
    QString competitionId;
    QString frManufacturer;
    QString frRecorderId;
    QString frType;
    QString gpsDatum;
    QString gpsManufacturer;
    QString firmewareVersion;
    QString hardwareVersion;
    QString altitudePressureSensor;
    QList<Waypoint*> waypoints;

    /** The current set QNH by the FlightLoader */
    int qnh;
  };

  /**
   * Creates a new flight-object.
   * @param  fileName  The name of the igc-file
   * @param  route  The logged flight-points
   * @param  flightData  The static data of the flight
   */
  Flight( const QString& fileName,
          const QList<FlightPoint*>& route,
          const FlightStaticData& flightStaticData );
  /**
   * Destroys the flight-object.
   */
  virtual ~Flight();

  /**
   * @return The name of the pilot
   */
  QString getPilot() const
  { return m_flightStaticData.pilot; }

  /**
   * @return The name of the copilot
   */
  QString getCopilot() const
  { return m_flightStaticData.copilot; }

  /**
   * @return the type of the glider.
   */
  QString getGliderType() const
  { return m_flightStaticData.gliderType; }

  /**
   * @return the registration of the glider.
   */
  QString getGliderRegistration() const
  {
    return m_flightStaticData.gliderRegistration;
  };

  /**
   * @return the competition class of the glider.
   */
  QString getCompetitionClass() const
  {
    return m_flightStaticData.competitionClass;
  };

  /**
   * @return the competition identifier of the glider.
   */
  QString getCompetitionId() const
  {
    return m_flightStaticData.competitionId;
  };

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
  FlightTask* getTask(bool isOrig = false);

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
   * @return the start time.
   */
  time_t getStartTime() const;
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
  QList<FlightPoint*>& getRoute()
    {
      return route;
    };
  /**
   * @return the number of logged points.
   */
  int getRouteLength() const { return route.size(); };

  /**
   * Creates a string list, that contains several info about the part
   * between the two given points.
   * @return the info-string
   * @param  start  the index of the first point of the section. If the
   *                index is 0, the first point of the flight will be
   *                used.
   * @param  end  the index of the last point of the section. If the index
   *              is 0, the last point of the flight will be used.
   */
  QStringList getFlightValues(unsigned int start = 0, unsigned int end = 0);
  /**
   * Creates a list with info about every state.
   * @return the info-state list
   * @param  start  the index of the first point of the section. If the
   *                index is 0, the first point of the flight will be
   *                used.
   * @param  end  the index of the last point of the section. If the index
   *              is 0, the last point of the flight will be used.
   */
  QList<statePoint*> getFlightStates(unsigned int start = 0, unsigned int end = 0);

  /**
   * @return a list of airspace intersections found in this flight.
   */
  QList<AirSpaceIntersection>& getFlightAirSpaceIntersections()
    {
      return m_airspaceIntersections;
    };

  /**
   * @return the date of the flight.
   */
  QString getDate() const
  {
    return m_flightStaticData.date;
  };

  /**
   * Searches the point of the flight, which time is the nearest
   * to the given time.
   * @return the point
   */
  FlightPoint getPointByTime(time_t time);
  /**
   * Searches the point of the flight, which time is the nearest
   * to the given time.
   * @return the index of the point
   */
  int getPointIndexByTime(time_t time);
  /**
   * Draws the flight an the task into the given painter. Reimplemented
   * from BaseMapElement.
   *
   * \param  targetP  The painter to draw the element into.
   *
   * \return always true
   */
  virtual bool drawMapElement( QPainter* targetP );
  /** */
  virtual void printMapElement(QPainter* printP, bool isText);
  /**
   * @return the point with the index "n"
   */
  FlightPoint getPoint(int n);
  /**
         * @return The list of optimized waypoints if task is optimized
         *         otherwise the original waypoints
   */
  virtual QList<Waypoint*> getWPList();
  /**
         * @return the original list of waypoints
   */
  QList<Waypoint*> getOriginalWPList();

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
   * mouse cursor is less than 30 pixel. If no point is found, -1 is
   * returned.
   * @param  cPoint  The map-position of the mouse cursor.
   * @param  searchPoint  A pointer to a flight point. Will be filled
   *                      with the flight point found.
   * @return the index of the flight point or -1 if no point is found.
   */
  int searchPoint(const QPoint& cPoint, FlightPoint& searchPoint);
  /**
   * Get the previous FlightPoint before number 'index'
   */
  int searchGetPrevPoint(int index, FlightPoint& searchPoint);
  /**
   * Get the next FlightPoint after number 'index'
   */
  int searchGetNextPoint(int index, FlightPoint& searchPoint);
  /**
   * Get the contents of the next FlightPoint 'step' indexes after number 'index'
   */
  int searchStepNextPoint(int index, FlightPoint & fP, int step);
  /**
   * Get the contents of the previous FlightPoint 'step' indexes before number 'index'
   */
  int searchStepPrevPoint(int index,  FlightPoint & fP, int step);
  /**
   * @return "true" if the flight has been optimized.
   */
  bool isOptimized() const
  {
    return optimized;
  };

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
  QStringList getHeader();
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
  void setLastAnimationPixmap(QPixmap& newPixmap)
  {
    pixAnimate = newPixmap;
  };

  /** No descriptions */
  QPixmap& getLastAnimationPixmap()
  {
    return pixAnimate;
  };

  /** No descriptions */
  QPoint getLastAnimationPos(void);
  /** No descriptions */
  void setLastAnimationPos(QPoint pos);
  /** Sets task begin and end time */
  void setTaskByTimes(int timeBegin,int timeEnd);
  /** Re-calculates all projections for this flight. */
  void reProject();

  /**
   * Set the flight draw type. Selects the draw pen color and width.
   */
  enum MapConfig::DrawFlightPointType getDrawFlightPointType() const
  {
    return m_dfpt;
  };

  /**
   * Get the flight draw type. Selects the draw pen color and width.
   */
  void setDrawFlightPointType( const enum MapConfig::DrawFlightPointType newType )
  {
    m_dfpt = newType;
  };

  /**
   * Return values for the Min/Max Points
   */
  enum MaxPoints {V_MAX = -1, H_MAX = -2, VA_MAX=-3, VA_MIN = -4};
  /**
   *  Flight State
   */
  enum FlightState {Straight = 0, LeftTurn = 1, RightTurn = 2, MixedTurn = 3};

  /**
   * Calculate the airspace intersections of this flight.
   */
  void calAirSpaceIntersections();

  /**
   * \return The statistical data of the stored flight.
   */
  FlightStaticData& getFlightStaticData()
  {
    return m_flightStaticData;
  };

  /**
   * Returns the current used QNH.
   *
   * \returns The current used QNH.
   */
  int getQNH() const
  {
    return m_flightStaticData.qnh;
  };

  /**
   * Sets the QNH of this flight to the new value and saves it into a setting
   * file.
   */
  void setQNH( const int value )
  {
    m_flightStaticData.qnh = value;
    saveQNH();
  };

  /**
   * Tries to load a stored QNH value from a setting file.
   *
   * \returns true in case of success otherwise false. In the false case
   *          the internal QNH value is set to 1013 hPa.
   */
  bool loadQNH();

  /**
   * Saves the current QNH into a setting file.
   *
   * \returns true in case of success otherwise false.
   */
  bool saveQNH();

  bool getQNHFromUser();

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
  double __calculateOptimizePoints(FlightPoint* fp1,
                                   FlightPoint* fp2,
                                   FlightPoint* fp3);
    /** */
  void __checkMaxMin();

  /** Kreisflug?? */
  void __flightState();

  /** calculate the basic en-route information, like dT, dH, dS, dBearing and bearing */
  void __calculateBasicInformation();

  /** calculates the smallest difference of two angles */
  float __diffAngle(float firstAngle, float secondAngle);

  /** The static data of the flight. */
  FlightStaticData m_flightStaticData;

  unsigned int v_max;
  unsigned int h_max;
  unsigned int va_min;
  unsigned int va_max;

  QList<FlightPoint*> route;

  QRect bBoxFlight;
  time_t startTime;
  time_t landTime;
  unsigned int startIndex; // index to take-off point
  unsigned int landIndex;  // index to landing point
  int taskBegin; //index to begin of task
  int taskEnd;   //index to end of task

  FlightTask origTask;
  FlightTask optimizedTask;
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
  QStringList header;

  /** List with airspaces, which were intersected during the flight. */
  QList<AirSpaceIntersection> m_airspaceIntersections;

  /* The data type to be used for flight drawing. */
  enum MapConfig::DrawFlightPointType m_dfpt;
};
