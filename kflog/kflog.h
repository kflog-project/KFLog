/***********************************************************************
**
**   kflog.h
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

#ifndef KFLOG_H
#define KFLOG_H
 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt

// include files for KDE
#include <kapp.h>
#include <kdockwidget.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kprogress.h>

class DataView;
class Map;
class MapCanvas;
class MapCanvasView;
class MapControlView;

/**
 * The base class for KFLog application windows. It sets up the main
 * window and reads the config file as well as providing a menubar, toolbar
 * and statusbar. An instance of KFLogView creates your center view, which is connected
 * to the window's Doc object.
 * KFLogApp reimplements the methods that KMainWindow provides for main window handling and supports
 * full session management as well as using KActions.
 * @see KDockMainWindow
 * @see KApplication
 * @see KConfig
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class KFLogApp : public KDockMainWindow
{
  Q_OBJECT

  friend class KFLogView;

  public:
    /** construtor of KFLogApp, calls all init functions to create the application.
     */
    KFLogApp(QWidget* parent=0, const char* name=0);
    /** */
    ~KFLogApp();
    /** */
    Map* getMap();

  protected:
    /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
     * file
     */ 	
    void saveOptions();
    /** read general Options again and initialize all variables like the recent file list
     */
    void readOptions();
    /** initializes the KActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */
    void initStatusBar();
    /* */
    void initMenuBar();
    /** creates the centerwidget of the KTMainWindow instance and sets it as the view
     */
    void initView();
    /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
     * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's	
     * properties.
     * @see KTMainWindow#queryExit
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryExit();

  public slots:
    /** open a new application window by creating a new instance of KFLogApp */
//    void slotFileNewWindow();
    /** open a file and load it into the document*/
    void slotFileOpen();
    /** */
    void slotFileClose();
    /** opens a file from the recent files menu */
    void slotFileOpenRecent(const KURL& url);
    /** print the actual file */
    void slotFilePrint();
    /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
     * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
     */
    void slotFileQuit();
    /** toggles the toolbar
     */
    void slotViewToolBar();
    /** toggles the statusbar
     */
    void slotViewStatusBar();
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const QString &text);
    /** */
    void slotToggleDataView();
    /** */
    void slotToggleMapControl();
    /** */
//    void slotCenterToHome();
    /** */
//    void slotCenterToFlight();
    /** */
//    void slotCenterToTask();
    /** */
//    void slotRedrawMap();
    /** */
//    void slotZoomIn();
    /** */
//    void slotZoomOut();
    /** */
    void slotConfigureToolbars();
    /** */
    void slotNewToolbarConfig();
    /** */
    void slotConfigureMap();
    /** */
    void slotSetProgress(int value);

  private:
    /** the configuration object of the application */
    KConfig *config;

    /** fileMenu contains all items of the menubar entry "File" */
    QPopupMenu* file_menu;
    /** viewMenu contains all items of the menubar entry "View" */
    QPopupMenu* viewMenu;
    /** mapMenu contains the items of the submenu "Layer" */
    QPopupMenu* mapMenu;
    /** confMenu contains all item of the menubar entry "Options" */
    QPopupMenu* confMenu;
    /** flightMenu contains all item of the menubar entry "Flight" */
    QPopupMenu* flightMenu;
    /** helpMenu contains all items of the menubar entry "Help" */
    QPopupMenu* helpMenu;

    /** */
    KDockWidget* mapControlDock;
    KDockWidget* mapViewDock;
    KDockWidget* dataViewDock;

    /** */
    MapControlView* mapControl;
    /** */
    DataView* dataView;

    /** */
    KProgress* statusProgress;
    KStatusBarLabel* statusLabel;

    // KAction pointers to enable/disable actions
//    KAction* fileNewWindow;
    KAction* fileOpen;
    KAction* fileClose;
    KRecentFilesAction* fileOpenRecent;
    KAction* filePrint;
    KAction* fileQuit;
    KToggleAction* viewToolBar;
    KToggleAction* viewStatusBar;
    KAction* configToolBar;
    KAction* viewRedraw;
    KAction* viewCenterTask;
    KAction* viewCenterFlight;
    KAction* viewCenterHome;
    KAction* viewZoomIn;
    KAction* viewZoomOut;
    KToggleAction* viewData;
    KToggleAction* viewMapControl;
    KAction* configMap;

    /** */
    Map* map;
};
 
#endif // KFLOG_H
