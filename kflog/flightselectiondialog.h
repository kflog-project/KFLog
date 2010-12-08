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

#include <qdialog.h>
#include <q3listbox.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3PtrList>

/**Dialog for selecting flights
  *@author Harald Maier
  */

class FlightSelectionDialog : public QDialog  {
   Q_OBJECT
public: // Public attributes
  /**  */
  Q3PtrList<BaseFlightElement> availableFlights;
  /**  */
  Q3PtrList<BaseFlightElement> selectedFlights;
  FlightSelectionDialog(QWidget *parent=0, const char *name=0);
  ~FlightSelectionDialog();
private: // Private methods
  /** No descriptions */
  void __initDialog();
private: // Private attributes
  /**  */
  Q3ListBox *aFlights;
  /**  */
  Q3ListBox *sFlights;
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
