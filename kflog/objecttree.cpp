/***********************************************************************
**
**   objecttree.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

//program specific includes
#include "objecttree.h"
#include "flightlistviewitem.h"
#include "tasklistviewitem.h"
#include "flight.h"
#include "flighttask.h"
#include "mapcontents.h"
#include "taskdialog.h"

//qt includes
#include <qcursor.h>
#include <qdragobject.h>

//kde includes
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>



ObjectTree::ObjectTree(QWidget *parent, const char *name ) : QListView(parent,name) {
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
  
  colName = addColumn(i18n("Name"));
  colDesc = addColumn(i18n("Description"));
  
  FlightRoot = new QListViewItem(this,i18n("Flights"));
  FlightRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("igc", KIcon::NoGroup, KIcon::SizeSmall));
  FlightRoot->setSelectable(false);

  /* removed for now, as the concept is likely to disapear
  FlightGroupRoot = new QListViewItem(this,FlightRoot,i18n("Flight groups"));
  FlightGroupRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("flightgroup", KIcon::NoGroup, KIcon::SizeSmall));
  FlightGroupRoot->setSelectable(false);
  */
  TaskRoot = new QListViewItem(this,FlightRoot,i18n("Tasks"));
  TaskRoot->setPixmap(0, KGlobal::instance()->iconLoader()->loadIcon("task", KIcon::NoGroup, KIcon::SizeSmall));
  TaskRoot->setSelectable(false);
  /*
   * end listview setup
   */
   
  connect(this, SIGNAL(selectionChanged(QListViewItem*)),SLOT(slotSelected(QListViewItem*)));
  connect(this, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)), SLOT(showTaskPopup(QListViewItem *, const QPoint &, int)));
  
  currentFlightElement=0;
}

ObjectTree::~ObjectTree(){
}

/**
 * Called if a new flight has been added.
 */
void ObjectTree::slotNewFlightAdded(Flight * flight){
  QListViewItem * node=new FlightListViewItem(FlightRoot,flight);
}

/**
 * Called if a new task has been created or loaded.
 */
void ObjectTree::slotNewTaskAdded(FlightTask * task){
  QListViewItem * node=new TaskListViewItem(TaskRoot,task);
}

/** Called if the selection has changed. */
void ObjectTree::slotSelected(QListViewItem * itm){
  extern MapContents _globalMapContents;
  
  if (!itm) return;
  BaseFlightElement * flt=0;

  switch (itm->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
    case FLIGHTLISTVIEWITEM_TYPEID:
      flt=((FlightListViewItem*)itm)->flight;
      break;
    case TASKLISTVIEWITEM_TYPEID:
      flt=((TaskListViewItem*)itm)->task;
      break;
    default:
      flt=0;
  }
  
  if (flt && flt!=_globalMapContents.getFlight())
    emit selectedFlight(flt);
  
}

/** This slot is called if the currently selected flight has changed. */
void ObjectTree::slotSelectedFlightChanged(BaseFlightElement * bfe){
  QListViewItem * itm=findFlightElement(bfe);

  if (itm) {
    if (!itm->isSelected()) setSelected(itm,true);
    ensureItemVisible(itm);
    currentFlightElement=bfe;
    return;
  }
}

/** Signaled if the current flight was somehow changed.  */
void ObjectTree::slotFlightChanged(){
  extern MapContents _globalMapContents;
  QListViewItem * itm=findFlightElement(_globalMapContents.getFlight());

  if (itm) {
    switch (itm->rtti()) { //the rtti (Run Time Type Identification is used to see what kind of listview item we are dealing with
      case FLIGHTLISTVIEWITEM_TYPEID:
        ((FlightListViewItem*)itm)->update();
        break;
      case TASKLISTVIEWITEM_TYPEID:
        ((TaskListViewItem*)itm)->update();
        break;
      default:
        warning("Listviewitem of unknown type");
    }
  }
    
}

/**
 * Searches the objecttree for the node representing the baseflightelement
 * given as an argument.
 * @returns a pointer to the QListViewItem if found, 0 otherwise.
 */
QListViewItem * ObjectTree::findFlightElement(BaseFlightElement * bfe){
  QListViewItem * itm=0;

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
    return currentFlightElement->getTypeID();
  }
}

void ObjectTree::slotCloseFlight(BaseFlightElement* bfe) {
  QListViewItem * itm=findFlightElement(bfe);
  if (bfe==currentFlightElement) currentFlightElement=0;
  delete itm;
} 

/** The following code has been taken from tasks.cpp.
  **   Copyright (c):  2002 by Harald Maier       */

void ObjectTree::showTaskPopup(QListViewItem *item, const QPoint &, int)
{
/*  if (item != 0) {
    extern MapContents _globalMapContents;
    FlightTask *ft = (FlightTask *)_globalMapContents.getFlight();
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
  taskPopup->setItemEnabled(idFlightClose, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);
  taskPopup->setItemEnabled(idFlightOptimize, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);
  taskPopup->setItemEnabled(idFlightOptimizeOLC, FlightRoot->childCount() && currentFlightElementType()==BaseMapElement::Flight);
 
  taskPopup->exec(QCursor::pos());
}

void ObjectTree::addPopupMenu()
{
  taskPopup = new KPopupMenu(this);
  taskPopup->insertTitle(SmallIcon("task"), i18n("Tasks"), 0);
  taskPopup->insertItem(SmallIcon("filenew"), i18n("&New task"), this,
    SLOT(slotNewTask()));
  taskPopup->insertItem(SmallIcon("fileopen"), i18n("Open &task"), this,
    SLOT(slotOpenTask()));
  idTaskEdit = taskPopup->insertItem(SmallIcon("wizard"), i18n("&Edit task"), this,
    SLOT(slotEditTask()));
  idTaskDelete = taskPopup->insertItem(SmallIcon("fileclose"), i18n("&Close task"), this,
    SLOT(slotDeleteTask()));
  taskPopup->insertSeparator();
  idTaskSave = taskPopup->insertItem(SmallIcon("filesave"), i18n("&Save this task"), this,
    SLOT(slotSaveTask()));
  idTaskSaveAll = taskPopup->insertItem(SmallIcon("save_all"), i18n("Save &all task's"), this,
    SLOT(slotSaveAllTask()));

  taskPopup->insertTitle(SmallIcon("flight"), i18n("Flights"), 0);
  taskPopup->insertItem(SmallIcon("fileopen"), i18n("&Open flight"), this, SIGNAL(openFlight()));
  idFlightOptimize = taskPopup->insertItem(SmallIcon("wizard"), i18n("O&ptimize flight"), this,
    SIGNAL(optimizeFlight()));
  idFlightOptimizeOLC = taskPopup->insertItem(SmallIcon("wizard"), i18n("O&ptimize flight (OLC)"), this,
    SIGNAL(optimizeFlightOLC()));
  idFlightClose = taskPopup->insertItem(SmallIcon("fileclose"), i18n("&Close flight"), this,
    SLOT(slotDeleteTask()));
}

void ObjectTree::slotEditTask()
{
  TaskDialog td(this, "taskdialog");
  QListViewItem *item = currentItem();
  FlightTask *ft;

  if (currentFlightElement != 0) {
    if (currentFlightElement->getTypeID()==BaseMapElement::Task) {
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
  if (currentFlightElementType()==BaseMapElement::Task || currentFlightElementType()==BaseMapElement::Flight) {
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
  QList <Waypoint> wpList;
  wpList.setAutoDelete(false);
  uint i;

  //check if we are dealing with a task, and if so, set ft to reference the flighttask, else exit.
  if (currentFlightElementType()==BaseMapElement::Task) {
    ft=(FlightTask*)currentFlightElement;
  } else {
    return;
  }
  
  fName = KFileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, i18n("Save task"));
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
      for (i = 0; i < wpList.count(); i++) {
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
        child.setAttribute("Runway", w->runway);
        child.setAttribute("Length", w->length);
        child.setAttribute("Surface", w->surface);
        child.setAttribute("Comment", w->comment);

        t.appendChild(child);
      }
    

    f.setName(fName);
    if (f.open(IO_WriteOnly)) {
      QString txt = doc.toString();
      f.writeBlock(txt, txt.length());
      f.close();
      path = fName;
    }
    else {
      KMessageBox::error(0, "<qt>" + i18n("<B>%1</B><BR>permission denied!").arg(fName) + "</qt>", i18n("Error occurred!"));
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
  QListViewItem *item;
  QList <Waypoint> wpList;
  wpList.setAutoDelete(false);
  uint i;

  fName = KFileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, i18n("Save task"));
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
      for (i = 0; i < wpList.count(); i++) {
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
        child.setAttribute("Runway", w->runway);
        child.setAttribute("Length", w->length);
        child.setAttribute("Surface", w->surface);
        child.setAttribute("Comment", w->comment);

        t.appendChild(child);
      }
      item = item->nextSibling();
    }

    f.setName(fName);
    if (f.open(IO_WriteOnly)) {
      QString txt = doc.toString();
      f.writeBlock(txt, txt.length());
      f.close();
      path = fName;
    }
    else {
      KMessageBox::error(0, "<qt>" + i18n("<B>%1</B><BR>permission denied!").arg(fName) + "</qt>", i18n("Error occurred!"));
    }

    QApplication::restoreOverrideCursor();
  }

}

void ObjectTree::slotNewTask()
{
  emit newTask();
}

void ObjectTree::slotOpenTask()
{
  emit openTask();
}

void ObjectTree::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept(QTextDrag::canDecode(event));
}

void ObjectTree::dropEvent(QDropEvent* event)
{
  QStringList dropList;

  if(QUriDrag::decodeToUnicodeUris(event, dropList))
    {
      for(QStringList::Iterator it = dropList.begin();
              it != dropList.end(); it++)
          emit openFile((*it).latin1());
    }
}

