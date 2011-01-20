/***********************************************************************
 **
 **   taskdialog.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Harald Maier
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtGui>
#include <Qt3Support>

#include "mapcalc.h"
#include "mapcontents.h"
#include "taskdialog.h"
#include "translationlist.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

extern TranslationList taskTypes;

TaskDialog::TaskDialog( QWidget *parent ) :
  QDialog(parent)
{
  setWindowTitle(tr("Task definition") );
  setModal( true );
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

  QErrorMessage* errorFai    = new QErrorMessage( this );
  QErrorMessage* errorRoute  = new QErrorMessage( this );

  errorFai->setWindowTitle(   tr("Task selection") );
  errorRoute->setWindowTitle( tr("Task selection") );

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
  planningTypes = new QComboBox(this);
  connect( planningTypes, SIGNAL(activated(const QString&)),
           SLOT(slotSetPlanningType(const QString&)) );
  l = new QLabel(planningTypes, tr("Task T&ype") + ":", this);

  // init comboboxes
  planningTypes->addItems( FlightTask::ttGetSortedTranslationList() );

  // Create an non-exclusive button group
  Q3ButtonGroup *bgrp2 = new Q3ButtonGroup(1, Qt::Vertical, tr("Side of FAI area"), this);
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
  connect(route, SIGNAL(currentChanged(Q3ListViewItem *)), SLOT(enableWaypointButtons()));
  
  colType = route->addColumn(tr("Type"));
  colWaypoint = route->addColumn(tr("Waypoint"));
  colDist = route->addColumn(tr("Distance"));
  colCourse = route->addColumn(tr("TC"));

  l = new QLabel(route, tr("&Task"), this);

  route->loadConfig();

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_up_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_down_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  leftLayout->addWidget(l);
  leftLayout->addWidget(route);
  leftLayout->addLayout(smallButtons);

  middleLayout->addStretch();
  back = new QPushButton(this);
  back->setIcon(_mainWindow->getPixmap("kde_back_16.png"));
  connect(back, SIGNAL(clicked()), SLOT(slotAddWaypoint()));
  middleLayout->addWidget(back);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_reload_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotReplaceWaypoint()));
  middleLayout->addWidget(b);
  forward = new QPushButton(this);
  forward->setIcon(_mainWindow->getPixmap("kde_forward_16.png"));
  connect(forward, SIGNAL(clicked()), SLOT(slotRemoveWaypoint()));
  middleLayout->addWidget(forward);
  middleLayout->addStretch();

  waypoints = new Q3ListBox(this, "waypoints");
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
  extern MapContents *_globalMapContents;
  QList<Waypoint*> *wpList = _globalMapContents->getWaypointList();
  Waypoint *wp;
  waypoints->clear();
  QString t;
  foreach(wp, *wpList) {
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

void TaskDialog::slotSetPlanningType( const QString& text )
{
  int id = FlightTask::ttText2Item( text );

  switch (id)
  {
  case FlightTask::FAIArea:
    {
      errorFai->showMessage( tr("Task Type FAI Area:\n"
        "You can define a task with either takeoff, start, end and landing or "
        "takeoff, start, end, landing and one additional route point.\n"
        "The FAI area calculation will be made with start and end point or start and route point, "
        "depending wether the route point is defined or not\n"
        "Deleting takeoff start, end and landing points is not possible, "
        "but you can replace them with other waypoints.\n"
        "New waypoints will be added after the selected one." ) );

      left->setEnabled(true);
      right->setEnabled(true);
      left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
      right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);

      int cnt = wpList.count();

      if( cnt > 5 )
        {
          // remove route points
          for( int n = cnt - 3; n > 2; n-- )
            {
              wpList.removeAt( n );
            }
          pTask->setWaypointList( wpList );
        }
      }

    break;

  case FlightTask::Route:

    errorRoute->showMessage(  tr("Task Type Traditional Route:\n"
      "You can define a task with takeoff, start, end, landing and route points. "
      "Deleting takeoff start, end and landing is not possible, "
      "but you can replace them with other waypoints.\n"
      "New waypoints will be added after the selected one.") );

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
  Q3ListViewItem *item, *lastItem = 0;
  QString txt;

  route->clear();
  foreach(wp, wpList) {
    item = new Q3ListViewItem(route, lastItem);
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

      txt.sprintf("%03.0f\260", getTrueCourse(wp->origP, wpPrev->origP));
      item->setText(colCourse, txt);
    }
    lastItem = item;
    wpPrev = wp;
  }

  taskType->setText(pTask->getTaskTypeString());
}

unsigned int TaskDialog::getCurrentPosition()
{
  Q3ListViewItemIterator item(route);
  int i = 0;
  while(item.current()) {
    if(item.current() == route->currentItem())
      return i;
    i++;
    ++item;
  }
  return 0;
}

void TaskDialog::setSelected(unsigned int position)
{
  Q3ListViewItemIterator item(route);
  uint i = 0;

  while( item.current() )
    {
      if( i == position )
        {
          route->setSelected( item.current(), true );
        }

      i++;
      ++item;
    }
}

void TaskDialog::slotMoveUp()
{
  unsigned int curPos = getCurrentPosition();
  Waypoint *wp;

  if (curPos) {
    wp = wpList.takeAt(curPos);
    wpList.insert(curPos - 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    setSelected(curPos - 1);
  }
}

void TaskDialog::slotMoveDown()
{
  int curPos = getCurrentPosition();
  Waypoint *wp;

  if( curPos < wpList.count() - 1 )
    {
      wp = wpList.takeAt( curPos );
      wpList.insert( curPos + 1, wp );
      pTask->setWaypointList( wpList );
      fillWaypoints();
      setSelected( curPos + 1 );
    }
}

void TaskDialog::slotReplaceWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = getCurrentPosition();
  QString selText = waypoints->currentText();

  if (!selText.isEmpty()) {
    wpList.removeAt(curPos);
    Waypoint *wp = new Waypoint;
    *wp = *waypointDict[selText];

    wpList.insert(curPos, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    setSelected(curPos + 1 < cnt ? curPos + 1 : cnt);
  }
}

void TaskDialog::slotAddWaypoint()
{
  QString selText = waypoints->currentText();
  unsigned int pos = getCurrentPosition();
  
  if (!selText.isEmpty()) {
    Waypoint *wp = new Waypoint;
    *wp = *waypointDict[selText];

    wpList.insert(pos + 1, wp);
    pTask->setWaypointList(wpList);
    fillWaypoints();
    setSelected(pos + 1);
    enableWaypointButtons();
  }
}

void TaskDialog::slotRemoveWaypoint()
{
  unsigned int cnt = wpList.count();
  unsigned int curPos = getCurrentPosition();

  if (cnt > 4) {
    if (curPos > 1 && curPos < cnt - 2) {
      wpList.removeAt(curPos);
      pTask->setWaypointList(wpList);
      fillWaypoints();
      setSelected(curPos);
      enableWaypointButtons();
    }
  }
}

void TaskDialog::setTask(FlightTask *orig)
{
  extern QSettings _settings;
  extern MapMatrix *_globalMapMatrix;

  if( pTask == 0 )
    {
      pTask = new FlightTask( orig->getFileName() );
    }

  // make a work copy of the task with at least 4 points
  wpList = orig->getWPList();
  Waypoint *wp;

  if (wpList.count() < 4)
    {
    for (unsigned int i = wpList.count(); i < 4; i++)
      {
        wp = new Waypoint;
        wp->origP.setLat( _settings.value("/MapData/HomesiteLatitude").toInt() );
        wp->origP.setLon( _settings.value("/MapData/HomesiteLongitude").toInt());
        wp->projP = _globalMapMatrix->wgsToMap(wp->origP);
        wp->name = _settings.value("/MapData/Homesite").toString().left(6).upper();

        wpList.append(wp);
      }
    }
  
  pTask->setWaypointList(wpList);
  pTask->setPlanningType(orig->getPlanningType());
  pTask->setPlanningDirection(orig->getPlanningDirection());

  name->setText(pTask->getFileName());

  planningTypes->setCurrentIndex( planningTypes->findText( FlightTask::ttItem2Text(pTask->getPlanningType())) );

  left->setChecked(pTask->getPlanningDirection() & FlightTask::leftOfRoute);
  right->setChecked(pTask->getPlanningDirection() & FlightTask::rightOfRoute);

  slotSetPlanningType( FlightTask::ttItem2Text(pTask->getPlanningType()) );
}

/** No descriptions */
void TaskDialog::enableWaypointButtons()
{
  unsigned int cnt = wpList.count();
  unsigned int pos = getCurrentPosition();

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
