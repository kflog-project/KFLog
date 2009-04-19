/***********************************************************************
**
**   flightselectiondialog.h
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

#ifndef FLIGHTSELECTIONDIALOG_H
#define FLIGHTSELECTIONDIALOG_H

#include "baseflightelement.h"

#include <qwidget.h>

#include <kdialog.h>
#include <klistbox.h>

/**Dialog for selecting flights
  *@author Harald Maier
  */

class FlightSelectionDialog : public KDialog  {
   Q_OBJECT
public: // Public attributes
  /**  */
  QPtrList<BaseFlightElement> availableFlights;
  /**  */
  QPtrList<BaseFlightElement> selectedFlights;
  FlightSelectionDialog(QWidget *parent=0, const char *name=0);
  ~FlightSelectionDialog();
private: // Private methods
  /** No descriptions */
  void __initDialog();
private: // Private attributes
  /**  */
  KListBox *aFlights;
  /**  */
  KListBox *sFlights;
private slots: // Private slots
  /** No descriptions */
  void slotAccept();
  /** No descriptions */
  void slotAddOne();
  /** No descriptions */
  void slotAddAll();
  /** No descriptions */
  void slotRemoveOne();
  /** No descriptions */
  void slotRemoveAll();
  /** No descriptions */
  void slotMoveUp();
  /** No descriptions */
  void slotMoveDown();
  void polish();
};

#endif
