/***********************************************************************
**
**   recorderdialog.cpp
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

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <sys/stat.h>
#include <unistd.h>

#include <QtGui>

#include "gliders.h"
#include "mainwindow.h"
#include "mapcalc.h"
#include "mapcontents.h"
#include "recorderdialog.h"
#include "rowdelegate.h"
#include "wgspoint.h"

extern MainWindow  *_mainWindow;
extern MapConfig   *_globalMapConfig;
extern MapContents *_globalMapContents;
extern MapMatrix   *_globalMapMatrix;
extern QSettings    _settings;

RecorderDialog::RecorderDialog( QWidget *parent ) :
  QDialog(parent),
  libHandle(0),
  activeRecorder(0)
{
  setObjectName( "RecorderDialog" );
  setWindowTitle( tr("KFLog Flight Recorder") );
  setAttribute( Qt::WA_DeleteOnClose );
  setModal( true );
  setSizeGripEnabled( true );

  waypoints = _globalMapContents->getWaypointList();

  qSort(waypoints.begin(), waypoints.end());

  QList<BaseFlightElement *> *tList = _globalMapContents->getFlightList();

  for( int i = 0; i < tList->size(); i++ )
    {
      BaseFlightElement* element = tList->at(i);

      if( element->getObjectType() == BaseMapElement::Task )
        {
          tasks.append( dynamic_cast<FlightTask *> (element) );
        }
    }

  configLayout = new QGridLayout(this);

  setupTree = new QTreeWidget(this);
  setupTree->setRootIsDecorated( false );
  setupTree->setItemsExpandable( false );
  setupTree->setSortingEnabled( true );
  setupTree->setSelectionMode( QAbstractItemView::SingleSelection );
  setupTree->setSelectionBehavior( QAbstractItemView::SelectRows );
  setupTree->setColumnCount( 1 );
  setupTree->setFocusPolicy( Qt::StrongFocus );
  setupTree->setHeaderLabel( tr( "Menu" ) );

  // Set additional space per row
  RowDelegate* rowDelegate = new RowDelegate( setupTree, 10 );
  setupTree->setItemDelegate( rowDelegate );

  QTreeWidgetItem* headerItem = setupTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );

  configLayout->addWidget( setupTree, 0, 0 );

  connect( setupTree, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT( slotPageClicked( QTreeWidgetItem*, int )) );

  QPushButton *closeButton = new QPushButton(tr("&Close"), this);
  connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

  statusBar = new QLabel;
  statusBar->setMargin( 5 );

  QGridLayout* gridBox = new QGridLayout;
  gridBox->setSpacing( 0 );
  gridBox->addWidget( statusBar, 0, 0 );
  gridBox->setColumnStretch( 0, 10 );
  gridBox->addWidget( closeButton, 0, 1, Qt::AlignRight );

  configLayout->addLayout( gridBox, 1, 0, 1, 2 );

  waypointColNo    = 0;
  waypointColName  = 1;
  waypointColLat   = 2;
  waypointColLon   = 3;
  waypointColDummy = 4;

  declarationColNo    = 0;
  declarationColName  = 1;
  declarationColLat   = 2;
  declarationColLon   = 3;
  declarationColDummy = 4;

  __createConfigurationPage();
  __createDeclarationPage();
  __createFlightPage();
  __createWaypointPage();
  __createTaskPage();
  __createRecorderPage();

  setupTree->sortByColumn ( 0, Qt::AscendingOrder );
  setupTree->resizeColumnToContents( 0 );
  setupTree->setFixedWidth( 170 );

  activePage = recorderPage;
  recorderPage->setVisible( true );

  // activePage->setFixedWidth(685);
  // setFixedWidth(830);
  // setMinimumHeight(350);

  slotEnablePages();
  restoreGeometry( _settings.value("/RecorderDialog/Geometry").toByteArray() );
}

RecorderDialog::~RecorderDialog()
{
  _settings.setValue( "/RecorderDialog/Name", selectType->currentText() );
  _settings.setValue( "/RecorderDialog/Port", selectPort->currentIndex() );
  _settings.setValue( "/RecorderDialog/Baud", selectSpeed->currentIndex() );
  _settings.setValue( "/RecorderDialog/URL", selectURL->text() );
  _settings.setValue( "/RecorderDialog/Geometry", saveGeometry() );

  slotCloseRecorder();

  qDeleteAll( dirList );
}

QString RecorderDialog::getLoggerPath()
{
  QString _installRoot = _settings.value( "/Path/InstallRoot", ".." ).toString();

  return QString( _installRoot + "/logger" );
}

QString RecorderDialog::getLibraryPath()
{
  QString _installRoot = _settings.value( "/Path/InstallRoot", ".." ).toString();

  return QString( _installRoot + "/lib" );
}

void RecorderDialog::__createRecorderPage()
{
  int typeLoop = 0;

  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Recorder") );
  item->setData( 0, Qt::UserRole, "Recorder" );
  item->setIcon( 0, _mainWindow->getPixmap("kde_media-tape_48.png") );
  setupTree->addTopLevelItem( item );
  setupTree->setCurrentItem( item );

  recorderPage = new QWidget(this);
  recorderPage->setObjectName( "RecorderPage" );
  recorderPage->setVisible( false );

  configLayout->addWidget( recorderPage, 0, 1 );

  //----------------------------------------------------------------------------

  QGroupBox* sGroup = new QGroupBox( tr("Settings") );

  selectType = new QComboBox;

  connect( selectType, SIGNAL(activated(const QString &)), this,
           SLOT(slotRecorderTypeChanged(const QString &)) );

  selectPortLabel = new QLabel(tr("Port:"));

  selectPort = new QComboBox;
  selectPort->addItem("ttyS0");
  selectPort->addItem("ttyS1");
  selectPort->addItem("ttyS2");
  selectPort->addItem("ttyS3");
  // the following devices are used for usb adapters
  selectPort->addItem("ttyUSB0");   // classical device
  selectPort->addItem("tts/USB0");  // devfs
  selectPort->addItem("usb/tts/0"); // udev
  // bluetooth
  selectPort->addItem("rfcomm0"); // 
  // we never know if the device name will change again; let the user have a chance
  selectPort->setEditable(true);

  selectSpeedLabel = new QLabel(tr("Transfer speed:"));
  selectSpeed = new QComboBox;

  selectURLLabel = new QLabel(tr("URL:"));
  selectURL = new QLineEdit;

  cmdConnect = new QPushButton(tr("Connect to recorder"));
  cmdConnect->setMaximumWidth(cmdConnect->sizeHint().width() + 5);

  QGridLayout* sGridLayout = new QGridLayout;
  sGridLayout->setSpacing(10);

  int row = 0;
  int col = 0;
  sGridLayout->addWidget( new QLabel( tr("Type:")), row, col );
  sGridLayout->addWidget( selectType,               row, ++col );
  sGridLayout->addWidget( selectPortLabel,          ++row, col=0 );
  sGridLayout->addWidget( selectPort,               row, ++col );
  sGridLayout->addWidget( selectURLLabel,           ++row, col=0 );
  sGridLayout->addWidget( selectURL,                row, ++col );
  sGridLayout->addWidget( selectSpeedLabel,         ++row, col=0 );
  sGridLayout->addWidget( selectSpeed,              row, ++col );
  sGridLayout->setRowMinimumHeight(                 3, 5 );
  sGridLayout->addWidget( cmdConnect, 4, 0, 1, 4, Qt::AlignLeft );

  sGroup->setLayout( sGridLayout );

  //----------------------------------------------------------------------------
  QGroupBox* iGroup = new QGroupBox( tr("Info") );

  lblApiID = new QLabel(tr("API-Version:"));
  apiID = new QLabel;
  apiID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  apiID->setBackgroundRole( QPalette::Light );
  apiID->setAutoFillBackground( true );
  apiID->setEnabled(false);

  lblDvcID = new QLabel(tr("Device ID:"));
  dvcID = new QLabel;
  dvcID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  dvcID->setBackgroundRole( QPalette::Light );
  dvcID->setAutoFillBackground( true );
  dvcID->setEnabled(false);

  lblSwVersion = new QLabel(tr("Software Version:"));
  swVersion = new QLabel;
  swVersion->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  swVersion->setBackgroundRole( QPalette::Light );
  swVersion->setAutoFillBackground( true );
  swVersion->setEnabled(false);

  lblSerID = new QLabel(tr("Serial-No.:"));
  serID = new QLabel(tr("No recorder connected"));
  serID->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  serID->setBackgroundRole( QPalette::Light );
  serID->setAutoFillBackground( true );
  serID->setEnabled(false);

  lblRecType = new QLabel(tr("Recorder Type:"));
  recType = new QLabel;
  recType->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  recType->setBackgroundRole( QPalette::Light );
  recType->setAutoFillBackground( true );
  recType->setEnabled(false);

  lblPltName = new QLabel(tr("Pilot Name:"));
  pltName = new QLineEdit;
  pltName->setEnabled(false);

  lblCoPltName = new QLabel(tr("Copilot Name:"));
  coPltName = new QLineEdit;
  coPltName->setEnabled(false);

  lblGldType = new QLabel(tr("Glider Type:"));
  gldType = new QLineEdit;
  gldType->setEnabled(false);

  lblGldID = new QLabel(tr("Glider Id:"));
  gldID = new QLineEdit;
  gldID->setEnabled(false);

  lblCompID = new QLabel(tr("Competition Id:"));
  compID = new QLineEdit;
  compID->setEnabled(false);

  cmdUploadBasicConfig = new QPushButton(tr("Write data to recorder"));
  cmdUploadBasicConfig->setMaximumWidth(cmdUploadBasicConfig->sizeHint().width() + 5);

  // disable this button until we read the information from the flight recorder:
  cmdUploadBasicConfig->setEnabled(false);
  connect( cmdUploadBasicConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()) );

  QGridLayout* iGridLayout = new QGridLayout;
  iGridLayout->setSpacing(10);
  iGridLayout->addWidget( lblApiID,     row=0, col=0 );
  iGridLayout->addWidget( apiID,        row, ++col );
  iGridLayout->addWidget( lblPltName,   row, ++col );
  iGridLayout->addWidget( pltName,      row, ++col );

  iGridLayout->addWidget( lblSerID,     ++row, col=0 );
  iGridLayout->addWidget( serID,        row, ++col );
  iGridLayout->addWidget( lblCoPltName, row, ++col );
  iGridLayout->addWidget( coPltName,    row, ++col );

  iGridLayout->addWidget( lblRecType,   ++row, col=0 );
  iGridLayout->addWidget( recType,      row, ++col );
  iGridLayout->addWidget( lblGldID,     row, ++col );
  iGridLayout->addWidget( gldID,        row, ++col );
  
  iGridLayout->addWidget( lblDvcID,     ++row, col=0);
  iGridLayout->addWidget( dvcID,        row, ++col );
  iGridLayout->addWidget( lblGldType,   row, ++col );
  iGridLayout->addWidget( gldType,      row, ++col );

  iGridLayout->addWidget( lblSwVersion, ++row, col=0);
  iGridLayout->addWidget( swVersion,    row, ++col );
  iGridLayout->addWidget( lblCompID,    row, ++col );
  iGridLayout->addWidget( compID,       row, ++col );

  iGridLayout->setRowMinimumHeight(     row, ++col );
  iGridLayout->addWidget( cmdUploadBasicConfig, 5, 0, 1, 4, Qt::AlignLeft );
  iGridLayout->setColumnStretch(      1, 5 );
  iGridLayout->setColumnStretch(      3, 5 );

  iGroup->setLayout( iGridLayout );

  QVBoxLayout* recorderLayout = new QVBoxLayout;
  recorderLayout->setContentsMargins( 0, 0, 0, 0 );
  recorderLayout->addWidget( sGroup );
  recorderLayout->addSpacing( 20 );
  recorderLayout->addWidget( iGroup );
  recorderLayout->addStretch( 10 );

  recorderPage->setLayout( recorderLayout );

  __setRecorderConnectionType( FlightRecorderPluginBase::none );

  QDir path = QDir( getLoggerPath() );

  QStringList configRec = path.entryList( QStringList("*.desktop") );

  if( configRec.count() == 0 )
    {
      QMessageBox::critical( this,
                             tr("No recorders installed!"),
                             tr("There are no recorder-libraries installed."),
                             QMessageBox::Ok );
    }

  libNameList.clear();

  selectPort->setCurrentIndex( _settings.value("/RecorderDialog/Port", 0).toInt() );
  selectSpeed->setCurrentIndex( _settings.value("/RecorderDialog/Baud", 0).toInt() );

  QString name( _settings.value("/RecorderDialog/Name", "").toString() );

  selectURL->setText(_settings.value("/RecorderDialog/URL", "").toString() );

  for( int i = 0; i < configRec.size(); i++ )
    {
      QString pluginName = "";
      QString currentLibName = "";

      QFile settingFile( getLoggerPath() + "/" + configRec[i] );

      if( !settingFile.exists() )
        {
          continue;
        }

      if( !settingFile.open( QIODevice::ReadOnly ) )
        {
          continue;
        }

      QTextStream stream( &settingFile );

      while( ! stream.atEnd() )
        {
          QString lineStream = stream.readLine();

          if( lineStream.mid( 0, 5 ) == "Name=" )
            {
              pluginName = lineStream.remove( 0, 5 );
            }
          else if( lineStream.mid( 0, 8 ) == "LibName=" )
            {
              currentLibName = lineStream.remove( 0, 8 );
            }
        }

      settingFile.close();

      if( pluginName != "" && currentLibName != "" )
        {
          selectType->addItem( pluginName );
          libNameList.insert( pluginName, currentLibName );
          typeLoop++;

          if( name == "" )
            {
              name = pluginName;
            }
        }
    }

  // sort if this style uses a listbox for the combobox
  if( selectType->model() )
    {
      selectType->model()->sort( 0 );
    }

  selectType->setCurrentIndex( selectType->findText(name) );

  slotRecorderTypeChanged( selectType->currentText() );

  connect( cmdConnect, SIGNAL(clicked()), SLOT(slotConnectRecorder()) );
}

void RecorderDialog::__setRecorderConnectionType(FlightRecorderPluginBase::TransferMode mode)
{
  selectPort->hide();
  selectPortLabel->hide();
  selectSpeed->hide();
  selectSpeedLabel->hide();
  selectURL->hide();
  selectURLLabel->hide();
  cmdConnect->setEnabled(false);

  switch(mode)
    {
      case FlightRecorderPluginBase::serial:
        selectPort->show();
        selectPortLabel->show();
        selectSpeed->show();
        selectSpeedLabel->show();
        cmdConnect->setEnabled(true);
        break;
      case FlightRecorderPluginBase::URL:
        selectURL->show();
        selectURLLabel->show();
        cmdConnect->setEnabled(true);
        break;
      default:
        break; //nothing to be done.
    }
}

void RecorderDialog::__setRecorderCapabilities()
{
  if (!activeRecorder)
    return;
  FlightRecorderPluginBase::FR_Capabilities cap = activeRecorder->capabilities();

  serID->setVisible(cap.supDspSerialNumber);
  lblSerID->setVisible(cap.supDspSerialNumber);

  dvcID->setVisible(cap.supDspDvcID);
  lblDvcID->setVisible(cap.supDspDvcID);

  swVersion->setVisible(cap.supDspSwVersion);
  lblSwVersion->setVisible(cap.supDspSwVersion);

  recType->setVisible(cap.supDspRecorderType);
  lblRecType->setVisible(cap.supDspRecorderType);

  pltName->setVisible(cap.supDspPilotName);
  lblPltName->setVisible(cap.supDspPilotName);

  coPltName->setVisible(cap.supDspCoPilotName);
  lblCoPltName->setVisible(cap.supDspCoPilotName);

  gldType->setVisible(cap.supDspGliderType);
  lblGldType->setVisible(cap.supDspGliderType);

  gldID->setVisible(cap.supDspGliderID);
  lblGldID->setVisible(cap.supDspGliderID);

  compID->setVisible(cap.supDspGliderID);
  lblCompID->setVisible(cap.supDspGliderID);

  bool edit = cap.supEditGliderID | cap.supEditGliderType | cap.supEditPilotName | cap.supEditCoPilotName;

  pltName->setEnabled( edit );
  coPltName->setEnabled( edit );
  gldType->setEnabled( edit );
  gldID->setEnabled( edit );
  compID->setEnabled( edit );
  cmdUploadBasicConfig->setEnabled( edit );
  cmdUploadBasicConfig->setVisible( edit );

  selectSpeed->clear();
  selectSpeed->setEnabled( true );

  if( cap.supAutoSpeed )
    {
      selectSpeed->addItem( tr("Auto") );
      selectSpeed->setCurrentIndex( selectSpeed->count() - 1 );
    }

    {
      // insert highest speed first
      for (int i = FlightRecorderPluginBase::transferDataMax-1; i >= 0; i--)
        {
          if ((FlightRecorderPluginBase::transferData[i]._bps & cap.transferSpeeds) ||
              (cap.transferSpeeds == FlightRecorderPluginBase::bps00000))
            {
              selectSpeed->addItem(QString("%1").arg(FlightRecorderPluginBase::transferData[i]._speed));
            }
        }
    }
}

void RecorderDialog::__createFlightPage()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Flights") );
  item->setData( 0, Qt::UserRole, "Flights" );
  item->setIcon( 0, _mainWindow->getPixmap("igc_48.png") );
  setupTree->addTopLevelItem( item );

  flightPage = new QWidget(this);
  flightPage->setObjectName( "FlightPage" );
  flightPage->setVisible( false );

  configLayout->addWidget( flightPage, 0, 1 );

  //----------------------------------------------------------------------------

  flightList = new KFLogTreeWidget( "RecorderDialog-FlightList", this );

  flightList->setSortingEnabled( true );
  flightList->setAllColumnsShowFocus( true );
  flightList->setFocusPolicy( Qt::StrongFocus );
  flightList->setRootIsDecorated( false );
  flightList->setItemsExpandable( true );
  flightList->setSelectionMode( QAbstractItemView::NoSelection );
  flightList->setAlternatingRowColors( true );
  flightList->addRowSpacing( 5 );
  flightList->setColumnCount( 8 );

  QStringList headerLabels;

  headerLabels  << tr("No.")
                << tr("Date")
                << tr("Pilot")
                << tr("Glider")
                << tr("First Point")
                << tr("Last Point")
                << tr("Duration")
                << "";

  flightList->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = flightList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );
  headerItem->setTextAlignment( 5, Qt::AlignCenter );
  headerItem->setTextAlignment( 6, Qt::AlignCenter );

  colNo         = 0;
  colDate       = 1;
  colPilot      = 2;
  colGlider     = 3;
  colFirstPoint = 4;
  colLastPoint  = 5;
  colDuration   = 6;
  colDummy      = 7;

  flightList->loadConfig();

  QPushButton* loadB = new QPushButton( tr( "Load list" ) );
  connect( loadB, SIGNAL(clicked()), SLOT(slotReadFlightList()) );

  QPushButton* saveB = new QPushButton( tr( "Save flight" ) );
  connect( saveB, SIGNAL(clicked()), SLOT(slotDownloadFlight()) );

  useLongNames = new QCheckBox( tr( "Long filenames" ) );

  // let's prefer short filenames. These are needed for OLC
  useLongNames->setChecked( false );
  useLongNames->setToolTip( tr("If checked, long filenames are used.") );

  useFastDownload = new QCheckBox( tr( "Fast download" ) );
  useFastDownload->setChecked( true );

  useFastDownload->setToolTip(
                  tr("<html>If checked, the IGC-file will not be signed.<BR>"
                     "<b>Note!</b> Do not use fast download<BR>"
                     " when using the file for competitions.</html>"));

  QVBoxLayout *flightPageLayout = new QVBoxLayout;
  flightPageLayout->setSpacing(10);
  flightPageLayout->setContentsMargins( 0, 0, 0, 0 );
  flightPageLayout->addWidget( flightList );

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( 10 );
  buttonBox->addWidget(loadB);
  buttonBox->addStretch( 10 );
  buttonBox->addWidget( saveB );
  buttonBox->addStretch( 10 );
  buttonBox->addWidget(useLongNames);
  buttonBox->addStretch( 10 );
  buttonBox->addWidget(useFastDownload);

  flightPageLayout->addLayout( buttonBox );
  flightPage->setLayout( flightPageLayout );
}

void RecorderDialog::__createDeclarationPage()
{
  qDebug ("__createDeclarationPage");
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Declaration") );
  item->setData( 0, Qt::UserRole, "Declaration" );
  item->setIcon( 0, _mainWindow->getPixmap("declaration_48.png") );
  setupTree->addTopLevelItem( item );

  declarationPage = new QWidget(this);
  declarationPage->setObjectName( "DeclarationPage" );
  declarationPage->setVisible( false );

  configLayout->addWidget( declarationPage, 0, 1 );

  //----------------------------------------------------------------------------

  declarationList = new KFLogTreeWidget( "RecorderDialog-DeclarationList", this );

  declarationList->setSortingEnabled( true );
  declarationList->setAllColumnsShowFocus( true );
  declarationList->setFocusPolicy( Qt::StrongFocus );
  declarationList->setRootIsDecorated( false );
  declarationList->setItemsExpandable( true );
  declarationList->setSelectionMode( QAbstractItemView::NoSelection );
  declarationList->setAlternatingRowColors( true );
  declarationList->addRowSpacing( 5 );
  declarationList->setColumnCount( 5 );

  QStringList headerLabels;

  headerLabels  << tr("No.")
                << tr("Name")
                << tr("Latitude")
                << tr("Longitude")
                << "";

  declarationList->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = declarationList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  declarationList->loadConfig();

  taskSelection = new QComboBox;

  QHBoxLayout* taskBox = new QHBoxLayout;
  taskBox->addWidget( new QLabel(tr("Task:")) );
  taskBox->addWidget( taskSelection );
  taskBox->addStretch( 10 );

  pilotName = new QLineEdit;
  copilotName = new QLineEdit;
  gliderID = new QLineEdit;
  gliderType = new QComboBox;
  gliderType->setEditable(true);
  editCompID = new QLineEdit;
  compClass = new QLineEdit;

  QGridLayout* gliderGLayout = new QGridLayout;
  gliderGLayout->setMargin( 0 );

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( tr("Pilot:"), pilotName );
  formLayout->addRow( tr("Glider Id:"), gliderID );
  formLayout->addRow( tr("Competition Id:"), editCompID );
  gliderGLayout->addLayout( formLayout, 0, 0 );

  formLayout = new QFormLayout;
  if (!activeRecorder || activeRecorder->capabilities().supDspCoPilotName)
    formLayout->addRow( tr("Copilot:"), copilotName );
  formLayout->addRow( tr("Glider Type:"), gliderType );
  formLayout->addRow( tr("Competition Class:"), compClass );
  gliderGLayout->addLayout( formLayout, 0, 1 );

  cmdUlDeclaration = new QPushButton( tr("Write declaration to recorder") );
  cmdUlDeclaration->setMaximumWidth(cmdUlDeclaration->sizeHint().width() + 15);

  cmdExportDeclaration = new QPushButton(tr("Export declaration to file"));

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( 0 );
  buttonBox->addWidget(cmdUlDeclaration);
  buttonBox->addWidget(cmdExportDeclaration);
  buttonBox->addStretch( 10 );

  QVBoxLayout *decPageLayout = new QVBoxLayout;
  decPageLayout->setContentsMargins( 0, 0, 0, 0 );
  decPageLayout->setSpacing(10);
  decPageLayout->addLayout( taskBox );
  decPageLayout->addWidget( declarationList );
  decPageLayout->addLayout( gliderGLayout );
  decPageLayout->addLayout( buttonBox );

  declarationPage->setLayout( decPageLayout );

  int idx = 0;

  while( gliderList[idx].index != -1 )
    {
      gliderType->addItem( QString( gliderList[idx++].name ) );
    }

  pilotName->setText( _settings.value("/PersonalData/PilotName", "").toString() );

  for( int i = 0; i < tasks.size(); i++ )
    {
      FlightTask *task = tasks.at(i);
      taskSelection->addItem(task->getFileName() + " " + task->getTaskTypeString());
    }

  if( tasks.count() )
    {
      slotSwitchTask( 0 );
    }
  else
    {
      qWarning( "No tasks planned ..." );

      // Isn't it possible to write an declaration without a task?
      cmdUlDeclaration->setEnabled( false );
      cmdExportDeclaration->setEnabled( false );
    }

  connect(taskSelection, SIGNAL(activated(int)), SLOT(slotSwitchTask(int)));
  connect(cmdUlDeclaration, SIGNAL(clicked()), SLOT(slotWriteDeclaration()));
  connect(cmdExportDeclaration, SIGNAL(clicked()), SLOT(slotExportDeclaration()));
}

void RecorderDialog::__createTaskPage()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Tasks") );
  item->setData( 0, Qt::UserRole, "Tasks" );
  item->setIcon( 0, _mainWindow->getPixmap("task_48.png") );
  setupTree->addTopLevelItem( item );

  taskPage = new QWidget;
  taskPage->setObjectName( "TaskPage" );
  taskPage->setVisible( false );

  configLayout->addWidget( taskPage, 0, 1 );

  //----------------------------------------------------------------------------

  taskList = new KFLogTreeWidget( "RecorderDialog-TaskList", this );

  taskList->setSortingEnabled( true );
  taskList->setAllColumnsShowFocus( true );
  taskList->setFocusPolicy( Qt::StrongFocus );
  taskList->setRootIsDecorated( false );
  taskList->setItemsExpandable( true );
  taskList->setSelectionMode( QAbstractItemView::NoSelection );
  taskList->setAlternatingRowColors( true );
  taskList->addRowSpacing( 5 );
  taskList->setColumnCount( 6 );

  QStringList headerLabels;

  headerLabels  << tr("No.")
                << tr("Name")
                << tr("Description")
                << tr("Distance")
                << tr("Total distance")
                << "";

  taskList->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = taskList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  taskColNo    = 0;
  taskColName  = 1;
  taskColDesc  = 2;
  taskColTask  = 3;
  taskColTotal = 4;
  taskColDummy = 5;

  taskList->loadConfig();

  cmdUploadTasks = new QPushButton(tr("Write tasks to recorder"));
  connect(cmdUploadTasks, SIGNAL(clicked()), SLOT(slotWriteTasks()));

  cmdDownloadTasks = new QPushButton(tr("Read tasks from recorder"));
  connect(cmdDownloadTasks, SIGNAL(clicked()), SLOT(slotReadTasks()));

  lblTaskList = new QLabel;

  QVBoxLayout *taskPageLayout = new QVBoxLayout;
  taskPageLayout->setContentsMargins( 0, 0, 0, 0 );
  taskPageLayout->setSpacing(10);
  taskPageLayout->addWidget( taskList );

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( 10 );
  buttonBox->addWidget(cmdUploadTasks);
  buttonBox->addStretch( 10 );
  buttonBox->addWidget( lblTaskList );
  buttonBox->addStretch( 10 );
  buttonBox->addWidget(cmdDownloadTasks);

  taskPageLayout->addLayout( buttonBox );
  taskPage->setLayout( taskPageLayout );

  fillTaskList( tasks );
}

void RecorderDialog::fillTaskList( QList<FlightTask *>& ftList )
{
  taskList->clear();

  for( int i = 0; i < ftList.size(); i++ )
    {
      FlightTask* task = ftList.at(i);

      QTreeWidgetItem *item = new QTreeWidgetItem;

      item->setText(taskColNo, QString("%1").arg( i + 1));
      item->setText(taskColName, task->getFileName());
      item->setText(taskColDesc, task->getTaskTypeString());
      item->setText(taskColTask, task->getTaskDistanceString() );
      item->setText(taskColTotal, task->getTotalDistanceString() );
      item->setText(taskColDummy, "");

      item->setTextAlignment( taskColNo, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( taskColName, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( taskColDesc, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( taskColTask, Qt::AlignCenter );
      item->setTextAlignment( taskColTotal, Qt::AlignRight|Qt::AlignVCenter );

      taskList->insertTopLevelItem( i, item );
    }

  taskList->slotResizeColumns2Content();
}

void RecorderDialog::__createWaypointPage()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr("Waypoints") );
  item->setData( 0, Qt::UserRole, "Waypoints" );
  item->setIcon( 0, _mainWindow->getPixmap("waypoint_48.png") );
  setupTree->addTopLevelItem( item );

  waypointPage = new QWidget(this);
  waypointPage->setObjectName( "WaypointsPage" );
  waypointPage->setVisible( false );

  configLayout->addWidget( waypointPage, 0, 1 );

  //----------------------------------------------------------------------------

  waypointList = new KFLogTreeWidget( "RecorderDialog-WaypointList", this );

  waypointList->setSortingEnabled( true );
  waypointList->setAllColumnsShowFocus( true );
  waypointList->setFocusPolicy( Qt::StrongFocus );
  waypointList->setRootIsDecorated( false );
  waypointList->setItemsExpandable( true );
  waypointList->setSelectionMode( QAbstractItemView::NoSelection );
  waypointList->setAlternatingRowColors( true );
  waypointList->addRowSpacing( 5 );
  waypointList->setColumnCount( 5 );

  QStringList headerLabels;

  headerLabels  << tr("No.")
                << tr("Name")
                << tr("Latitude")
                << tr("Longitude")
                << "";

  waypointList->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = waypointList->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );

  waypointList->loadConfig();

  cmdUploadWaypoints = new QPushButton(tr("Write waypoints to recorder"));
  connect(cmdUploadWaypoints, SIGNAL(clicked()), SLOT(slotWriteWaypoints()));

  cmdDownloadWaypoints = new QPushButton(tr("Read waypoints from recorder"));
  connect(cmdDownloadWaypoints, SIGNAL(clicked()), SLOT(slotReadWaypoints()));

  lblWpList = new QLabel;

  QVBoxLayout *wpPageLayout = new QVBoxLayout;
  wpPageLayout->setContentsMargins( 0, 0, 0, 0 );
  wpPageLayout->setSpacing(10);
  wpPageLayout->addWidget( waypointList );

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( 0 );
  buttonBox->addWidget(cmdUploadWaypoints);
  buttonBox->addStretch( 10 );
  buttonBox->addWidget( lblWpList );
  buttonBox->addStretch( 10 );
  buttonBox->addWidget(cmdDownloadWaypoints);

  wpPageLayout->addLayout( buttonBox );
  waypointPage->setLayout( wpPageLayout );

  fillWaypointList( waypoints );
}

void RecorderDialog::fillWaypointList( QList<Waypoint *>& wpList )
{
  waypointList->clear();

  for( int i = 0; i < wpList.size(); i++ )
    {
      Waypoint *wp = wpList.at(i);

      QTreeWidgetItem *item = new QTreeWidgetItem;

      item->setIcon(waypointColName, _globalMapConfig->getPixmap(wp->type, false, true) );

      item->setText(waypointColNo, QString("%1").arg( i + 1));
      item->setText(waypointColName, wp->name);
      item->setText(waypointColLat, WGSPoint::printPos(wp->origP.lat(), true));
      item->setText(waypointColLon, WGSPoint::printPos(wp->origP.lon(), false));
      item->setText(waypointColDummy, "");

      item->setTextAlignment( waypointColNo, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( waypointColName, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( waypointColLat, Qt::AlignCenter );
      item->setTextAlignment( waypointColLon, Qt::AlignCenter );

      waypointList->insertTopLevelItem( i, item );
    }

  waypointList->slotResizeColumns2Content();
  // waypointList->sortByColumn(waypointColName, Qt::AscendingOrder);
}

void RecorderDialog::slotConnectRecorder()
{
  if( !activeRecorder )
    {
      return;
    }

  portName = "/dev/" + selectPort->currentText();

  QString name= libNameList[selectType->currentText()];

  int speed = 0;

  speed = selectSpeed->currentText().toInt();

  if( ! __openLib( name ) )
    {
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Connecting to recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  // check if we have valid parameters, is that true, try to connect!
  switch( activeRecorder->getTransferMode() )
  {

    case FlightRecorderPluginBase::serial:

      if( portName.isEmpty() )
          {
            qWarning() << "slotConnectRecorder(): Missing port!";
            break;
          }

      activeRecorder->openRecorder( portName.toLatin1().data(), speed );
      break;

    case FlightRecorderPluginBase::URL:
    {
      selectURL->setText( selectURL->text().trimmed() );

      QString URL = selectURL->text();

      if( URL.isEmpty() )
          {
            qWarning() <<  "slotConnectRecorder(): Missing URL!";
            break;
          };

      activeRecorder->openRecorder( URL );
      break;
    }

  default:

    QApplication::restoreOverrideCursor();
    statusBar->setText( "" );
    return;
  }

  if( activeRecorder->isConnected() )
    {
      connect(activeRecorder, SIGNAL(newSpeed(int)),this,SLOT(slotNewSpeed(int)));
      slotEnablePages();
      slotReadDatabase();
      QApplication::restoreOverrideCursor();
    }
  else
    {
      QApplication::restoreOverrideCursor();

      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Sorry, could not connect to recorder.\n"
                              "Please check connections and settings.\n");

      if( ! errorDetails.isEmpty() )
        {
          errorText += errorDetails;
        }

      QMessageBox::warning( this,
                            tr("Recorder Connection"),
                            errorText,
                            QMessageBox::Ok );
    }

  statusBar->setText( "" );
}

void RecorderDialog::slotCloseRecorder()
{
  if( activeRecorder )
    {
      QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

      statusBar->setText( tr("Closing connection to recorder") );
      QCoreApplication::processEvents();
      QCoreApplication::flush();

      qDebug( "A recorder is active. Checking connection." );

      if( activeRecorder->isConnected() )
        {
          qDebug( "Recorder is connected. Closing..." );

          activeRecorder->closeRecorder();
        }

      qDebug( "Going to delete recorder object..." );
      delete activeRecorder;
      activeRecorder = 0;
      qDebug( "Done." );

      QApplication::restoreOverrideCursor();
      statusBar->setText( "" );
    }
}

void RecorderDialog::slotPageClicked( QTreeWidgetItem * item, int column )
{
  Q_UNUSED( column );

  QString itemText = item->data( 0, Qt::UserRole ).toString();

  // qDebug() << "RecorderDialog::slotPageClicked(): Page=" << itemText;

  activePage->setVisible( false );

  if( itemText == "Configuration" )
    {
      configPage->setVisible( true );
      activePage = configPage;
    }
  else if( itemText == "Declaration" )
    {
      declarationPage->setVisible( true );
      activePage = declarationPage;
    }
  else if( itemText == "Flights" )
    {
      flightPage->setVisible( true );
      activePage = flightPage;
    }
  else if( itemText == "Recorder" )
    {
      recorderPage->setVisible( true );
      activePage = recorderPage;
    }
  else if( itemText == "Tasks" )
    {
      taskPage->setVisible( true );
      activePage = taskPage;
    }
  else if( itemText == "Waypoints" )
    {
      waypointPage->setVisible( true );
      activePage = waypointPage;
    }
  else
    {
      activePage->setVisible( true );

      qWarning() << "RecorderDialog::slotPageClicked: Unknown item"
                 << itemText
                 << "received!";
    }
}

void RecorderDialog::slotReadFlightList()
{
  if( !activeRecorder )
    {
      return;
    }

  if( !activeRecorder->capabilities().supDlFlight )
    {
      QMessageBox::warning( this,
                            tr("Flight download"),
                            tr("Function not implemented"),
                            QMessageBox::Ok );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Reading flights from recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  flightList->clear();

  int ret = __fillDirList();

  int error = 0;

  if( ret < FR_OK )
    {
      QApplication::restoreOverrideCursor();

      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot read flights from recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
      error++;
    }

  if( dirList.count() == 0 )
    {
      QApplication::restoreOverrideCursor();

      QMessageBox::information( this,
                                tr( "Download result" ),
                                tr( "No flights are stored in the recorder." ),
                                QMessageBox::Ok );
      error++;
    }

  if( error )
    {
      QApplication::restoreOverrideCursor();
      statusBar->setText("");
      return;
    }

  for( int i = 0; i < dirList.size(); i++ )
    {
      QString day;
      FRDirEntry* dirListItem = dirList.at(i);

      QTreeWidgetItem *item = new QTreeWidgetItem;

      item->setText(colNo, QString("%1").arg( i + 1));

      day.sprintf( "%d-%.2d-%.2d",
                   dirListItem->firstTime.tm_year + 1900,
                   dirListItem->firstTime.tm_mon + 1,
                   dirListItem->firstTime.tm_mday );
      item->setText(colDate, day);

      item->setText(colPilot, dirListItem->pilotName);
      item->setText(colGlider, dirListItem->gliderID);

      QTime time( dirListItem->firstTime.tm_hour,
                  dirListItem->firstTime.tm_min,
                  dirListItem->firstTime.tm_sec );

      item->setText(colFirstPoint, time.toString("hh:mm"));

      time = QTime( dirListItem->lastTime.tm_hour,
                    dirListItem->lastTime.tm_min,
                    dirListItem->lastTime.tm_sec );

      item->setText(colLastPoint, time.toString("hh:mm"));

      time = QTime().addSecs (dirListItem->duration);
      item->setText(colDuration, time.toString("hh:mm"));

      item->setTextAlignment( colNo, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( colDate, Qt::AlignCenter );
      item->setTextAlignment( colPilot, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( colGlider, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( colFirstPoint, Qt::AlignCenter );
      item->setTextAlignment( colLastPoint, Qt::AlignCenter );
      item->setTextAlignment( colDuration, Qt::AlignCenter );

      flightList->insertTopLevelItem( i, item );
    }

  flightList->slotResizeColumns2Content();

  QApplication::restoreOverrideCursor();
  statusBar->setText("");
}

void RecorderDialog::slotDownloadFlight()
{
  QTreeWidgetItem *item = flightList->currentItem();

  if( item == 0 || !activeRecorder )
    {
      return;
    }

  // If no DefaultFlightDirectory is configured, we must use $HOME instead of the root-directory
  QString flightDir = _settings.value( "/Path/DefaultFlightDirectory",
                                       _mainWindow->getApplicationDataDirectory() ).toString();

  QString fileName = flightDir + "/";

  int flightID( item->text( colNo ).toInt() - 1 );

  qDebug() << "PathName:" << fileName;
  qDebug() << "LongFileName: " << dirList.at(flightID)->longFileName;
  qDebug() << "ShortFileName:" << dirList.at(flightID)->shortFileName;

  if( useLongNames->isChecked() )
    {
      fileName += dirList.at( flightID )->longFileName.toUpper();
    }
  else
    {
      fileName += dirList.at( flightID )->shortFileName.toUpper();
    }

  QString filter;
  filter.append(tr("IGC") + " (*.igc)");

  fileName = QFileDialog::getSaveFileName( this,
                                           tr( "Select IGC file to save to" ),
                                           fileName,
                                           filter );
  if( fileName.isEmpty() )
    {
      return;
    }

  slotDisablePages();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  statusBar->setText( tr("Downloading flight from recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  int ret = activeRecorder->downloadFlight( flightID,
                                            !useFastDownload->isChecked(),
                                            fileName );

  QApplication::restoreOverrideCursor();

  if( ret < FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot download flight from recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
    }
  else
    {
      QMessageBox::information( this,
                                tr("Flight download finished"),
                                tr("Flight successfully downloaded from the recorder."),
                                QMessageBox::Ok );
    }

  statusBar->setText("");
  slotEnablePages();
}

void RecorderDialog::slotWriteDeclaration()
{
  if( !activeRecorder || taskSelection->currentIndex() < 0 )
    {
      return;
    }

  if( !activeRecorder->capabilities().supUlDeclaration )
    {
      QMessageBox::warning( this,
                            tr( "Declaration upload" ),
                            tr( "Function not implemented" ),
                            QMessageBox::Ok );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  statusBar->setText( tr("Sending flight declaration to the recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  FRTaskDeclaration taskDecl;
  taskDecl.pilotA = pilotName->text();
  taskDecl.pilotB = copilotName->text();
  taskDecl.gliderID = gliderID->text();
  taskDecl.gliderType = gliderType->currentText();
  taskDecl.compID = editCompID->text();
  taskDecl.compClass = compClass->text();

  QList<Waypoint*> wpList = tasks.at( taskSelection->currentIndex() )->getWPList();
  QString name = tasks.at( taskSelection->currentIndex ())->getFileName ();

  qDebug() << "Writing task "<< name << " to logger...";

  int ret = activeRecorder->writeDeclaration( &taskDecl, &wpList, name );

  QApplication::restoreOverrideCursor();

  if( ret == FR_NOTSUPPORTED )
    {
      QMessageBox::warning( this,
                            tr( "Declaration upload" ),
                            tr( "Function not implemented" ),
                            QMessageBox::Ok );

      statusBar->setText("");
      return;
    }

  if( ret < FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot write declaration to recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
    }
  else
    {
      QMessageBox::information( this,
                                tr( "Declaration upload" ),
                                tr( "The declaration was uploaded to the recorder." ),
                                QMessageBox::Ok );
    }

  statusBar->setText("");

  qDebug() << "   ... ready with" << ret;
}

void RecorderDialog::slotExportDeclaration()
{
  qDebug ("RecorderDialog::slotExportDeclaration");
  if( !activeRecorder || taskSelection->currentIndex() < 0 )
    {
       QMessageBox::warning( this,
                            tr( "Declaration export" ),
                            tr( "No Task selected" ),
                            QMessageBox::Ok );
       return;
    }

  if( !activeRecorder->capabilities().supExportDeclaration ) {
      QMessageBox::warning( this,
                            tr( "Declaration export" ),
                            tr( "Function not implemented" ),
                            QMessageBox::Ok );
      return;
  }

  if (pilotName->text().isEmpty()) {
      QMessageBox::warning( this,
                            tr( "Required" ),
                            tr( "Pilot name" ),
                            QMessageBox::Ok );
      return;
  }

  if (gliderID->text().isEmpty()) {
      QMessageBox::warning( this,
                            tr( "Required" ),
                            tr( "Glider Id" ),
                            QMessageBox::Ok );
      return;
  }

  if (gliderType->currentText().isEmpty()) {
      QMessageBox::warning( this,
                            tr( "Required" ),
                            tr( "Glider Type" ),
                            QMessageBox::Ok );
      return;
  }

  if (editCompID->text().isEmpty()) {
      QMessageBox::warning( this,
                            tr( "Required" ),
                            tr( "Competition ID" ),
                            QMessageBox::Ok );
      return;
  }

  if (compClass->text().isEmpty()) {
      QMessageBox::warning( this,
                            tr( "Required" ),
                            tr( "Competition Class" ),
                            QMessageBox::Ok );
      return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  statusBar->setText( tr("Exporting flight declaration to file") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  FRTaskDeclaration taskDecl;
  taskDecl.pilotA = pilotName->text();
  taskDecl.pilotB = copilotName->text();
  taskDecl.gliderID = gliderID->text();
  taskDecl.gliderType = gliderType->currentText();
  taskDecl.compID = editCompID->text();
  taskDecl.compClass = compClass->text();

  QList<Waypoint*> wpList = tasks.at( taskSelection->currentIndex ())->getWPList();
  
  QString name = tasks.at( taskSelection->currentIndex ())->getFileName ();

  qDebug() << "Exporting declaration " << name << " to file...";

  int ret = activeRecorder->exportDeclaration( &taskDecl, &wpList, name );

  QApplication::restoreOverrideCursor();

  if( ret == FR_NOTSUPPORTED )
    {
      QMessageBox::warning( this,
                            tr( "Declaration upload" ),
                            tr( "Function not implemented" ),
                            QMessageBox::Ok );

      statusBar->setText("");
      return;
    }

  if( ret < FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot write declaration to recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
    }
    else
    {
      QMessageBox::information( this,
                                tr( "Declaration export" ),
                                tr( "The declaration was exported to file." ),
                                QMessageBox::Ok );
    }

  statusBar->setText("");

  qDebug() << "   ... ready with" << ret;
}

int RecorderDialog::__fillDirList()
{
  qDeleteAll( dirList );
  dirList.clear();

  if( !activeRecorder )
    {
      return FR_ERROR;
    }

  if( activeRecorder->capabilities().supDlFlight )
    {
      return activeRecorder->getFlightDir( &dirList );
    }
  else
    {
      return FR_NOTSUPPORTED;
    }
}

bool RecorderDialog::__openLib( const QString& libN )
{
#ifndef _WIN32
  qDebug() << "RecorderDialog::__openLib: " << libN;

  if( libName == libN )
    {
      qWarning( "OK, Library is already opened." );
      return true;
    }

  libName = "";
  apiID->setText("");
  dvcID->setText("");
  swVersion->setText("");
  serID->setText("");
  recType->setText("");
  pltName->setText("");
  coPltName->setText("");
  gldType->setText("");
  gldID->setText("");
  compID->setText("");

  QString libPath = getLibraryPath() + "/" + libN;

  libHandle = dlopen( libPath.toAscii().data(), RTLD_NOW );

  char *error = (char *) dlerror();

  if( libHandle == 0 )
    {
      QString errMsg;

      if( error != 0 )
        {
          qWarning() << "RecorderDialog::__openLib() Error:" << error;

          errMsg = QString(error);
        }

      QMessageBox::critical( this,
                             tr("Plugin is missing!"),
                             tr("Cannot open plugin library:") +
                             "\n\n" + libPath + "\n\n" + errMsg,
                             QMessageBox::Ok );

      return false;
    }

  FlightRecorderPluginBase* (*getRecorder)();

  getRecorder = (FlightRecorderPluginBase* (*) ()) dlsym(libHandle, "getRecorder");

  if( !getRecorder )
    {
      qWarning( "getRecorder function not defined in library!" );
      return false;
    }

  activeRecorder = getRecorder();

  if( !activeRecorder )
    {
      qWarning( "No recorder object returned!" );
      return false;
    }

  activeRecorder->setParent(this);

  apiID->setText(activeRecorder->getLibName());

  libName = libN;

  return true;
#else
    return false;
#endif
}

void RecorderDialog::slotSwitchTask( int idx )
{
  FlightTask *task = tasks.at(idx);

  if( ! task )
    {
      return;
    }

  declarationList->clear();

  QList<Waypoint*> wpList = ((FlightTask*) task)->getWPList();

  for( int i = 0; i < wpList.size(); i++ )
    {
      Waypoint *wp = wpList.at(i);

      QTreeWidgetItem *item = new QTreeWidgetItem;

      item->setIcon(declarationColName, _globalMapConfig->getPixmap(wp->type, false, true) );

      item->setText(declarationColNo, QString("%1").arg( i + 1));
      item->setText(declarationColName, wp->name);
      item->setText(declarationColLat, WGSPoint::printPos(wp->origP.lat(), true));
      item->setText(declarationColLon, WGSPoint::printPos(wp->origP.lon(), false));
      item->setText(declarationColDummy, "");

      qDebug ("waypointColNo: %d", waypointColNo);
      item->setTextAlignment( waypointColNo, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( waypointColName, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( waypointColLat, Qt::AlignCenter );
      item->setTextAlignment( waypointColLon, Qt::AlignCenter );

      declarationList->insertTopLevelItem( i, item );
    }

  declarationList->slotResizeColumns2Content();
}

void RecorderDialog::slotReadTasks()
{
  if( !activeRecorder )
    {
      return;
    }

  if( !activeRecorder->capabilities().supDlTask )
    {
      QMessageBox::warning( this,
                            tr("Download task"),
                            tr("Function not implemented"),
                            QMessageBox::Ok );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Reading Tasks from recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  int ret = activeRecorder->readTasks( &tasks );

  int cnt = 0;

  if( ret < FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot read tasks from recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
    }
  else
    {
      FlightTask *task;
      Waypoint *wp;

      foreach(task, tasks)
        {
          QList<Waypoint*> wpList = task->getWPList();

          // here we overwrite the original task name (if needed) to get a unique internal name
          task->setTaskName( _globalMapContents->genTaskName( task->getFileName() ) );

          foreach(wp, wpList)
            {
              wp->projP = _globalMapMatrix->wgsToMap( wp->origP );
            }

          task->setWaypointList( wpList );
          emit addTask( task );
          cnt++;
        }

    // fill task list with new tasks
    fillTaskList( tasks );

    lblTaskList->setText( tr("Recorder Tasks") );

    QApplication::restoreOverrideCursor();

    QMessageBox::information( this,
                              tr("Task download"),
                              tr("%1 task(s) are downloaded from the recorder.").arg(cnt),
                              QMessageBox::Ok );
  }

  QApplication::restoreOverrideCursor();
  statusBar->setText("");
}

void RecorderDialog::slotWriteTasks()
{
  if( !activeRecorder || tasks.size() == 0 )
    {
      return;
    }

  if( !activeRecorder->capabilities().supUlTask )
    {
      QMessageBox::warning( this,
                            tr("Task upload"),
                            tr("Function not implemented"),
                            QMessageBox::Ok );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Writing Tasks to recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();


  int maxNrTasks = activeRecorder->capabilities().maxNrTasks;

  if( maxNrTasks == 0 )
    {
      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             tr( "Cannot obtain maximum number of tasks!" ),
                             QMessageBox::Ok );

      QApplication::restoreOverrideCursor();
      statusBar->setText("");
      return;
    }

  int maxNrWayPointsPerTask = activeRecorder->capabilities().maxNrWaypointsPerTask;

  if( maxNrWayPointsPerTask == 0 )
    {
      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             tr( "Cannot obtain maximum number of waypoints per task!" ),
                             QMessageBox::Ok );

      QApplication::restoreOverrideCursor();
      statusBar->setText("");
      return;
    }

  int cnt;
  QList<Waypoint*> wpListCopy;
  Waypoint *wp;
  QList<FlightTask*> frTasks;

  for( cnt = 0; cnt < tasks.size(); cnt++ )
    {
      if( frTasks.size() > maxNrTasks )
        {
          QString msg = QString( tr( "Maximum number of %1 tasks reached!\n"
                                     "Further tasks will be ignored." ) ).arg(maxNrTasks);

          int res = QMessageBox::warning( this,
                                          tr( "Recorder Warning" ),
                                          msg,
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No );

          if( res == QMessageBox::No )
            {
              qDeleteAll( frTasks );
              QApplication::restoreOverrideCursor();
              statusBar->setText( "" );
              return;
            }
          else
            {
              break;
            }
        }

      FlightTask *task = tasks.at(cnt);

      QList<Waypoint*> wpListOrig = task->getWPList();

      wpListCopy.clear();

      foreach(wp, wpListOrig)
      {
        if( wpListCopy.count() > maxNrWayPointsPerTask )
          {
            QString msg = QString( tr( "Maximum number of turnpoints/task %1 in %2 reached!\n"
                                       "Further turnpoints will be ignored." ) )
                                   .arg(maxNrWayPointsPerTask)
                                   .arg(task->getFileName() );

            int res = QMessageBox::warning( this,
                                            tr( "Recorder Warning" ),
                                            msg,
                                            QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::No );

            if( res == QMessageBox::No )
              {
                qDeleteAll( frTasks );
                QApplication::restoreOverrideCursor();
                statusBar->setText( "" );
                return;
              }
            else
              {
                break;
              }
          }

        wpListCopy.append(wp);
      }

      frTasks.append(new FlightTask(wpListCopy, true, task->getFileName()));
    }

  int ret = activeRecorder->writeTasks( &frTasks );

  if( ret < FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot write tasks to recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );
    }
  else
    {
      QMessageBox::information( this,
                                tr("Task upload"),
                                tr("%1 tasks were uploaded to the recorder.").arg(cnt),
                                QMessageBox::Ok );
    }

  qDeleteAll( frTasks );
  QApplication::restoreOverrideCursor();
  statusBar->setText( "" );
}

void RecorderDialog::slotReadWaypoints()
{
  QList<Waypoint*> frWaypoints;

  if( !activeRecorder )
    {
      return;
    }

  if( !activeRecorder->capabilities().supDlWaypoint )
    {
      QMessageBox::warning( this,
                            tr("Waypoints download"),
                            tr("Function not implemented"),
                            QMessageBox::Ok );
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Reading Waypoints from recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  int ret = activeRecorder->readWaypoints( &frWaypoints );

  if( ret < FR_OK )
    {
      QApplication::restoreOverrideCursor();

      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Cannot read waypoints from recorder." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::critical( this,
                             tr( "Library Error" ),
                             errorText,
                             QMessageBox::Ok );

      statusBar->setText("");
      return;
    }

  WaypointCatalog *wpCat = new WaypointCatalog( selectType->currentText() + "_" + serID->text() );
  wpCat->modified = true;

  for( int i = 0; i < frWaypoints.size(); i++ )
    {
      wpCat->insertWaypoint( frWaypoints.at( i ) );
    }

  emit addCatalog(wpCat);

  fillWaypointList( frWaypoints );

  lblWpList->setText( tr("Recorder Waypoints") );

  QApplication::restoreOverrideCursor();

  QMessageBox::information( this,
                            tr("Waypoints reading finished"),
                            tr("%1 waypoints have been read from the recorder.").arg(frWaypoints.size()),
                            QMessageBox::Ok );

  statusBar->setText("");
}

void RecorderDialog::slotWriteWaypoints()
{
  if( !activeRecorder || waypoints.size() == 0 )
    {
      return;
    }

  int res = QMessageBox::warning( this,
                                  tr("Waypoints upload"),
                                  tr("Uploading waypoints to the recorder will overwrite existing waypoints on the recorder. Do want to continue uploading?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No );

  if( res == QMessageBox::No )
    {
      return;
    }

  if( !activeRecorder->capabilities().supUlWaypoint )
    {
      QMessageBox::warning( this,
                            tr("Waypoints upload"),
                            tr("Function not implemented"),
                            QMessageBox::Ok );
      return;
    }

  statusBar->setText( tr("Writing Waypoints to recorder") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  int maxNrWaypoints = activeRecorder->capabilities().maxNrWaypoints;

  if( maxNrWaypoints == 0 )
    {
      QMessageBox::critical( this,
                             tr("Library Error"),
                             tr("Cannot obtain maximum number of waypoints from library"),
                             QMessageBox::Ok );

      statusBar->setText( "" );
      return;
    }

  QList<Waypoint*> frWaypoints;

  int cnt;

  for( cnt = 0; cnt < waypoints.size(); cnt++ )
    {
      if( frWaypoints.size() > maxNrWaypoints )
        {
          QString msg = QString( tr( "Maximum number of %1 waypoints reached!\n"
                                     "Further waypoints will be ignored." ) ).arg(maxNrWaypoints);

          int res = QMessageBox::warning( this,
                                          tr( "Recorder Warning" ),
                                          msg,
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No );

          if( res == QMessageBox::No )
            {
              statusBar->setText( "" );
              return;
            }
          else
            {
              break;
            }
          }

        frWaypoints.append( waypoints.at(cnt) );
      }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    int ret = activeRecorder->writeWaypoints( &frWaypoints );

    if( ret < FR_OK )
      {
        QApplication::restoreOverrideCursor();

        QString errorDetails = activeRecorder->lastError();

        QString errorText = tr( "Cannot write waypoints to recorder." );

        if( ! errorDetails.isEmpty() )
          {
            errorText += "\n" + errorDetails;
          }

        QMessageBox::critical( this,
                               tr( "Library Error" ),
                               errorText,
                               QMessageBox::Ok );
      }
    else
      {
        QMessageBox::information( this,
                                  tr("Waypoints upload"),
                                  QString(tr("%1 waypoints have been uploaded to the recorder.")).arg(cnt),
                                  QMessageBox::Ok );
      }

  QApplication::restoreOverrideCursor();
  statusBar->setText( "" );
}

void RecorderDialog::slotReadDatabase()
{
  if( !activeRecorder )
    {
      return;
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Reading recorder data") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  FlightRecorderPluginBase::FR_Capabilities cap = activeRecorder->capabilities();

  int ret = activeRecorder->getBasicData(basicdata);

  if( ret == FR_OK )
    {
      if (cap.supDspSerialNumber)
        serID->setText(basicdata.serialNumber);
      if (cap.supDspDvcID)
        dvcID->setText(basicdata.dvcID);
      if (cap.supDspSwVersion)
        swVersion->setText(basicdata.swVersion);
      if (cap.supDspRecorderType)
        recType->setText(basicdata.recorderType);
      if (cap.supDspPilotName)
        pltName->setText(basicdata.pilotName.trimmed());
      if (cap.supDspCoPilotName)
        coPltName->setText(basicdata.copilotName.trimmed());
      if (cap.supDspGliderType)
        gldType->setText(basicdata.gliderType.trimmed());
      if (cap.supDspGliderID)
        gldID->setText(basicdata.gliderID.trimmed());
      if (cap.supDspCompetitionID)
        compID->setText(basicdata.competitionID.trimmed());
    }
  else
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Sorry, could not connect to recorder.\n"
                              "Please check connections and settings." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::warning( this,
                             tr( "Recorder Connection" ),
                             errorText,
                             QMessageBox::Ok );

      QApplication::restoreOverrideCursor();
      statusBar->setText("");
      return;
    }

  if (cap.supEditGliderID     ||
      cap.supEditGliderType   ||
      cap.supEditGliderPolar  ||
      cap.supEditPilotName    ||
      cap.supEditUnits        ||
      cap.supEditGoalAlt      ||
      cap.supEditArvRadius    ||
      cap.supEditAudio        ||
      cap.supEditLogInterval)
    {
      int ret = activeRecorder->getConfigData( configdata );

      if( ret == FR_OK )
      {
        // we read the information from the logger, so we can enable the write button
        cmdUploadConfig->setEnabled(true);

        LD->setValue(configdata.LD);
        speedLD->setValue(configdata.speedLD);
        speedV2->setValue(configdata.speedV2);
        dryweight->setValue(configdata.dryweight);
        maxwater->setValue(configdata.maxwater);
        sinktone->setChecked(configdata.sinktone);
        approachradius->setValue(configdata.approachradius);
        arrivalradius->setValue(configdata.arrivalradius);
        goalalt->setValue(configdata.goalalt);
        sloginterval->setValue(configdata.sloginterval);
        floginterval->setValue(configdata.floginterval);
        gaptime->setValue(configdata.gaptime);
        minloggingspd->setValue(configdata.minloggingspd);
        stfdeadband->setValue(configdata.stfdeadband);

        int buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Vario_kts )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Vario_kts;
          }

        unitVarioButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Alt_ft )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Alt_ft;
          }

        unitAltButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Temp_F )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Temp_F;
          }

        unitTempButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Baro_inHg )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Baro_inHg;
          }

        unitQNHButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Baro_inHg )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Baro_inHg;
          }

        unitQNHButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Dist_nm )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Dist_nm;
          }
        else if( configdata.units & FlightRecorderPluginBase::FR_Unit_Dist_sm )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Dist_sm;
          }

        unitDistButtonGroup->button( buttonId )->setChecked( true );

        buttonId = 0;

        if( configdata.units & FlightRecorderPluginBase::FR_Unit_Spd_kts )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Spd_kts;
          }
        else if( configdata.units & FlightRecorderPluginBase::FR_Unit_Spd_mph )
          {
            buttonId = FlightRecorderPluginBase::FR_Unit_Spd_mph;
          }

        unitSpeedButtonGroup->button( buttonId )->setChecked( true );
      }
  }

  if (cap.supEditGliderID     ||
      cap.supEditGliderType   ||
      cap.supEditPilotName    ||
      cap.supEditCoPilotName)
    {
      cmdUploadBasicConfig->setEnabled(true);
      pltName->setEnabled(true);
      coPltName->setEnabled(true);
      gldType->setEnabled(true);
      gldID->setEnabled(true);
    }

  QApplication::restoreOverrideCursor();
  statusBar->setText("");
}

void RecorderDialog::slotWriteConfig()
{
  if( !activeRecorder )
    {
      return;
    }

  basicdata.pilotName = pltName->text();
  basicdata.copilotName = coPltName->text();
  basicdata.gliderType = gldType->text();
  basicdata.gliderID = gldID->text();
  basicdata.competitionID = compID->text();

  configdata.LD = LD->text().toInt();
  configdata.speedLD = speedLD->text().toInt();
  configdata.speedV2 = speedV2->text().toInt();
  configdata.dryweight = dryweight->text().toInt();
  configdata.maxwater = maxwater->text().toInt();

  configdata.sinktone = (int) sinktone->isChecked();

  configdata.approachradius = approachradius->text().toInt();
  configdata.arrivalradius = arrivalradius->text().toInt();
  configdata.goalalt = goalalt->text().toInt();
  configdata.sloginterval = sloginterval->text().toInt();
  configdata.floginterval = floginterval->text().toInt();
  configdata.gaptime = gaptime->text().toInt();
  configdata.minloggingspd = minloggingspd->text().toInt();
  configdata.stfdeadband = stfdeadband->text().toInt();

  configdata.units = 0;

  if( unitAltButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitAltButtonGroup->checkedId();
    }

  if( unitVarioButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitVarioButtonGroup->checkedId();
    }

  if( unitSpeedButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitSpeedButtonGroup->checkedId();
    }

  if( unitQNHButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitQNHButtonGroup->checkedId();
    }

  if( unitTempButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitTempButtonGroup->checkedId();
    }

  if( unitDistButtonGroup->checkedId() != -1 )
    {
      configdata.units |= unitDistButtonGroup->checkedId();
    }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  statusBar->setText( tr("Writing recorder data") );
  QCoreApplication::processEvents();
  QCoreApplication::flush();

  int ret = activeRecorder->writeConfigData(basicdata, configdata);

  if( ret != FR_OK )
    {
      QString errorDetails = activeRecorder->lastError();

      QString errorText = tr( "Sorry, could not write configuration to recorder.\n"
                              "Please check connections and settings." );

      if( ! errorDetails.isEmpty() )
        {
          errorText += "\n" + errorDetails;
        }

      QMessageBox::warning( this,
                             tr( "Recorder Connection" ),
                             errorText,
                             QMessageBox::Ok );
    }

  QApplication::restoreOverrideCursor();
  statusBar->setText("");
}

void RecorderDialog::slotDisablePages()
{
  flightPage->setEnabled(false);
  waypointPage->setEnabled(false);
  taskPage->setEnabled(false);
  declarationPage->setEnabled(false);
  configPage->setEnabled(false);
}

/** Enable/Disable pages when (not) connected to a recorder */
void RecorderDialog::slotEnablePages()
{
  qDebug ("slotEnablePages");
  //first, disable all pages
  configPage->setEnabled(false);
  declarationPage->setEnabled(false);
  flightPage->setEnabled(false);
  taskPage->setEnabled(false);
  waypointPage->setEnabled(false);
  cmdUploadTasks->setEnabled(false);
  cmdDownloadTasks->setEnabled(false);
  cmdUlDeclaration->setEnabled(false);
  cmdExportDeclaration->setEnabled(false);

  // If there is an active recorder and that recorder is connected,
  // selectively re-activate them.
  if( !activeRecorder )
    {
      return;
    }

  FlightRecorderPluginBase::FR_Capabilities cap=activeRecorder->capabilities();

  if( activeRecorder->isConnected() )
    {
      // flight page
      if( cap.supDlFlight )
        {
          flightPage->setEnabled( true );
          useFastDownload->setEnabled( cap.supSignedFlight );
        }

      // waypoint page
      if( cap.supDlWaypoint || cap.supUlWaypoint ) {
          waypointPage->setEnabled( true );
          cmdUploadWaypoints->setEnabled( cap.supUlWaypoint );
          cmdDownloadWaypoints->setEnabled( cap.supDlWaypoint );
      }

      // task page
      if( cap.supDlTask || cap.supUlTask ) {
          taskPage->setEnabled( true );
          cmdUploadTasks->setEnabled( cap.supUlTask );
          cmdDownloadTasks->setEnabled( cap.supDlTask );
      }

      // declaration page
      if( cap.supUlDeclaration )
        {
          declarationPage->setEnabled( true );
          cmdUlDeclaration->setEnabled (true);
        }

    // configuration page
    if (cap.supEditGliderPolar  ||
        cap.supEditUnits        ||
        cap.supEditGoalAlt      ||
        cap.supEditArvRadius    ||
        cap.supEditAudio        ||
        cap.supEditLogInterval)
      {
        configPage->setEnabled(true);
      }
  }
  // enable export even if recorder is not connected. export writes to local file
  if( cap.supExportDeclaration )
  {
    declarationPage->setEnabled( true );
    cmdExportDeclaration->setEnabled( true );
  }
}

/** Opens the new recorder plugin library, if necessary. */
void RecorderDialog::slotRecorderTypeChanged(const QString& newRecorderName )
{
#ifndef _WIN32
  qDebug ("slotRecorderTypeChanged");
  if( newRecorderName.isEmpty() )
    {
      return;
    }

  QString newLibName = libNameList[newRecorderName];

  if( libHandle && libName != newLibName )
  {
      slotCloseRecorder();

      // closing old library handle
      dlclose( libHandle );
      libHandle = 0;
  }

  if( ! __openLib( newLibName ) )
  {
      __setRecorderConnectionType( FlightRecorderPluginBase::none );
      return;
  }

  __setRecorderConnectionType( activeRecorder->getTransferMode() );
  __setRecorderCapabilities();

  slotEnablePages();
#endif
}

/**
  *  If the recorder supports auto-detection of transfer speed,
  *  it will signal the new speed to adjust the GUI.
  */
void RecorderDialog::slotNewSpeed(int speed)
{
  selectSpeed->setCurrentIndex(selectSpeed->findText(QString("%1").arg(speed)));
}

/** Create configuration page. */
void RecorderDialog::__createConfigurationPage()
{
  QTreeWidgetItem* item = new QTreeWidgetItem;
  item->setText( 0, tr( "Configuration" ) );
  item->setData( 0, Qt::UserRole, "Configuration" );
  item->setIcon( 0, _mainWindow->getPixmap( "kde_configure_48.png" ) );
  setupTree->addTopLevelItem( item );

  configPage = new QWidget( this );
  configPage->setObjectName( "ConfigurationPage" );
  configPage->setVisible( false );

  configLayout->addWidget( configPage, 0, 1 );

  //----------------------------------------------------------------------------
  LD = new QSpinBox;
  LD->setRange( 0, 255 );
  LD->setSingleStep( 1 );
  LD->setButtonSymbols( QSpinBox::PlusMinus );
  LD->setValue( 0 );

  speedLD = new QSpinBox;
  speedLD->setRange( 0, 255 );
  speedLD->setSingleStep( 1 );
  speedLD->setButtonSymbols( QSpinBox::PlusMinus );
  speedLD->setEnabled( true );
  speedLD->setValue( 0 );
  speedLD->setSuffix( " Km/h" );

  speedV2 = new QSpinBox;
  speedV2->setRange( 0, 255 );
  speedV2->setSingleStep( 1 );
  speedV2->setButtonSymbols( QSpinBox::PlusMinus );
  speedV2->setValue( 0 );
  speedV2->setSuffix( " Km/h" );

  dryweight = new QSpinBox;
  dryweight->setRange( 0, 1000 );
  dryweight->setSingleStep( 1 );
  dryweight->setButtonSymbols( QSpinBox::PlusMinus );
  dryweight->setValue( 0 );
  dryweight->setSuffix( " Kg" );

  maxwater = new QSpinBox;
  maxwater->setRange( 0, 500 );
  maxwater->setSingleStep( 1 );
  maxwater->setButtonSymbols( QSpinBox::PlusMinus );
  maxwater->setValue( 0 );
  maxwater->setSuffix( " l" );

  QGridLayout *ggrid = new QGridLayout;
  ggrid->setSpacing( 10 );

  QFormLayout* formLayout = new QFormLayout;
  formLayout->addRow( tr( "Best L/D:" ), LD );
  formLayout->addRow( tr( "Best L/D speed:" ), speedLD );
  formLayout->addRow( tr( "2 m/s sink speed:" ), speedV2 );
  ggrid->addLayout( formLayout, 0, 0 );
  formLayout = new QFormLayout;
  formLayout->addRow( tr( "Dry weight:" ), dryweight );
  formLayout->addRow( tr( "Max. water ballast:" ), maxwater );
  ggrid->addLayout( formLayout, 0, 1 );
  ggrid->setColumnStretch( 2, 10 );

  QGroupBox* gGroup = new QGroupBox( tr( "Glider Settings" ) );
  gGroup->setLayout( ggrid );

  //----------------------------------------------------------------------------
  approachradius = new QSpinBox;
  approachradius->setRange( 0, 65535 );
  approachradius->setSingleStep( 10 );
  approachradius->setButtonSymbols( QSpinBox::PlusMinus );
  approachradius->setValue( 0 );
  approachradius->setSuffix( " m" );

  arrivalradius = new QSpinBox;
  arrivalradius->setRange( 0, 65535 );
  arrivalradius->setSingleStep( 10 );
  arrivalradius->setButtonSymbols( QSpinBox::PlusMinus );
  arrivalradius->setValue( 0 );
  arrivalradius->setSuffix( " m" );

  goalalt = new QSpinBox;
  goalalt->setRange( 0, 6553 );
  goalalt->setSingleStep( 1 );
  goalalt->setButtonSymbols( QSpinBox::PlusMinus );
  goalalt->setValue( 0 );
  goalalt->setSuffix( " m" );

  gaptime = new QSpinBox;
  gaptime->setRange( 0, 600 );
  gaptime->setSingleStep( 1 );
  gaptime->setButtonSymbols( QSpinBox::PlusMinus );
  gaptime->setValue( 0 );
  gaptime->setSuffix( " min" );

  sloginterval = new QSpinBox;
  sloginterval->setRange( 0, 600 );
  sloginterval->setSingleStep( 1 );
  sloginterval->setButtonSymbols( QSpinBox::PlusMinus );
  sloginterval->setValue( 0 );
  sloginterval->setSuffix( " s" );

  floginterval = new QSpinBox;
  floginterval->setRange( 0, 600 );
  floginterval->setSingleStep( 1 );
  floginterval->setButtonSymbols( QSpinBox::PlusMinus );
  floginterval->setValue( 0 );
  floginterval->setSuffix( " s" );

  minloggingspd = new QSpinBox;
  minloggingspd->setRange( 0, 100 );
  minloggingspd->setSingleStep( 1 );
  minloggingspd->setButtonSymbols( QSpinBox::PlusMinus );
  minloggingspd->setValue( 0 );
  minloggingspd->setSuffix( " Km/h" );

  stfdeadband = new QSpinBox;
  stfdeadband->setRange( 0, 90 );
  stfdeadband->setSingleStep( 1 );
  stfdeadband->setButtonSymbols( QSpinBox::PlusMinus );
  stfdeadband->setValue( 0 );
  stfdeadband->setSuffix( " Km/h" );

  sinktone = new QCheckBox( tr( "Sink tone" ) );
  sinktone->setChecked( true );

  QGridLayout *vgridLeft = new QGridLayout;

  formLayout = new QFormLayout;
  formLayout->addRow( tr( "Approach radius:" ), approachradius );
  formLayout->addRow( tr( "Arrival radius:" ), arrivalradius );
  formLayout->addRow( tr( "Goal altitude:" ), goalalt );
  formLayout->addRow( tr( "Min. flight time:" ), gaptime );
  vgridLeft->addLayout( formLayout, 0, 0 );

  vgridLeft->setColumnMinimumWidth( 30, 1 );

  formLayout = new QFormLayout;
  formLayout->addRow( tr( "Slow log interval:" ), sloginterval );
  formLayout->addRow( tr( "Fast log interval:" ), floginterval );
  formLayout->addRow( tr( "Min. logging speed:" ), minloggingspd );
  formLayout->addRow( tr( "Audio dead-band:" ), stfdeadband );
  vgridLeft->addLayout( formLayout, 0, 2 );

  QGridLayout *vgridAll = new QGridLayout;
  vgridAll->addLayout( vgridLeft, 0, 0 );
  vgridAll->setColumnMinimumWidth( 30, 1 );
  vgridAll->addWidget( sinktone, 0, 2, Qt::AlignBottom );

  //----------------------------------------------------------------------------
  QHBoxLayout *rbsLayout = new QHBoxLayout;
  rbsLayout->setSpacing( 10 );

  QVBoxLayout* vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitDistButtonGroup = new QButtonGroup( this );
  unitDistButtonGroup->setExclusive( true );

  QRadioButton* rb = new QRadioButton( tr( "km" ) );
  unitDistButtonGroup->addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "nm" ) );
  unitDistButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Dist_nm );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "sm" ) );
  unitDistButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Dist_sm );
  vbl->addWidget( rb );

  QGroupBox* gb = new QGroupBox( tr( "Distance" ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );

  //-------------------------------------------------
  vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitSpeedButtonGroup = new QButtonGroup( this );
  unitSpeedButtonGroup->setExclusive( true );

  rb = new QRadioButton( tr( "km/h" ) );
  unitSpeedButtonGroup->addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "kts" ) );
  unitSpeedButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Spd_kts );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "mph" ) );
  unitSpeedButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Spd_mph );
  vbl->addWidget( rb );

  gb = new QGroupBox( tr( "Speed" ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );

  //-------------------------------------------------
  vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitAltButtonGroup = new QButtonGroup( this );
  unitAltButtonGroup->setExclusive( true );

  rb = new QRadioButton( tr( "m" ) );
  unitAltButtonGroup->addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "ft" ) );
  unitAltButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Alt_ft );
  vbl->addWidget( rb );

  gb = new QGroupBox( tr( "Altitude" ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );

  //-------------------------------------------------
  vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitQNHButtonGroup = new QButtonGroup( this );
  unitQNHButtonGroup->setExclusive( true );

  rb = new QRadioButton( tr( "mbar" ) );
  unitQNHButtonGroup->addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "inHg" ) );
  unitQNHButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Baro_inHg );
  vbl->addWidget( rb );

  gb = new QGroupBox( tr( "QNH" ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );

  //-------------------------------------------------
  vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitVarioButtonGroup = new QButtonGroup( this );
  unitVarioButtonGroup->setExclusive( true );

  rb = new QRadioButton( tr( "m/s" ) );
  unitVarioButtonGroup->addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( tr( "kts" ) );
  unitVarioButtonGroup->addButton( rb, FlightRecorderPluginBase::FR_Unit_Vario_kts );
  vbl->addWidget( rb );

  gb = new QGroupBox( tr( "Vario" ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );

  //-------------------------------------------------
  vbl = new QVBoxLayout;
  vbl->setSpacing( 5 );

  unitTempButtonGroup = new QButtonGroup( this );
  unitTempButtonGroup->setExclusive( true );

  rb = new QRadioButton( QString(Qt::Key_degree) + tr( "C" ) );
  unitTempButtonGroup-> addButton( rb, 0 );
  vbl->addWidget( rb );

  rb = new QRadioButton( QString(Qt::Key_degree) + tr( "F" ) );
  unitTempButtonGroup-> addButton( rb, FlightRecorderPluginBase::FR_Unit_Temp_F );
  vbl->addWidget( rb );

  gb = new QGroupBox( tr( "Temp." ) );
  gb->setLayout( vbl );

  rbsLayout->addWidget( gb );
  rbsLayout->addStretch( 2 );

  //-------------------------------------------------
  // combine both variometer layouts
  QVBoxLayout* varioBox = new QVBoxLayout;
  varioBox->setSpacing( 10 );
  varioBox->addLayout( rbsLayout );
  varioBox->addSpacing( 10 );
  varioBox->addLayout( vgridAll );

  QGroupBox* vGroup = new QGroupBox( tr( "Variometer Settings" ) );
  vGroup->setLayout( varioBox );

  //----------------------------------------------------------------------------
  cmdUploadConfig = new QPushButton( tr( "Write configuration to recorder" ) );

  // Disable this button until we read the information from the flight recorder
  cmdUploadConfig->setEnabled( false );
  connect( cmdUploadConfig, SIGNAL(clicked()), SLOT(slotWriteConfig()) );

  QHBoxLayout *buttonBox = new QHBoxLayout;
  buttonBox->setSpacing( 0 );
  buttonBox->addWidget( cmdUploadConfig );
  buttonBox->addStretch( 10 );

  QVBoxLayout* pageLayout = new QVBoxLayout;
  pageLayout->setContentsMargins( 0, 0, 0, 0 );
  pageLayout->setSpacing( 10 );
  pageLayout->addWidget( gGroup );
  pageLayout->addWidget( vGroup );
  pageLayout->addLayout( buttonBox );

  configPage->setLayout( pageLayout );
}

