/***********************************************************************
**
**   waypointdialog.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WAYPOINTDIALOG_H
#define WAYPOINTDIALOG_H

#include "guicontrols/coordedit.h"

#include <qwidget.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kcombobox.h>
#include <kdialog.h>

/**Create and edit waypoints
  *@author Harald Maier
  */

class WaypointDialog : public KDialog  {
   Q_OBJECT
public: 
	WaypointDialog(QWidget *parent=0, const char *name=0);
	~WaypointDialog();
  /** return interna type of surface */
  int getSurface();
  /** return internal type of waypoint */
  int getWaypointType();
  /** set surface type in combo box
translate internal id to index */
  void setSurface(int s);
  /** set waypoint type in combo box
translate internal id to index */
  void setWaypointType(int type);
private: // Private methods
  /** No descriptions */
  void __initDialog();
public: // Public attributes
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
  QLineEdit *runway;
  /**  */
  QLineEdit *length;
  /**  */
  QLineEdit *comment;
  /**  */
  LatEdit *latitude;
  /**  */
  LongEdit *longitude;
  /** */
  QCheckBox *isLandable;
public slots: // Public slots
  /** clear all entries */
  void clear();
signals: // Signals
  /** No descriptions */
  void addWaypoint();
private slots: // Private slots
  /** No descriptions */
  void slotAddWaypoint();
private:
  /**  */
  KComboBox *waypointType;
  /**  */
  KComboBox *surface;
};

#endif
