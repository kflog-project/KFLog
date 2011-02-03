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
#include <QRadioButton>

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
  WGSPoint getAirfieldRef();

  QCheckBox *useAll;
  QCheckBox *airfields;
  QCheckBox *gliderfields;
  QCheckBox *otherSites;
  QCheckBox *outlandings;
  QCheckBox *obstacles;
  QCheckBox *landmarks;
  QCheckBox *stations;

  QRadioButton *rb0;
  QRadioButton *rb1;
  QRadioButton *rb2;
  QRadioButton *rb3;

  LatEdit *fromLat;
  LongEdit *fromLong;
  LatEdit *toLat;
  LongEdit *toLong;
  LatEdit *radiusLat;
  LongEdit *radiusLong;

  QComboBox *radius;

  QString airfieldRefTxt;

protected:

  void showEvent( QShowEvent *event );

protected slots:

  void slotChangeUseAll();

public slots:

  /** reset all dialog items to default values */
  void slotClear();

  /** No descriptions */
  void selectRadius(int n);

private slots:

  /** Called, if a new airfield reference is selected. */
  void slotAirfieldRefChanged( const QString& text );

private:

  /**
   * Loads the content into the airfield combo box.
   */
  void loadAirfieldComboBox();

  int center;
  QHash<QString, SinglePoint*> airfieldDict;
  QComboBox *refAirfieldBox;
  QLabel* radiusUnit;
};

#endif
