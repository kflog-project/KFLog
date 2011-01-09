/***********************************************************************
**
**   igc3ddialog.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFlog-Team
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef IGC_3D_DIALOG_H
#define IGC_3D_DIALOG_H

// Qt headers
#include <QDialog>
#include <QResizeEvent>

#include "flight.h"
#include "igc3dview.h"

class Igc3DView;

/**
 * @author Thomas Nielsen
 * @version $Id$
 *
 *
 */
class Igc3DDialog : public QDialog
{
  Q_OBJECT

 public:
  /** */
  Igc3DDialog(QWidget *parent);
  /** */
  ~Igc3DDialog();

  signals:

  /** No descriptions */
  void flightChanged();

 public slots:
  /**
   * shows the flight data
   */
  void slotShowFlightData();

 protected:
  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

 private:
  /**
   */
  Igc3DView* igc3dView;
};

#endif
