/***********************************************************************
**
**   recorderdialog.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RECORDERDIALOG_H
#define RECORDERDIALOG_H

#include <kcombobox.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <klineedit.h>

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qwidget.h>

#include "frstructs.h"
#include "waypointcatalog.h"
#include "flighttask.h"
#include "waypointlist.h"
#include "guicontrols/kfloglistview.h"
#include "flightrecorderpluginbase.h"

/**
 * Provides a dialog-window for accessing the flightrecorder.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class RecorderDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /** */
    RecorderDialog(QWidget *parent, KConfig* cnf, const char *name);
    /** */
    ~RecorderDialog();

  public slots:
    /** */
    void slotConnectRecorder();
    /** */
    void slotReadFlightList();
    /** */
    void slotDownloadFlight();
    /** */
    void slotWriteDeclaration();
    /** */
    void slotReadTasks();
    /** */
    void slotWriteTasks();
    /** */
    void slotCloseRecorder();
    /** */
    void slotReadDatabase();
    /** */
    void slotReadWaypoints();
    /** */
    void slotWriteWaypoints();
  private:
    /** */
    int __fillDirList();
    /** */
    int __openLib(const QString& libN);
    /** */
    void __addSettingsPage();
    void __setRecorderConnectionType(FlightRecorderPluginBase::TransferMode);
    void __setRecorderCapabilities();
    /** */
    void __addFlightPage();
    /** */
    void __addDeclarationPage();
    /** */
    void __addTaskPage();
    /** */
    void __addWaypointPage();
    /** */
    void fillTaskList();
  /** No descriptions */
  void __addConfigPage();
  /** No descriptions */
  void __addPilotPage();
    /** */
    QFrame* flightPage;
    /** */
    QFrame* settingsPage;
    /** */
    QFrame* waypointPage;
    /** */
    QFrame* taskPage;
    /** */
    QFrame* declarationPage;
    /** */
    QFrame* pilotPage;
    /** */
    QFrame* configPage;
    /** */
    KConfig* config;
    /** */
    KConfig* loggerConf;
    /** */
    KComboBox* selectType;
    KComboBox* selectPort;
    QLabel* selectPortLabel;
    KComboBox* selectBaud;
    QLabel* selectBaudLabel;
    KLineEdit* selectURL;
    QLabel* selectURLLabel;
    QPushButton* cmdConnect;
    
    /** */
    QLabel* serID;
    QLabel* lblSerID;
    QLabel* apiID;
    QLabel* lblApiID;
    QLabel* recType;
    QLabel* lblRecType;
    QLabel* pltName;
    QLabel* lblPltName;
    QLabel* gldType;
    QLabel* lblGldType;
    QLabel* gldID;
    QLabel* lblGldID;
    QLabel* compID;
    /** */
    KFLogListView* flightList;
    /** */
    KFLogListView* declarationList;
    /** */
    KFLogListView* taskList;
    /** */
    KFLogListView* waypointList;
    /** */
    QCheckBox* useFastDownload;
    /** */
    QCheckBox* useLongNames;
    /** */
    void* libHandle;
    /** */
    QString libName;
    /** */
    QString portName;
    /** */
    bool isOpen;
    /** */
    QPtrList<FRDirEntry> dirList;
    QPtrList<FlightTask> *tasks;
    WaypointList *waypoints;
    /** */
    QStringList libNameList;
    /** */
    int colID;
    int colDate;
    int colPilot;
    int colGlider;
    int colFirstPoint;
    int colLastPoint;
    /** */
    int declarationColID;
    int declarationColName;
    int declarationColLat;
    int declarationColLon;
    /** */
    int taskColID;
    int taskColName;
    int taskColDesc;
    int taskColTask;
    int taskColTotal;
    /** */
    int waypointColID;
    int waypointColName;
    int waypointColLat;
    int waypointColLon;

    KLineEdit* pilotName;
    KLineEdit* copilotName;
    KLineEdit* gliderID;
    KComboBox* gliderType;
    KLineEdit* compClass;
    KLineEdit* editCompID;
    KComboBox* taskSelection;

    QPushButton* cmdDownloadWaypoints;
    QPushButton* cmdUploadWaypoints;
    
    QPushButton* cmdDownloadTasks;
    QPushButton* cmdUploadTasks;
  /**  */
  bool isConnected;
  FlightRecorderPluginBase * activeRecorder;
  
  private slots:
    void slotSwitchTask(int idx);
  /** Enable/Disable pages when not connected to a recorder */
  void slotEnablePages();
  /** No descriptions */
  void slotRecorderTypeChanged(const QString &name);
signals: // Signals
  /** No descriptions */
  void addCatalog(WaypointCatalog *w);
  void addTask(FlightTask *t);
};

#endif
