/***********************************************************************
**
**   elevpoint.h
**
**   This file is part of KFLog.
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

#ifndef ELEVPOINT_H
#define ELEVPOINT_H

#include <singlepoint.h>

/**
 * This class is used for all mapelements, that have an elevation-value.
 * The object can be one of: Outlanding, Obstacle, LightObstacle,
 * ObstacleGroup, LightObstacleGroup and Spot.
 *
 * @see BaseMapElement#objectType
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class ElevPoint : public SinglePoint
{
  public:
    /**
     * Creates a new elevpoint.
     * @param  name  The name
     * @param  gps  The abbreviation, used for the gps-logger
     * @param  pos  The position
     * @param  elevation The elevation
     */
    ElevPoint(QString name, QString gps, unsigned int typdID, QPoint pos,
        unsigned int elevation);
    /**
     * Destructor
     */
    ~ElevPoint();
    /**
     * Draws the element into the given painter. Reimplemented from
     * BaseMapElement.
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /**
     * Prints the element. Reimplemented from BaseMapElement.
     * @param  printP  The painter to draw the element into.
     */
    virtual void printMapElement(QPainter* printP);
    /**
     * @return the elevation of the element.
     */
    unsigned int getElevation() const;

  protected:
    /**
     * The elevation.
     */
    unsigned int elevation;
};

#endif
