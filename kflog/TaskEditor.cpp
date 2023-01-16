/***********************************************************************
 **
 **   TaskEditor.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **                   2011-2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtWidgets>

#include "mapcalc.h"
#include "mapcontents.h"
#include "mainwindow.h"
#include "MetaTypes.h"
#include "TaskEditor.h"

extern MapConfig*     _globalMapConfig;
extern MapContents*   _globalMapContents;
extern MapMatrix*     _globalMapMatrix;
extern QSettings      _settings;

TaskEditor::TaskEditor( QWidget *parent ) :
  QDialog(parent),
  m_editedTask(0),
  m_task(QString("task"))
{
  setWindowTitle(tr("Task Editor") );
  setModal( true );
  setSizeGripEnabled( true );
  createDialog();
  setMinimumWidth(800);
  setMinimumHeight(450);
  restoreGeometry( _settings.value("/TaskEditor/Geometry").toByteArray() );
  show();
}

TaskEditor::~TaskEditor()
{
  _settings.setValue( "/TaskEditor/Geometry", saveGeometry() );
}

/** No descriptions */
void TaskEditor::createDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout( this );
  //topLayout->setMargin( 0 );

  //----------------------------------------------------------------------------
  // Setup a menubar with a help menu entry
  //----------------------------------------------------------------------------
  QMenuBar *menuBar = new QMenuBar( this );
  topLayout->addWidget( menuBar );

  QMenu *help = menuBar->addMenu( tr("&Help") );

  help->addAction( MainWindow::instance()->getPixmap( "kde_contexthelp_16.png"),
						      tr("Help Task"),
						      this,
						      SLOT(slotHelp()), QKeySequence::HelpContents );
  QHBoxLayout *header = new QHBoxLayout;
  QHBoxLayout *type = new QHBoxLayout;
  QHBoxLayout *buttons = new QHBoxLayout;
  QVBoxLayout *leftLayout = new QVBoxLayout;
  QVBoxLayout *middleLayout = new QVBoxLayout;
  QVBoxLayout *rightLayout = new QVBoxLayout;
  QHBoxLayout *topGroup = new QHBoxLayout;
  QHBoxLayout *smallButtons = new QHBoxLayout;

  buttons->addStretch();
  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(slotAccept()));
  buttons->addWidget(b);
  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  //----------------------------------------------------------------------------
  // Row 1
  m_taskNameEditor = new QLineEdit;
  m_taskNameEditor->setReadOnly(false);
  l = new QLabel(tr("Name") + ":");
  header->addWidget(l);
  header->addWidget(m_taskNameEditor);

  //----------------------------------------------------------------------------
  // Row 2
  // Create a combo box for task types
  m_planningTypes = new QComboBox;
  connect( m_planningTypes, SIGNAL(activated(const QString&)),
           SLOT(slotSetPlanningType(const QString&)) );

  l = new QLabel( tr( "Task Type" ) + ":" );
  l->setBuddy( m_planningTypes );

  // Load task types into combo box
  m_planningTypes->addItems( FlightTask::ttGetSortedTranslationList() );

  // Create an non-exclusive button group
  QButtonGroup *bgrp2 = new QButtonGroup( this );
  connect(bgrp2, SIGNAL(buttonClicked(int)), SLOT(slotSetPlanningDirection(int)));
  bgrp2->setExclusive(false);

  // insert 2 check buttons
  m_left = new QCheckBox(tr("left"));
  m_left->setChecked(true);
  bgrp2->addButton( m_left, 0 );
  m_right = new QCheckBox(tr("right"));
  m_left->setChecked(false);
  bgrp2->addButton( m_right, 1 );

  // Align check boxes into a group box.
  QGroupBox *faiGroupBox = new QGroupBox(tr("Side of FAI area"));

  QHBoxLayout *hbBox = new QHBoxLayout;
  hbBox->addWidget(m_left);
  hbBox->addWidget(m_right);
  faiGroupBox->setLayout(hbBox);

  m_taskType = new QLabel;
  m_taskType->setMinimumWidth(100);

  type->addWidget(l);
  type->addWidget(m_planningTypes);
  type->addWidget(faiGroupBox);
  type->addStretch(1);
  type->addWidget(m_taskType);

  //----------------------------------------------------------------------------
  // Row 3
  m_route = new KFLogTreeWidget("TaskEditor-Route");
  m_route->setToolTip( tr("Open Help menu to get help about a task definition.") );
  m_route->setSortingEnabled( false );
  m_route->setAllColumnsShowFocus( true );
  m_route->setFocusPolicy( Qt::StrongFocus );
  m_route->setRootIsDecorated( false );
  m_route->setItemsExpandable( true );
  m_route->setSelectionMode( QAbstractItemView::SingleSelection );
  m_route->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_route->setAlternatingRowColors( true );
  m_route->addRowSpacing( 5 );
  m_route->setColumnCount( 5 );

  m_route->setDragEnabled(true);
  m_route->viewport()->setAcceptDrops(true);
  m_route->setDropIndicatorShown(true);
  m_route->setDragDropMode(QAbstractItemView::InternalMove);

  QStringList headerLabels;

  headerLabels  << tr("Type")
                << tr("Taskpoint")
                << tr("Length")
                << tr("Course")
		<< tr("Leg");

  m_route->setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = m_route->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );
  headerItem->setTextAlignment( 4, Qt::AlignCenter );

  connect( m_route, SIGNAL(itemClicked( QTreeWidgetItem*, int )),
           this, SLOT( slotItemClicked( QTreeWidgetItem*, int )) );

  connect( m_route, SIGNAL(rightButtonPressed( QTreeWidgetItem*, const QPoint& )),
           this, SLOT( slotRightButtonPressed( QTreeWidgetItem*, const QPoint& )) );

  colRouteType     = 0;
  colRouteWaypoint = 1;
  colRouteDist     = 2;
  colRouteCourse   = 3;
  colRouteLeg      = 4;

  m_route->loadConfig();

  l = new QLabel(tr("Task"));
  l->setBuddy( m_route );

  smallButtons->addStretch(1);
  m_upCmd = new QPushButton(this);
  m_upCmd->setIcon(MainWindow::instance()->getPixmap("kde_up_16.png"));
  m_upCmd->setIconSize(QSize(16, 16));
  m_upCmd->setToolTip( tr("Moves selected task point up"));
  connect(m_upCmd, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(m_upCmd);

  m_downCmd = new QPushButton(this);
  m_downCmd->setIcon(MainWindow::instance()->getPixmap("kde_down_16.png"));
  m_downCmd->setIconSize(QSize(16, 16));
  m_downCmd->setToolTip( tr("Moves selected task point down"));
  connect(m_downCmd, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(m_downCmd);
  smallButtons->addStretch(1);

  leftLayout->addWidget(l);
  leftLayout->addWidget(m_route);
  leftLayout->addLayout(smallButtons);

  middleLayout->addStretch(1);
  m_addCmd = new QPushButton;
  m_addCmd->setIcon(MainWindow::instance()->getPixmap("kde_back_16.png"));
  m_addCmd->setIconSize(QSize(16, 16));
  m_addCmd->setToolTip( tr("Adds the selected waypoint to the task list"));
  connect(m_addCmd, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  middleLayout->addWidget(m_addCmd);

  m_invertCmd = new QPushButton(this);
  m_invertCmd->setIcon(MainWindow::instance()->getPixmap("kde_reload_16.png"));
  m_invertCmd->setIconSize(QSize(16, 16));
  m_invertCmd->setToolTip( tr("Inverts the task. Last point becomes the first point, a.s.o."));
  connect(m_invertCmd, SIGNAL(clicked()), SLOT(slotInvertWaypoints()));
  middleLayout->addWidget(m_invertCmd);

  m_removeCmd = new QPushButton;
  m_removeCmd->setIcon(MainWindow::instance()->getPixmap("kde_forward_16.png"));
  m_removeCmd->setIconSize(QSize(16, 16));
  m_removeCmd->setToolTip( tr("Removes the selected task point from the task list"));
  connect(m_removeCmd, SIGNAL(clicked()), SLOT(slotRemoveWaypoint()));
  middleLayout->addWidget(m_removeCmd);
  middleLayout->addStretch(1);

  m_wpListView = new KFLogTreeWidget("TaskEditor-Waypoints");
  m_wpListView->setSortingEnabled( true );
  m_wpListView->setAllColumnsShowFocus( true );
  m_wpListView->setFocusPolicy( Qt::StrongFocus );
  m_wpListView->setRootIsDecorated( false );
  m_wpListView->setItemsExpandable( true );
  m_wpListView->setSelectionMode( QAbstractItemView::SingleSelection );
  m_wpListView->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_wpListView->setAlternatingRowColors( true );
  m_wpListView->addRowSpacing( 5 );
  m_wpListView->setColumnCount( 4 );

  headerLabels.clear();

  headerLabels  << tr("Name")
                << tr("Description")
                << tr("ICAO")
                << tr("Country");

  m_wpListView->setHeaderLabels( headerLabels );

  headerItem = m_wpListView->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );
  headerItem->setTextAlignment( 2, Qt::AlignCenter );
  headerItem->setTextAlignment( 3, Qt::AlignCenter );

  colWpName        = 0;
  colWpDescription = 1;
  colWpIcao        = 2;
  colWpCountry     = 3;

  m_wpListView->loadConfig();

  QHBoxLayout* hbox = new QHBoxLayout;
  hbox->setMargin(0);
  hbox->addWidget(new QLabel(tr("WP Source:")));

  m_pointSourceBox = new QComboBox;
  m_pointSourceBox->setToolTip( tr("Select list to be used as waypoint source.") );
  hbox->addWidget( m_pointSourceBox );

  hbox->addStretch( 5 );

  m_pointColumnSelector = new QComboBox;
  m_pointColumnSelector->setToolTip( tr("Select list column to be searched"));
  m_pointColumnSelector->addItem( "1", 0 );
  m_pointColumnSelector->addItem( "2", 1 );
  m_pointColumnSelector->addItem( "3", 2 );
  m_pointColumnSelector->addItem( "4", 3 );
  m_pointColumnSelector->setCurrentIndex( 0 );
  hbox->addWidget( m_pointColumnSelector );

  connect( m_pointColumnSelector, SIGNAL(currentIndexChanged(int)),
	   this, SLOT(slotSearchColumnIndexChanged(int)) );

  // The minimum input field length should be 12 characters. We calculate
  // that as 12 characters M.
  QFontMetrics fm( font() );
  int charWidth = fm.horizontalAdvance(QChar('M'));

  m_pointSearchInput = new QLineEdit;
  m_pointSearchInput->setToolTip( tr("Enter a search string, to navigate to a certain list entry.") );
  m_pointSearchInput->setMinimumWidth( 12 * charWidth );
  hbox->addWidget( m_pointSearchInput );

  connect( m_pointSearchInput, SIGNAL(textEdited(const QString&)),
	   this, SLOT(slotSearchInputEdited(const QString&)) );

  connect( m_pointSearchInput, SIGNAL(returnPressed()),
	   this, SLOT(slotTakeFoundItem()) );

  QPushButton *m_clearPointSearchInput = new QPushButton( tr("Clear") );
  m_clearPointSearchInput->setToolTip( tr("Click Clear to remove the search string.") );
  hbox->addWidget( m_clearPointSearchInput );

  connect( m_clearPointSearchInput, SIGNAL(clicked()), SLOT(slotClearSearchInput()));

  rightLayout->addLayout( hbox );
  rightLayout->addWidget(m_wpListView);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);

  topLayout->addLayout(header);
  topLayout->addLayout(type);
  topLayout->addLayout(topGroup);
  topLayout->addLayout(buttons);

  setEntriesInPointSourceBox();
  enableCommandButtons();
}

void TaskEditor::showEvent( QShowEvent *event )
{
  // That is the trick to prevent dialog closing, if the return key is pressed.
  // But note, it has only effect, if the dialog is shown!
  QList<QPushButton *> buttonList = findChildren<QPushButton *>();

  foreach( QPushButton *pb, buttonList )
    {
      pb->setDefault( false );
      pb->setAutoDefault( false );
    }

  QDialog::showEvent( event );
}

void TaskEditor::slotRightButtonPressed( QTreeWidgetItem *item,
                                         const QPoint &position )
{
  Q_UNUSED(item)
  Q_UNUSED(position)

  if( m_taskWpList.size() == 0 || item == 0 )
    {
      return;
    }

  QMenu *menu = new QMenu(this);

  m_actionDuplicate = menu->addAction( tr("Duplicate"),
                                     this,
                                     SLOT(slotDuplicateWayoint()) );
  menu->addAction( m_actionDuplicate );

  m_actionRemove = menu->addAction( tr("Remove"),
                                  this,
                                  SLOT(slotRemoveWaypoint()) );
  menu->addAction( m_actionRemove );

  if( m_taskWpList.size() > 1 )
    {

      if( m_route->indexOfTopLevelItem(item) > 0 )
	{
	  m_actionMoveUp = menu->addAction( tr("Move up"),
					  this,
					  SLOT(slotMoveUp()) );
	  menu->addAction( m_actionMoveUp );
	}

      if( m_route->indexOfTopLevelItem(item) < m_route->topLevelItemCount() - 1 )
	{
	  m_actionMoveDown = menu->addAction( tr("Move down"),
					    this,
					    SLOT(slotMoveDown()) );
	  menu->addAction( m_actionMoveDown );
	}

      m_actionInvert = menu->addAction( tr("Invert"),
                                      this,
                                      SLOT(slotInvertWaypoints()) );
      menu->addAction( m_actionInvert );
    }

  menu->exec(  QCursor::pos() );
}

void TaskEditor::setEntriesInPointSourceBox()
{
  if( m_pointSourceBox == 0 )
    {
      return;
    }

  // Gets all available lists from MapContents
  QList<Waypoint*> &wpList = _globalMapContents->getWaypointList();

  QList<Airfield> &airfieldList = _globalMapContents->getAirfieldList();

  QList<Airfield> &gliderfieldList = _globalMapContents->getGliderfieldList();

  QList<Airfield> &outLandingList = _globalMapContents->getOutLandingList();

  QList<RadioPoint> &navaidList = _globalMapContents->getNavaidList();

  QList<ThermalPoint> &hotspotList = _globalMapContents->getHotspotList();

  m_pointSourceBox->clear();

  if( wpList.size() > 0 )
    {
      m_pointSourceBox->addItem( tr("Waypoints"), Waypoints );
    }

  if( airfieldList.size() > 0 || gliderfieldList.size() > 0 )
    {
      m_pointSourceBox->addItem( tr("Airfields"), Airfields );
    }

  if( outLandingList.size() > 0 )
    {
      m_pointSourceBox->addItem( tr("Outlandings"), Outlandings );
    }

  if( hotspotList.size() > 0 )
    {
      m_pointSourceBox->addItem( tr("Hotspots"), Hotspots );
    }

  if( navaidList.size() > 0 )
    {
      m_pointSourceBox->addItem( tr("Navaids"), Navaids );
    }

  if( m_pointSourceBox->count() == 0 )
    {
      m_pointSourceBox->addItem( tr("No point data found"), None );
    }
  else
    {
      // Activate point list loading, if combo box index is changed.
      connect( m_pointSourceBox, SIGNAL(currentIndexChanged(int)),
	       SLOT(slotLoadSelectableWaypoints(int)) );

      if( m_pointSourceBox->currentIndex() != 0 )
	{
	  m_pointSourceBox->setCurrentIndex( 0 );
	}
      else
	{
	  slotLoadSelectableWaypoints( 0 );
	}
    }
}

void TaskEditor::slotLoadSelectableWaypoints( int index )
{
  if( m_pointSourceBox == 0 || m_pointSourceBox->count() == 0 )
    {
      return;
    }

  int selectedItem = m_pointSourceBox->itemData(index).toInt();

  // Check, which point source is selected.
  if( selectedItem == None )
    {
      return;
    }

  // Clear the current content in the list.
  m_wpListView->clear();

  if( selectedItem == Waypoints )
    {
      // Gets current waypoint list from MapContents
      QList<Waypoint*> &wpList = _globalMapContents->getWaypointList();

      for( int i = 0; i < wpList.size(); i++ )
	{
	  Waypoint* wp = wpList.at(i);

	  QTreeWidgetItem *item = new QTreeWidgetItem;

	  item->setText(colWpName, wp->name);
	  item->setIcon(colWpName, _globalMapConfig->getPixmap(wp->type, false, true));
	  item->setText(colWpDescription, wp->description);
	  item->setText(colWpCountry, wp->country);
	  item->setText(colWpIcao, wp->icao);

	  item->setTextAlignment( colWpName, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpDescription, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpCountry, Qt::AlignCenter );
	  item->setTextAlignment( colWpIcao, Qt::AlignCenter );

	  QVariant v;
	  v.setValue(wp);
	  item->setData( 0, Qt::UserRole, v );
	  m_wpListView->insertTopLevelItem( i, item );
	}
    }
  else if( selectedItem == Airfields )
    {
      QList<Airfield>* lists[2];

      // Gets current airfield list from MapContents
      lists[0] = &_globalMapContents->getAirfieldList();

      // Gets current gliderfield list from MapContents
      lists[1] = &_globalMapContents->getGliderfieldList();

      for( int i = 0; i < 2; i++ )
	{
	  for( int k = 0; k < lists[i]->size(); k++ )
	    {
	      Airfield& af = const_cast<Airfield &>(lists[i]->at(k));

	      QTreeWidgetItem *item = new QTreeWidgetItem;

	      item->setText(colWpName, af.getShortName() );
	      item->setIcon(colWpName, _globalMapConfig->getPixmap(af.getTypeID(), false, true) );
	      item->setText(colWpDescription, af.getName() );
	      item->setText(colWpCountry, af.getCountry() );
	      item->setText(colWpIcao, af.getICAO() );

	      item->setTextAlignment( colWpName, Qt::AlignLeft|Qt::AlignVCenter );
	      item->setTextAlignment( colWpDescription, Qt::AlignLeft|Qt::AlignVCenter );
	      item->setTextAlignment( colWpCountry, Qt::AlignCenter );
	      item->setTextAlignment( colWpIcao, Qt::AlignCenter );

	      QVariant v;
	      v.setValue(&af);
	      item->setData( 0, Qt::UserRole, v );
	      m_wpListView->insertTopLevelItem( i, item );
	    }
	}
    }
  else if( selectedItem == Outlandings )
    {
      // Gets current outlanding list from MapContents
      QList<Airfield>& list = _globalMapContents->getOutLandingList();

      for( int i = 0; i < list.size(); i++ )
	{
	  Airfield &af = list[i];

	  QTreeWidgetItem *item = new QTreeWidgetItem;

	  item->setText(colWpName, af.getShortName() );
	  item->setIcon(colWpName, _globalMapConfig->getPixmap(af.getTypeID(), false, true) );
	  item->setText(colWpDescription, af.getName() );
	  item->setText(colWpCountry, af.getCountry() );
	  item->setText(colWpIcao, af.getICAO() );

	  item->setTextAlignment( colWpName, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpDescription, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpCountry, Qt::AlignCenter );
	  item->setTextAlignment( colWpIcao, Qt::AlignCenter );

	  QVariant v;
	  v.setValue(&af);
	  item->setData( 0, Qt::UserRole, v );
	  m_wpListView->insertTopLevelItem( i, item );
	}
    }
  else if( selectedItem == Hotspots )
    {
      // Gets current hotspot list from MapContents
      QList<ThermalPoint> &list = _globalMapContents->getHotspotList();

      for( int i = 0; i < list.size(); i++ )
        {
          SinglePoint& sp = list[i];

          QTreeWidgetItem *item = new QTreeWidgetItem;

          item->setText(colWpName, sp.getShortName() );
          item->setIcon(colWpName, _globalMapConfig->getPixmap(sp.getTypeID(), false, true) );
          item->setText(colWpDescription, sp.getName() );
          item->setText(colWpCountry, sp.getCountry() );
          item->setText(colWpIcao, "" );

          item->setTextAlignment( colWpName, Qt::AlignLeft|Qt::AlignVCenter );
          item->setTextAlignment( colWpDescription, Qt::AlignLeft|Qt::AlignVCenter );
          item->setTextAlignment( colWpCountry, Qt::AlignCenter );
          item->setTextAlignment( colWpIcao, Qt::AlignCenter );

          QVariant v;
          v.setValue(&sp);
          item->setData( 0, Qt::UserRole, v );
          m_wpListView->insertTopLevelItem( i, item );
        }
    }
  else if( selectedItem == Navaids )
    {
      // Gets current navaid list from MapContents
      QList<RadioPoint> &list = _globalMapContents->getNavaidList();

      for( int i = 0; i < list.size(); i++ )
	{
	  RadioPoint& rp = list[i];

	  QTreeWidgetItem *item = new QTreeWidgetItem;

	  item->setText(colWpName, rp.getShortName() );
	  item->setIcon(colWpName, _globalMapConfig->getPixmap(rp.getTypeID(), false, true) );
	  item->setText(colWpDescription, rp.getName() );
	  item->setText(colWpCountry, rp.getCountry() );
	  item->setText(colWpIcao, rp.getICAO() );

	  item->setTextAlignment( colWpName, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpDescription, Qt::AlignLeft|Qt::AlignVCenter );
	  item->setTextAlignment( colWpCountry, Qt::AlignCenter );
	  item->setTextAlignment( colWpIcao, Qt::AlignCenter );

	  QVariant v;
	  v.setValue(&rp);
	  item->setData( 0, Qt::UserRole, v );
	  m_wpListView->insertTopLevelItem( i, item );
	}
    }
  else
    {
      // Unknown category
      qWarning() << "TaskEditor::slotLoadSelectableWaypoints(): Unknown list item"
	         << selectedItem;
      return;
    }

  m_wpListView->sortItems(0, Qt::AscendingOrder);
  m_wpListView->slotResizeColumns2Content();
}

void TaskEditor::slotSetPlanningType( const QString& text )
{
  int id = FlightTask::ttText2Item( text );

  switch (id)
  {
  case FlightTask::FAIArea:
    {
      if( m_taskWpList.size() > 5 )
        {
          QString msg (tr( "FAI area task violation!"
                           "<br><br>Please remove all not necessary route points."
                           "<br><br>See Help menu for more information.") );

          QMessageBox::warning( this, tr("Too much route points"), msg );
        }

      m_left->setEnabled(true);
      m_right->setEnabled(true);
      m_left->setChecked(m_editedTask->getPlanningDirection() & FlightTask::leftOfRoute);
      m_right->setChecked(m_editedTask->getPlanningDirection() & FlightTask::rightOfRoute);
    }

    break;

  case FlightTask::Route:

    m_left->setEnabled(false);
    m_right->setEnabled(false);
    m_left->setChecked(false);
    m_right->setChecked(false);
    break;

  default:
    break;
  }

  m_editedTask->setPlanningType(id);
  loadRouteWaypoints();
  m_route->setCurrentItem( m_route->topLevelItem(0) );
  enableCommandButtons();
}

void TaskEditor::slotHelp()
{
  QString msgFAI( tr("<html>"
      "You can define a FAI task with either Take-off, Start, End and Landing or "
      "Take-off, Start, End, Landing and <b>one</b> additional Route point.<br>"
      "The points <i>Take-off</i>, <i>Start</i>, <i>End</i> and <i>Landing</i> "
      "are <b>mandatory!</b><br><br>"
      "The FAI area calculation will be made with Start and End point or Start "
      "and Route point, depending weather the route point is defined or not.<br><br>"
      "New points are inserted always after the selected one.</html>" ) );

  QString msgRoute(  tr("<html>"
    "You can define a task with Take-off, Start, End, Landing and Route points. "
    "The points <i>Take-off</i>, <i>Start</i>, <i>End</i> and <i>Landing</i> "
    "are <b>mandatory!</b> "
    "Additional route points can be added.<br><br>"
    "New points are inserted always after the selected one.</html>") );

  switch( m_editedTask->getPlanningType() )
  {
    case FlightTask::FAIArea:

      QMessageBox::information( this, tr("Help FAI"), msgFAI );
      break;

    case FlightTask::Route:

      QMessageBox::information( this, tr("Help Route"), msgRoute );
      break;

    case FlightTask::AAT:
      // Not yet supported.
    default:

      break;
  }
}

void TaskEditor::slotSetPlanningDirection(int)
{
  int dir = 0;

  if( m_left->isChecked() )
    {
      dir |= FlightTask::leftOfRoute;
    }

  if( m_right->isChecked() )
    {
      dir |= FlightTask::rightOfRoute;
    }

  m_editedTask->setPlanningDirection(dir);
}

void TaskEditor::loadRouteWaypoints()
{
  Waypoint *wpPrev = 0;

  m_route->clear();

  int number = 0;
  double distanceTotal = 0.0;

  for( int i = 2; i < m_taskWpList.size() -1; i++ )
     {
	distanceTotal += m_taskWpList.at(i)->distance;
	number++;
     }

  for( int i = 0; i < m_taskWpList.size(); i++ )
    {
      QString txt;

      Waypoint *wp = m_taskWpList.at(i);

      switch( wp->tpType )
        {
          case FlightTask::TakeOff:
            txt = tr("Take Off");
            break;
          case FlightTask::Begin:
            txt = tr("Begin of Task");
            break;
          case FlightTask::RouteP:
            txt = tr("Route Point");
            break;
          case FlightTask::End:
            txt = tr("End of Task");
            break;
          case FlightTask::FreeP:
            txt = tr("Free Point");
            break;
          case FlightTask::Landing:
            txt = tr("Landing");
            break;
          default:
            txt = tr("Unkown");
            break;
        }

      QTreeWidgetItem *item = new QTreeWidgetItem;

      item->setText(colRouteType, txt);
      item->setText(colRouteWaypoint, wp->name);
      item->setIcon(colRouteWaypoint, _globalMapConfig->getPixmap(wp->type, false, true));

      if( i > 0 )
        {
          item->setText(colRouteDist, QString("%1 Km").arg(wp->distance, 0, 'f', 3));
          item->setText(colRouteCourse, QString("%1%2")
                       .arg(getTrueCourse(wp->origP, wpPrev->origP), 3, 'f', 0, QChar('0') )
                       .arg(QChar(Qt::Key_degree)) );
        }

      if( distanceTotal > 0.0 && wp->distance> 0.0 &&
	  i > 1 && i < m_taskWpList.size() -1 )
	{
	  double leg = wp->distance/distanceTotal*100;

	  item->setText(colRouteLeg, QString("%1 %").arg(leg, 2, 'f', 1));
	}

      item->setTextAlignment( colRouteType, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( colRouteWaypoint, Qt::AlignLeft|Qt::AlignVCenter );
      item->setTextAlignment( colRouteDist, Qt::AlignRight|Qt::AlignVCenter );
      item->setTextAlignment( colRouteCourse, Qt::AlignCenter );
      item->setTextAlignment( colRouteLeg, Qt::AlignCenter );

      m_route->insertTopLevelItem( i, item );

      wpPrev = wp;
    }

  m_route->slotResizeColumns2Content();

  m_taskType->setText( m_editedTask->getTaskTypeString() );
}

int TaskEditor::getCurrentPosition()
{
  QTreeWidgetItem* item = m_route->currentItem();

  if( item )
    {
      return m_route->indexOfTopLevelItem( item );
    }

  return -1;
}

void TaskEditor::setSelected( int position )
{
  if( position >= 0 && m_route->topLevelItemCount() > 0 )
    {
      QTreeWidgetItem* item = m_route->topLevelItem( position );

      if( item )
        {
          m_route->setCurrentItem( item );
        }
    }

  enableCommandButtons();
}

void TaskEditor::slotClearSearchInput()
{
  m_pointSearchInput->clear();

  if( m_wpListView->topLevelItemCount() > 0 )
    {
      m_wpListView->setCurrentItem( m_wpListView->topLevelItem(0) );
    }
}

void TaskEditor::slotSearchInputEdited( const QString& text )
{
  if( m_wpListView->topLevelItemCount() == 0 )
    {
      // List is empty, do nothing.
      return;
    }

  if( m_pointSearchInput->text().trimmed().isEmpty() )
    {
      // Search input is empty, go back to first list entry.
      m_wpListView->setCurrentItem( m_wpListView->topLevelItem(0) );
      return;
    }

  QList<QTreeWidgetItem *> items =
      m_wpListView->findItems( text,
			       Qt::MatchStartsWith,
                               m_pointColumnSelector->currentIndex() );

  if( items.size() > 0 )
    {
      m_wpListView->setCurrentItem( items.at(0) );
    }
}

void TaskEditor::slotTakeFoundItem()
{
  if( m_wpListView->topLevelItemCount() == 0 ||
      m_pointSearchInput->text().trimmed().isEmpty() )
    {
      // List or input are empty, do nothing.
      return;
    }

  // TODO Maybe we should restricted that a little bit more,
  // that the item must exist? Otherwise the current list item is taken.
  slotAddWaypoint();
}

void TaskEditor::slotSearchColumnIndexChanged( int /* index */ )
{
  if( m_wpListView->topLevelItemCount() == 0 ||
      m_pointSearchInput->text().trimmed().isEmpty() )
    {
      // List or input are empty, do nothing.
      return;
    }

  slotSearchInputEdited( m_pointSearchInput->text() );
}

void TaskEditor::slotMoveUp()
{
  int curPos = getCurrentPosition();

  if( curPos < 1 || m_route->topLevelItemCount() < 2 )
    {
      return;
    }

  Waypoint *wp = m_taskWpList.takeAt( curPos );
  m_taskWpList.insert( curPos - 1, wp );

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();
  setSelected( curPos - 1 );
}

void TaskEditor::slotMoveDown()
{
  int curPos = getCurrentPosition();

  if( curPos < 0 ||
      m_route->topLevelItemCount() < 2 ||
      curPos >= m_route->topLevelItemCount() - 1 ||
      curPos >= m_taskWpList.size() - 1 )
    {
      return;
    }

  Waypoint *wp = m_taskWpList.takeAt( curPos );
  m_taskWpList.insert( curPos + 1, wp );

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();
  setSelected( curPos + 1 );
}

void TaskEditor::slotDuplicateWayoint()
{
  int curPos = getCurrentPosition();

  if( curPos < 0 || m_taskWpList.count() == 0 )
    {
      // not possible to duplicate nothing
      return;
    }

  // Make a deep copy of waypoint before insert
  Waypoint *wp = new Waypoint(m_taskWpList.at( curPos ));
  m_taskWpList.insert( curPos + 1, wp );

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();
  setSelected( curPos + 1 );
}

void TaskEditor::slotInvertWaypoints()
{
  if ( m_taskWpList.count() < 2 )
    {
      // not possible to invert order, if elements are less 2
      return;
    }

  // invert list order
  for( int i= m_taskWpList.count()-2; i >= 0; i-- )
    {
      Waypoint *wp = m_taskWpList.at(i);
      m_taskWpList.removeAt(i);
      m_taskWpList.append( wp );
    }

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();

  // After an invert the first waypoint item is always selected.
  setSelected( 0 );
}

void TaskEditor::slotAddWaypoint()
{
  int pos = getCurrentPosition();

  if( pos < 0 )
    {
      pos = 0;
    }

  // Gets the selected item from the waypoint list.
  QTreeWidgetItem* item = m_wpListView->currentItem();

  if( item == 0 )
    {
      return;
    }

  Waypoint *newWp = 0;
  Waypoint *wp = 0;
  Airfield *af = 0;
  RadioPoint *rp = 0;
  SinglePoint *sp = 0;

  // Retrieve the point data from the selected  item. At first we have to find
  // the item type.
  if( item->data(0, Qt::UserRole).canConvert<WaypointPtr>() )
    {
      wp = item->data(0, Qt::UserRole).value<WaypointPtr>();

      // Make a deep copy of waypoint data.
      newWp = new Waypoint( wp );
    }
  else if( item->data(0, Qt::UserRole).canConvert<AirfieldPtr>() )
    {
      af = item->data(0, Qt::UserRole).value<AirfieldPtr>();
      newWp = new Waypoint;
      newWp->icao = af->getICAO();
      newWp->frequencyList = af->getFrequencyList();
      newWp->rwyList = af->getRunwayList();
      sp = af;
    }
  else if( item->data(0, Qt::UserRole).canConvert<RadioPointPtr>() )
    {
      rp = item->data(0, Qt::UserRole).value<RadioPointPtr>();
      newWp = new Waypoint;
      newWp->icao = rp->getICAO();
      newWp->frequencyList = rp->getFrequencyList();
      newWp->comment = rp->getAdditionalText();
      sp = rp;
    }
  else if( item->data(0, Qt::UserRole).canConvert<SinglePointPtr>() )
    {
      sp = item->data(0, Qt::UserRole).value<SinglePointPtr>();
      newWp = new Waypoint;
    }
  else
    {
      qWarning() << "TaskEditor::slotAddWaypoint(): Item data not handled!";
      return;
    }

  if( sp != 0 && newWp != 0 )
    {
      newWp->name = sp->getShortName();
      newWp->description = sp->getName();
      newWp->country = sp->getCountry();
      newWp->type = sp->getTypeID();
      newWp->origP = sp->getWGSPosition();
      newWp->elevation = sp->getElevation();

      if( newWp->comment.isEmpty() )
	{
	  newWp->comment = sp->getComment();
	}
      else
	{
	  newWp->comment += "\n\n" + sp->getComment();
	}
    }

  // Set projected coordinates
  newWp->projP = _globalMapMatrix->wgsToMap(newWp->origP);

  if( pos >= m_taskWpList.size() || pos < 0 )
    {
      m_taskWpList.append( newWp );
      pos = m_taskWpList.size() - 1;
    }
  else
    {
      m_taskWpList.insert(pos + 1, newWp);
      pos++;
    }

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();
  setSelected( pos );
}

void TaskEditor::slotRemoveWaypoint()
{
  int curPos = getCurrentPosition();

  if ( curPos < 0 || m_taskWpList.count() == 0 )
    {
      return;
    }

  delete m_route->takeTopLevelItem( curPos );
  delete m_taskWpList.takeAt( curPos );

  m_editedTask->setWaypointList( m_taskWpList );
  loadRouteWaypoints();

  // Remember last position.
  if( curPos >= m_taskWpList.size() )
    {
      setSelected( m_taskWpList.size() - 1);
    }
  else
    {
      setSelected( curPos );
    }
}

void TaskEditor::setTask(FlightTask *task)
{
  if( task == static_cast<FlightTask *>(0) )
    {
      // As fall back an internal empty task object is setup
      m_task = FlightTask( MapContents::instance()->genTaskName() );
      m_editedTask = &m_task;

      qWarning() << "TaskEditor::setTask(): Null object passed as task!";
    }
  else
    {
      m_editedTask = task;
    }

  // get waypoint list of task
  m_taskWpList = m_editedTask->getWPList();

  m_taskNameEditor->setText( m_editedTask->getFileName() );

  // Save initial name of task. Is checked during accept for change.
  m_taskInitName = m_taskNameEditor->text();

  m_planningTypes->setCurrentIndex( m_planningTypes->findText( FlightTask::ttItem2Text(m_editedTask->getPlanningType())) );

  m_left->setChecked(m_editedTask->getPlanningDirection() & FlightTask::leftOfRoute);
  m_right->setChecked(m_editedTask->getPlanningDirection() & FlightTask::rightOfRoute);

  slotSetPlanningType( FlightTask::ttItem2Text(m_editedTask->getPlanningType()) );
}

void TaskEditor::slotItemClicked( QTreeWidgetItem* item, int column )
{
  Q_UNUSED( item )
  Q_UNUSED( column )

  enableCommandButtons();
}

void TaskEditor::enableCommandButtons()
{
  if( m_taskWpList.count() == 0 )
    {
      m_addCmd->setEnabled( true );
      m_removeCmd->setEnabled( false );
      m_upCmd->setEnabled( false );
      m_downCmd->setEnabled( false );
      m_invertCmd->setEnabled( false );
    }
  else if( m_taskWpList.count() == 1 )
    {
      m_addCmd->setEnabled( true );
      m_removeCmd->setEnabled( true );
      m_upCmd->setEnabled( false );
      m_downCmd->setEnabled( false );
      m_invertCmd->setEnabled( false );
    }
  else
    {
      m_addCmd->setEnabled( true );
      m_removeCmd->setEnabled( true );
      m_invertCmd->setEnabled( true );

      if( m_route->topLevelItemCount() && m_route->currentItem() == 0 )
        {
          // If no item is selected we select the first one.
          m_route->setCurrentItem(m_route->topLevelItem(m_route->indexOfTopLevelItem(0)));
        }

      if( m_route->indexOfTopLevelItem(m_route->currentItem()) > 0 )
        {
          m_upCmd->setEnabled( true );
        }
      else
        {
          m_upCmd->setEnabled( false );
        }

      if( m_route->indexOfTopLevelItem(m_route->currentItem()) < m_route->topLevelItemCount() - 1 )
        {
          m_downCmd->setEnabled( true );
        }
      else
        {
          m_downCmd->setEnabled( false );
        }
    }
}

void TaskEditor::slotAccept()
{
  // Here we check the task constrains.
  if( m_taskInitName != m_taskNameEditor->text() )
    {
      // User has changed the task name. We must check, if the new name
      // is already in use.
      if( _globalMapContents->taskNameInUse( m_taskNameEditor->text()) )
        {
          QMessageBox::warning( this,
                                 tr("Task name already in use!"),
                                 tr("<html>The chosen task name is already in use!<br><br>"
                                    "Please enter another one.</html>"),
                                 QMessageBox::Ok );
          return;
        }
    }

  if( m_taskWpList.size() < 4 )
    {
      QMessageBox::warning( this,
                             tr("Task is incomplete!"),
                             tr("<html>A task consist of at least four waypoints!<br><br>"
                                "Please add the missing points."
                        	"<br><br>See Help menu for more information.</html>"),
                             QMessageBox::Ok );
      return;
    }

  if( m_taskWpList.size() > 5 && m_editedTask->getPlanningType() == FlightTask::FAIArea )
    {
      QMessageBox::warning( this,
                             tr("FAI area task violation!"),
                             tr("<html>A FAI area task can have one additional "
                                 "route point only!<br><br>"
                                 "Please remove all not needed other points."
                        	 "<br><br>See Help menu for more information.</html>"),
                             QMessageBox::Ok );
      return;
    }

  m_editedTask->setTaskName( m_taskNameEditor->text() );
  accept();
}
