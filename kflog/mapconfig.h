/***********************************************************************
**
**   mapconfig.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCONFIG_H
#define MAPCONFIG_H

#include <kconfig.h>
#include <qbrush.h>
#include <qlist.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qwidget.h>

/**
 * This class takes care of the configuration-data for displaying
 * and printing map-elements.
 *
 * @author Heiner Lamprecht
 */
class MapConfig
{
  public:
    /** */
    MapConfig(KConfig* config);
    /** */
    ~MapConfig();
    /**
     * Forces MapConfig to read the configdata.
     */
    void readConfig();
    /** */
    bool isBorder(unsigned int type);
    /**
     * Returns the pen for drawing a mapelement.
     */
    QPen getDrawPen(unsigned int typeID);
    /**
     * Returns the pen for printing a mapelement.
     */
    QPen getPrintPen(unsigned int typeID);
    /**
     * Returns the brush for drawing an areaelement.
     */
    QBrush getDrawBrush(unsigned int typeID);
    /**
     * Returns the brush for printing an areaelement.
     */
    QBrush getPrintBrush(unsigned int typeID);
    /**
     * Returns the icon-pixmap of the element.
     */
    QPixmap getPixmap(unsigned int typeID, bool isWinch = true);
    /**
     * Sets the scaleindex an the flag for small icons. Called from
     * MapMatrix.
     *
     * @see MapMatrix#scaleAdd
     */
    void setMatrixValues(int index, bool isSwitch);
    /** */
    enum ListType {RoadList = 0, HighwayList, RailwayList, RiverList,
        CityList};

  private:
    KConfig* config;
    /**
     */
    QList<QPen> airCPenList;
    QList<QBrush> airCBrushList;
    QList<QPen> airDPenList;
    QList<QBrush> airDBrushList;
    QList<QPen> airElPenList;
    QList<QBrush> airElBrushList;
    QList<QPen> airEhPenList;
    QList<QBrush> airEhBrushList;
    QList<QPen> airFPenList;
    QList<QBrush> airFBrushList;
    QList<QPen> ctrCPenList;
    QList<QBrush> ctrCBrushList;
    QList<QPen> ctrDPenList;
    QList<QBrush> ctrDBrushList;
    QList<QPen> lowFPenList;
    QList<QBrush> lowFBrushList;
    QList<QPen> dangerPenList;
    QList<QBrush> dangerBrushList;
    QList<QPen> restrPenList;
    QList<QBrush> restrBrushList;
    QList<QPen> tmzPenList;
    QList<QBrush> tmzBrushList;

    QList<QPen> roadPenList;
    QList<QPen> railPenList;
    QList<QPen> riverPenList;
    QList<QPen> highwayPenList;
    QList<QPen> cityPenList;
    QList<QBrush> cityBrushList;
    /**
     */
    bool* airCBorder;
    bool* airDBorder;
    bool* airElBorder;
    bool* airEhBorder;
    bool* airFBorder;
    bool* ctrCBorder;
    bool* ctrDBorder;
    bool* dangerBorder;
    bool* lowFBorder;
    bool* restrBorder;
    bool* tmzBorder;

    bool* roadBorder;
    bool* highwayBorder;
    bool* railBorder;
    bool* riverBorder;
    bool* cityBorder;
    /**
     * The current scaleindex. The index is set from the mapmatrix-object
     * each time, the map is zoomed.
     */
    int scaleIndex;
    /**
     * true, if small icons should be drawn. Set from the mapmatrix-object
     * each time, the map is zoomed.
     */
    bool isSwitch;
};

#endif
