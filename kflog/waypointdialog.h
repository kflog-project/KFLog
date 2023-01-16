/***********************************************************************
**
**   waypointdialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * @date 2001-2023
 *
 * @version 1.1
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "coordedit.h"
#include "Frequency.h"
#include "runway.h"
#include "waypoint.h"

class WaypointDialog : public QDialog
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( WaypointDialog )

public:

  WaypointDialog( QWidget *parent=0 );

  virtual ~WaypointDialog();

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

private slots:

  /**
   * Called to make all text to upper cases.
   */
  void slotTextEditedName( const QString& text );

  /**
   * Called to make all text to upper cases.
   */
  void slotTextEditedCountry( const QString& text );

  /** Called if the ok button is pressed. */
  void slotAccept();

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
  /** */
  QString startName;
  /**  */
  QLineEdit *description;
  /**  */
  QLineEdit *country;
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

  /**
   * Runway Frequency, only one frequency is supported.
   */
  Frequency rwyFrequency;

  /**
   * Runway data, only one runway is supported
   */
  Runway rwy;

  /** Flag to indicate if an existing waypoint is edited. */
  bool edit;

  /**
   * Sets the elevation according to the user's selection.
   */
  void setElevation( float newValue );

  /**
   * Returns the elevation always as meters.
   */
  float getElevation();

private:

  void setElevationLabelText();

  /**  */
  QLineEdit *elevation;

  QLabel *elevationLabel;
};

