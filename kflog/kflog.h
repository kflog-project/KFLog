/***********************************************************************
**
**   kflog.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef KFLOG_H
#define KFLOG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapp.h>
#include <kdockwidget.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kprogress.h>

class DataView;
class Map;
class MapControlView;
class KFLogConfig;
class KFLogStartLogo;

/**
 * Mainwindow for KFLog.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class KFLogApp : public KDockMainWindow
{
  Q_OBJECT

  public:
    /**
     * Constructor.
     */
    KFLogApp();
    /**
     * Destructor.
     */
    ~KFLogApp();

  signals:
    /**
     * Emitted, when the user selects a new flightdatatype.
     *
     * @param  type  The id of the selected data-type
     */
    void flightDataTypeChanged(int type);

  protected:
    /**
     * Writes the window-geometry, statusbar- and toolbarstate and the
     * layoutstate of the dockwidgets.
     */
    void saveOptions();
    /**
     * Reads the window-geometry, statusbar- and toolbarstate and the
     * layoutstate of the dockwidgets.
     */
    void readOptions();
    /**
     * Initializes all actions
     */
    void initActions();
    /**
     * Initializes the statusbar
     */
    void initStatusBar();
    /**
     * Initializes the mainwindow.
     */
    void initView();

  public slots:
    /**
     * Displays the position of the mousecursor and some info (time,
     * altitude, speed, vario) about the selected flightpoint in the
     * statusbar.
     *
     * @param mouseP   The lat/lon position under the mousecursor.
     * @param flightP  Pointer to the flightpoint.
     */
    void slotShowPointInfo(const QPoint mousePosition,
        const struct flightPoint& point);
    /**
     * Displays the position of the mousecursor in the statusbar and
     * deletes the text of the other statusbar-fields.
     */
    void slotShowPointInfo(const QPoint);
    /**
     * Opens a file-open-dialog.
     */
    void slotFileOpen();
    /**
     * Opens a selected recently opened flight.
     *
     * @param  url  The url that the user has selected.
     */
    void slotFileOpenRecent(const KURL& url);
    /**
     * Closes all displayed flights.
     */
    void slotFileClose();
    /**
     * Opens the printing-dialog to print the map.
     */
    void slotFilePrint();
    /** */
    void slotFlightPrint();
    /**
     * Calls saveConfig() and closes the application.
     */
    void slotFileQuit();
    /**
     * Shows or hides the toolbar.
     */
    void slotViewToolBar();
    /**
     * Shows or hides the statusbar.
     */
    void slotViewStatusBar();
    /**
     * Displays a message in the statusbar.
     *
     * @param  text  The message to be displayed.
     */
    void slotStatusMsg(const QString& text);
    /**
     * Shows or hides the dataview-widget.
     */
    void slotToggleDataView();
    /**
     * Shows or hides the mapcontrol-widget.
     */
    void slotToggleMapControl();
    /**
     * Shows or hides the map-widget.
     */
    void slotToggleMap();
    /**
     * Opens a dialog for configuration of the toolbar.
     */
    void slotConfigureToolbars();
    /**
     * Opens a dialog for configuration of the keybindings of the actions.
     */
    void slotConfigureKeyBindings();
    /**
     * Opens a dialog for configuraion of KFLog.
     */
    void slotConfigureKFLog();
    /**
     * Rereads the configuration of the toolbar.
     */
    void slotNewToolbarConfig();
    /**
     * Updates the progressbar in the statusbar.
     *
     * @param  value  The new value of the progressbar, given in percent;
     */
    void slotSetProgress(int value);
    /**
     * Hides the startup-window.
     */
    void slotStartComplete();
    /**
     * Opens an evaluation-dialog.
     */
    void slotEvaluateFlight();
    /**
     * Hides the mapcontrol-widget. Called, when the user has closed or
     * undocked the widget.
     */
    void slotHideMapControlDock();
    /**
     * Hides the map-widget. Called, when the user has closed or
     * undocked the widget.
     */
    void slotHideMapViewDock();
    /**
     * Hides the dataview-widget. Called, when the user has closed or
     * undocked the widget.
     */
    void slotHideDataViewDock();
    /**
     * Checks the status of all dock-widgets and updates the menu.
     */
    void slotCheckDockWidgetStatus();
    /**
     * Called, when the user selects a data-type  from the menu. Emits
     * flightDataTypeChanged(int)
     *
     * @param  menuItem  The id of the selected listitem.
     *
     * @see #flightDataTypeChanged(int)
     */
    void slotSelectFlightData(int listItem);
    /** */
    void slotOptimizeFlight();
    /** */
    void slotTaskAndWaypoint();

  private:
    /**
     * The configuration object of the application
     */
    KConfig *config;
    /**
     * Dockwidget to handle the mapcontrol.
     *
     * @see mapControl
     */
    KDockWidget* mapControlDock;
    /**
     * Dockwidget to handle the map.
     *
     * @see map
     */
    KDockWidget* mapViewDock;
    /**
     * Dockwidget to handle the dataview-widget.
     *
     * @see dataView
     */
    KDockWidget* dataViewDock;
    /**
     * The mapcontrol-widget. Embedded in mapControlDock
     *
     * @see mapControlDock
     */
    MapControlView* mapControl;
    /**
     * The dataview-widget. Embedded in dataViewDock
     *
     * @see dataViewDock
     */
    DataView* dataView;
    /**
     * The progessbar in the statusbar. Used during drawing the map to display
     * the percentage of what is allready drawn.
     *
     * @see slotSetProgress
     */
    KProgress* statusProgress;
    /**
     * The label to display a message in the statusbar.
     *
     * @see slotStatusMsg
     */
    KStatusBarLabel* statusLabel;
    /**
     * The label to display the time of a selected flight-point.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusTimeL;
    /**
     * The label to display the altitude of a selected flight-point.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusAltitudeL;
    /**
     * The label to display the vario of a selected flight-point.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusVarioL;
    /**
     * The label to display the speed of a selected flight-point.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusSpeedL;
    /**
     * The label to display the latitude of the position under the mousecursor.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusLatL;
    /**
     * The label to display the longitude of the position under the mousecursor.
     *
     * @see slotShowPointInfo
     */
    KStatusBarLabel* statusLonL;
    /**
     * Action for closing all displayed flight. Disabled, when no flight
     * is loaded.
     */
    KAction* fileClose;
    /**
     * Standard-action to handle the recently opened flights.
     */
    KRecentFilesAction* fileOpenRecent;
    /**
     * Action to handle the toolbar.
     */
    KToggleAction* viewToolBar;
    /**
     * Action to handle the statusbar.
     */
    KToggleAction* viewStatusBar;
    /**
     * Action to center the map to display the task. Disabled when no
     * flight is loaded.
     */
    KAction* viewCenterTask;
    /**
     * Action to center the map to display the flight. Disabled when no
     * flight is loaded.
     */
    KAction* viewCenterFlight;
    /**
     * Action to handle the dataview.
     */
    KToggleAction* viewData;
    /**
     * Action to handle the mapcontrol.
     */
    KToggleAction* viewMapControl;
    /**
     * Action to handle the map.
     */
    KToggleAction* viewMap;
    /**
     * Action to show the evaluationdialog.
     */
    KAction* flightEvaluation;
    KAction* flightOptimization;
    /** */
    KAction* flightPrint;
    /** */
    KAction* taskAndWaypoint;
		/**
		 * Action to start the animation
		 */		
		KAction* animateFlightStart;
		/**
		 * Action to stop the animation
		 */		
		KAction* animateFlightStop;
		/**
		 * Steppings actions.
		 */
		KAction* stepFlightNext;
 		KAction* stepFlightPrev;
		KAction* stepFlightHome;
		KAction* stepFlightEnd;
 		KAction* stepFlightStepNext;
 		KAction* stepFlightStepPrev;
		/**
     * The map-widget.
     */
    Map* map;
    /**
     * The flight-directory.
     */
    QString flightDir;
    /**
     * The startup-window.
     */
    KFLogStartLogo* startLogo;
    /**
     * True, when the startup-window should be displayed.
     */
    bool showStartLogo;
};
 
#endif // KFLOG_H
