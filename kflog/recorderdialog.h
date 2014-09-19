/***********************************************************************
**
**   recorderdialog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RECORDER_DIALOG_H
#define RECORDER_DIALOG_H

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "flightrecorderpluginbase.h"
#include "flighttask.h"
#include "frstructs.h"
#include "kflogtreewidget.h"
#include "waypointcatalog.h"

/**
 * \class RecorderDialog
 *
 * \brief Flight recorder dialog
 *
 * Provides a dialog window for working with the flight recorder.
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \date 2002-2011
 *
 * \version $Id$
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
  /**
   * Called, if the menu tree is clicked.
   */
  void slotPageClicked( QTreeWidgetItem * item, int column );
  /**
   * Read the flight list from the recorder
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
   * Exports the declaration to the file
   */
  void slotExportDeclaration();
  /**
   * Close the connection with the recorder
   */
  void slotCloseRecorder();
  /**
   * Reads the data on things like pilot names from the flight recorder
   */
  void slotReadDatabase();
  /**
   * Writes the data on things like glider polar and variometer settings
   */
  void slotWriteConfig();
  /**
   * Reads the waypoint list from the recorder
   */
  void slotReadWaypoints();
  /**
   * Writes the waypoint list to the recorder
   */
  void slotWriteWaypoints();
  /**
   */
  void slotDisablePages();
  /**
   */
  void slotNewSpeed(int);

 private slots:

  void slotSwitchTask(int idx);

  /** Enable/Disable pages when not connected to a recorder */
  void slotEnablePages();

  /** No descriptions */
  void slotRecorderTypeChanged(const QString &name);

 signals:

  void addCatalog(WaypointCatalog *w);

  void addTask(FlightTask *t);

 public:

  /**
   * \return Path to the directory, where are the known loggers defined.
   */
  QString getLoggerPath();

  /**
   * \return Path to the directory, where are the plugin libraries are located.
   */
  QString getLibraryPath();

 private:

  /** */
  int __fillDirList();

  /**
   * Opens the library with the indicated name
   */
  bool __openLib(const QString& libN);

  void __setRecorderConnectionType(FlightRecorderPluginBase::TransferMode);
  void __setRecorderCapabilities();

  /**
   * Creates and adds the Recorder page to the dialog
   */
  void __createRecorderPage();

  /**
   * Creates and adds the Flight page to the dialog
   */
  void __createFlightPage();

  /**
   * Creates and adds the Declaration page to the dialog
   */
  void __createDeclarationPage();

  /**
   * Creates and adds the Tasks page to the dialog
   */
  void __createTaskPage();

  /**
   * Creates and adds the Waypoints page to the dialog
   */
  void __createWaypointPage();

  /**
   * Creates and adds the Configuration page to the dialog
   */
  void __createConfigurationPage();

  void fillTaskList( QList<FlightTask *>& ftList );

  void fillWaypointList( QList<Waypoint *>& wpList );

  QGridLayout *configLayout;

  QTreeWidget *setupTree;

  /** Page widgets */
  QWidget *activePage;
  QWidget* flightPage;
  QWidget* recorderPage;
  QWidget* waypointPage;
  QWidget* taskPage;
  QWidget* declarationPage;
  QWidget* configPage;

  QLabel* statusBar;

  /** */
  QComboBox* selectType;
  QComboBox* selectPort;
  QLabel* selectPortLabel;
  QComboBox* selectSpeed;
  QLabel* selectSpeedLabel;
  QLineEdit* selectURL;
  QLabel* selectURLLabel;
  QPushButton* cmdConnect;

  /** */
  QLabel* serID;
  QLabel* lblSerID;
  QLabel* dvcID;
  QLabel* lblDvcID;
  QLabel* swVersion;
  QLabel* lblSwVersion;
  QLabel* apiID;
  QLabel* lblApiID;
  QLabel* recType;
  QLabel* lblRecType;
  QLineEdit* pltName;
  QLineEdit* coPltName;
  QLabel* lblPltName;
  QLabel* lblCoPltName;
  QLineEdit* gldType;
  QLabel* lblGldType;
  QLineEdit* gldID;
  QLabel* lblGldID;
  QLineEdit* compID;
  QLabel* lblCompID;

  QLabel* lblTaskList;
  QLabel* lblWpList;

  /** Flight list for the flight page. */
  KFLogTreeWidget* flightList;

  /** Task declaration list */
  KFLogTreeWidget* declarationList;

  /** Task list for the tasks page */
  KFLogTreeWidget* taskList;

  /** Waypoint list for the waypoints page */
  KFLogTreeWidget *waypointList;

  /** */
  QCheckBox* useFastDownload;
  /** */
  QCheckBox* useLongNames;

  /** Handle to the bound library plugin. */
  void* libHandle;
  /** */
  QString libName;
  /** */
  QString portName;

  /** */
  QList<FRDirEntry *> dirList;
  QList<FlightTask *> tasks;
  QList<Waypoint *> waypoints;

  /**
   * Contains a list of library names which can be accessed using the
   * displayed name from the drop down as a key.
   */
  QMap<QString, QString> libNameList;

  /** Column variables of flight page */
  int colNo;
  int colDate;
  int colPilot;
  int colGlider;
  int colFirstPoint;
  int colLastPoint;
  int colDuration;
  int colDummy;

  /** Column variables of declaration page */
  int declarationColNo;
  int declarationColName;
  int declarationColLat;
  int declarationColLon;
  int declarationColDummy;

  /** Column variables of task page */
  int taskColNo;
  int taskColName;
  int taskColDesc;
  int taskColTask;
  int taskColTotal;
  int taskColDummy;

  /** Column variables of waypoint page */
  int waypointColNo;
  int waypointColName;
  int waypointColLat;
  int waypointColLon;
  int waypointColDummy;

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
  
  QPushButton* cmdUlDeclaration;
  QPushButton* cmdExportDeclaration;

  QPushButton* cmdUploadBasicConfig;
  QPushButton* cmdUploadConfig;

  /**
   * Used by declaration page
   */
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

  /**
   * Reference to the connected flight recorder plugin.
   */
  FlightRecorderPluginBase* activeRecorder;
};

#endif
