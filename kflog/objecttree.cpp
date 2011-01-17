/***********************************************************************
**
**   objecttree.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Andr� Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>
#include <QtXml>
#include <Qt3Support>

//program specific includes
#include "flightgrouplistviewitem.h"
#include "flightlistviewitem.h"
#include "flightselectiondialog.h"
#include "mapcontents.h"
#include "objecttree.h"
#include "taskdialog.h"
#include "tasklistviewitem.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

ObjectTree::ObjectTree(QWidget *parent, const char *name ) : Q3ListView(parent,name) {
  addPopupMenu();
  setAcceptDrops(true);

  /*
   * setup listview
   */
  setShowSortIndicator(false);
  setAllColumnsShowFocus(true);
  setRootIsDecorated(true);
  setSorting(-1);
  //setSelectionMode(Extended);

  colName = addColumn(tr("Name"));
  colDesc = addColumn(tr("Description"));

  FlightRoot = new Q3ListViewItem(this,tr("Flights"));
  FlightRoot->setPixmap(0, _mainWindow->getPixmap("igc_16.png"));
  FlightRoot->setSelectable(false);

  TaskRoot = new Q3ListViewItem(this,FlightRoot,tr("Tasks"));
  TaskRoot->setPixmap(0, _mainWindow->getPixmap("task_16.png"));
  TaskRoot->setSelectable(false);
  /*
   * end listview setup
   */

  connect(this, SIGNAL(selectionChanged(Q3ListViewItem*)),SLOT(slotSelected(Q3ListViewItem*)));
  connect(this, SIGNAL(rightButtonPressed(Q3ListViewItem *, const QPoint &, int)), SLOT(showTaskPopup(Q3ListViewItem *, const QPoint &, int)));

  currentFlightElement=0;
}

ObjectTree::~ObjectTree(){
}

/**
 * Called if a new flight has been added.
 */
void ObjectTree::slotNewFlightAdded(Flight * flight){
  new FlightListViewItem(FlightRoot,flight);
}

/**
 * Called if a new flightgroup has been created or loaded.
 */
void ObjectTree::slotNewFlightGroupAdded(FlightGroup * flightGroup){
  new FlightGroupListViewItem(FlightRoot,flightGroup);
}

/**
 * Called if a new task has been created or loaded.
 */
void ObjectTree::slotNewTaskAdded(FlightTask * task){
  new TaskListViewItem(TaskRoot,task);
}

/** Called if the selection has changed. */
void ObjectTree::slotSelected(Q3ListViewItem * itm){
  extern MapContents *_globalMapContents;

  if (!itm) return;
  BaseFlightElement * flt=0;

  switch (itm->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
    case FLIGHTLISTVIEWITEM_TYPEID:
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

  if (flt && flt!=_globalMapContents->getFlight())
    emit selectedFlight(flt);

}

/** This slot is called if the currently selected flight has changed. */
void ObjectTree::slotSelectedFlightChanged(){
  extern MapContents *_globalMapContents;
  Q3ListViewItem * itm=findFlightElement(_globalMapContents->getFlight());

  if (itm) {
    if (!itm->isSelected()) setSelected(itm,true);
    slotFlightChanged();
    ensureItemVisible(itm);
    currentFlightElement=_globalMapContents->getFlight();
    return;
  }
}

/** Signaled if the current flight was somehow changed.  */
void ObjectTree::slotFlightChanged(){
  extern MapContents *_globalMapContents;
  Q3ListViewItem * itm=findFlightElement(_globalMapContents->getFlight());

  if (itm) {
    switch (itm->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
      case FLIGHTLISTVIEWITEM_TYPEID:
        ((FlightListViewItem*)itm)->update();
        break;
      case FLIGHTGROUPLISTVIEWITEM_TYPEID:
        ((FlightGroupListViewItem*)itm)->update();
        break;
      case TASKLISTVIEWITEM_TYPEID:
        ((TaskListViewItem*)itm)->update();
        break;
      default:
        qWarning("Listviewitem of unknown type");
    }
  }
}

/**
 * Searches the objecttree for the node representing the baseflightelement
 * given as an argument.
 * @returns a pointer to the QListViewItem if found, 0 otherwise.
 */
Q3ListViewItem * ObjectTree::findFlightElement(BaseFlightElement * bfe){
  Q3ListViewItem * itm=0;

  if (FlightRoot->childCount()!=0) {
    itm = FlightRoot->firstChild();
    while (itm!=0) {
      if (((FlightListViewItem*)itm)->flight==bfe) {
        return itm;
      }
      itm=itm->nextSibling();
    }
  }

  if (TaskRoot->childCount()!=0) {
    itm = TaskRoot->firstChild();
    while (itm!=0) {
      if (((TaskListViewItem*)itm)->task==bfe) {
        return itm;
      }
      itm=itm->nextSibling();
    }
  }

  return 0;
}

/*
  Returns -1 if no element has been selected, and the rtti() of the selected element otherwise.
*/
int ObjectTree::currentFlightElementType() {
  if (!currentFlightElement) {
    return -1;
  } else {
    return currentFlightElement->getObjectType();
  }
}

void ObjectTree::slotCloseFlight(BaseFlightElement* bfe) {
  Q3ListViewItem * itm=findFlightElement(bfe);
  if (bfe==currentFlightElement) currentFlightElement=0;
  delete itm;
}

/** The following code has been taken from tasks.cpp.
  **   Copyright (c):  2002 by Harald Maier       */

void ObjectTree::showTaskPopup(Q3ListViewItem */*item*/, const QPoint &, int)
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
  taskPopup->setItemEnabled(idTaskEdit, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskPopup->setItemEnabled(idTaskDelete, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskPopup->setItemEnabled(idTaskSave, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);
  taskPopup->setItemEnabled(idTaskSaveAll, TaskRoot->childCount() && currentFlightElementType()==BaseMapElement::Task);

  //flight items
  taskPopup->setItemEnabled(idFlightGroupEdit, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::FlightGroup);
  taskPopup->setItemEnabled(idFlightClose, FlightRoot->childCount() && (currentFlightElementType()==BaseMapElement::Flight || currentFlightElementType()==BaseMapElement::FlightGroup));
  taskPopup->setItemEnabled(idFlightOptimize, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);
  taskPopup->setItemEnabled(idFlightOptimizeOLC, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);

  taskPopup->exec(QCursor::pos());
}

void ObjectTree::addPopupMenu()
{
  taskPopup = new Q3PopupMenu(this);
//  taskPopup->insertTitle(SmallIcon("flight"), tr("Flights"), 0);
  taskPopup->insertItem(_mainWindow->getPixmap("kde_fileopen_16.png"), tr("&Open flight"), this, SIGNAL(openFlight()));
  taskPopup->insertItem(_mainWindow->getPixmap("kde_filenew_16.png"), tr("New flight &group"), this,
    SIGNAL(newFlightGroup()));
  idFlightGroupEdit = taskPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), tr("Edit flight group"), this,
    SIGNAL(editFlightGroup()));
  idFlightOptimize = taskPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), tr("O&ptimize flight"), this,
    SIGNAL(optimizeFlight()));
  idFlightOptimizeOLC = taskPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), tr("O&ptimize flight (OLC)"), this,
    SIGNAL(optimizeFlightOLC()));
  idFlightClose = taskPopup->insertItem(_mainWindow->getPixmap("kde_fileclose_16.png"), tr("&Close flight (group)"), this,
    SLOT(slotDeleteTask()));

//  taskPopup->insertTitle(SmallIcon("task"), tr("Tasks"), 0);
  taskPopup->insertSeparator();
  taskPopup->insertItem(_mainWindow->getPixmap("kde_filenew_16.png"), tr("&New task"), this,
    SIGNAL(newTask()));
  taskPopup->insertItem(_mainWindow->getPixmap("kde_fileopen_16.png"), tr("Open &task"), this,
    SIGNAL(openTask()));
  idTaskEdit = taskPopup->insertItem(_mainWindow->getPixmap("kde_wizard_16.png"), tr("&Edit task"), this,
    SLOT(slotEditTask()));
  idTaskDelete = taskPopup->insertItem(_mainWindow->getPixmap("kde_fileclose_16.png"), tr("&Close task"), this,
    SLOT(slotDeleteTask()));
  taskPopup->insertSeparator();
  idTaskSave = taskPopup->insertItem(_mainWindow->getPixmap("kde_filesave_16.png"), tr("&Save this task"), this,
    SLOT(slotSaveTask()));
  idTaskSaveAll = taskPopup->insertItem(_mainWindow->getPixmap("kde_save_all_16.png"), tr("Save &all task's"), this,
    SLOT(slotSaveAllTask()));

}

void ObjectTree::slotEditTask()
{
  TaskDialog td(this, "taskdialog");
  FlightTask *ft;

  if (currentFlightElement != 0) {
    if (currentFlightElement->getObjectType()==BaseMapElement::Task) {
      ft = (FlightTask*) currentFlightElement;

      //td.name->setText(ft->getFileName());
      td.setTask(ft);
      if (td.exec() == QDialog::Accepted) {
        ft->setWaypointList(td.getTask()->getWPList());
        ft->setPlanningType(td.getTask()->getPlanningType());
        ft->setPlanningDirection(td.getTask()->getPlanningDirection());
        //slotUpdateTask();
        //flightSelected(ft);
        selectedFlight(ft);
        slotFlightChanged();
      }
    }
  }
}

/*
 Used to close a flightelement in general, not only a task
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

  fName = Q3FileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, 0, tr("Save task"));
  if(!fName.isEmpty()) {
    if (fName.right(9) != ".kflogtsk") {
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
      for(int i = 0; i < wpList.count(); i++) {
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

    f.setName(fName);
    if (f.open(QIODevice::WriteOnly)) {
      QString txt = doc.toString();
      f.writeBlock(txt, txt.length());
      f.close();
      path = fName;
    }
    else {
      QMessageBox::warning(0, tr("No permission"), "<qt>" + tr("<B>%1</B><BR>permission denied!").arg(fName) + "</qt>", QMessageBox::Ok, 0);
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

  fName = Q3FileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, 0, tr("Save task"));
  if(!fName.isEmpty()) {
    if (fName.right(9) != ".kflogtsk") {
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
      for(int i = 0; i < wpList.count(); i++) {
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
      QMessageBox::warning(0, tr("Permission denied"), "<qt>" + tr("<B>%1</B><BR>permission denied!").arg(fName) + "</qt>", QMessageBox::Ok, 0);
    }

    QApplication::restoreOverrideCursor();
  }

}

void ObjectTree::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(Q3TextDrag::canDecode(event));
}

void ObjectTree::dropEvent(QDropEvent* event)
{
  QStringList dropList;

  if(Q3UriDrag::decodeToUnicodeUris(event, dropList))
    {
      for(QStringList::Iterator it = dropList.begin();
              it != dropList.end(); it++)
          emit openFile((*it).toLatin1().data());
    }
}

