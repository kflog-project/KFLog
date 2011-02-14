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
//#include <Qt3Support>

#include "flightgrouplistviewitem.h"
#include "flightlistviewitem.h"
#include "flightselectiondialog.h"
#include "mapcontents.h"
#include "objecttree.h"
#include "taskdialog.h"
#include "tasklistviewitem.h"
#include "mainwindow.h"

extern MainWindow  *_mainWindow;
extern MapContents *_globalMapContents;

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

  QTreeWidgetItem* headerItem = waypointTree->headerItem();
  headerItem->setTextAlignment( 0, Qt::AlignCenter );
  headerItem->setTextAlignment( 1, Qt::AlignCenter );

  // Store used columns
  colName = 0;
  colDesc = 1;

  // Load the save header configuration.
  loadConfig();

  FlightRoot = new QTreeWidgetItem(this);
  FlightRoot->setText(0,tr("Flights"));
  FlightRoot->setFlags( Qt::ItemIsEnabled );
  FlightRoot->setIcon(0, _mainWindow->getPixmap("igc_16.png"));

  TaskRoot = new QTreeWidgetItem(this);
  TaskRoot->setText(0,tr("Tasks"));
  TaskRoot->setFlags( Qt::ItemIsEnabled );
  TaskRoot->setIcon(0, _mainWindow->getPixmap("task_16.png"));

  connect( this, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
           SLOT(slotSelectionChanged(QTreeWidgetItem *, int)) );

  connect( this, SIGNAL(rightButtonPressed( QTreeWidgetItem *, const QPoint&)),
           SLOT( slotShowTaskMenu( QTreeWidgetItem*, const QPoint&)) );
}

ObjectTree::~ObjectTree()
{
  qDebug() << "~ObjectTree()";
}

/**
 * Called if a new flight has been added.
 */
void ObjectTree::slotNewFlightAdded( Flight* flight )
{
  new FlightListViewItem( FlightRoot, flight );
}

/**
 * Called if a new flight group has been created or loaded.
 */
void ObjectTree::slotNewFlightGroupAdded( FlightGroup* flightGroup )
{
  new FlightGroupListViewItem( FlightRoot, flightGroup );
}

/**
 * Called if a new task has been created or loaded.
 */
void ObjectTree::slotNewTaskAdded( FlightTask* task )
{
  new TaskListViewItem( TaskRoot, task );
}

/** Called if the selection has changed. */
void ObjectTree::slotSelectionChanged( QTreeWidgetItem * item, int column )
{
  extern MapContents *_globalMapContents;

  if( item == 0 )
    {
      return;
    }

  BaseFlightElement* flt = 0;

  switch( item->rtti() )
    {
      // The Run Time Type Identification is used, to see what kind of
      // list view item we are dealing with:
      case FLIGHT_LIST_VIEW_ITEM_TYPEID:
        flt=((FlightListViewItem*)itm)->flight;
        break;
      case FLIGHTGROUPLISTVIEWITEM_TYPEID:
        flt=((FlightGroupListViewItem*)itm)->flightGroup;
        break;
      case TASKLISTVIEWITEM_TYPEID:
        flt=((TaskListViewItem*)itm)->task;
        break;
      default:
        flt=0;
    }

  if( flt && flt != _globalMapContents->getFlight() )
    {
      emit selectedFlight( flt );
    }
}

/** This slot is called if the currently selected flight has changed. */
void ObjectTree::slotSelectedFlightChanged()
{
  QTreeWidgetItem* item = findFlightElement( _globalMapContents->getFlight() );

  if( item )
    {
      if( !item->isSelected() )
        setSelected( item, true );
      slotFlightChanged();
      ensureItemVisible( item);
      currentFlightElement = _globalMapContents->getFlight();
      return;
    }
}

/** Signaled if the current flight was somehow changed.  */
void ObjectTree::slotFlightChanged()
{
  extern MapContents *_globalMapContents;
  Q3ListViewItem * item=findFlightElement(_globalMapContents->getFlight());

  if (item) {
    switch (item->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
      case FLIGHT_LIST_VIEW_ITEM_TYPEID:
        ((FlightListViewItem*)item)->update();
        break;
      case FLIGHTGROUPLISTVIEWITEM_TYPEID:
        ((FlightGroupListViewItem*)item)->update();
        break;
      case TASKLISTVIEWITEM_TYPEID:
        ((TaskListViewItem*)item)->update();
        break;
      default:
        qWarning("Listviewitem of unknown type");
    }
  }
}

/**
 * Searches the object tree for the node representing the base flight element
 * given as an argument.
 * @returns a pointer to the QListViewItem if found, 0 otherwise.
 */
Q3ListViewItem * ObjectTree::findFlightElement(BaseFlightElement * bfe)
{
  Q3ListViewItem * item=0;

  if (FlightRoot->childCount()!=0) {
    item = FlightRoot->firstChild();
    while (item!=0) {
      if (((FlightListViewItem*)item)->flight==bfe) {
        return item;
      }
      item=item->nextSibling();
    }
  }

  if (TaskRoot->childCount()!=0) {
    item = TaskRoot->firstChild();
    while (item!=0) {
      if (((TaskListViewItem*)item)->task==bfe) {
        return item;
      }
      item=item->nextSibling();
    }
  }

  return 0;
}

/*
  Returns -1 if no element has been selected, and the rtti() of the selected element otherwise.
*/
int ObjectTree::currentFlightElementType()
{
  if (!currentFlightElement) {
    return -1;
  } else {
    return currentFlightElement->getObjectType();
  }
}

void ObjectTree::slotCloseFlight(BaseFlightElement* bfe)
{
  Q3ListViewItem * item=findFlightElement(bfe);
  if (bfe==currentFlightElement) currentFlightElement=0;
  delete item;
}

/** The following code has been taken from tasks.cpp.
  **   Copyright (c):  2002 by Harald Maier       */

void ObjectTree::slotShowTaskMenu(WidgetItem *item, const QPoint &position )
{
/*  if (item != 0) {
    extern MapContents *_globalMapContents;
    FlightTask *ft = (FlightTask *)_globalMapContents->getFlight();
    if (ft != taskList.find(item->text(colName))) {
      slotSelectTask(item);
    }
  }
*/
  //task items
  taskMenu->setItemEnabled(idTaskEdit, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskMenu->setItemEnabled(idTaskDelete, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskMenu->setItemEnabled(idTaskSave, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskMenu->setItemEnabled(idTaskSaveAll, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);

  //flight items
  taskMenu->setItemEnabled(idFlightGroupEdit, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::FlightGroup);
  taskMenu->setItemEnabled(idFlightClose, FlightRoot->childCount() && (currentFlightElementType()==BaseMapElement::Flight || currentFlightElementType()==BaseMapElement::FlightGroup));
  taskMenu->setItemEnabled(idFlightOptimize, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);
  taskMenu->setItemEnabled(idFlightOptimizeOLC, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);

  taskMenu->exec( QCursor::pos() );
}

void ObjectTree::createMenu()
{
  taskMenu = new QMenu(this);
  taskMenu->setTitle( tr("Flights") );

  taskMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                       tr("&Open flight"),
                       this,
                       SIGNAL(openFlight()) );

  taskMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                      tr("New flight &group"),
                      this,
                      SIGNAL(newFlightGroup()) );

  actionFlightGroupEdit = taskMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                               tr("Edit flight group"),
                                               this,
                                               SIGNAL(editFlightGroup()) );

  actionFlightOptimize = taskMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                              tr("O&ptimize flight"),
                                              this,
                                              SIGNAL(optimizeFlight()) );

  actionFlightOptimizeOLC = taskMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                                 tr("O&ptimize flight (OLC)"),
                                                 this,
                                                 SIGNAL(optimizeFlightOLC()) );

  actionFlightClose = taskMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
                                           tr("&Close flight (group)"),
                                           this,
                                           SLOT(slotDeleteTask()) );
  taskMenu->addSeparator();

  taskMenu->addAction( _mainWindow->getPixmap("kde_filenew_16.png"),
                       tr("&New task"),
                       this,
                       SIGNAL(newTask()) );

  taskMenu->addAction( _mainWindow->getPixmap("kde_fileopen_16.png"),
                       tr("Open &task"),
                       this,
                       SIGNAL(openTask()) );

  actionTaskEdit = taskMenu->addAction( _mainWindow->getPixmap("kde_wizard_16.png"),
                                        tr("&Edit task"),
                                        this,
                                        SLOT(slotEditTask()) );

  actionTaskDelete = taskMenu->addAction( _mainWindow->getPixmap("kde_fileclose_16.png"),
                                          tr("&Close task"),
                                          this,
                                          SLOT(slotDeleteTask()) );
  taskMenu->addSeparator();

  actionTaskSave = taskMenu->addAction( _mainWindow->getPixmap("kde_filesave_16.png"),
                                        tr("&Save this task"),
                                        this,
                                        SLOT(slotSaveTask()) );

  actionTaskSaveAll = taskMenu->addAction( _mainWindow->getPixmap("kde_save_all_16.png"),
                                           tr("Save &all task's"),
                                           this,
                                           SLOT(slotSaveAllTask()) );
}

void ObjectTree::slotEditTask()
{
  TaskDialog td( this );
  FlightTask *ft;

  if( currentFlightElement != 0 )
    {
      if( currentFlightElement->getObjectType() == BaseMapElement::Task )
        {
          ft = (FlightTask*) currentFlightElement;

          //td.name->setText(ft->getFileName());
          td.setTask( ft );

          if( td.exec() == QDialog::Accepted )
            {
              ft->setWaypointList( td.getTask()->getWPList() );
              ft->setPlanningType( td.getTask()->getPlanningType() );
              ft->setPlanningDirection( td.getTask()->getPlanningDirection() );
              //slotUpdateTask();
              //flightSelected(ft);
              selectedFlight( ft );
              slotFlightChanged();
            }
        }
    }
}

/*
 Used to close a flight element in general, not only a task
*/
void ObjectTree::slotDeleteTask()
{
  if (currentFlightElementType()==BaseMapElement::Task || currentFlightElementType()==BaseMapElement::Flight || currentFlightElementType()==BaseMapElement::FlightGroup) {
//    currentFlightElement=0;
//    delete currentItem();
    emit closeTask();
  }
}

void ObjectTree::slotSaveTask()
{

  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  QDomElement t, child;
  FlightTask *ft;
  Waypoint *w;
  QFile f;
  QString fName;
  //QListViewItem *item = currentItem();
  QList<Waypoint*> wpList;

  //check if we are dealing with a task, and if so, set ft to reference the flighttask, else exit.
  if (currentFlightElementType()==BaseMapElement::Task) {
    ft=(FlightTask*)currentFlightElement;
  } else {
    return;
  }

  fName = QFileDialog::getSaveFileName ( this,
                                         tr("Save task"),
                                         path,
                                         tr("KFLOG tasks (*.kflogtsk *.KFLOGTSK)" ) );
  if (!fName.isEmpty())
    {
      if (fName.right(9).toLower() != ".kflogtsk")
        {
          fName += ".kflogtsk";
        }

    QApplication::setOverrideCursor( Qt::waitCursor );

    doc.appendChild(root);

      t = doc.createElement("Task");
      t.setAttribute("Type", ft->getTaskTypeString());
      t.setAttribute("PlanningType", ft->getPlanningType());
      t.setAttribute("PlanningDirection", ft->getPlanningDirection());
      root.appendChild(t);

      wpList = ft->getWPList();

      for(int i = 0; i < wpList.count(); i++)
      {
        w = wpList.at(i);

        child = doc.createElement("Waypoint");
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

    f.setName( fName );
      if( f.open( QIODevice::WriteOnly ) )
        {
          QString txt = doc.toString();
          f.writeBlock( txt, txt.length() );
          f.close();
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

}

void ObjectTree::slotSaveAllTask()
{

  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  QDomElement t, child;
  Waypoint *w;
  FlightTask *ft;
  QFile f;
  QString fName;
  Q3ListViewItem *item;
  QList<Waypoint*> wpList;

  fName = QFileDialog::getSaveFileName ( this,
                                         tr("Save task"),
                                         path,
                                         tr("KFLOG tasks (*.kflogtsk *.KFLOGTSK)" ) );
  if (!fName.isEmpty())
    {
      if (fName.right(9).toLower() != ".kflogtsk")
        {
          fName += ".kflogtsk";
        }

    QApplication::setOverrideCursor( Qt::waitCursor );
    doc.appendChild(root);

    item = TaskRoot->firstChild();
    while (item != 0) {
      ft = ((TaskListViewItem*)item)->task;

      t = doc.createElement("Task");
      t.setAttribute("Type", ft->getTaskTypeString());
      t.setAttribute("PlanningType", ft->getPlanningType());
      t.setAttribute("PlanningDirection", ft->getPlanningDirection());
      root.appendChild(t);

      wpList = ft->getWPList();

      for(int i = 0; i < wpList.count(); i++)
      {
        w = wpList.at(i);

        child = doc.createElement("Waypoint");
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
      item = item->nextSibling();
    }

    f.setName(fName);

    if (f.open(QIODevice::WriteOnly)) {
      QString txt = doc.toString();
      f.writeBlock(txt, txt.length());
      f.close();
      path = fName;
    }
    else {
      QMessageBox::warning( this,
                            tr("Permission denied"), "<html>" + tr("<B>%1</B><BR>permission denied!").arg(fName) + "</html>",
                            QMessageBox::Ok );
    }

    QApplication::restoreOverrideCursor();
  }

}

void ObjectTree::dragEnterEvent(QDragEnterEvent* event)
{
  if( event->mimeData()->hasUrls() )
    {
      event->acceptProposedAction();
    }
}

void ObjectTree::dropEvent(QDropEvent* event)
{
  QList<QUrl> urlList = event->mimeData()->urls();

  for( int i = 0; i < urlList.size(); i++ )
    {
      emit openFile( urlList.at(i) );
    }
}

