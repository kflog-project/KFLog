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

#include <lineelement.h>

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
    Isohypse(QPointArray pA, unsigned int elev, bool isValley);
    /**
     * Destructor
     */
    ~Isohypse();
    /**
     * Draws the element into the given painter. Reimplemented from
     * BaseMapElement.
     *
     * @param  targetP  The painter to draw the element into.
     * @param  maskP  The maskpainter for targetP
     */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /**
     * @return the elevation of the line
     */
    int getElevation() const;

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
