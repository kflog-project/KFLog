/***********************************************************************
 **
 **   baseflightelement.h
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef BASE_FLIGHT_ELEMENT_H
#define BASE_FLIGHT_ELEMENT_H

#include <QList>
#include <QString>
#include <QPainter>

#include "basemapelement.h"
#include "waypoint.h"

/**
 * @author Harald Maier
 *
 * @version $Id$
 */
class BaseFlightElement : public BaseMapElement
{
public:

  BaseFlightElement( const QString& name,
                     const BaseMapElement::objectType type,
                     const QString& fName );

  virtual ~BaseFlightElement();

  virtual QList<Waypoint*> getWPList() = 0;

  virtual QString& getFileName()
    {
      return sourceFileName;
    };

  virtual void setFileName( QString fn )
    {
      sourceFileName = fn;
    };

  /**
   * Searches the first point of the flight, which distance to the
   * mouse cursor is less than 30 pixel. If no point is found, -1 is
   * returned.
   * @param  cPoint  The map-position of the mouse cursor.
   * @param  searchPoint  A pointer to a flight point. Will be filled
   *                      with the flight point found.
   * @return the index of the flight point or -1 if no point is found.
   */
  virtual int searchPoint(const QPoint& cPoint, FlightPoint& searchPoint);
  /**
   * Get the previous FlightPoint before number 'index'
   */
  virtual int searchGetPrevPoint(int index, FlightPoint& searchPoint);
  /**
   * Get the next FlightPoint after number 'index'
   */
  virtual int searchGetNextPoint(int index, FlightPoint& searchPoint);
  /**
   * Get the contents of the next FlightPoint 'step' indexes after number 'index'
   */
  virtual int searchStepNextPoint(int index, FlightPoint & fP, int step);
  /**
   * Get the contents of the previous FlightPoint 'step' indexes before
   * number 'index'
   */
  virtual int searchStepPrevPoint(int index,  FlightPoint & fP, int step);
  /**
   * Virtual function for printing the element.
   *
   * The function must be implemented in the child-classes.
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some map elements should
   *                 be printed.
   */
  virtual void printMapElement(QPainter* printP, bool isText) = 0;
  /**
   * Virtual function to trigger re-projecting of the coordinates of the object.
   */
  virtual void reProject() = 0;

protected:
  /** */
  QString sourceFileName;
};

#endif
