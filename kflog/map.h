/***********************************************************************
**
**   map.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**                   2010-2014  by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class Map
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief This class provides basic functions for displaying the map.
 *
 * This class provides basic functions for displaying the map.
 *
 * \date 1999-2014
 *
 * \version $Id$
 */

#ifndef MAP_H
#define MAP_H

#include <QBitmap>
#include <QList>
#include <QMenu>
#include <QRegion>
#include <QSize>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>
#include <QWidget>

#include "flighttask.h"
#include "waypointcatalog.h"

class Flight;
class WaypointDialog;

class Map : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( Map )

  public:
    /**
     * The constructor creates a new Map-object and
     * creates the icon used as a cursor in the map.
     */
    Map( QWidget* parent=0 );
    /**
     * Destroys the Map-object.
     */
    virtual ~Map();

    /**
     * Returns the current task planning state.
     */
    int getPlanningState()
    {
      return planning;
    };

    /**
     * \return List with all current active flights.
     */
    QList<Flight *> getFlightList();

    /**
     * Calculate the real map scale according to the display resolution from
     * a meters per pixel value.
     *
     * \param metersPerPixel Map scale as meters per pixel
     *
     * \return Real map scale as 1:x or empty string, if real map scale is not
     * calculable.
     */
    QString calculateRealMapScale( const int metersPerPixel );

    /**
     * Find a map point at a certain position.
     *
     * \param delta Maximum distance to site position
     *
     * \param mapPosition Position at the drawn map
     *
     * \param w Pointer to an existing Waypoint class object. The found data
     *          are copied into that class.
     */
    bool findMapPoint( int delta, const QPoint& mapPosition, Waypoint *w );

  public slots:

    /**  */
   void slotSavePixmap(QUrl fUrl, int width, int height);
    /** */
    void slotRedrawMap();
    /** */
    void slotScheduleRedrawMap();
    /** */
    void slotCenterToFlight();
    /** */
    void slotCenterToTask();
    /** */
    void slotCenterToWaypoint(const int idx);
    /** */
    void slotDrawCursor(const QPoint& p1, const QPoint& p2);
    /** Clears the flight cursors. */
    void slotClearCursor();
    /** */
    void slotRedrawFlight();
    /**
     * Animation slot.
     * Called for every timeout of the animation timer. Advances the cross-hair
     * one single step.
     */
    void slotAnimateFlightTimeout();
    /**
     * Animation slot. Called to start the animation timer.
     */
    void slotAnimateFlightStart();
    /**
     * Animation slot. Called to pause the animation timer.
     */
    void slotAnimateFlightPause();
    /**
     * Animation slot. Called to stop the animation timer.
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
    /** append a waypoint to the current task */
    void slotAppendWaypoint2Task(Waypoint *p);
    /** Slot signalled when user selects another waypointcatalog.  */
    void slotWaypointCatalogChanged(WaypointCatalog* c);
    /**
    * Zooms into a rectangular region
    */
    void slotZoomRect();

  private slots:

    /**
     * Called from the context menu to center the map.
     */
    void slotMpCenterMap();
    /**
     * Called from the MapPopupmenu to add a new waypoint. The new waypoint can
     * be a found point item at the map. If no item is found, the waypoint
     * editor will popup with the coordinates of the current mouse pointer
     * position.
     */
    void slotMpNewWaypoint();
    /**
     * Called to add a map point item to the waypoint list.
     */
    void slotMpAddAsWaypoint();
    /**
     * called from the MapPopupmenu to edit waypoint.
     */
    void slotMpEditWaypoint();
    /**
     * called from the MapPopupmenu to delete a waypoint.
     */
    void slotMpDeleteWaypoint();
    /**
     * Called from the MapPopupmenu to show item info on the current location.
     */
    void slotMpShowMapItemInfo();
    /**
     * Called from the MapPopupmenu to show item info on the current location.
     */
    void slotMpShowAirspaceInfo();
    /**
     * Called on timeout of the MapInfoTimer. Triggers display of the mapinfo box.
     */
    void slotMapInfoTimeout();
    /**
     * Called on timeout of the MapMoveTimer. Triggers a display of the map.
     */
    void slotMapMoveTimeout();

  signals:

    /** */
    void changed(QSize);
    /** */
    void setStatusBarProgress(int value);
    /** */
    void setStatusBarMsg(const QString&);
    /** */
    void openFile(const QUrl& surl);
    /** */
    void showFlightPoint(const QPoint& pos, const FlightPoint& point);
    /** */
    void showTaskText(FlightTask* task);
    /** */
    void taskPlanningEnd();
    /** */
    void showPoint(const QPoint& pos);
    /** */
    void pointClicked(const QPoint&);
    /** is emitted when shift + left button click on the map */
    void waypointSelected(Waypoint *);
    /** is emitted when a waypoint should be deleted */
    void waypointDeleted(Waypoint *);
    /** is emitted when a waypoint should be edited */
    void waypointEdited(Waypoint *);
    /** Emitted when the mouse hits a new elevation. The argument is the level. */
    void elevation(int);
    /** Send to kflogApp to make KFLogApp connect the dialogs signals to the waypoint object. */
    void regWaypointDialog(QWidget *);
    /** Emitted, if the flight task has been modified. */
    void flightTaskModified();
    /** Emitted, if the map menu item new task is activated. */
    void newTask();

  protected:
    /**
     * Moves the center of the map to point given in the event, if the
     * middle-button is pressed.
     */
    virtual void mousePressEvent(QMouseEvent* event);
    /**
     * For the rectangular zoom
     */
    virtual void mouseReleaseEvent(QMouseEvent* event);
    /**
     * Displays the coordinates of the mouse pointer in the status-bar.
     */
    virtual void mouseMoveEvent(QMouseEvent* event);
    /**
     * Used for zoom action.
     */
    virtual void wheelEvent(QWheelEvent *event);
    /**
     * redraw the current map if state == ShiftButton
     */
    virtual void keyReleaseEvent(QKeyEvent* event);
    /**
     * Redefinition of the paintEvent.
     */
    virtual void paintEvent(QPaintEvent* event);
    /**
     * Redefinition of the resizeEvent.
     */
    virtual void resizeEvent(QResizeEvent* event);
    /**
     * Redefinition of the dragEnterEvent.
     */
    virtual void dragEnterEvent(QDragEnterEvent* event);
    /**
     * Redefinition of the dropEvent.
     */
    virtual void dropEvent(QDropEvent* event);
    /**
     * Called if the mouse leaves the map.
     */
    virtual void leaveEvent( QEvent* event );

  private:

    /**
     * Creates the popup menu for the map.
     */
    void __createPopupMenu();

    /**
     * Creates a new waypoint dialog and returns the prepared object.
     *
     * \param position Latitude and longitude of current position.
     *
     * \return A pointer to the created widget.
     */
    WaypointDialog* __openWaypointDialog( const QPoint &position );

    /**
     * tries to find a waypoint at current position
     */
    Waypoint* findWaypoint (const QPoint& current);

    /**
     * Redraws the map.
     */
    void __redrawMap();
    /**
     * Copies the pixmaps into pixBuffer and calls a paintEvent().
     */
    void __showLayer();

    /**
     *  Show flight data, if position is in the near of a flight.
     *
     *  \param mapPos map position to be used for search.
     */
    void __showFlightData( const QPoint& mapPos );

    /**
     * Draws the map. The type of map objects to be drawn is controlled
     * via slotConfigureMap.
     * @see #slotConfigureMap
     */
    void __drawMap();
    /**
     * Draws all airspaces on the map.
     */
    void __drawAirspaces();
    /**
     */
    void __drawFlight();
    /**
     * Draws the Task which is currently planned
     *
     */
    void __drawPlannedTask(bool solid=true);
    /**
     * Draws the grid on the map.
     */
    void __drawGrid();
    /**
      * Draws a scale on the pixmap.
      */
    void __drawScale(  QPixmap& scalePixmap );
    /**
     * Draws the waypoints of the active waypoint catalog to the map.
     */
    void __drawWaypoints();
    /**
     * Draws the city labels on the passed pixmap.
     */
    void __drawCityLabels( QPixmap& pixmap );
    /**
     * Sets the cross hair cursor
     */
    void __setCursor();
     /**
      * Display Info about a Map item.
      */
    void __displayMapInfo(const QPoint& current, bool automatic=true);
    /**
     * Display Infos about Map airspaces.
     */
   void __displayAirspaceInfo(const QPoint& current, bool automatic=true);
    /**
      * graphical planning
      */
    void __graphicalPlanning(const QPoint& current, QMouseEvent* event);
    /**
     *   search for a waypoint
     *   First look in task itself
     *   Second look in map contents
     */
    bool __getTaskWaypoint(const QPoint& current, Waypoint *wp, QList<Waypoint*> &taskPointList);
    /** Tries to locate the elevation for the given point, and emits a signal elevation if found. */
    void __findElevation(const QPoint& coord);

    /** Enable/disable the correct items for the map menu and then show it. */
    void __showPopupMenu(QMouseEvent * Event);
    /**
     * This pixmap is used to store the currently displayed map.
     * This painter is about the same size as the map widget, but is only
     * used for internal buffering the map. Whenever the widget is about
     * to be drawn, this buffer is used to get the content.
     */
    QPixmap pixBuffer;

    QPixmap pixCursor1;
    QPixmap pixCursor2;
    /**
     * Contains the grid, needed as the basis.
     */
    QPixmap pixGrid;
    /**
     * Contains the topography
     */
    QPixmap pixIsoMap;
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
     * Contains the used cursor.
     */
    QPixmap pixCursor;
    /**
     * Contains the flight evaluation and optimization cursors.
     */
    QPixmap pixFlightCursors;
    /**
     * Flag to signal, that flight cursors shall be redrawn.
     */
    bool drawFlightCursors;
    /**
     * Pixmap containing the Zoom rectangle.
     */
    QPixmap pixZoomRect;
    /**
     * Pixmap to hold waypoints of active WaypointsCatalog
     */
    QPixmap pixWaypoints;
    /**
     * Pixmap with rotating glider symbols, used for flight animation.
     */
    QPixmap pixGliders;

    /** Previous cursor positions. */
    QPoint lastCur1Pos;
    QPoint lastCur2Pos;

    QPoint prePos;
    QPoint prePlanPos;
    QPoint preAnimationPos;

    /** Map coordinates of previous flight step point. */
    QPoint preStepPos;

    /** Flight index of previous step action. */
    int preStepIndex;

    /** Flag to handle flight step cursor drawing. */
    bool drawFlightStepCursor;

    /**
     * Animation timer
     */
    QTimer* timerAnimate;
    /** Flag to indicate an animation pause. */
    bool animationPaused;
    /**
     * contains planning task points
     * enthält die Punkte!!!
     */
    // 0 keine Planung 1 Planung 2 Planung Aufgabe Abgeschlossen
    // enum planning {NoPlanning = 0, Planning = 1, TaskFinished = 2};
    int planning;
    // Index des WP welcher verschoben wird bei planning == 3
    int moveWPindex;
    // indicates if a WP was Added in Snapping Mode (Planning)
    bool lastAdd;
    // indicates if the mouse is in a snapping area
    bool isSnapping;
    // temp task (snapping)
    FlightTask tempTask;
    // Indicates that drag zoom was started by a button or by a command key.
    // That is checked in mouse press and mouse move event to handle zoom action.
    bool startDragZoom;
    // indicates that the drag for the dragZoom has started
    bool isDragZoomActive;
    // contains the point where the dragZoom was started
    QPoint beginDrag;
    // contains the size of the drag.
    QSize sizeDrag;
    // indicates that the map move action has started
    bool isMapMoveActive;
    // contains the point where the map move was started
    QPoint beginMapMove;
    // Timer to control map moving.
    QTimer* timerMapMove;

    /**
     * Added for a Workaround: There are not two messages
     * boxes during the first start anymore
     */
    bool firstStart;
    /**
     * Popup menu under the right mouse button
     */
    QMenu *mapPopup;
    /**
     * Actions of menu items
     */
    QAction *miCenterMapAction;
    QAction *miAddAsWaypointAction;
    QAction *miAddWaypointAction;
    QAction *miEditWaypointAction;
    QAction *miDeleteWaypointAction;
    QAction *miNewTaskAction;
    QAction *miAddTaskPointAction;
    QAction *miShowMapInfoAction;
    QAction *miShowMapAirspaceInfoAction;
    QAction *miZoomInAction;
    QAction *miZoomOutAction;
        /** */
    bool isDrawing;
    bool redrawRequest;

    /** Reference to the redraw timer */
    QTimer *redrawMapTimer;

    /** Contains the position where the mouse was clicked to show the popup menu */
    QPoint popupPos;
    //
    QPoint preSnapPoint;
    //
    QTimer *mapInfoTimer;
    QPoint mapInfoTimerStartpoint;

    /** List of drawn cities. */
    QList<BaseMapElement *> m_drawnCityList;
};

#endif
