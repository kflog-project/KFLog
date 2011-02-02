/***********************************************************************
**
**   waypointimpfilterdialog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class WaypointImpFilterDialog
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Waypoint filter dialog
 *
 * This dialog is used to define waypoint filter parameters.
 *
 * \date 2002-2011
 *
 * \version $Id$
 */

#ifndef WAYPOINT_IMP_FILTER_DIALOG_H
#define WAYPOINT_IMP_FILTER_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QHash>
#include <QLabel>
#include <QPoint>

#include "singlepoint.h"
#include "coordedit.h"

#define CENTER_POS      0
#define CENTER_HOMESITE 1
#define CENTER_MAP      2
#define CENTER_AIRFIELD 3

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
  QCheckBox *airfields;
  QCheckBox *gliderfields;
  QCheckBox *otherSites;
  QCheckBox *outlandings;
  QCheckBox *obstacles;
  QCheckBox *landmarks;
  QCheckBox *stations;

  LatEdit *fromLat;
  LongEdit *fromLong;
  LatEdit *toLat;
  LongEdit *toLong;
  LatEdit *posLat;
  LongEdit *posLong;

  QComboBox *radius;

protected:

  void showEvent( QShowEvent *event );

protected slots:

  void slotChangeUseAll();

public slots:

  /** reset all dialog items to default values */
  void slotClear();

private slots:

  /** No descriptions */
  void selectRadius(int n);

private:

  /**
   * Loads the content into the airfield combo box.
   */
  void loadAirfieldComboBox();

  int center;
  QHash<QString, SinglePoint*> airportDict;
  QComboBox *refAirport;
  QLabel* radiusUnit;
};

#endif
