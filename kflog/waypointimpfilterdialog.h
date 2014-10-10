/***********************************************************************
**
**   waypointimpfilterdialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
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
 * \date 2002-2014
 *
 * \version $Id$
 */

#ifndef WAYPOINT_IMP_FILTER_DIALOG_H
#define WAYPOINT_IMP_FILTER_DIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QPoint>
#include <QRadioButton>

#include "coordedit.h"
#include "singlepoint.h"
#include "waypointcatalog.h"

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
  int getCenterRef() const;

  WGSPoint getAirfieldRef();

  /**
   * Gets the center radius of this filter.
   *
   * \return filter center radius
   */
  double getCenterRadius();

  /**
   * Selects the given entry in the radiusbox.
   *
   * \param radius element to be selected in the box.
   */
  void setCenterRadius( QString& radius );

  /**
   * Returns the activated filter.
   *
   * \return The activated filter
   */
  enum WaypointCatalog::FilterType getFilter();

  /**
   * Sets the filter to be activated.
   *
   * \param filter Filter to be used.
   */
  void setFilter( enum WaypointCatalog::FilterType newFilter );

  QCheckBox *useAll;
  QCheckBox *airfields;
  QCheckBox *gliderfields;
  QCheckBox *navaids;
  QCheckBox *outlandings;
  QCheckBox *obstacles;
  QCheckBox *landmarks;
  QCheckBox *stations;

  LatEdit *fromLat;
  LongEdit *fromLong;
  LatEdit *toLat;
  LongEdit *toLong;
  LatEdit *centerLat;
  LongEdit *centerLong;

  QString airfieldRefTxt;

protected:

  void showEvent( QShowEvent *event );

protected slots:

  void slotChangeUseAll();

public slots:

  /** reset all dialog items to default values */
  void slotClear();

  /** Selects the radius item to be used. */
  void selectRadius(int n);

private slots:

  /** Called, if a new airfield reference is selected. */
  void slotAirfieldRefChanged( const QString& text );

  /** Called, if the radius is changed. */
  void slotRadiusChanged( int newIndex );

  /** Called, if the filter is changed. */
  void slotFilterChanged( int filter );

  /** Called, if the cancel button is pressed. */
  void slotCancel();

  /** Called if the Ok button is pressed. */
  void slotOk();

private:

  /**
   * Loads the airfield content into the airfield combo box.
   */
  void loadAirfieldComboBox();

  /**
   * Saves all current values for a later restore.
   */
  void saveValues();

  /**
   * Loads the selected radius value.
   */
  void loadRadiusValue();

  /**
   * Saves the selected radius value.
   */
  void saveRadiusValue();

  /**
   * Restores all saved values.
   */
  void restoreValues();

  QRadioButton *rb0;
  QRadioButton *rb1;
  QRadioButton *rb2;
  QRadioButton *rb3;

  QRadioButton *rbf0;
  QRadioButton *rbf1;

  QGroupBox* filterGroup;
  QGroupBox* fromGroup;
  QGroupBox* toGroup;
  QGroupBox* radiusGroup;

  int centerRef;
  QHash<QString, SinglePoint*> airfieldDict;
  QComboBox *airfieldRefBox;
  QComboBox *radius;
  QLabel* radiusUnit;

  enum WaypointCatalog::FilterType usedFilter;

  /** Structure to save the initial values of the dialog. */
  struct
    {
      bool useAll;
      bool airfields;
      bool gliderfields;
      bool navaids;
      bool outlandings;
      bool obstacles;
      bool landmarks;
      bool stations;

      enum WaypointCatalog::FilterType usedFilter;
      bool rb0;
      bool rb1;
      bool rb2;
      bool rb3;

      bool rbf0;
      bool rbf1;

      int fromLat;
      int fromLong;
      int toLat;
      int toLong;
      int centerLat;
      int centerLong;
      int centerRef;

      int radiusIdxPosition;
      int radiusIdxHome;
      int radiusIdxMap;
      int radiusIdxAirfield;
      int airfieldRefIdx;
    } save;
};

#endif
