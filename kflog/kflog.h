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
class MapConfig;
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

  friend class KFLogView;

  public:
    /** */
    KFLogApp(QWidget* parent=0, const char* name=0);
    /** */
    ~KFLogApp();
    /** */
    void showPointInfo(QPoint, struct flightPoint* point);
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
    void slotToggleMap();
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
    /** */
    void slotHideMapControlDock();
    /** */
    void slotHideMapViewDock();
    /** */
    void slotHideDataViewDock();
    /** */
    void slotCheckDockWidgetStatus();

  private:
    /** the configuration object of the application */
    KConfig *config;
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
    KAction* fileClose;
    KRecentFilesAction* fileOpenRecent;
//    KAction* filePrint;
    KToggleAction* viewToolBar;
    KToggleAction* viewStatusBar;
    KAction* viewRedraw;
    KAction* viewCenterTask;
    KAction* viewCenterFlight;
//    KAction* viewCenterHome;
    KToggleAction* viewData;
    KToggleAction* viewMapControl;
    KToggleAction* viewMap;
    KAction* flightEvaluation;
//    KAction* flightOptimization;
    /** */
    Map* map;
    /** */
    QString flightDir;
    KFLogStartLogo* startLogo;
    bool showStartLogo;
    MapConfig* mapConfig;
};
 
#endif // KFLOG_H
