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

#include <qregion.h>
#include <qwidget.h>
#include <qbitmap.h>

#include <qpointarray.h>

#include <wp.h>
#include <flighttask.h>

// class SinglePoint;
class Flight;

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
     * The constructor creates a new Map-object and
     * creates the icon used as a cursor in the map.
     */
    Map(KFLogApp* main, QFrame* parent, const char* name);
    /**
     * Destroys the Map-object.
     */
    ~Map();

  public slots:
    /** */
    void slotRedrawMap();
    /** */
    void slotCenterToFlight();
    /** */
    void slotCenterToTask();
    /** */
    void slotCenterToWaypoint(const unsigned int id);
    /** */
    void slotDrawCursor(QPoint p1, QPoint p2);
    /** */
    void slotRedrawFlight();
    /**
		 * Animation slot.
		 * Called for every timeout of the animation timer. Advances the crosshair one single step.
		 */
    void slotAnimateFlightTimeout();
    /**
		 * Animation slot.
		 * Called to start the animation timer
		 */
    void slotAnimateFlightStart();
	  /**
		 * Animation slot.
		 * Called to stop the animation timer
		 */
    void slotAnimateFlightStop();
		/**
		 * Stepping slots.
		 */
    void slotFlightNext();
    void slotFlightPrev();
    void slotFlightStepNext();
    void slotFlightStepPrev();
    void slotFlightHome();
    void slotFlightEnd();

    void slotActivatePlanning();
  /** No descriptions */
  void slotShowCurrentFlight();
  signals:
    /** */
    void changed(QSize);
    /** */
    void showFlightPoint(const QPoint pos, const struct flightPoint& point);
    /**
    	*
    	*/
    void showTaskText(FlightTask* task, QPoint current);
    /** */
    void taskPlanningEnd();
    /** */
    void showPoint(const QPoint pos);
    /** */
    void pointClicked(QPoint);
  /** is emited when shift + left button click on the map */
  void waypointSelected(wayPoint *);

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
    /**
     * Redifinition of the dragEnterEvent.
     */
    virtual void dragEnterEvent(QDragEnterEvent* event);
    /**
     * Redifinition of the dropEvent.
     */
    virtual void dropEvent(QDropEvent* event);

  private:
    /**
     * functionality for the graphical Planning
     */
    void __graphicalPlanning();
    /**
     * Redraws the map.
     */
    void __redrawMap();
    /**
     * Copies the pixmaps into pixBuffer and calls a paintEvent().
     */
    void __showLayer();
    /**
     * Draws the map. The type of mapobjects to be drawn is controlled
     * via slotConfigureMap.
     * @see #slotConfigureMap
     */
    void __drawMap();
    /**
     */
    void __drawFlight();
    /**
     * Draws the Task which is currently planned
     *
     */
    void __drawPlannedTask();
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
      * Display Infos about MapItems
      */
    void __displayMapInfo(QPoint current);
    /**
      * graphical planning
      */
    void __graphicalPlanning(QPoint current);

    /**
     * This pixmap is used to store the currently displayed map.
     * This painter is about the same size as the map-widget, but is only
     * used for internal buffering the map. Whenever the widget is about
     * to be drawn, this buffer is used to get the content.
     */
    QPixmap pixBuffer;
    /**
      * Contains the Map under the CursorA to erase the cursor later
      *
      */
    QPixmap pixCursorBuffer1;
    QPixmap pixCursorBuffer2;
    QPixmap pixCursor1;
    QPixmap pixCursor2;
    QPixmap pixAnimate;
    /**
     * Contains the grid, needed as the basis.
     */
    QPixmap pixGrid;
    /**
     * Contains the underlying map (contours, rivers, roads, cities, ...)
     */
    QPixmap pixUnderMap;
    /**
     * The layer containing all aeronautical elements.
     */
    QPixmap pixAero;
    /**
     * Contains the flights.
     */
    QPixmap pixFlight;
    /**
     * Planning Task.
     */
    QPixmap pixPlan;
    /**
     * Contains the airspace-structure.
     */
    QPixmap pixAirspace;
    /**
     * Contains the topography
     */
    QPixmap pixIsoMap;
    /**
     *
     */
    QPixmap pixCursor;
    /** */
    QBitmap bitMapMask;
    /** */
    QBitmap bitAirspaceMask;
    /** */
    QBitmap bitFlightMask;
    QBitmap bitPlanMask;
    /** */
    QPoint prePos;
    QPoint prePlanPos;
    QPoint preAnimationPos;
    QPoint preCur1;
    QPoint preCur2;
    int preIndex;
    /**
     * Contains the regions of all visible airspaces. The list is needed to
     * find the airspace-data when the users selects a airspace in the map.
     */
    QList<QRegion>* airspaceRegList;
		/**
		 * Animation timer
		 */
		QTimer* timerAnimate;
		/**
		 * FlightList to animate
		 */
  	Flight* flightToAnimate;
		/**
			* contains planning task points
			* enthält die Punkte!!!
			*/
    QList<wayPoint> taskPointList;
		// 0 keine Planung 1 Planung 2 Planung Aufgabe Abgeschlossen
//		enum planning {NoPlanning = 0, Planning = 1, TaskFinished = 2};
		int planning;
		// Index des WP welcher verschoben wird bei planning == 3
		int moveWPindex;
			
};

#endif
