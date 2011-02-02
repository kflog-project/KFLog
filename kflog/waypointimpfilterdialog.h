/***********************************************************************
**
**   waypointimpfilterdialog.h
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

#ifndef WAYPOINT_IMP_FILTER_DIALOG_H
#define WAYPOINT_IMP_FILTER_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <q3dict.h>
#include <QPoint>

#include "singlepoint.h"
#include "coordedit.h"

/**filter import of waypoints from map
  *@author Harald Maier
  */

#define CENTER_POS      0
#define CENTER_HOMESITE 1
#define CENTER_MAP      2
#define CENTER_AIRPORT  3

class WaypointImpFilterDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointImpFilterDialog )

public:

	WaypointImpFilterDialog( QWidget *parent=0 );

	virtual ~WaypointImpFilterDialog();

  /** No descriptions */
  int getCenterRef();
  WGSPoint getAirportRef();

  QCheckBox *useAll;
  QCheckBox *airports;
  QCheckBox *gliderSites;
  QCheckBox *otherSites;
  QCheckBox *outlanding;
  QCheckBox *obstacle;
  QCheckBox *landmark;
  QCheckBox *station;

  LatEdit *fromLat;
  LongEdit *fromLong;
  LatEdit *toLat;
  LongEdit *toLong;
  LatEdit *posLat;
  LongEdit *posLong;

  QComboBox *radius;

protected slots:

  void slotChangeUseAll();

public slots:

  /** reset all dialog items to default values */
  void slotClear();

private: // Private attributes
  /**  */
  int center;
  Q3Dict<SinglePoint> airportDict;
  QComboBox *refAirport;

private slots:

  /** No descriptions */
  void selectRadius(int n);
  void polish();
};

#endif
