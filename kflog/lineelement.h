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

/*
 * PEN_THICKNESS füllt den Array mit den Strichstärken ...
 */
#define PEN_THICKNESS(a, b, c, d, e, f, g, h, i) \
    drawThickness[0] = a; drawThickness[1] = b; drawThickness[2] = c; \
    drawThickness[3] = d; drawThickness[4] = e; drawThickness[5] = f; \
    drawThickness[6] = g; drawThickness[7] = h; drawThickness[8] = i;

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
    virtual QRegion* drawRegion(QPainter* targetPainter, QPainter* maskPainter);
    /**
     * Draws the element into the given painter.
     */
    virtual void drawMapElement(QPainter* targetPainter, QPainter* maskPainter,
        bool isFirst = true);
    /** */
    virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);
    /** */
    virtual bool isValley() const;

  protected:
    /** */
    virtual bool __isVisible() const;
    /** */
    QColor drawColor;
    /** */
    Qt::PenStyle drawPenStyle;
    /** */
    QColor fillColor;
    /** */
    QBrush fillBrush;
    /** */
    int* drawThickness;
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

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const LineElement& element);
//QTextStream& operator<<(QTextStream& outStream, const LineElement& element);
//QDataStream& operator>>(QDataStream& outStream, LineElement& element);

#endif
