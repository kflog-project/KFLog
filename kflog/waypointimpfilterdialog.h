/***********************************************************************
**
**   waypointimpfilterdialog.h
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

#ifndef WAYPOINTIMPFILTERDIALOG_H
#define WAYPOINTIMPFILTERDIALOG_H

#include <guicontrols/coordedit.h>

#include <qcheckbox.h>
#include <qcombobox.h>

#include <kdialog.h>

/**filter import of waypoints from map
  *@author Harald Maier
  */

class WaypointImpFilterDialog : public KDialog  {
   Q_OBJECT
public: 
	WaypointImpFilterDialog(QWidget *parent=0, const char *name=0);
	~WaypointImpFilterDialog();

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
public slots: // Public slots
  /** reset all dialog items to default values */
  void slotClear();
};

#endif
