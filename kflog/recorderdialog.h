/***********************************************************************
**
**   recorderdialog.h
**
**   This file is part of KFLog4.
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

#ifndef RECORDER_DIALOG_H
#define RECORDER_DIALOG_H

#include <QtGui>
#include <Qt3Support>

#include "flightrecorderpluginbase.h"
#include "flighttask.h"
#include "frstructs.h"
#include "guicontrols/kfloglistview.h"
#include "waypointcatalog.h"
//#include "waypointlist.h

/**
 * Provides a dialog-window for accessing the flight recorder.
 *
 * @author Heiner Lamprecht
 *
 * @version $Id$
 */
class RecorderDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( RecorderDialog )

  public:
    /**
     * Constructor
     */
    RecorderDialog(QWidget *parent);
    /**
     * Destructor
     */
    ~RecorderDialog();

  public slots:
    /**
     * Connects the currently selected recorder using the information entered (bautrate, port, URL, etc.)
     */
    void slotConnectRecorder();
    void slotPageChanged(Q3ListViewItem *currentItem);
    /**
     * Read the flightlist from the recorder
     */
    void slotReadFlightList();
    /**
     * Downloads the currently selected flight from the recorder. You need to call slotReadFlightList before calling this slot.
     */
    void slotDownloadFlight();
    /**
     * Sends a declaration to the recorder
     */
    void slotWriteDeclaration();
    /**
     * Reads the tasks from the recorder
     */
    void slotReadTasks();
    /**
     * Writes the tasks to the recorder
     */
    void slotWriteTasks();
    /**
     * Close the connection with the recorder
     */
    void slotCloseRecorder();
    /**
     * Reads the data on things like pilotnames from the flightrecorder
     */
    void slotReadDatabase();
    /**
     * Writes the data on things like glider polar and variometer settings
     */
    void slotWriteConfig();
    /**
     * Reads the waypointlist from the recorder
     */
    void slotReadWaypoints();
    /**
     * Writes the waypointlist to the recorder
     */
    void slotWriteWaypoints();
    /**
     */
    void slotDisablePages();
    /**
     */
    void slotNewSpeed(int);
    
  private:
    /** */
    int __fillDirList();
    /**
     * Opens the library with the indicated name
     */
    bool __openLib(const QString& libN);
    /**
     * Creates and add the Settings page (the first page) to the dialog
     */
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
    QGridLayout *configLayout;
    Q3ListView *setupTree;
    Q3Frame *activePage;
    /** */
    Q3Frame* flightPage;
    /** */
    Q3Frame* settingsPage;
    /** */
    Q3Frame* waypointPage;
    /** */
    Q3Frame* taskPage;
    /** */
    Q3Frame* declarationPage;
    /** */
    Q3Frame* pilotPage;
    /** */
    Q3Frame* configPage;
    /** */
    QComboBox* selectType;
    QComboBox* selectPort;
    QLabel* selectPortLabel;
    QComboBox* _selectSpeed;
    QLabel* selectSpeedLabel;
    QLineEdit* selectURL;
    QLabel* selectURLLabel;
    QPushButton* cmdConnect;
    
    /** */
    QLabel* serID;
    QLabel* lblSerID;
    QLabel* apiID;
    QLabel* lblApiID;
    QLabel* recType;
    QLabel* lblRecType;
    QLineEdit* pltName;
    QLabel* lblPltName;
    QLineEdit* gldType;
    QLabel* lblGldType;
    QLineEdit* gldID;
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
    QList<FRDirEntry*> dirList;
    QList<FlightTask*> *tasks;
    QList<Waypoint*> waypoints;
    /**
     * Contains a list of library names which can be accessed using the displayed name from the dropdown as a key
     */
    Q3Dict<QString> libNameList;
    /** */
    int colID;
    int colDate;
    int colPilot;
    int colGlider;
    int colFirstPoint;
    int colLastPoint;
    int colDuration;
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

    FlightRecorderPluginBase::FR_BasicData basicdata;
    FlightRecorderPluginBase::FR_ConfigData configdata;
    QLineEdit* pilotName;
    QLineEdit* copilotName;
    QLineEdit* gliderID;
    QComboBox* gliderType;
    QLineEdit* compClass;
    QLineEdit* editCompID;
    QComboBox* taskSelection;

    QPushButton* cmdDownloadWaypoints;
    QPushButton* cmdUploadWaypoints;
    
    QPushButton* cmdDownloadTasks;
    QPushButton* cmdUploadTasks;

    QPushButton* cmdUploadBasicConfig;
    QPushButton* cmdUploadConfig;

    Q3ButtonGroup* unitAltButtonGroup;
    Q3ButtonGroup* unitVarioButtonGroup;
    Q3ButtonGroup* unitSpeedButtonGroup;
    Q3ButtonGroup* unitQNHButtonGroup;
    Q3ButtonGroup* unitTempButtonGroup;
    Q3ButtonGroup* unitDistButtonGroup;

    QCheckBox* sinktone;

    QSpinBox* LD;
    QSpinBox* speedLD;
    QSpinBox* speedV2;
    QSpinBox* dryweight;
    QSpinBox* maxwater;

    QSpinBox* approachradius;
    QSpinBox* arrivalradius;
    QSpinBox* goalalt;
    QSpinBox* sloginterval;
    QSpinBox* floginterval;
    QSpinBox* gaptime;
    QSpinBox* minloggingspd;
    QSpinBox* stfdeadband;
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
