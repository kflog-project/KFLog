/***********************************************************************
**
**   waypointdialog.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * @class WaypointDialog
 *
 * @short Create and edit a waypoint
 *
 * Create and/or modify a waypoint and its attributes.
 *
 * @author Harald Maier, Axel Pauli
 *
 * @date 2001-2011
 *
 * @version  $Id$
 */

#ifndef WAYPOINT_DIALOG_H
#define WAYPOINT_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

#include "coordedit.h"
#include "runway.h"
#include "waypoint.h"

class WaypointDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointDialog )

public:

	WaypointDialog( QWidget *parent=0 );

	~WaypointDialog();
  /**
   * return internal type of surface
   */
  enum Runway::SurfaceType getSurface();
  /**
   * return internal type of waypoint
   */
  int getWaypointType();
  /**
   * set surface type in combo box translate internal id to index
   */
  void setSurface( enum Runway::SurfaceType st );
  /**
   * set waypoint type in combo box translate internal id to index
   */
  void setWaypointType(int type);

  /** No descriptions */
  void enableApplyButton(bool enable = true);

private:

  /** No descriptions */
  void __initDialog();

public slots:

  /** clear all entries */
  void clear();

  /**
   * Called to make all text to upper cases.
   */
  void slotTextEdited( const QString& text );

signals:

  /** No descriptions */
  void addWaypoint(Waypoint *);

private slots: // Private slots

  /** No descriptions */
  void slotAddWaypoint();

public:

  /**  */
  LatEdit *latitude;
  /**  */
  LongEdit *longitude;
  /**  */
  QLineEdit *name;
  /**  */
  QLineEdit *description;
  /**  */
  QLineEdit *elevation;
  /**  */
  QLineEdit *icao;
  /**  */
  QLineEdit *frequency;
  /**  */
  QComboBox *runway;
  /**  */
  QLineEdit *length;
  /**  */
  QLineEdit *comment;
  /** */
  QCheckBox *isLandable;
  /**  */
  QComboBox *waypointType;
  /**  */
  QComboBox *surface;

  QPushButton *applyButton;

  /** Flag to indicate if an existing waypoint is edited. */
  bool edit;
};

#endif
