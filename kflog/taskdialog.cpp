/***********************************************************************
 **
 **   taskdialog.cpp
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
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>

#include <kiconloader.h>

#include "taskdialog.h"
#include "mapcontents.h"
#include "translationlist.h"
#include "mapcalc.h"

extern TranslationList taskTypes;

TaskDialog::TaskDialog(QWidget *parent, const char *name )
  : QDialog(parent, name, true)
{
  setCaption(tr("Task definition"));
  __initDialog();
  setMinimumWidth(500);
  setMinimumHeight(300);
  pTask = 0;
}

TaskDialog::~TaskDialog()
{
}

/** No descriptions */
void TaskDialog::__initDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout(this, 10);
  QHBoxLayout *header = new QHBoxLayout(10);
  QHBoxLayout *type = new QHBoxLayout(10);
  QHBoxLayout *buttons = new QHBoxLayout(10);
  QVBoxLayout *leftLayout = new QVBoxLayout(5);
  QVBoxLayout *middleLayout = new QVBoxLayout(5);
  QVBoxLayout *rightLayout = new QVBoxLayout(5);
  QHBoxLayout *topGroup = new QHBoxLayout(10);
  QHBoxLayout *smallButtons = new QHBoxLayout(5);

  buttons->addStretch();
  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(accept()));
  buttons->addWidget(b);
  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  name = new QLineEdit(this);
  name->setReadOnly(true);
  l = new QLabel(name, tr("&Name") + ":", this);
  header->addWidget(l);
  header->addWidget(name);

  // Create a combo box for taskTypes
  planningTypes = new QComboBox(false, this, "planningType");
  connect(planningTypes, SIGNAL(activated(int)), SLOT(slotSetPlanningType(int)));
  l = new QLabel(planningTypes, tr("Task T&ype") + ":", this);

  TranslationElement *te;
  // init comboboxes
  for (te = taskTypes.first(); te != 0; te = taskTypes.next()) {
    planningTypes->insertItem(te->text);
  }

  // Create an non-exclusive button group
  QButtonGroup *bgrp2 = new QButtonGroup(1, QGroupBox::Vertical, tr("Side of FAI area"), this);
  connect(bgrp2, SIGNAL(clicked(int)), SLOT(slotSetPlanningDirection(int)));
  bgrp2->setExclusive(false);

  // insert 2 checkbuttons
  left = new QCheckBox(tr( "&left"), bgrp2);
  left->setChecked(true);
  right = new QCheckBox(tr("ri&ght"), bgrp2);

  taskType = new QLabel(this);
  taskType->setMinimumWidth(100);

  type->addWidget(l);
  type->addWidget(planningTypes);
  type->addWidget(bgrp2);
  type->addStretch();
  type->addWidget(taskType);

  route =  new KFLogListView("Route", this, "route");
  route->setShowSortIndicator(false);
  route->setSorting(-1);
  route->setAllColumnsShowFocus(true);
  connect(route, SIGNAL(currentChanged(QListViewItem *)), SLOT(enableWaypointButtons()));
  
  colType = route->addColumn(tr("Type"));
  colWaypoint = route->addColumn(tr("Waypoint"));
  colDist = route->addColumn(tr("Distance"));
  colCourse = route->addColumn(tr("TC"));

  l = new QLabel(route, tr("&Task"), this);

  route->loadConfig();

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("up", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("down", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  leftLayout->addWidget(l);
  leftLayout->addWidget(route);
  leftLayout->addLayout(smallButtons);

  middleLayout->addStretch();
  back = new QPushButton(this);
  back->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("back", KIcon::NoGroup, KIcon::SizeSmall));
  connect(back, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  middleLayout->addWidget(back);
  b = new QPushButton(this);
  b->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("reload", KIcon::NoGroup, KIcon::SizeSmall));
  connect(b, SIGNAL(clicked()), SLOT(slotReplaceWaypoint()));
  middleLayout->addWidget(b);
  forward = new QPushButton(this);
  forward->setPixmap(KGlobal::instance()->iconLoader()->loadIcon("forward", KIcon::NoGroup, KIcon::SizeSmall));
  connect(forward, SIGNAL(clicked()), SLOT(slotRemoveWaypoint()));
  middleLayout->addWidget(forward);
  middleLayout->addStretch();

  waypoints = new QListBox(this, "waypoints");
  l = new QLabel(waypoints, tr("&Waypoint's"), this);
  rightLayout->addWidget(l);
  rightLayout->addWidget(waypoints);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);

  topLayout->addLayout(header);
  topLayout->addLayout(type);
  topLayout->addLayout(topGroup);
  //topLayout->addWidget(new KSeparator(this));
  topLayout->addLayout(buttons);
}

/** No descriptions */
void TaskDialog::polish()
{
  QDialog::polish();

  if (pTask == 0) {
    QMessageBox::critical(this, "Internal error", "INTERNAL ERROR:\nNo task set!", QMessageBox::Ok, 0);
    reject();
  }

  // load current waypoint list from mapcontents
  extern MapContents _globalMapContents;
  QPtrList<Waypoint>* wpList = _globalMapContents.getWaypointList();
  Waypoint *wp;
  waypoints->clear();
  QString t;
  for (wp = wpList->first(); wp; wp = wpList->next()) {
    if (!wp->description.isEmpty()) {
      t = wp->description;
    }
    else {
      t = wp->name;
    }
    waypoints->insertItem(t);
    waypointDict.insert(t, wp);
  }
  waypoints->sort();
}

void TaskDialog::slotSetPlanningType(int idx)
{
  unsigned int n, cnt;
  int id;

  id = taskTypes.at(idx)->id;
  switch (id) {
  case FlightTask::FAIArea:
    QMessageBox::information (this, tr("Task Type FAI Area:\n"
      "You can define a task with either takeoff, start, end and landing or "
      "takeoff, start, end, landing and one additional route point.\n"
      "The FAI area calculation will be made with start and end point or start and route point, "
      "depending wether the route point is defined or not\n"
      "Deleting takeoff start, end and landing points is not possible, "
      "but you can replace them with other waypoints.\n"
      "New waypoints will be added after the selected one."), tr("Task selection"),
      tr("Do not show this message again."));
    cnt = wpList.count();    
    left->setEnabled(true);
    right->setEnabled(true);
    left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
    right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);
    if (cnt > 5) {
      // remove route points
      for(n = cnt - 3; n > 2; n--) {
        wpList.take(n);
      }
      pTask->setWaypointList(wpList);
    }        
    break;
  case FlightTask::Route:
    QMessageBox::information (this, tr("Task Type Traditional Route:\n"
      "You can define a task with takeoff, start, end, landing and route points. "
      "Deleting takeoff start, end and landing is not possible, "
      "but you can replace them with other waypoints.\n"
      "New waypoints will be added after the selected one."), tr("Task selection"),
      tr("Do not show this message again."));
    left->setEnabled(false);
    right->setEnabled(false);
    left->setChecked(false);
    right->setChecked(false);
    break;
  }

  pTask->setPlanningType(id);
  fillWaypoints();
  enableWaypointButtons();
  route->setSelected(route->firstChild(), true);
}

void TaskDialog::slotSetPlanningDirection(int)
{
  int dir = 0;

  if (left->isChecked()) {
    dir |= FlightTask::leftOfRoute;
  }

  if (right->isChecked()) {
    dir |= FlightTask::rightOfRoute;
  }

  pTask->setPlanningDirection(dir);
}

void TaskDialog::fillWaypoints()
{
  Waypoint *wp, *wpPrev;
  QListViewItem *item, *lastItem = 0;
  QString txt;

  route->clear();
  for (wp = wpList.first(); wp; wpPrev = wp, wp = wpList.next()) {
    item = new QListViewItem(route, lastItem);
    switch (wp->type) {
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
      txt = "???";
    }
    item->setText(colType, txt);
    item->setText(colWaypoint, wp->name);

    if (lastItem) {
      txt.sprintf("%.2f km", wp->distance);
      item->setText(colDist, txt);

      txt.sprintf("%03.0f°", getTrueCourse(wp->origP, wpPrev->origP));
      item->setText(colCourse, txt);
    }
    lastItem = item;
  }

  taskType->setText(pTask->getTaskTypeString());
}

void TaskDialog::slotMoveUp()
{
  int curPos = route->itemIndex(route->currentItem());
  Waypoint *wp;

  if (curPos) {
    wp = wpList.take(curPos);
    wpList.insert(curPos - 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos - 1), true);
  }
}

void TaskDialog::slotMoveDown()
{
  unsigned int curPos = route->itemIndex(route->currentItem());
  Waypoint *wp;

  if (curPos < wpList.count() - 1) {
    wp = wpList.take(curPos);
    wpList.insert(curPos + 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos + 1), true);
  }
}

void TaskDialog::slotReplaceWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = route->itemIndex(route->currentItem());
  QString selText = waypoints->currentText();

  if (!selText.isEmpty()) {
    wpList.remove(curPos);
    Waypoint *wp = new Waypoint;
    *wp = *waypointDict[selText];

    wpList.insert(curPos, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(curPos + 1 < cnt ? curPos + 1 : cnt), true);
  }
}

void TaskDialog::slotAddWaypoint()
{
  QString selText = waypoints->currentText();
  unsigned int pos = route->itemIndex(route->selectedItem());
  
  if (!selText.isEmpty()) {
    Waypoint *wp = new Waypoint;
    *wp = *waypointDict[selText];

    wpList.insert(pos + 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    route->setSelected(route->itemAtIndex(pos + 1), true);
    enableWaypointButtons();
  }
}

void TaskDialog::slotRemoveWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = route->itemIndex(route->currentItem());

  if (cnt > 4) {
    if (curPos > 1 && curPos < cnt - 2) {
      wpList.remove(curPos);
      pTask->setWaypointList(wpList);
      fillWaypoints();
      route->setSelected(route->itemAtIndex(curPos), true);
      enableWaypointButtons();
    }
  }
}

void TaskDialog::setTask(FlightTask *orig)
{
  extern QSettings _settings;
  extern MapMatrix _globalMapMatrix;

  if (pTask == 0) {
    pTask = new FlightTask(orig->getFileName());
  }

  // make a work copy of the task with at least 4 points
  wpList = orig->getWPList();
  Waypoint *wp;
  if (wpList.count() < 4) {
    for (unsigned int i = wpList.count(); i < 4; i++) {
      wp = new Waypoint;
      wp->origP.setLat(_settings.readNumEntry("/MapData/HomesiteLatitude"));
      wp->origP.setLon(_settings.readNumEntry("/MapData/HomesiteLongitude"));
      wp->projP = _globalMapMatrix.wgsToMap(wp->origP);
      wp->name = _settings.readEntry("/MapData/Homesite").left(6).upper();

      wpList.append(wp);
    }
  }
  
  pTask->setWaypointList(wpList);
  pTask->setPlanningType(orig->getPlanningType());
  pTask->setPlanningDirection(orig->getPlanningDirection());

  name->setText(pTask->getFileName());
  planningTypes->setCurrentItem(taskTypes.idxById(pTask->getPlanningType()));

  left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
  right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);

  slotSetPlanningType(taskTypes.idxById(pTask->getPlanningType()));
}

/** No descriptions */
void TaskDialog::enableWaypointButtons()
{
  unsigned int cnt = wpList.count();
  unsigned int pos = route->itemIndex(route->selectedItem());

  switch (pTask->getPlanningType()) {
  case FlightTask::Route:
    // disable add button for start, landing, end
    // enable for takeoff, we cann append to it
    back->setEnabled(pos > 0 && pos < cnt - 2);

    // disable remove button for start, takeoff, landing, end
    forward->setEnabled(pos > 1 && pos < cnt - 2);
    break;
  case FlightTask::FAIArea:
    forward->setEnabled(cnt > 4 && pos == 2);
    back->setEnabled(pos > 0 && pos < cnt - 2 && cnt == 4);
    break;
  }
}
