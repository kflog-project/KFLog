/***********************************************************************
**
**   flightselectiondialog.h
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
 * \class FlightSelectionDialog
 *
 * \author Harald Maier, Axel Pauli
 *
 * \brief Dialog for grouping of flights
  *
  * Dialog for grouping of flights
  *
  * \date 2002-2011
  *
  * \version $Id$
  */

#ifndef FLIGHT_SELECTION_DIALOG_H
#define FLIGHT_SELECTION_DIALOG_H

#include <QDialog>
#include <QList>
#include <QListWidget>
#include <QShowEvent>

#include "baseflightelement.h"

class FlightSelectionDialog : public QDialog
{
   Q_OBJECT

private:

  Q_DISABLE_COPY ( FlightSelectionDialog )

public:

  FlightSelectionDialog(QWidget *parent=0);

  virtual ~FlightSelectionDialog();

  /**  List with available flights. */
  QList<BaseFlightElement *> availableFlights;

  /** List containing selected flights. */
  QList<BaseFlightElement *> selectedFlights;

protected:

  void showEvent( QShowEvent *event );

private:

  /** Setups the dialog. */
  void __initDialog();

private:

  QListWidget *aFlights;
  QListWidget *sFlights;

private slots:

  /** No descriptions */
  void slotAccept();
  /** No descriptions */
  void slotAddOne();
  /** No descriptions */
  void slotAddAll();
  /** No descriptions */
  void slotMoveOne();
  /** No descriptions */
  void slotMoveAll();
  /** No descriptions */
  void slotMoveUp();
  /** No descriptions */
  void slotMoveDown();
};

#endif
