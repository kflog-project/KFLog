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
 * This class is used for all mapelements, that have a elevation-value.
 * The object can be one of: Outlanding, Obstacle, LightObstacle,
 * ObstacleGroup, LightObstacleGroup, Spot, Pass.
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
     */
    ElevPoint(QString n, QString abbr, unsigned int t, QPoint pos,
        unsigned int e, bool wP = false);
    /**
     * Destructor, does nothing special.
     */
    ~ElevPoint();
    /**
     * Draws the element into the given painter.
     */
    virtual void drawMapElement(QPainter* targetPainter, QPainter* maskPainter);
    /** */
    virtual void printMapElement(QPainter* printPainter);
    /**
     * Returns the elevation of the element.
     */
    unsigned int getElevation() const;
    /**
     */
    void setValues(int elevation, QPoint pos);

  protected:
    /**
     * Contains the elevation.
     */
    unsigned int elevation;
};

#endif
