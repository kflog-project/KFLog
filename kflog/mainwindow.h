/***********************************************************************
 **
 **   mainwindow.h
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2001      by Heiner Lamprecht, Florian Ehinger
 **                   2010-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \class MainWindow
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief This class provides the main window of KFLog.
 *
 * This class provides the main window of KFLog. All needed GUI stuff
 * is initialized and handled here.
 *
 * \date 2001-2011
 *
 * \version $Id$
 */

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QPixmap>
#include <QProgressBar>
#include <QUrl>

class DataView;
class EvaluationDialog;
class FlightPoint;
class HelpWindow;
class Map;
class MapControlView;
class ObjectTree;
class WaypointTreeView;
class TopoLegend;
class TranslationList;

class MainWindow : public QMainWindow
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( MainWindow )

public:

  MainWindow( QWidget * parent = 0, Qt::WindowFlags flags = 0 );

  virtual ~MainWindow();

  /**
   * \return The single instance of this class.
   */
  static MainWindow* instance();

  /**
   * \return The requested pixmap.
   */
  QPixmap getPixmap( const QString& pixmapName );

  /**
   * Creates the application data directory, if it not exists.
   *
   * \return True on success otherwise false
   */
  bool createApplicationDataDirectory();

  /**
   * \return The application's data directory.
   */
  QString getApplicationDataDirectory();

  /**
   * \return The application's task directory.
   */
  QString getApplicationTaskDirectory();

  /**
   * \return The application's flight directory.
   */
  QString getApplicationFlightDirectory();

  /**
   * Sets the selected flight data action to active and all other to
   * inactive.
   *
   * \param index The index of the related action.
   */
  void selectFlightDataAction( const int index );

signals:

  /**
  * Emitted, when the user selects a new flight data type.
  *
  * @param  type The index of the selected data-type
  */
  void flightDataTypeChanged(int type);

public slots:

  /**
  * Display dialog to ask for coordinates and center map on that point.
  */
  void slotCenterTo();
  /**
   * Opens a dialog for configuration of KFLog.
   */
  void slotConfigureKFLog();
  /**
   * Opens the printing dialog to print the map into a file.
   */
  void slotFilePrint();
  /** */
  void slotFlightPrint();
  /** */
  void slotFlightViewIgc3D();
  /** Called, if window is closed by the user. */
  void slotFlightViewIgc3DClosed();
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
  void slotOpenFile(const QUrl& url);
  /**
   * Opens a task-file-open-dialog.
   */
  void slotOpenTask();
  /**
   * Opens a selected recently opened flight.
   */
  void slotOpenRecentFile( QAction *action );
  /** */
  void slotOpenRecorderDialog();
  /** optimize flight for OLC declaration*/
  void slotOptimizeFlightOLC();
  /** */
  void slotOptimizeFlight();
  /** Connects the dialogs addWaypoint signal to the waypoint object. */
  void slotRegisterWaypointDialog(QWidget * dialog);

  /**
   * Called, when the user selects a data type from the menu. Emits
   * flightDataTypeChanged(int)
   *
   * @param  index  The index of the selected list item.
   *
   * @see #flightDataTypeChanged(int)
   */
  void slotSelectFlightData( const int index );
  /**
   * Updates the recent file list.
   */
  void slotSetCurrentFile(const QString &fileName);
  /**
   * Displays the position of the mouse cursor and some info (time,
   * altitude, speed, vario) about the selected flight point in the
   * status bar.
   *
   * @param mousePosition The lat/lon position under the mouse cursor.
   * @param point The related flight point.
   */
  void slotSetPointInfo( const QPoint& mousePosition,
                         const FlightPoint& point );
  /**
   * Displays the position of the mouse cursor in the status bar and
   * deletes the text of the other statusbar fields.
   */
  void slotSetPointInfo(const QPoint& point);
  /**
   * Updates the progress bar in the status bar.
   *
   * @param  value  The new value of the progress bar, given in percent;
   */
  void slotSetProgress(int value);
  /**
   * Displays a message in the status bar.
   *
   * @param text The message to be displayed.
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
   * Called to the What's This? mode.
   */
  void slotWhatsThis();
  /**
   * Insert available flights into the related menu.
   */
  void slotWindowsMenuAboutToShow();

  /**
   * Called, if an action of the group is triggered.
   */
  void slotFlightDataTypeGroupAction( QAction *action );

  /**
   * Shows version and copyright.
   */
  void slotShowAbout();

protected:
  /**
   * Catch the close event from QWidget.
   */
  virtual void closeEvent( QCloseEvent *event );

  /**
   * Writes the window geometry, statusbar and toolbar state and the
   * layout state of the doc widgets.
   */
  void saveOptions();

  /**
   * Reads the window geometry, statusbar and toolbar state and the
   * layout state of the dock widgets.
   */
  void readOptions();

private:
  /** Initializes all QDockWindows */
  void createDockWindows();
  /** Initializes QMenuBar */
  void createMenuBar();
  /** Initializes the QStatusBar */
  void createStatusBar();
  /** Initializes toolbar*/
  void createToolBar();

  /**
   * Dockwidget to handle the dataview widget.
   * The dataview widget. Embedded in flightDataViewDock
   */
  QDockWidget* flightDataViewDock;
  DataView* dataView;
  /**
   * Dockwidget to handle the EvaluationWindow.
   * The evalutionWindow. Embedded in evaluationWindowDock
   */
  QDockWidget* evaluationWindowDock;
  EvaluationDialog* evaluationWindow;
  /**
   * Dockwidget to handle the helpWindow.
   * The helpWindow. Embedded in helpWindowDock
   */
  QDockWidget* helpWindowDock;
  HelpWindow* helpWindow;
  /**
   * Dockwidget to handle the legend-widget.
   *
   * @see TopoLegend
   */
  QDockWidget* legendDock;
  TopoLegend* legend;
  /**
   * Dockwidget to handle the map.
   * The map-widget.
   */
  QDockWidget* mapViewDock;
  Map* map;
  /**
   * Dockwidget to handle the map control.
   * The map control widget. Embedded in mapControlDock
   */
  QDockWidget* mapControlDock;
  MapControlView* mapControl;
  /**
   * Dockwidget to handle the object view
   *
   * @see ObjectView
   */
  QDockWidget* objectTreeDock;
  ObjectTree* objectTree;
  /**
   * Dockwidget to handle the waypoint tree view widget.
   * The waypoint tree view widget.
   */
  QDockWidget* waypointsDock;
  WaypointTreeView* waypointTreeView;

  QToolBar* toolBar;

  /**
   * Actions for the menu File
   */
  QAction* fileNewWaypointAction;
  QAction* fileNewTaskAction;
  QAction* fileNewFlightGroupAction;
  QAction* fileOpenFlightAction;
  QAction* fileOpenTaskAction;
  QMenu*   fileOpenRecentMenu;
  QAction* fileCloseAction;
  QAction* fileSavePixmapAction;
  QAction* filePrintAction;
  QAction* filePrintFlightAction;
  QAction* fileOpenRecorderAction;
  QAction* fileQuitAction;
  /**
   * Actions for the menu View
   */
  QAction* viewCenterTaskAction;
  QAction* viewCenterFlightAction;
  QAction* viewCenterHomesiteAction;
  QAction* viewCenterToAction;
  QAction* viewZoomInAction;
  QAction* viewZoomOutAction;
  QAction* viewZoomAction;
  QAction* viewRedrawAction;
  QAction* viewMoveNWAction;
  QAction* viewMoveNAction;
  QAction* viewMoveNEAction;
  QAction* viewMoveWAction;
  QAction* viewMoveEAction;
  QAction* viewMoveSWAction;
  QAction* viewMoveSAction;
  QAction* viewMoveSEAction;
  /**
   * Actions for the menu Flight
   */
  QAction* flightEvaluationWindowAction;
  QAction* flightOptimizationAction;
  QAction* flightOptimizationOLCAction;
  QAction* flightIgc3DAction;
  QAction* flightIgcOpenGLAction;
  QAction* flightAnimateStartAction;
  QAction* flightAnimateStopAction;
  QAction* flightAnimateNextAction;
  QAction* flightAnimatePrevAction;
  QAction* flightAnimate10NextAction;
  QAction* flightAnimate10PrevAction;
  QAction* flightAnimateHomeAction;
  QAction* flightAnimateEndAction;

  QActionGroup* flightDataTypeGroupAction;
  QAction *altitudeAction;
  QAction *cyclingAction;
  QAction *speedAction;
  QAction *varioAction;
  QAction *solidAction;

  /**
   * Action for the menu Window
   */
  QMenu* windowMenu;
  /**
   * Actions for the menu Settings
   */
  // QAction* settingsEvaluationWindow;
  QAction* settingsFlightDataAction;
  QAction* settingsHelpWindowAction;
  QAction* settingsLegendAction;
  QAction* settingsMapAction;
  QAction* settingsMapControlAction;
  QAction* settingsObjectTreeAction;
  QAction* settingsStatusBarAction;
  QAction* settingsToolBarAction;
  QAction* settingsWaypointsAction;

  /**
   * The progressbar in the statusbar. Used during drawing the map to display
   * the percentage of what is already drawn.
   *
   * @see slotSetProgress
   */
  QProgressBar* statusProgress;
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
   * The label to display the latitude of the position under the mouse cursor.
   *
   * @see slotShowPointInfo
   */
  QLabel* statusLatL;
  /**
   * The label to display the longitude of the position under the mouse cursor.
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

#endif // MAIN_WINDOW_H
