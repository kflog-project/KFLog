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

#include <QAction>
#include <QLabel>
#include <q3mainwindow.h>
#include <q3networkprotocol.h>
#include <q3progressbar.h>
#include <QUrl>

#include "dataview.h"
#include "evaluationdialog.h"
#include "helpwindow.h"
#include "kflogstartlogo.h"
#include "map.h"
#include "mapcontrolview.h"
#include "objecttree.h"
#include "translationlist.h"
#include "topolegend.h"
#include "waypoints.h"

class KFLog : public Q3MainWindow
{
  Q_OBJECT

public:
  KFLog();
  virtual
  ~KFLog();

  signals:
   /**
    * Emitted, when the user selects a new flightdatatype.
    *
    * @param  type  The id of the selected data-type
    */
   void flightDataTypeChanged(int type);

public slots:
  // Reimplemented from QWidget
  void closeEvent(QCloseEvent *e);
  /**
  * Display dialog to ask for coordinates and center map on that point.
  */
  void slotCenterTo();
  /**
   * Opens a dialog for configuraion of KFLog.
   */
  void slotConfigureKFLog();
  /**
   * Opens the printing-dialog to print the map.
   */
  void slotFilePrint();
  /** */
  void slotFlightPrint();
  /** */
  void slotFlightViewIgc3D();
  /** */
  void slotFlightViewIgcOpenGL();
  /** set menu items enabled/disabled */
  void slotModifyMenu();
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
  /** */
  void slotOpenRecorderDialog();
  /** optimize flight for OLC declaration*/
  void slotOptimizeFlightOLC();
  /** */
  void slotOptimizeFlight();
  /** Connects the dialogs addWaypoint signal to the waypoint object. */
  void slotRegisterWaypointDialog(QWidget * dialog);
  /**
   * Called, when the user selects a data-type from the menu. Emits
   * flightDataTypeChanged(int)
   *
   * @param  menuItem  The id of the selected listitem.
   *
   * @see #flightDataTypeChanged(int)
   */
  void slotSelectFlightData(int listItem);
  /**
   * Updates the recent file list.
   */
  void slotSetCurrentFile(const QString &fileName);
  /**
   * Displays the position of the mousecursor and some info (time,
   * altitude, speed, vario) about the selected flightpoint in the
   * statusbar.
   *
   * @param mouseP   The lat/lon position under the mousecursor.
   * @param flightP  Pointer to the flightpoint.
   */
  void slotSetPointInfo(const QPoint& mousePosition,
                         const flightPoint& point);
  /**
   * Displays the position of the mousecursor in the statusbar and
   * deletes the text of the other statusbar-fields.
   */
  void slotSetPointInfo(const QPoint&);
  /**
   * Updates the progressbar in the statusbar.
   *
   * @param  value  The new value of the progressbar, given in percent;
   */
  void slotSetProgress(int value);
  /**
   * Displays a message in the statusbar.
   *
   * @param  text  The message to be displayed.
   */
  void slotSetStatusMsg(const QString& text);
  /** */
  void slotSetWaypointCatalog(QString catalog);
  /**
   * Checks the status of all dock-widgets and updates the menu.
   */
  void slotCheckDockWidgetStatus();
  /** */
  void slotSavePixmap(QUrl url, int width, int height);
  /**
   * Hides the startup-window.
   */
  void slotStartComplete();
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
//  /**
//   * Shows or hides the map-widget.
//   */
//  void slotToggleMap();
  /**
   * Shows or hides the mapcontrol-widget.
   */
  void slotToggleMapControl();
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
   * Called to the What's This? mode.
   */
  void slotWhatsThis();
  /**
   * insert available flights into menu
   */
  void slotWindowsMenuAboutToShow();

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

private:
  /** Initialises all QDockWindows */
  void initDockWindows();
  /** Initialises QMenuBar */
  void initMenuBar();
  /** Initialises the QStatusBar */
  void initStatusBar();
  /** Initialises surface types */
  void initSurfaceTypes();
  /** Initialises task types*/
  void initTaskTypes();
  /** Initialises toolbar*/
  void initToolBar();
  /** Initialises waypoint types*/
  void initWaypointTypes();
  /**
   * Dockwidget to handle the dataview-widget.
   * The dataview-widget. Embedded in dataViewDock
   */
  Q3DockWindow* dataViewDock;
  DataView* dataView;
  /**
   * Dockwidget to handle the EvaluationWindow.
   * The evalutionWindow. Embedded in evaluationWindowDock
   */
  Q3DockWindow* evaluationWindowDock;
  EvaluationDialog* evaluationWindow;
  /**
   * Dockwidget to handle the helpWindow.
   * The helpWindow. Embedded in helpWindowDock
   */
  Q3DockWindow* helpWindowDock;
  HelpWindow* helpWindow;
  /**
   * Dockwidget to handle the legend-widget.
   *
   * @see TopoLegend
   */
  Q3DockWindow* legendDock;
  TopoLegend* legend;
  /**
   * Dockwidget to handle the map.
   * The map-widget.
   */
  Q3DockWindow* mapViewDock;
  Map* map;
  /**
   * Dockwidget to handle the mapcontrol.
   * The mapcontrol-widget. Embedded in mapControlDock
   */
  Q3DockWindow* mapControlDock;
  MapControlView* mapControl;
  /**
   * Dockwidget to handle the object view
   *
   * @see ObjectView
   */
  Q3DockWindow* objectTreeDock;
  ObjectTree* objectTree;
  /**
   * Dockwidget to handle the waypoints-widget.
   * The waypoints-widget.
   */
  Q3DockWindow* waypointsDock;
  Waypoints* waypoints;
  Q3ToolBar* toolBar;
  /**
   * Actions for the menu File
   */
  QAction* fileNewWaypoint;
  QAction* fileNewTask;
  QAction* fileNewFlightGroup;
  QAction* fileOpenFlight;
  QAction* fileOpenTask;
  Q3PopupMenu* fileOpenRecent;
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
  Q3PopupMenu* flightDataType;
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
  Q3PopupMenu* windowMenu;
  /**
   * Actions for the menu Settings
   */
  Q3PopupMenu* settings;
  QAction* settingsEvaluationWindow;
  QAction* settingsFlightData;
  QAction* settingsHelpWindow;
  QAction* settingsLegend;
//  QAction* settingsMap;
  QAction* settingsMapControl;
  QAction* settingsObjectTree;
  QAction* settingsStatusBar;
  QAction* settingsToolBar;
  QAction* settingsWaypoints;
  /**
   * True, when the startup-window should be displayed.
   */
  bool showStartLogo;
  /**
   * The startup-window.
   */
  KFLogStartLogo* startLogo;
  /**
   * The progressbar in the statusbar. Used during drawing the map to display
   * the percentage of what is allready drawn.
   *
   * @see slotSetProgress
   */
  Q3ProgressBar* statusProgress;
  /**
   * The label to display a message in the statusbar.
   *
   * @see slotStatusMsg
   */
  QLabel* statusLabel;
  /**
   * The label to display the time of a selected flight-point.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusTimeL;
  /**
   * The label to display the altitude of a selected flight-point.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusAltitudeL;
  /**
   * The label to display the vario of a selected flight-point.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusVarioL;
  /**
   * The label to display the speed of a selected flight-point.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusSpeedL;
  /**
   * The label to display the latitude of the position under the mousecursor.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusLatL;
  /**
   * The label to display the longitude of the position under the mousecursor.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusLonL;
  /**
   * The flight-directory.
   */
  QString flightDir;
  /**
   * The task and waypoints directory.
   */
  QString taskDir;
};
 
#endif // KFLOG_H
