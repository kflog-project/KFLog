/***********************************************************************
**
**   map.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_H
#define MAP_H

#include <kflog.h>
#include <mapcontents.h>
#include <mapmatrix.h>

#include <qregion.h>
#include <qwidget.h>
#include <qbitmap.h>

class MapView;

/**
 * This class provides basic functions for displaying the map.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class Map : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor creates a new Map-object, read the configdata for the
     * standard parallels, the center of the map, the scale used the last
     * time. It also creates the icon used as a cursor in the map, and
     * connects some tool-bar-button for controling the map.
     */
    Map(KFLogApp* main, QFrame* parent);
    /**
     * Destroys the Map-object and saves the current scale and center of the
     * map, so the map will appear the same, when KFLog is started next.
     */
    ~Map();
    /**  */
    void showFlightLayer(bool redrawFlight = false);

  public slots:
    /** */
    void slotShowMapElement();
    /** */
    void slotConfigureMap();
    /**
     * Forces increasing the scale.
     * @see #scaleAdd
     * @see #scale
     */
    void slotZoomIn();
    /**
     * Forces decreasing the scale.
     * @see #scaleSub
     * @see #scale
     */
    void slotZoomOut();
    /**
     * Displays the flight in the map.
     */
    void slotDrawFlight();
    /** */
    void slotRedrawMap();
    /** */
    void slotShowLayer();
    /** */
    void slotCenterToItem(int listIndex, int elementIndex);
    /** */
    void slotCenterToFlight();
    /** */
    void slotCenterToTask();
    /** */
    void slotCenterToHome();
    /** */
    void slotCenterToWaypoint(struct wayPoint* centerPoint);
    /** */
    void slotMoveMapNW();
    /** */
    void slotMoveMapN();
    /** */
    void slotMoveMapNE();
    /** */
    void slotMoveMapW();
    /** */
    void slotMoveMapE();
    /** */
    void slotMoveMapSW();
    /** */
    void slotMoveMapS();
    /** */
    void slotMoveMapSE();
    /** */
    void slotPrintMap();
    /** */
    void slotScaleChanged(int);

  signals:
    /** */
    void changed(QSize);

  protected:
    /**
     * Moves the center of the map to point given in the event, if the
     * middle-button is pressed.
     */
    virtual void mousePressEvent(QMouseEvent* event);
    /**
     * Displays the coordinates of the mousepointer in the status-bar.
     */
    virtual void mouseMoveEvent(QMouseEvent* event);
    /**
     * Redefinition of the paintEvent.
     */
    virtual void paintEvent(QPaintEvent* event);
    /**
     * Redefinition of the resizeEvent.
     */
    virtual void resizeEvent(QResizeEvent* event);

  private:
    /**
     * Redraws the map.
     */
    void __redrawMap();
    /**
     * Draws the map. The type of mapobjects to be drawn is controlled
     * via slotConfigureMap.
     * @see #slotConfigureMap
     */
    void __drawMap();
    /**
     * Draws the grid tino the map.
     */
    void __drawGrid();
    /** */
    void __setMapBorder();
    /**
     * Pointer to the current KFLog-instance.
     */
    KFLogApp* mainApp;
    /**
     * Coordinates of the current mouse-position, needed
     * to show in the status bar.
     */
    QPoint _currentPos;
    /**
     * This pixmap is used to store the currently displayed map.
     * This painter is about the same size as the map-widget, but is only
     * used for internal buffering the map. Whenever the widget is about
     * to be drawn, this buffer is used to get the content.
     */
    QPixmap pixBuffer;
    /**
     * Contains the grid, needed as the basis.
     */
    QPixmap pixGrid;
    /**
     * Contains the underlying map (contours, rivers, roads, cities, ...)
     */
    QPixmap pixUnderMap;
    /**
     * The layer containing the waypoints.
     */
    QPixmap pixWaypoints;
    /**
     * The layer containing the glider-sites.
     */
    QPixmap pixGlider;
    /**
     * Contains all airports, airfields, additional sites, ...
     */
    QPixmap pixAllSites;
    /**
     * Contains the flights.
     */
    QPixmap pixFlight;
    /**
     * Contains the airspace-structure.
     */
    QPixmap pixAirspace;
    /**
     *
     */
    QPixmap pixCursor;
    /**
      * The minimum scale. Here the height of 1 point is about 6360.0m.
      */
    static const int minScale = 1000;
    /**
      * The maximum scale. Here the height of 1 point is about 12.72m.
      */
    static const int maxScale = 750000;
    /**
     * The mapCenter is the position displayed in the center of the map.
     * It is used in two different ways:
     * 1.: Determine the area shown in the map-widget
     * 2.: Calculating the difference in latitude between a point in the
     * map and the center.
     *
     * The latitude of the center of the map.
     */
    int mapCenterLat;
    /**
     * The longitude of the center of the map.
     */
    int mapCenterLon;
    bool displayFlights;
    bool showAddSites;
    bool showAirport;
    bool showAirspace;
    bool showCity;
    bool showFlight;
    bool showGlider;
    bool showHydro;
    bool showLand;
    bool showNav;
    bool showOut;
    bool showRail;
    bool showRoad;
    bool showTopo;
    bool showWaypoint;

    /**  */
    int* xPos;
    int* yPos;

    /** */
    unsigned int posNum;
    QList<QRegion>* airspaceRegList;
    QList<QRegion>* cityRegList;

    /**
     * Contains two values for each entry in the popupmenu of the map.
     * The first value contains the typeID of the airspace or glidersite,
     * the second value contains the itemID in the mapcontentslist.
     */
    unsigned int* indexList;
    /**
     * Contains the number of entries in the popupmenu of the map.
     */
    unsigned int indexLength;
    /** */
    QPoint prePos;
};

#endif
