/***********************************************************************
**
**   baseflightelement.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef BASEFLIGHTELEMENT_H
#define BASEFLIGHTELEMENT_H

#include <qwidget.h>

#include "basemapelement.h"
#include "wp.h"

/**
 * @author Harald Maier
 * @version $Id$
 */
class BaseFlightElement : public BaseMapElement
{
  public:
    /** */
    BaseFlightElement(QString name, unsigned int typeID, QString fName);
    /** */
    ~BaseFlightElement();
    /** No descriptions */
//    virtual QString getFlightInfoString() = 0;
    virtual QList<wayPoint> getWPList() = 0;
    QString getFileName() { return sourceFileName; }
    /**
     * Searches the first point of the flight, which distance to the
     * mousecursor is less than 30 pixel. If no point is found, -1 is
     * returned.
     * @param  cPoint  The map-position of the mousecursor.
     * @param  searchPoint  A pointer to a flightpoint. Will be filled
     *                      with the flightpoint found.
     * @return the index of the flightpoint or -1 if no point is found.
     */
    virtual int searchPoint(QPoint cPoint, flightPoint& searchPoint);
    /**
  	 * Get the previous FlightPoint before number 'index'
	   */
    virtual int searchGetPrevPoint(int index, flightPoint& searchPoint);
    /**
  	 * Get the next FlightPoint after number 'index'
	   */
    virtual int searchGetNextPoint(int index, flightPoint& searchPoint);
	  /**
  	 * Get the contents of the next FlightPoint 'step' indexes after number 'index'
	   */
  	virtual int searchStepNextPoint(int index, flightPoint & fP, int step);
	  /**
     * Get the contents of the previous FlightPoint 'step' indexes before
     * number 'index'
	   */
    virtual int searchStepPrevPoint(int index,  flightPoint & fP, int step);
    /**
     * Virtual function for printing the element.
     *
     * The function must be implemented in the child-classes.
     * @param  printP  The painter to draw the element into.
     *
     * @param  isText  Shows, if the text of some mapelements should
     *                 be printed.
     */
    virtual void printMapElement(QPainter* printP, bool isText);

  protected:
    /** */
    QString sourceFileName;
};

#endif
