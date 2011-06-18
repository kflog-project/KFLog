/***********************************************************************
**
**   objecttree.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <QtXml>

#include "flightgrouplistviewitem.h"
#include "flightlistviewitem.h"
#include "flightselectiondialog.h"
#include "mapcontents.h"
#include "objecttree.h"
#include "taskdialog.h"
#include "tasklistviewitem.h"
#include "mainwindow.h"

extern MainWindow  *_mainWindow;

ObjectTree::ObjectTree( QWidget *parent ) :
  KFLogTreeWidget( "ObjectTree", parent ),
  currentFlightElement(0)
{
  setObjectName( "ObjectTree" );
  setAcceptDrops(true);

  createMenu();
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

  FlightRoot = new QTreeWidgetItem(this);
  FlightRoot->setToolTip( 0, tr("All your read flights are to find under this node.") );
  FlightRoot->setText(0,tr("Flights"));
  FlightRoot->setFlags( Qt::ItemIsEnabled );
  FlightRoot->setIcon(0, _mainWindow->getPixmap("igc_16.png"));

  FlightGroupRoot = new QTreeWidgetItem(this);
  FlightGroupRoot->setToolTip( 0, tr("Grouped single flights are to find under this node.") );
  FlightGroupRoot->setText(0,tr("Groups"));
  FlightGroupRoot->setFlags( Qt::ItemIsEnabled );
  FlightGroupRoot->setIcon(0, _mainWindow->getPixmap("igc_16.png"));


  TaskRoot = new QTreeWidgetItem(this);
  TaskRoot->setToolTip( 0, tr("All yours tasks are to find under this node.") );
  TaskRoot->setText(0,tr("Tasks"));
  TaskRoot->setFlags( Qt::ItemIsEnabled );
  TaskRoot->setIcon(0, _mainWindow->getPixmap("task_16.png"));

  connect( this, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
           SLOT(slotSelectionChanged(QTreeWidgetItem *, int)) );

  connect( this, SIGNAL(rightButtonPressed( QTreeWidgetItem *, const QPoint&)),
           SLOT(slotShowObjectTreeMenu( QTreeWidgetItem*, const QPoint&)) );

  connect( this, SIGNAL(itemExpanded(QTreeWidgetItem *)),
           SLOT(slotResizeColumns2Content()) );
}

ObjectTree::~ObjectTree()
{
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
      // list view item we are dealing with:
      case FLIGHT_LIST_VIEW_ITEM_TYPEID:
        bfe = ((FlightListViewItem*) item)->flight;
        break;
      case FLIGHT_GROUP_LIST_VIEW_ITEM_TYPEID:
        bfe = ((FlightGroupListViewItem*) item)->flightGroup;
        break;
      case TASK_LIST_VIEW_ITEM_TYPEID:
        bfe = ((TaskListViewItem*) item)->task;
        break;
      default:
        bfe = static_cast<BaseFlightElement *>(0);
    }

  if( bfe && bfe != MapContents::instance()->getFlight() )
    {
      emit newFlightSelected( bfe );
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
  // qDebug() << "ObjectTree::slotFlightChanged()";

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

      if( ((FlightListViewItem*) childItem)->flight == bfe )
        {
          return childItem;
        }
    }

  for( int i = 0; i < FlightGroupRoot->childCount(); i++ )
    {
      QTreeWidgetItem* childItem = FlightGroupRoot->child( i );

      if( ((FlightListViewItem*) childItem)->flight == bfe )
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
      return currentFlightElement->getObjectType();
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
  if( bfe->getObjectType() == BaseMapElement::Flight )
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
  Q_UNUSED( item )
  Q_UNUSED( position )

  // task items
  actionTaskEdit->setEnabled(TaskRoot->childCount() && currentFlightElementType() == BaseMapElement::Task);
  actionTaskDelete->setEnabled(TaskRoot->childCount() && currentFlightElementType() == BaseMapElement::Task);
  actionTaskSave->setEnabled(TaskRoot->childCount() && currentFlightElementType() == BaseMapElement::Task);
  actionTaskSaveAll->setEnabled(TaskRoot->childCount() && currentFlightElementType() == BaseMapElement::Task);

  // flight items
  actionFlightGroupNew->setEnabled(FlightRoot->childCount());
  actionFlightGroupEdit->setEnabled(FlightRoot->childCount() && currentFlightElementType() == BaseMapElement::FlightGroup);

  QTreeWidgetItem* curentItem = item;

  if( item )
    {
      // We allow the close of an item only, when the right child item is selected.
      if( curentItem->parent() == FlightRoot )
        {
          actionFlightClose->setEnabled( true );
          actionFlightClose->setText( tr("Close flight") );
        }
      else if( curentItem->parent() == FlightGroupRoot )
        {
          actionFlightClose->setEnabled( true );
          actionFlightClose->setText( tr("Close flight group") );
        }
      else
        {
          actionFlightClose->setEnabled( false );
        }
    }
  else
    {
      actionFlightClose->setEnabled( false );
    }

  actionFlightOptimize->setEnabled(FlightRoot->childCount() && currentFlightElementType() == BaseMapElement::Flight);
  actionFlightOptimizeOLC->setEnabled(FlightRoot->childCount() && currentFlightElementType() == BaseMapElement::Flight);

  objectTreeMenu->exec( QCursor::pos() );
}

void ObjectTree::createMenu()
{
  objectTreeMenu = new QMenu(this);
  objectTreeMenu->setTitle( tr("Flights") );

  objectTreeMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                       tr("&Open flight"),
                       this,
                       SIGNAL(openFlight()) );

  actionFlightGroupNew = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                                                    tr("New flight &group"),
                                                    this,
                                                    SIGNAL(newFlightGroup()) );

  actionFlightGroupEdit = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                               tr("Edit flight group"),
                                               this,
                                               SIGNAL(editFlightGroup()) );

  actionFlightOptimize = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                              tr("O&ptimize flight"),
                                              this,
                                              SIGNAL(optimizeFlight()) );

  actionFlightOptimizeOLC = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                                 tr("O&ptimize flight for OLC"),
                                                 this,
                                                 SIGNAL(optimizeFlightOLC()) );

  actionFlightClose = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
                                           tr("&Close flight or flight group"),
                                           this,
                                           SLOT(slotCloseFlightElement()) );
  objectTreeMenu->addSeparator();

  objectTreeMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                       tr("&New task"),
                       this,
                       SIGNAL(newTask()) );

  objectTreeMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                       tr("Open &task"),
                       this,
                       SIGNAL(openTask()) );

  actionTaskEdit = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                        tr("&Edit task"),
                                        this,
                                        SLOT(slotEditTask()) );

  actionTaskDelete = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
                                          tr("&Close task"),
                                          this,
                                          SLOT(slotCloseFlightElement()) );
  objectTreeMenu->addSeparator();

  actionTaskSave = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_filesave_16.png"),
                                        tr("&Save this task"),
                                        this,
                                        SLOT(slotSaveTask()) );

  actionTaskSaveAll = objectTreeMenu->addAction( _mainWindow->getPixmap("kde_save_all_16.png"),
                                           tr("Save &all task's"),
                                           this,
                                           SLOT(slotSaveAllTask()) );
}

void ObjectTree::slotEditTask()
{
  if( currentFlightElement != 0 )
    {
      if( currentFlightElement->getObjectType() == BaseMapElement::Task )
        {
          FlightTask *ft = dynamic_cast<FlightTask *> (currentFlightElement);

          TaskDialog* td = new TaskDialog( this );

          td->setTask( ft );

          if( td->exec() == QDialog::Accepted )
            {
              emit newFlightSelected( ft );
            }

          delete td;
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
    child.setAttribute("Frequency", w->frequency);
    child.setAttribute("Landable", w->isLandable);
    child.setAttribute("Runway", w->runway.first);
    child.setAttribute("Length", w->length);
    child.setAttribute("Surface", w->surface);
    child.setAttribute("Comment", w->comment);

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
        child.setAttribute("Frequency", w->frequency);
        child.setAttribute("Landable", w->isLandable);
        child.setAttribute("Runway", w->runway.first);
        child.setAttribute("Length", w->length);
        child.setAttribute("Surface", w->surface);
        child.setAttribute("Comment", w->comment);

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

void ObjectTree::dropEvent( QDropEvent* event )
{
  QList<QUrl> urlList = event->mimeData()->urls();

  for( int i = 0; i < urlList.size(); i++ )
    {
      emit openFile( urlList.at(i) );
    }
}
