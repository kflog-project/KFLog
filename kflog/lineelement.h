/***********************************************************************
**
**   lineelement.h
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

#ifndef LINEELEMENT_H
#define LINEELEMENT_H

#include <basemapelement.h>

/**
 * Class used for all elements, which consist of a pointarray.
 *
 * @see BaseMapElement#objectType
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class LineElement : public BaseMapElement
{
  public:
    /**
     * Creates a new mapelement.
     */
    LineElement(QString n, unsigned int t, QPointArray pA, bool isVal = false);
    /**
     * Destructor, does nothing special.
     */
    ~LineElement();
    /**
     * Draws the element into the given painter.
     */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /** */
    virtual void printMapElement(QPainter* printPainter) const;
    /** */
    virtual bool isValley() const;

  protected:
    /** */
    virtual bool __isVisible() const;
    /**
     * Contains the projected positions of the item.
     */
    QPointArray projPointArray;
    /** */
    QRect bBox;
    /** */
    bool valley;
    /** */
    bool closed;
};

#endif
