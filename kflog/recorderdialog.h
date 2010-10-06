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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qdict.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qwidget.h>

#include "flightrecorderpluginbase.h"
#include "flighttask.h"
#include "frstructs.h"
#include "guicontrols/kfloglistview.h"
#include "waypointcatalog.h"
#include "waypointlist.h"

/**
 * Provides a dialog-window for accessing the flightrecorder.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class RecorderDialog : public QDialog
{
  Q_OBJECT

  public:
    /**
     * Constructor
     */
    RecorderDialog(QWidget *parent, const char *name);
    /**
     * Destructor
     */
    ~RecorderDialog();

  public slots:
    /**
     * Connects the currently selected recorder using the information entered (bautrate, port, URL, etc.)
     */
    void slotConnectRecorder();
    void slotPageChanged(QListViewItem *currentItem);
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
    QListView *setupTree;
    QFrame *activePage;
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
    QPtrList<FRDirEntry> dirList;
    QPtrList<FlightTask> *tasks;
    WaypointList *waypoints;
    /**
     * Contains a list of librarynames wich can be accessed using the displayed name from the dropdown as a key
     */
    QDict<QString> libNameList;
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

    QButtonGroup* unitAltButtonGroup;
    QButtonGroup* unitVarioButtonGroup;
    QButtonGroup* unitSpeedButtonGroup;
    QButtonGroup* unitQNHButtonGroup;
    QButtonGroup* unitTempButtonGroup;
    QButtonGroup* unitDistButtonGroup;

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
