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
 * @author Heiner Lamprecht, Florian Ehinger
 * @version $Id$
 */
class KFLogApp : public KDockMainWindow
{
  Q_OBJECT

  friend class KFLogView;

  public:
    /** */
    KFLogApp(QWidget* parent=0, const char* name=0);
    /** */
    ~KFLogApp();
    /** */
    void showPointInfo(QPoint, struct flightPoint* point);
    /** display the coordinates in the statusbar*/
//    void showCoords(QPoint);
    /** */
    void clearPointInfo(QPoint);

  protected:
    /** */
    void saveOptions();
    /** */
    void readOptions();
    /** */
    void initActions();
    /** */
    void initStatusBar();
    /** */
    void initView();
    /**  */
    virtual bool queryExit();

  public slots:
    /** */
    void slotFileOpen();
    /** */
    void slotFileOpenRecent(const KURL& url);
    /** */
    void slotFileClose();
    /** */
    void slotFilePrint();
    /** */
    void slotFileQuit();
    /** */
    void slotViewToolBar();
    /** */
    void slotViewStatusBar();
    /** */
    void slotStatusMsg(const QString &text);
    /** */
    void slotToggleDataView();
    /** */
    void slotToggleMapControl();
    /** */
    void slotConfigureToolbars();
    /** */
    void slotConfigureKeyBindings();
    /** */
    void slotConfigureKFLog();
    /** */
    void slotNewToolbarConfig();
    /** */
    void slotSetProgress(int value);
    /** */
    void slotStartComplete();
    /** */
    void slotEvaluateFlight();

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
    KStatusBarLabel* statusTimeL;
    KStatusBarLabel* statusHeightL;
    KStatusBarLabel* statusVarioL;
    KStatusBarLabel* statusSpeedL;
    KStatusBarLabel* statusLatL;
    KStatusBarLabel* statusLonL;
    // KAction pointers to enable/disable actions
    KAction* fileOpen;
    KAction* fileClose;
    KRecentFilesAction* fileOpenRecent;
//    KAction* filePrint;
    KAction* fileQuit;
    KToggleAction* viewToolBar;
    KToggleAction* viewStatusBar;
    KAction* configToolBar;
    KAction* configKeyBindings;
    KAction* viewRedraw;
    KAction* viewCenterTask;
    KAction* viewCenterFlight;
    KAction* viewCenterHome;
//    KAction* viewZoomIn;
//    KAction* viewZoomOut;
    KToggleAction* viewData;
    KToggleAction* viewMapControl;
//    KAction* configKFLog;
    KAction* flightEvaluation;
//    KAction* flightOptimization;
    /** */
    Map* map;
    /** */
    QString flightDir;
    KFLogStartLogo* startLogo;
    bool showStartLogo;
};
 
#endif // KFLOG_H
