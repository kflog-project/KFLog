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
#include <qlist.h>
#include <qscrollview.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qtextview.h>

// Appplication specific headers
#include <flight.h>
#include <igc3dview.h>

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
  Igc3DDialog();
  /** */
  ~Igc3DDialog();

 signals:

 public slots:
  /**
   * shows the flight data
	 */
  void slotShowFlightData(int n);
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
};

#endif
