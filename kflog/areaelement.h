/***********************************************************************
**
**   areaelement.h
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

#ifndef AREAELEMENT_H
#define AREAELEMENT_H

#include <lineelement.h>

/**
  * This class is used for mapelements which effect an area. The object
  * can be one of: [Big, Mid, Small]Lake, [Huge, Big, Mid, Small]City,
  * Glacier. Mainly, the class reimplements the drawing-function, so that
  * the object will create a QRegion.
  *
  * @see BaseMapElement#objectType
  *
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class AreaElement : public LineElement
{
  public:
    /**
     * Creates a new AreaElement.
     */
//    AreaElement(QString n, unsigned int t, unsigned int length,
//              int* latList, int* lonList, struct elementBorder elB);
    /**
     * Creates a new AreaElement.
     */
    AreaElement(QString n, unsigned int t);
    /**
     * Destructor, does nothing special.
     */
    ~AreaElement();
    /**
     * Draws the element into the given painter.
     */
    virtual QRegion* drawRegion(QPainter* targetPainter, const double dX,
        const double dY, const int mapCenterLon,
        const struct elementBorder mapBorder);
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);

  protected:
    /** */
    QPen drawPen;
    /** */
    QBrush fillBrush;
};

#endif
