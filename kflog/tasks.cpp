/***********************************************************************
**
**   tasks.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/
#include <pwd.h>
#include <unistd.h>

#include <qlayout.h>
#include <qdom.h>
#include <qapplication.h>
#include <qlist.h>

#include <klocale.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include "tasks.h"
#include "mapcontents.h"

Tasks::Tasks(QWidget *parent, const char *name ) : QFrame(parent,name)
{
  KConfig* config = KGlobal::config();
  config->setGroup("Path");
  QString taskDir = config->readEntry("DefaultWaypointDirectory", getpwuid(getuid())->pw_dir);
  path = taskDir;

  addTaskWindow(this);
  addPopupMenu();

  taskList.setAutoDelete(false);
}

Tasks::~Tasks()
{
}

void Tasks::addTaskWindow(QWidget *parent)
{
  tasks =  new KFLogListView("Tasks", parent, "tasks");
  tasks->setShowSortIndicator(false);
  tasks->setAllColumnsShowFocus(true);

  colName = tasks->addColumn(i18n("Name"));
  colDesc = tasks->addColumn(i18n("Description"));

  tasks->loadConfig();

   connect(tasks, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
    SLOT(showTaskPopup(QListViewItem *, const QPoint &, int)));
  connect(tasks, SIGNAL(doubleClicked(QListViewItem *)),
    SLOT(slotEditTask()));
  connect(tasks, SIGNAL(clicked(QListViewItem *)),
    SLOT(slotSelectTask(QListViewItem *)));

  QVBoxLayout *layout = new QVBoxLayout(parent, 5, 5);
  layout->addWidget(tasks);
}

void Tasks::addPopupMenu()
{
  taskPopup = new KPopupMenu(tasks);
  taskPopup->insertTitle(SmallIcon("task"), i18n("Tasks's"), 0);
  taskPopup->insertItem(SmallIcon("filenew"), i18n("&New task"), this,
    SLOT(slotNewTask()));
  taskPopup->insertItem(SmallIcon("fileopen"), i18n("&Open task"), this,
    SLOT(slotOpenTask()));
  idTaskEdit = taskPopup->insertItem(SmallIcon("wizard"), i18n("&Edit task"), this,
    SLOT(slotEditTask()));
  idTaskDelete = taskPopup->insertItem(SmallIcon("editdelete"), i18n("&Delete task"), this,
    SLOT(slotDeleteTask()));
  taskPopup->insertSeparator();
  idTaskSave = taskPopup->insertItem(SmallIcon("filesave"), i18n("&Save this task"), this,
    SLOT(slotSaveTask()));
  idTaskSaveAll = taskPopup->insertItem(SmallIcon("save_all"), i18n("Save &all task's"), this,
    SLOT(slotSaveAllTask()));
  taskPopup->insertSeparator();
  taskPopup->insertItem(i18n("Down&load from logger"), this,
    SLOT(slotNotHandledItem()));
  taskPopup->insertItem(i18n("&Upload to logger"), this,
    SLOT(slotNotHandledItem()));

}

void Tasks::slotNotHandledItem()
{
  KMessageBox::sorry(this, "This function is not yet available!", "ooops");
}

void Tasks::slotNewTask()
{
  emit newTask();
}

void Tasks::slotOpenTask()
{
  emit openTask();
}

void Tasks::slotEditTask()
{
}

void Tasks::slotDeleteTask()
{
  QListViewItem *item = tasks->currentItem();
  if (item != 0) {
    taskList.take(item->text(colName));
    delete item;
    emit closeTask();
  }
}

void Tasks::slotSaveTask()
{
  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  QDomElement t, child;
  FlightTask *ft;
  wayPoint *w;
  QFile f;
  QString fName;
  QListViewItem *item = tasks->currentItem();
  QList <wayPoint> wpList;
  wpList.setAutoDelete(false);
  uint i;

  fName = KFileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, i18n("Save task"));
  if(!fName.isEmpty()) {
    if (fName.right(9) != ".kflogtsk") {
      fName += ".kflogtsk";
    }

    QApplication::setOverrideCursor( Qt::waitCursor );
    
    doc.appendChild(root);
    if (item != 0) {
      ft = taskList.find(item->text(colName));

      t = doc.createElement("Task");
      t.setAttribute("Type", ft->getTaskTypeString());
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
    }

    f.setName(fName);
    if (f.open(IO_WriteOnly)) {
      QString txt = doc.toString();
      f.writeBlock(txt, txt.length());
      f.close();
      path = fName;
    }
    else {
      KMessageBox::error(0, i18n("<B>%1</B><BR>permission denied!").arg(fName), i18n("Error occurred!"));
    }

    QApplication::restoreOverrideCursor();
  }
}

void Tasks::slotSaveAllTask()
{
  QDomDocument doc("KFLogTask");
  QDomElement root = doc.createElement("KFLogTask");
  QDomElement t, child;
  wayPoint *w;
  FlightTask *ft;
  QFile f;
  QString fName;
  QListViewItem *item;
  QList <wayPoint> wpList;
  wpList.setAutoDelete(false);
  uint i;

  fName = KFileDialog::getSaveFileName(path, "*.kflogtsk *.KFLOGTSK|KFLog tasks (*.kflogtsk)", 0, i18n("Save task"));
  if(!fName.isEmpty()) {
    if (fName.right(9) != ".kflogtsk") {
      fName += ".kflogtsk";
    }

    QApplication::setOverrideCursor( Qt::waitCursor );
    doc.appendChild(root);

    item = tasks->firstChild();
    while (item != 0) {
      ft = taskList.find(item->text(colName));

      t = doc.createElement("Task");
      t.setAttribute("Type", ft->getTaskTypeString());
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
      KMessageBox::error(0, i18n("<B>%1</B><BR>permission denied!").arg(fName), i18n("Error occurred!"));
    }
    
    QApplication::restoreOverrideCursor();
  }
}

void Tasks::showTaskPopup(QListViewItem *item, const QPoint &, int)
{
  if (item != 0) {
    extern MapContents _globalMapContents;
    FlightTask *ft = (FlightTask *)_globalMapContents.getFlight();
    if (ft != taskList.find(item->text(colName))) {
      slotSelectTask(item);
    }
  }

  taskPopup->setItemEnabled(idTaskEdit, tasks->childCount());
  taskPopup->setItemEnabled(idTaskDelete, tasks->childCount());
  taskPopup->setItemEnabled(idTaskSave, tasks->childCount());
  taskPopup->setItemEnabled(idTaskSaveAll, tasks->childCount());

  taskPopup->exec(QCursor::pos());
}

/** No descriptions */
void Tasks::slotAppendTask(FlightTask *f)
{
  QListViewItem *item;

  taskList.insert(f->getFileName(), f);

  item = new QListViewItem(tasks);
  item->setText(colName, f->getFileName());
  item->setText(colDesc, f->getTaskTypeString());

//  tasks->setCurrentItem(item);
}
/** No descriptions */
void Tasks::setCurrentTask()
{
  extern MapContents _globalMapContents;
  BaseFlightElement *e = _globalMapContents.getFlight();
  if (e != 0) {
    QString n = e->getFileName();

    tasks->setCurrentItem(0);
    QListViewItem *item = tasks->firstChild();
    while (item != 0) {
      if (item->text(colName) == n) {
        tasks->setCurrentItem(item);
        break;
      }
      item = item->nextSibling();
    }
  }
}

void Tasks::slotSelectTask(QListViewItem *item)
{
  FlightTask *ft;

  if (item != 0) {
    ft = taskList.find(item->text(colName));
    if (ft != 0) {
      emit flightSelected(ft);
    }
  }
}

/** No descriptions */
void Tasks::slotUpdateTask()
{
  FlightTask *ft;
  QListViewItem *item = tasks->currentItem();
  if (item != 0) {
    ft = taskList.find(item->text(colName));
    item->setText(colDesc, ft->getTaskTypeString());
  }
}

