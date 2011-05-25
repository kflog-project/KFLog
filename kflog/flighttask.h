/***********************************************************************
 **
 **   flighttask.h
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
 **                   2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef FLIGHT_TASK_H
#define FLIGHT_TASK_H

#include "baseflightelement.h"
#include "lineelement.h"

#include <QHash>
#include <QList>
#include <QRect>
#include <QPolygon>

struct faiRange
{
  double minLength28;
  double maxLength28;
  double minLength25;
  double maxLength25;
};

struct faiAreaSector
{
  /* total distance with route and this sector */
  double dist;
  /* pos on map */
  LineElement *pos;
};

/**
 * \class FlightTask
 *
 * \brief Contains and manages the data of a task.
 *
 * Contains and manages the data of a task.
 *
 * \author Heiner Lamprecht
 *
 * \version $Id$
 *
 * \date 2001-2011
 */
class FlightTask : public BaseFlightElement
{
 public:
  /**
   * Creates an empty task and sets isOrig to false.
   */
  FlightTask(const QString& fName);
  /**
   * Creates a task with the given points.
   *
   * @param  wpList  the list of waypoints.
   * @param  isOrig  true, if the task is the original task
   *                 of a flight.
   */
  FlightTask(const QList<Waypoint*>& wpList, bool isOrig, const QString& fName);
  /**
   */
  virtual ~FlightTask();
  /**
   * Returns true, if a triangle represented by the four length,
   * is a valid FAI-triangle.
   *
   * A triangle is a valig FAI-triangle, if no side is less than
   * 28% of the total length (totallength less than 500 km), or no
   * side is less than 25% or larger than 45% of the total length
   * (totallength >= 500km).
   *
   * @param  d_wp  totallength
   * @param  d1  first side
   * @param  d2  second side
   * @param  d3  third side
   */
  static bool isFAI(double d_wp, double d1, double d2, double d3);
  /**
   * Returns the waypoint list.
   */
  QList<Waypoint*> getWPList() { return wpList; };
  /**
   * Returns the type of the task.
   * @see #TaskType
   */
  int getTaskType() const;
  /** */
  QString getTaskTypeString() const;
  /**
   * Draws the flight an the task into the given painter. Reimplemented
   * from BaseMapElement.
   * @param  targetP  The painter to draw the element into.
   */
  bool drawMapElement( QPainter* targetPainter );
  /** */
  void printMapElement(QPainter* targetP, bool isText);
  void printMapElement(QPainter* targetP, bool isText, double dX, double dY);
  /** */
  void checkWaypoints(QList<FlightPoint*> route, const QString& gliderType);
  /** */
  double getOlcPoints();
  /** */
  int getPlannedPoints() ;
  /** */
  QString getTotalDistanceString() ;
  /** */
  double getAverageSpeed();
    /** */
  QString getTaskDistanceString();
  /** calc min and max distance for FAI triangles*/
  QString getFAIDistanceString() ;
  /** */
  QString getPointsString() ;
  /** */
  QRect getRect() const;
  /** */
  struct faiRange getFAIDistance(double leg);
  void setOptimizedTask(double points, double distance);
  void setWaypointList(const QList<Waypoint*>& wpL);
  /** No descriptions */
  void setPlanningType(int type);
  int getPlanningType() const { return __planningType; };
  void setPlanningDirection(int dir);
  int getPlanningDirection() const { return __planningDirection; };
  /** set new task name */
  void setTaskName(const QString& fName);
  /** No descriptions */
  QString getPlanningTypeString();
  /**
   * The taskpoint-types.
   */
  enum TaskPointType {NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                      End = 8, FreeP = 16, Landing = 32};
  /**
   * The flight-types.
   */
  enum FlightType {ZielS = 1, ZielR = 2, FAI = 3, Dreieck = 4, FAI_S = 5,
                   Dreieck_S = 6, Abgebrochen = 7, Unknown = 8, FAI_2 = 9,
                   FAI_S2 = 10, FAI_3 = 11, FAI_S3 = 12, Vieleck = 13, OLC2003 = 14};
  /*
   * The task-types
   */
  enum TaskType {Route, FAIArea, AAT};
  /*
   * The directions FAIAreas
   */
  enum AreaDirection {leftOfRoute = 1, rightOfRoute = 2};

  /**
   * Get translation string for task type.
   */
  static QString ttItem2Text( const int item, QString defaultValue=QString("") );

  /**
   * Get task type for translation string.
   */
  static int ttText2Item( const QString& text );

  /**
   * Get sorted translations of task types
   */
  static QStringList& ttGetSortedTranslationList();

 public slots:

   /**
    * Reprojects the points along the route to make sure the route is drawn
    * correctly if the projection has been changed.
    */
   void reProject();

 private:
  /**
   * Checkes the type of the task.
   */
  void __checkType();
  /**
   * Calculates the sector.
   */
  double __sectorangle(int loop, bool isDraw);
  /**
   * Proofes the type of the task and sets the status of the waypoints.
   */
  void __setWaypointType();
  /**
   * Set the DMST_Points
   */
  void __setDMSTPoints();
  /**
   * calculate an area for all FAI triangle depending on 2 points
   */
  void calcFAIArea();
  /**
   * calculate sectors for valid FAI Areas
   */
  void calcFAISector(double leg, double legBearing, double from, double to, double step, double dist,
                     double toLat, double toLon, QPolygon *pA, bool upwards, bool isRightOfRoute);
  /**
   * calculate side sectors for valid FAI Areas
   */
  void calcFAISectorSide(double leg, double legBearing, double from, double to, double step, double toLat,
                         double toLon, bool less500, QPolygon *pA, bool upwards, bool isRightOfRoute);
  /**
   * true, if the task is the original task of a flight as read from
   * the igc-file.
   */
  bool isOrig;
  /** */
  QList<Waypoint*> wpList;
  /** */
//  unsigned int task_end;
  /** */
//  unsigned int task_begin;
  /** */
  QRect bBoxTask;
  /** */
  double olcPoints;
  /** */
  double taskPoints;
  /** */
  unsigned int flightType;
  /** Gesamte Länge*/
  double distance_total;
  /** WertungsDistanz für DMST*/
  double distance_wert;
  /** Aufgaben Länge*/
  double distance_task;
  int __planningType;
  QList<faiAreaSector*> FAISectList;
  /* direction of area planning */
  int __planningDirection;
  /* Route of flight */
  QList<FlightPoint*> flightRoute;

  /**
   * Static pointer to TaskType translations
   */
  static QHash<int, QString> taskTypeTranslations;
  static QStringList sortedTaskTypeTranslations;

  /**
   * Static method for loading of object translations
   */
  static void loadTaskTypeTranslations();
};

#endif
