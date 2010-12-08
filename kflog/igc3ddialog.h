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

#ifndef IGC3DDIALOG_H
#define IGC3DDIALOG_H

// Qt headers
#include <qdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <q3ptrlist.h>
#include <q3scrollview.h>
#include <qspinbox.h>
#include <qslider.h>
#include <q3textview.h>
//Added by qt3to4:
#include <QResizeEvent>

// Appplication specific headers
#include "flight.h"
#include "igc3dview.h"

class Flight;
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

 public slots:
  /**
   * shows the flight data
	 */
  void slotShowFlightData();
  /**
   * Hide slot
	 */
  void hide();

 protected:
  /**
   * Redefinition of the resizeEvent.
   */
  virtual void resizeEvent(QResizeEvent* event);

 private:
  /*
   *
   */
  Igc3DView* igc3dView;
signals: // Signals
  /** No descriptions */
  void flightChanged();
};

#endif
