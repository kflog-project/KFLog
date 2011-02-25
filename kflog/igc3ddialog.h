/***********************************************************************
**
**   igc3ddialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFlog-Team
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef IGC_3D_DIALOG_H
#define IGC_3D_DIALOG_H

#include <QDialog>
#include <QResizeEvent>

#include "flight.h"
#include "igc3dview.h"

class Igc3DView;

/**
 * \class Igc3DDialog
 *
 * \brief Dialog widget for IGC 3D view handling.
 *
 * \author Thomas Nielsen., Axel Pauli
 *
 * \date 2002-2011
 *
 * \version $Id$
 */
class Igc3DDialog : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( Igc3DDialog )

 public:

  Igc3DDialog(QWidget *parent);

  virtual ~Igc3DDialog();

  signals:

  /** No descriptions */
  void flightChanged();

  /** Emitted to show the help text. */
  void igc3dHelp( QString& text );

 public slots:
  /**
   * shows the flight data
   */
  void slotShowFlightData();

 protected:

  virtual void resizeEvent(QResizeEvent* event);

  virtual void showEvent( QShowEvent* event );

 private:

  Igc3DView* igc3dView;
};

#endif
