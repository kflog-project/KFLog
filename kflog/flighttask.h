/***********************************************************************
 **
 **   flighttask.h
 **
 **   This file is part of KFLog2.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef FLIGHTTASK_H
#define FLIGHTTASK_H

#include "baseflightelement.h"
#include "lineelement.h"

#include <qlist.h>
#include <qrect.h>

struct faiRange {
  double minLength28;
  double maxLength28;
  double minLength25;
  double maxLength25;
};

struct faiAreaSector {
  /* total distance with route and this sector */
  double dist;
  /* pos on map */
  LineElement *pos;
};

/**
 * Contains the data of a task.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class FlightTask : public BaseFlightElement
{
 public:
  /**
   * Creates an empty task and sets isOrig to false.
   */
  FlightTask(QString fName);
  /**
   * Creates a task with the given points.
   *
   * @param  wpList  the list of waypoints.
   * @param  isOrig  true, if the task is the original task
   *                 of a flight.
   */
  FlightTask(QList<Waypoint> wpList, bool isOrig, QString fName);
  /**
   */
  ~FlightTask();
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
   * Returns the waypointlist.
   */
  QList<Waypoint> getWPList() { return wpList; };
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
   * @param  maskP  The maskpainter for targetP
   */
  void drawMapElement(QPainter* targetPainter, QPainter* maskPainter);
  /** */
  void printMapElement(QPainter* targetP, bool isText);
  void printMapElement(QPainter* targetP, bool isText, double dX, double dY);
  /** */
  void checkWaypoints(QList<flightPoint> route,
                      QString gliderType);
  /** */
  double getOlcPoints();
  /** */
  int getPlannedPoints() ;
  /** */
  QString getTotalDistanceString() ;
  /** */
  double getAverageSpeed();
    /** */
  QString getTaskDistanceString() ;
  /** calc min and max distance for FAI triangles*/
  QString getFAIDistanceString() ;
  /** */
  QString getPointsString() ;
  /** */
  QRect getRect() const;
  /** */
  struct faiRange getFAIDistance(double leg);
  void setOptimizedTask(double points, double distance);
  void setWaypointList(QList<Waypoint> wpL);
  /** No descriptions */
  void setPlanningType(int type);
  int getPlanningType() { return __planningType; };
  void setPlanningDirection(int dir);
  int getPlanningDirection() { return __planningDirection; };
  /** set new task name */
  void setTaskName(QString fName);
  /** No descriptions */
  QString getPlanningTypeString();
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
                   FAI_S2 = 10, FAI_3 = 11, FAI_S3 = 12, Vieleck = 13, OLC2003 = 14};
  /*
   * The task-types
   */
  enum TaskType {Route, FAIArea, AAT};
  /*
   * The directions FAIAreas
   */
  enum AreaDirection {leftOfRoute = 1, rightOfRoute = 2};
 private:
  /**
   * Checkes the type of the task.
   */
  void __checkType();
  /**
   * Calculates the sector.
   */
  double __sectorangle(unsigned int loop, bool isDraw);
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
                     double toLat, double toLon, QPointArray *pA, bool upwards, bool isRightOfRoute);
  /**
   * calculate side sectors for valid FAI Areas
   */    
  void calcFAISectorSide(double leg, double legBearing, double from, double to, double step, double toLat,
                         double toLon, bool less500, QPointArray *pA, bool upwards, bool isRightOfRoute);
  /**
   * true, if the task is the original task of a flight as read from
   * the igc-file.
   */
  bool isOrig;
  /** */
  QList<Waypoint> wpList;
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
  QList<faiAreaSector> FAISectList;
  /* direction of area planning */
  int __planningDirection;
  /* Route of flight */
  QList<flightPoint> flightRoute;
};

#endif
