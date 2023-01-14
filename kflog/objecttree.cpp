/***********************************************************************
**
**   objecttree.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>
#include <QFileDialog>
#include <QtXml>

#include "airspacelistviewitem.h"
#include "flightgrouplistviewitem.h"
#include "flightlistviewitem.h"
#include "flightselectiondialog.h"
#include "Frequency.h"
#include "mapcontents.h"
#include "objecttree.h"
#include "TaskEditor.h"
#include "tasklistviewitem.h"
#include "mainwindow.h"

extern MainWindow  *_mainWindow;

// Item types to be used for root items
#define FlightRootItem 		1
#define FlightRootGroupItem 	2
#define TaskRootItem 		3

ObjectTree::ObjectTree( QWidget *parent ) :
  KFLogTreeWidget( "ObjectTree", parent ),
  currentFlightElement(0)
{
  setObjectName( "ObjectTree" );
  setToolTip( tr("Select a tree node and press right mouse button to open the action menu.") );

  // Enable drag drop of file to export them.
  setAcceptDrops(true);
  setDropIndicatorShown(true);

  setHelpText();
  createMenus();
  /*
   * setup tree view
   */
  setSortingEnabled( false );
  setAllColumnsShowFocus( true );
  setFocusPolicy( Qt::StrongFocus );
  setRootIsDecorated( true );
  setSelectionMode( QAbstractItemView::SingleSelection );
  setSelectionBehavior( QAbstractItemView::SelectRows );
  addRowSpacing( 5 );
  setColumnCount( 2 );

  QStringList headerLabels;

  headerLabels  << tr("Name")
                << tr("Description");

  setHeaderLabels( headerLabels );

  QTreeWidgetItem* headerItem = QTreeWidget::headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );

  // Store used columns
  colName = 0;
  colDesc = 1;

  // Load the save header configuration.
  loadConfig();

  FlightRoot = new QTreeWidgetItem(this, FlightRootItem);
  FlightRoot->setToolTip( 0, tr("All your loaded flights are to find under this node.") );
  FlightRoot->setText(0,tr("Flights"));
  FlightRoot->setFlags( Qt::ItemIsEnabled );
  FlightRoot->setIcon(0, _mainWindow->getPixmap("igc_16.png"));

  FlightGroupRoot = new QTreeWidgetItem(this, FlightRootGroupItem);
  FlightGroupRoot->setToolTip( 0, tr("Grouped single flights are to find under this node.") );
  FlightGroupRoot->setText(0,tr("Groups"));
  FlightGroupRoot->setFlags( Qt::ItemIsEnabled );
  FlightGroupRoot->setIcon(0, _mainWindow->getPixmap("igc_16.png"));

  TaskRoot = new QTreeWidgetItem(this, TaskRootItem);
  TaskRoot->setToolTip( 0, tr("All yours tasks are to find under this node.") );
  TaskRoot->setText(0,tr("Tasks"));
  TaskRoot->setFlags( Qt::ItemIsEnabled );
  TaskRoot->setIcon(0, _mainWindow->getPixmap("task_16.png"));

  connect( this, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
           SLOT(slotSelectionChanged(QTreeWidgetItem *, int)) );

  connect( this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
           SLOT(slotItemDoubleClicked(QTreeWidgetItem*,int)));

  connect( this, SIGNAL(rightButtonPressed( QTreeWidgetItem *, const QPoint&)),
           SLOT(slotShowObjectTreeMenu( QTreeWidgetItem*, const QPoint&)) );

  connect( this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
           SLOT(slotResizeColumns2Content()) );

  connect( this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
           SLOT(slotItemExpanded(QTreeWidgetItem *)) );
}

ObjectTree::~ObjectTree()
{
}

void ObjectTree::setHelpText()
{
  setWhatsThis( tr(
   "<html><b>The object tree help</b><br><br>"
   "The object tree depicts the opened flights and tasks under three root nodes:"
   "<ul>"
   "<li><i>Flights</i> Contains all opened flights."
   "<li><i>Groups</i>  Contains groups of flights. All flights of a group are drawn together at the map."
   "<li><i>Tasks</i>   Contains all opened flight tasks."
   "</ul>"
   "Node related actions are provided via popup menus. Select a tree node and "
   "press the <i>Right</i> mouse button to open the menu."
   "<br><br>"
   "Flight and task files can be also imported by using drop and drag actions."
   "<br><br></html>"
  ) );
}

/**
 * Called if a new flight has been added.
 */
void ObjectTree::slotNewFlightAdded( Flight* flight )
{
  // qDebug() << "ObjectTree::slotNewFlightAdded";

  new FlightListViewItem( FlightRoot, flight );
  FlightRoot->sortChildren( 0, Qt::AscendingOrder );
  slotResizeColumns2Content();
}

/**
 * Called if a new flight group has been created or loaded.
 */
void ObjectTree::slotNewFlightGroupAdded( FlightGroup* flightGroup )
{
  new FlightGroupListViewItem( FlightGroupRoot, flightGroup );
  FlightGroupRoot->sortChildren( 0, Qt::AscendingOrder );
  slotResizeColumns2Content();
}

/**
 * Called if a new task has been created or loaded.
 */
void ObjectTree::slotNewTaskAdded( FlightTask* task )
{
  // qDebug() << "ObjectTree::slotNewTaskAdded() Task=" << task;

  new TaskListViewItem( TaskRoot, task );
  TaskRoot->sortChildren( 0, Qt::AscendingOrder );
  slotResizeColumns2Content();
}

/** Called if the user has changed the selection. */
void ObjectTree::slotSelectionChanged( QTreeWidgetItem* item, int column )
{
  // qDebug() << "ObjectTree::slotSelectionChanged() text(0)=" << item->text(0) << "Type=" << item->type();

  Q_UNUSED( column )

  if( item == static_cast<QTreeWidgetItem *>(0) )
    {
      // No item has been passed, ignore call.
      return;
    }

  BaseFlightElement* bfe = static_cast<BaseFlightElement *>(0);

  switch( item->type() )
    {
      // The Run Time Type Identification is used, to see what kind of
      // list view item we are dealing with. The passed item can also be a
      // root item of the list.
      case FLIGHT_LIST_VIEW_ITEM_TYPEID:
	{
	  FlightListViewItem *fi = dynamic_cast<FlightListViewItem *>(item);

	  if( fi )
	    {
	      bfe = fi->getFlight();
	    }

	  break;
	}
      case FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID:
	{
	  FlightGroupListViewItem *fgi = dynamic_cast<FlightGroupListViewItem *>(item);

	  if( fgi )
	    {
	      bfe = fgi->flightGroup;
	    }

	  break;
	}
      case TASK_LIST_VIEW_ITEM_TYPEID:
	{
	  TaskListViewItem *ti = dynamic_cast<TaskListViewItem *>(item);

	  if( ti )
	    {
	      bfe = ti->task;
	    }

	  break;
	}
      case AIRSPACE_FLAG_LIST_VIEW_ITEM_TYPEID:
	{
	  AirSpaceListViewItem::AirSpaceFlagListViewItem *aai =
	      dynamic_cast<AirSpaceListViewItem::AirSpaceFlagListViewItem *>(item);

	  if( aai )
	    {
	      bfe = aai->getFlight();
	    }

	  break;
	}
      default:
        bfe = static_cast<BaseFlightElement *>(0);
        break;
    }

  if( bfe == 0 )
    {
      // Seems to be a root item, ignore that selection.
      return;
    }

  if( bfe != MapContents::instance()->getFlight() )
    {
      emit newFlightSelected( bfe );
      return;
    }

  // Check, if current flight is drawn in altitude mode. If not make a
  // flight redrawing.
  if( item && item->type() == FLIGHT_LIST_VIEW_ITEM_TYPEID )
    {
      FlightListViewItem * fItem = (FlightListViewItem *) item;

      if( fItem->getFlight()->getDrawFlightPointType() != MapConfig::Altitude )
        {
          fItem->getFlight()->setDrawFlightPointType( MapConfig::Altitude );
          emit showCurrentFlight();
          fItem->activate();
          return;
        }
    }

  if( item && item->type() == AIRSPACE_FLAG_LIST_VIEW_ITEM_TYPEID )
    {
      // If the item is selected, we update the flight according to the item data.
      AirSpaceListViewItem::AirSpaceFlagListViewItem* aItem = (AirSpaceListViewItem::AirSpaceFlagListViewItem *) item;

      aItem->getFlight()->setDrawFlightPointType( MapConfig::Airspace );
      emit showCurrentFlight();
      aItem->activate();
      return;
    }
}

void ObjectTree::slotItemDoubleClicked( QTreeWidgetItem *item, int /* column */ )
{
  switch( item->type() )
    {
      case FLIGHT_LIST_VIEW_ITEM_TYPEID:
        {
          FlightListViewItem * fItem = (FlightListViewItem *) item;

          if( fItem->getFlight() != currentFlightElement )
            {
              // Flight selection was changed, do noting in this case.
              return;
            }

          fItem->getFlight()->setDrawFlightPointType( MapConfig::Altitude );
          emit showCurrentFlight();
          fItem->activate();
        }

      break;

      default:
      break;
  }
}

void ObjectTree::slotItemExpanded( QTreeWidgetItem* item )
{
  if( item && item->type() == AIRSPACE_FLAG_LIST_VIEW_ITEM_TYPEID )
    {
      // If the item is expanded, we update the flight according to the item data.
      AirSpaceListViewItem::AirSpaceFlagListViewItem* aItem = (AirSpaceListViewItem::AirSpaceFlagListViewItem *) item;

      if( aItem->getFlight() != currentFlightElement )
        {
          // Flight selection was changed, do noting in this case.
          return;
        }

      aItem->getFlight()->setDrawFlightPointType( MapConfig::Airspace );
      emit showCurrentFlight();
      aItem->activate();
    }
}

/** This slot is called if the currently selected flight has changed. */
void ObjectTree::slotSelectedFlightChanged()
{
  // qDebug() << "ObjectTree::slotSelectedFlightChanged()";

  BaseFlightElement* bfe = MapContents::instance()->getFlight();

  if( ! bfe )
    {
      return;
    }

  QTreeWidgetItem* item = findFlightElement( bfe );

  if( item )
    {
      // That includes a selection.
      setCurrentItem( item );

      slotFlightChanged();
      scrollToItem( item );
      currentFlightElement = MapContents::instance()->getFlight();
    }
}

/** Signaled if the current flight was somehow changed.  */
void ObjectTree::slotFlightChanged()
{
  QTreeWidgetItem* item = findFlightElement( MapContents::instance()->getFlight() );

  if (item)
    {
      switch( item->type() )
        {
          // the Run Time Type Identification is used to see what kind of
          // list view item we are dealing with.
          case FLIGHT_LIST_VIEW_ITEM_TYPEID:
            ((FlightListViewItem*)item)->update();

            // A flight can be contained in a flight group. So all flight groups
            // need an update too.
            for( int i = 0; i < FlightGroupRoot->childCount(); i++ )
              {
                QTreeWidgetItem* item = FlightGroupRoot->child( i );

                if( item->type() == FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID )
                  {
                    static_cast<FlightGroupListViewItem *>(item)->update();
                  }
              }
            break;

          case FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID:
            ((FlightGroupListViewItem*)item)->update();
            break;

          case TASK_LIST_VIEW_ITEM_TYPEID:
            ((TaskListViewItem*)item)->update();
            break;

          default:
            qWarning("ObjectTree::slotFlightChanged(): List view item of unknown type!");
            break;
        }

      slotResizeColumns2Content();
    }
}

/**
 * Searches the object tree for the node representing the base flight element
 * given as an argument.
 * @returns a pointer to the QTreeWidgetItem if found, otherwise 0.
 */
QTreeWidgetItem* ObjectTree::findFlightElement( BaseFlightElement* bfe )
{
  for( int i = 0; i < FlightRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = FlightRoot->child( i );

      if( ((FlightListViewItem*) childItem)->getFlight() == bfe )
        {
          return childItem;
        }
    }

  for( int i = 0; i < FlightGroupRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = FlightGroupRoot->child( i );

      if( ((FlightListViewItem*) childItem)->getFlight() == bfe )
        {
          return childItem;
        }
    }

  for( int i = 0; i < TaskRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = TaskRoot->child( i );

      if( ((TaskListViewItem*) childItem)->task == bfe )
        {
          return childItem;
        }
    }

  return static_cast<QTreeWidgetItem *>(0);
}

/*
  Returns -1 if no element has been selected or the type of the selected
  element otherwise.
*/
int ObjectTree::currentFlightElementType()
{
  if( !currentFlightElement )
    {
      return -1;
    }
  else
    {
      return currentFlightElement->getTypeID();
    }
}

void ObjectTree::slotCloseFlight( BaseFlightElement* bfe )
{
  QTreeWidgetItem* item = findFlightElement( bfe );

  if( bfe == currentFlightElement )
    {
      currentFlightElement = 0;
    }

  delete item;

  // If a flight was deleted, all flight groups must be updated too because
  // the flight could be contained in one or all of them.
  if( bfe->getTypeID() == BaseMapElement::Flight )
    {
      for( int i = 0; i < FlightGroupRoot->childCount(); i++ )
        {
          QTreeWidgetItem* item = FlightGroupRoot->child( i );

          if( item->type() == FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID )
            {
               static_cast<FlightGroupListViewItem *>(item)->update();
            }
        }
    }
}

void ObjectTree::slotShowObjectTreeMenu(QTreeWidgetItem *item, const QPoint &position )
{
  Q_UNUSED( position )

  bool enable = false;

  // Show only the related menu depending on the item's tree position

  if( item )
    {
      if( item->type() == TaskRootItem ||
	  item->type() == TASK_LIST_VIEW_ITEM_TYPEID )
	{
	  enable = ( TaskRoot->childCount() ) ? true : false;

	  actionTaskEdit->setEnabled(enable);
	  actionTaskClose->setEnabled(enable);
	  actionTaskSave->setEnabled(enable);
	  actionTaskSaveAll->setEnabled(enable);

	  taskMenu->exec( QCursor::pos() );
	}
      else if( item->type() == FlightRootItem ||
	       item->type() == FLIGHT_LIST_VIEW_ITEM_TYPEID )
	{
	  bool enable = ( FlightRoot->childCount() > 0 ) ? true : false;

	  actionFlightClose->setEnabled(enable);
	  actionFlightSetQNH->setEnabled(enable);
	  actionFlightOptimize->setEnabled(enable);
	  actionFlightOptimizeOLC->setEnabled(enable);

	  flightMenu->exec( QCursor::pos() );
	}
      else if( item->type() == FlightRootGroupItem ||
	       item->type() == FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID )
	{
	  enable = ( FlightGroupRoot->childCount() ) ? true : false;

	  actionFlightGroupEdit->setEnabled(enable);
	  actionFlightGroupClose->setEnabled(enable);

	  if( FlightRoot->childCount() > 0 )
	    {
	      flightGroupMenu->exec( QCursor::pos() );
	    }
	}
      else if( item->type() == AIRSPACE_LIST_VIEW_ITEM_TYPEID ||
	       item->type() == AIRSPACE_FLAG_LIST_VIEW_ITEM_TYPEID )
	{
	  airspaceMenu->exec( QCursor::pos() );
	}
    }
}

void ObjectTree::createMenus()
{
  flightMenu = new QMenu(this);
  flightMenu->setTitle( tr("Flights") );

  actionFlightOpen = flightMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
					   tr("Open flight"),
					   this,
					   SIGNAL(openFlight()) );

  actionFlightOptimize = flightMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
						tr("Optimize flight"),
						this,
						SIGNAL(optimizeFlight()) );

  actionFlightSetQNH = flightMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
					      tr("Set QNH"),
					      this,
					      SIGNAL(setFlightQNH()) );

  actionFlightOptimizeOLC = flightMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
						   tr("Optimize flight for OLC"),
						   this,
						   SIGNAL(optimizeFlightOLC()) );

  actionFlightClose = flightMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
					     tr("Close flight"),
					     this,
					     SLOT(slotCloseFlightElement()) );

  airspaceMenu = new QMenu(this);
  airspaceMenu->setTitle( tr("Airspaces") );

  airspaceMenu->addAction( _mainWindow->getPixmap( "kde_wizard_16.png"),
						   tr("Set QNH"),
						   this,
						   SIGNAL(setFlightQNH()) );

  airspaceMenu->addAction( tr("Check Airspace violations"),
			   this,
			   SIGNAL(updateFlightWindows()) );

  flightGroupMenu = new QMenu(this);
  flightGroupMenu->setTitle( tr("Flight Groups") );


  actionFlightGroupNew = flightGroupMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
						     tr("New flight group"),
						     this,
						     SIGNAL(newFlightGroup()) );

  actionFlightGroupEdit = flightGroupMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
						      tr("Edit flight group"),
						      this,
						      SIGNAL(editFlightGroup()) );

  actionFlightGroupClose = flightGroupMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
						       tr("Close flight group"),
						       this,
						       SLOT(slotCloseFlightElement()) );

  taskMenu = new QMenu(this);
  taskMenu->setTitle( tr("Tasks") );

  actionTaskNew = taskMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
				       tr("New task"),
				       this,
				       SIGNAL(newTask()) );

  actionTaskOpen = taskMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
					tr("Open task"),
					this,
					SIGNAL(openTask()) );

  actionTaskEdit = taskMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                        tr("Edit task"),
                                        this,
                                        SLOT(slotEditTask()) );

  actionTaskClose = taskMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
					tr("Close task"),
					this,
					SLOT(slotCloseFlightElement()) );

  taskMenu->addSeparator();

  actionTaskSave = taskMenu->addAction( _mainWindow->getPixmap("kde_filesave_16.png"),
					tr("Save this task"),
					this,
					SLOT(slotSaveTask()) );

  actionTaskSaveAll = taskMenu->addAction( _mainWindow->getPixmap("kde_save_all_16.png"),
					   tr("Save all tasks"),
					   this,
					   SLOT(slotSaveAllTask()) );
}

void ObjectTree::slotEditTask()
{
  if( currentFlightElement != 0 )
    {
      if( currentFlightElement->getTypeID() == BaseMapElement::Task )
        {
          FlightTask *ft = dynamic_cast<FlightTask *> (currentFlightElement);

          TaskEditor* te = new TaskEditor( this );

          te->setTask( ft );

          if( te->exec() == QDialog::Accepted )
            {
              emit newFlightSelected( ft );
            }

          delete te;
        }
    }
}

/*
 Used to close a flight element in general, not only a task
*/
void ObjectTree::slotCloseFlightElement()
{
  if( currentFlightElementType() == BaseMapElement::Task ||
      currentFlightElementType() == BaseMapElement::Flight ||
      currentFlightElementType() == BaseMapElement::FlightGroup)
    {
      emit closeFlightElement();
    }
}

void ObjectTree::slotSaveTask()
{
  FlightTask *ft;
  QString fName;
  QList<Waypoint*> wpList;

  // check if we are dealing with a task, and if so, set ft to reference the
  // flight task otherwise exit.
  if( currentFlightElementType() == BaseMapElement::Task )
    {
      ft = (FlightTask*) currentFlightElement;
    }
  else
    {
      return;
    }

  fName = QFileDialog::getSaveFileName( this,
                                        tr("Save task"),
                                        _mainWindow->getApplicationTaskDirectory() + "/" + ft->getFileName(),
                                        tr("KFLOG tasks (*.kflogtsk *.KFLOGTSK)" ) );

  if ( fName.isEmpty() )
    {
      return;
    }

  if (fName.right(9).toLower() != ".kflogtsk")
    {
      fName += ".kflogtsk";
    }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  doc.appendChild(root);

  QDomElement t = doc.createElement("Task");
  t.setAttribute("Name", ft->getFileName());
  t.setAttribute("Type", ft->getTaskTypeString());
  t.setAttribute("PlanningType", ft->getPlanningType());
  t.setAttribute("PlanningDirection", ft->getPlanningDirection());
  root.appendChild(t);

  wpList = ft->getWPList();

  for(int i = 0; i < wpList.count(); i++)
  {
    Waypoint *w = wpList.at(i);

    QDomElement child = doc.createElement("Waypoint");
    child.setAttribute("Name", w->name);
    child.setAttribute("Description", w->description);
    child.setAttribute("ICAO", w->icao);
    child.setAttribute("Type", w->type);
    child.setAttribute("Latitude", w->origP.lat());
    child.setAttribute("Longitude", w->origP.lon());
    child.setAttribute("Elevation", w->elevation);

    Frequency fq;
    Frequency::getMainFrequency( w->frequencyList, fq );

    child.setAttribute("Frequency", fq.getValue());
    child.setAttribute("Comment", w->comment);

    Runway rwy;

    if( w->rwyList.size() > 0 )
      {
        rwy = Runway::getMainRunway( w->rwyList );
      }

    quint16 head1 = rwy.getHeading() / 10;
    quint16 head2 = ( head1 <= 18 ) ? (head1 + 18) : (head1 - 18);

    child.setAttribute( "Landable", (head1 > 0) ? true : false );
    child.setAttribute( "Runway", head1 * 256 + head2 & 0xff );
    child.setAttribute( "Length", (int) rwy.getLength() );
    child.setAttribute( "Surface", rwy.getSurface() );
    t.appendChild(child);
  }

  QFile file( fName );

  if( file.open( QIODevice::WriteOnly ) )
    {
      const int IndentSize = 4;

      QTextStream out( &file );
      doc.save( out, IndentSize );
      file.close();
      path = fName;
    }
  else
    {
      QMessageBox::warning( this, tr( "No permission" ),
          "<html>" +
          tr("<B>%1</B><BR>permission denied!" ).arg( fName ) + "</html>",
          QMessageBox::Ok );
    }

  QApplication::restoreOverrideCursor();
}

void ObjectTree::slotUpdateAllFlights()
{
  for( int i = 0; i < FlightRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = FlightRoot->child( i );

      FlightListViewItem* flvi = dynamic_cast<FlightListViewItem*>(childItem);

      if( flvi )
        {
          flvi->update();
        }
    }

  // A flight can be contained in a flight group. So all flight groups
  // need an update too.
  for( int i = 0; i < FlightGroupRoot->childCount(); i++ )
    {
      QTreeWidgetItem* item = FlightGroupRoot->child( i );

      if( item->type() == FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID )
        {
          static_cast<FlightGroupListViewItem *>(item)->update();
        }
    }
}

void ObjectTree::slotSaveAllTask()
{
  FlightTask *ft;
  QString fName;
  QList<Waypoint*> wpList;

  fName = QFileDialog::getSaveFileName ( this,
                                         tr("Save all tasks"),
                                         _mainWindow->getApplicationTaskDirectory(),
                                         tr("KFLOG tasks (*.kflogtsk *.KFLOGTSK)" ) );
  if( fName.isEmpty() )
    {
      return;
    }

  if( fName.right( 9 ).toLower() != ".kflogtsk" )
    {
      fName += ".kflogtsk";
    }

  QApplication::setOverrideCursor( Qt::WaitCursor  );

  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  doc.appendChild(root);

  for( int i = 0; i < TaskRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = TaskRoot->child( i );
      ft = (dynamic_cast<TaskListViewItem *>(childItem))->task;

      QDomElement t = doc.createElement("Task");
      t.setAttribute("Name", ft->getFileName());
      t.setAttribute("Type", ft->getTaskTypeString());
      t.setAttribute("PlanningType", ft->getPlanningType());
      t.setAttribute("PlanningDirection", ft->getPlanningDirection());
      root.appendChild(t);

      wpList = ft->getWPList();

      for(int i = 0; i < wpList.count(); i++)
      {
          Waypoint *w = wpList.at(i);

        QDomElement child = doc.createElement("Waypoint");
        child.setAttribute("Name", w->name);
        child.setAttribute("Description", w->description);
        child.setAttribute("ICAO", w->icao);
        child.setAttribute("Type", w->type);
        child.setAttribute("Latitude", w->origP.lat());
        child.setAttribute("Longitude", w->origP.lon());
        child.setAttribute("Elevation", w->elevation);

        Frequency fq;
        Frequency::getMainFrequency( w->frequencyList, fq );

        child.setAttribute("Frequency", fq.getValue());
        child.setAttribute("Comment", w->comment);

        Runway rwy;

        if( w->rwyList.size() > 0 )
          {
            rwy = Runway::getMainRunway( w->rwyList );
          }

        quint16 head1 = rwy.getHeading() / 10;
        quint16 head2 = ( head1 <= 18 ) ? (head1 + 18) : (head1 - 18);

        child.setAttribute( "Landable", (head1 > 0) ? true : false );
        child.setAttribute( "Runway", head1 * 256 + head2 & 0xff );
        child.setAttribute( "Length", (int) rwy.getLength() );
        child.setAttribute( "Surface", rwy.getSurface() );
        t.appendChild(child);
      }
    }

  QFile file( fName );

  if( file.open( QIODevice::WriteOnly ) )
    {
      const int IndentSize = 4;

      QTextStream out( &file );
      doc.save( out, IndentSize );
      file.close();
      path = fName;
    }
  else
    {
      QMessageBox::warning( this,
                            tr("Permission denied"), "<html>" +
                            tr("<B>%1</B><BR>permission denied!").arg(fName) +
                            "</html>",
                            QMessageBox::Ok );
    }

  QApplication::restoreOverrideCursor();
}

void ObjectTree::dragEnterEvent( QDragEnterEvent* event )
{
  if( event->mimeData()->hasUrls() )
    {
      event->acceptProposedAction();
    }
}

void ObjectTree::dragMoveEvent( QDragMoveEvent * event )
  {
    if( event->mimeData()->hasUrls() )
      {
        event->acceptProposedAction();
      }
  }

void ObjectTree::dropEvent( QDropEvent* event )
{
  QList<QUrl> urlList = event->mimeData()->urls();

  for( int i = 0; i < urlList.size(); i++ )
    {
      emit openFile( urlList.at(i) );
    }
}
