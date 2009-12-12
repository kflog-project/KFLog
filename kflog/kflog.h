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

#include "translationlist.h"

#include <kapp.h>
#include <kdockwidget.h>
#include <kmainwindow.h>
#include <kprogress.h>
#include <kstatusbar.h>

#include <qaction.h>
#include <qurl.h>

class DataView;
class HelpWindow;
class EvaluationDialog;
class Map;
class MapControlView;
class KFLogConfig;
class KFLogStartLogo;
class Waypoints;
class flightPoint;
class EvaluationDialog;
class TopoLegend;
class ObjectTree;

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
  /** No descriptions */
  void initSurfaces();
  /** No descriptions */
  void initTypes();
  /** No descriptions */
  void initTaskTypes();
  /* ask for outstanding changes */
  bool queryClose();

  public slots:
  /**
   * Opens a about-dialog.
   */
  void slotShowAbout();
  /**
   * Displays the position of the mousecursor and some info (time,
   * altitude, speed, vario) about the selected flightpoint in the
   * statusbar.
   *
   * @param mouseP   The lat/lon position under the mousecursor.
   * @param flightP  Pointer to the flightpoint.
   */
  void slotShowPointInfo(const QPoint& mousePosition,
                         const flightPoint& point);
  /**
   * Displays the position of the mousecursor in the statusbar and
   * deletes the text of the other statusbar-fields.
   */
  void slotShowPointInfo(const QPoint&);
  /**
   * Opens a file-open-dialog.
   */
  void slotOpenFile();
  /**
   * Opens the file given in url.
   */
  void slotOpenFile(const char* surl);
  /**
   * Opens a task-file-open-dialog.
   */
  void slotOpenTask();
  /**
   * Opens a selected recently opened flight.
   */
  void slotOpenRecentFile();
  /**
   * Updates the recent file list.
   */
  void slotSetCurrentFile(const QString &fileName);
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
    * Shows or hides the Evaluation window
    */
  void slotToggleEvaluationWindow();
  /**
   * Shows or hides the Help Window.
   */
  void slotToggleHelpWindow();
  /**
   * Shows or hides the object-widget.
   */
  void slotToggleLegendDock();
  /**
   * Shows or hides the mapcontrol-widget.
   */
  void slotToggleMapControl();
  /**
   * Shows or hides the map-widget.
   */
  void slotToggleMap();
  /**
   * Shows or hides the legend-widget.
   */
  void slotToggleObjectTreeDock();
  /**
   * Shows or hides the toolbar.
   */
  void slotToggleToolBar();
  /**
   * Shows or hides the statusbar.
   */
  void slotToggleStatusBar();
  /**
   * Shows or hides the waypoints-widget.
   */
  void slotToggleWaypointsDock();
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
   * Opens an evaluation-dialog. // Now as a Dockwidget
   */
//   void slotEvaluateFlight();
  /**
   * Hides the dataview-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideDataViewDock();
  /**
    * Hides the EvaluationWindow. Called, when the user has closed or
    * undocked the widget.
    */
  void slotHideEvaluationWindowDock();
  /**
   * Hides the HelpWindow. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideHelpWindowDock();
  /**
   * Hides the Legend-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideLegendDock();
  /**
   * Hides the map-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideMapViewDock();
  /**
   * Hides the mapcontrol-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideMapControlDock();
  /**
   * Hides the Objects-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideObjectTreeDock();
  /**
   * Hides the Waypoints-widget. Called, when the user has closed or
   * undocked the widget.
   */
  void slotHideWaypointsDock();
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
  /** optimize flight for OLC declaration*/
  void slotOptimizeFlightOLC();
  /** */
  void slotOptimizeFlight();
  /** */
  void slotSavePixmap(QUrl url, int width, int height);
  /** */
  void slotFlightViewIgc3D();
  /** */
  void slotFlightViewIgcOpenGL();
  /** set menu items enabled/disabled */
  void slotModifyMenu();
  /** */
  void slotOpenRecorderDialog();
  /** No descriptions */
  void slotSetWaypointCatalog(QString catalog);
  /**
   * Display dialog to ask for coordinates and center map on that point.
   */
  void slotCenterTo();
  /** Connects the dialogs addWaypoint signal to the waypoint object. */
  void slotRegisterWaypointDialog(QWidget * dialog);
 private:
  /**
   * The startup-window.
   */
  KFLogStartLogo* startLogo;
  /**
   * The configuration object of the application
   */
  KConfig *config;
  /**
   * Dockwidget to handle the dataview-widget.
   * The dataview-widget. Embedded in dataViewDock
   */
  KDockWidget* dataViewDock;
  DataView* dataView;
  /**
   * Dockwidget to handle the EvaluationWindow.
   * The evalutionWindow. Embedded in evaluationWindowDock
   */
  KDockWidget* evaluationWindowDock;  
  EvaluationDialog* evaluationWindow;
  /**
   * Dockwidget to handle the helpWindow.
   * The helpWindow. Embedded in helpWindowDock
   */
  KDockWidget* helpWindowDock;
  HelpWindow* helpWindow;
  /**
   * Dockwidget to handle the legend-widget.
   *
   * @see TopoLegend
   */
  KDockWidget* legendDock;
  TopoLegend* legend;
  /**
   * Dockwidget to handle the map.
   * The map-widget.
   */
  KDockWidget* mapViewDock;
  Map* map;
  /**
   * Dockwidget to handle the mapcontrol.
   * The mapcontrol-widget. Embedded in mapControlDock
   */
  KDockWidget* mapControlDock;
  MapControlView* mapControl;
  /**
   * Dockwidget to handle the object view
   *
   * @see ObjectView
   */
  KDockWidget* objectTreeDock;
  ObjectTree* objectTree;
  /**
   * Dockwidget to handle the waypoints-widget.
   * The waypoints-widget.
   */
  KDockWidget* waypointsDock;
  Waypoints *waypoints;
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
   * Actions for the menu File
   */
  QAction* fileNewWaypoint;
  QAction* fileNewTask;
  QAction* fileNewFlightGroup;
  QAction* fileOpenFlight;
  QAction* fileOpenTask;
  QPopupMenu* fileOpenRecent;
  QAction* fileClose;
  QAction* fileSavePixmap;
  QAction* filePrint;
  QAction* filePrintFlight;
  QAction* fileOpenRecorder;
  QAction* fileQuit;
  /**
   * Actions for the menu View
   */
  QAction* viewCenterTask;
  QAction* viewCenterFlight;
  QAction* viewCenterHomesite;
  QAction* viewCenterTo;
  QAction* viewZoomIn;
  QAction* viewZoomOut;
  QAction* viewZoom;
  QAction* viewRedraw;
  QAction* viewMoveNW;
  QAction* viewMoveN;
  QAction* viewMoveNE;
  QAction* viewMoveW;
  QAction* viewMoveE;
  QAction* viewMoveSW;
  QAction* viewMoveS;
  QAction* viewMoveSE;
  /**
   * Actions for the menu Flight
   */
  QAction* flightEvaluationWindow;
  QAction* flightOptimization;
  QAction* flightOptimizationOLC;
  QPopupMenu* flightDataType;
  QAction* flightIgc3D;
  QAction* flightIgcOpenGL;
  QAction* flightAnimateStart;
  QAction* flightAnimateStop;
  QAction* flightAnimateNext;
  QAction* flightAnimatePrev;
  QAction* flightAnimate10Next;
  QAction* flightAnimate10Prev;
  QAction* flightAnimateHome;
  QAction* flightAnimateEnd;
  /**
   * Action for the menu Window
   */
  QPopupMenu* windowMenu;
  /**
   * Actions for the menu Settings
   */
  QPopupMenu* settings;
  QAction* settingsEvaluationWindow;
  QAction* settingsFlightData;
  QAction* settingsHelpWindow;
  QAction* settingsLegend;
  QAction* settingsMap;
  QAction* settingsMapControl;
  QAction* settingsObjectTree;
  QAction* settingsStatusBar;
  QAction* settingsToolBar;
  QAction* settingsWaypoints;
  /**
   * The flight-directory.
   */
  QString flightDir;
  /**
   * The task and waypoints directory.
   */
  QString taskDir;
  /**
   * True, when the startup-window should be displayed.
   */
  bool showStartLogo;

  private slots: // Private slots
  /**
   * insert available flights into menu
   */
  void slotWindowsMenuAboutToShow();
  /**
   * Called to the What's This? mode.
   */
  void slotWhatsThis();
  /**
   * Called to force display of the "Tip of the Day" dialog.
   */
  void slotTipOfDay();

};
 
#endif // KFLOG_H
