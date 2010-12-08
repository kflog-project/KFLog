/***********************************************************************
**
**   isohypse.h
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

#ifndef ISOHYPSE_H
#define ISOHYPSE_H

#include <QRegion>

#include "lineelement.h"

/**
 * This class is used for isohypses.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 */
class Isohypse : public LineElement
{
  public:
    /**
     * Creates a new isohypse.
     *
     * @param  pA  The pointarray containing the positions.
     * @param  elev  The elevation
     * @param  isValles "true", if the area is a valley
     */
    Isohypse(Q3PointArray pA, unsigned int elev, bool isValley);
    /**
     * Destructor
     */
    ~Isohypse();
    /**
     * Draws the element into the given painter. 
     *
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     * @return the region the isohypse covers
     */
    virtual QRegion* drawRegion(QPainter* targetP, QPainter* maskP);
    /**
     * @return the elevation of the line
     */
    int getElevation() const;
    /**
     * @return the region for this line (projected)
     */
    QRegion* getRegion();
    /**
     * Is this isoline's region allready stored in our list?
     */
    bool regionStored;

  private:
    /**
     * The elevation
     */
    int elevation;
    /**
     * "true", if element is a valley.
     */
    bool valley;
};

#endif
