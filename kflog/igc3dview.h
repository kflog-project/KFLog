/***********************************************************************
**
**   igc3dview.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFLog-Team
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef IGC_3D_VIEW_H
#define IGC_3D_VIEW_H

// Qt headers
#include <QWidget>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>

// Application headers
#include "igc3dviewstate.h"
#include "igc3dpolyhedron.h"
#include "igc3ddialog.h"
#include "igc3dflightdata.h"

class Flight;
class Igc3DDialog;
class Igc3DViewState;
class Igc3DPolyhedron;
class Igc3DFlightData;

/**
  * \author Thomas Nielsen, Axel Pauli
  *
  * Based on Igc3D by Jan Max Krueger <Jan.Krueger@uni-konstanz.de>
  *
  * \date 2002-2011
  *
  * \version $Id$
  */
class Igc3DView : public QWidget
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( Igc3DView )

 public:

  Igc3DView(Igc3DDialog* dialog);

  ~Igc3DView();

  QSize sizeHint();

  Igc3DViewState* getState()
    {
      return state;
    };

  Igc3DViewState* setState(Igc3DViewState* vs);	

  void reset();
  /**
   * Various movement functions
   */
  void change_mag(int i);
  void change_dist(int i);
  void change_alpha(int i);
  void change_beta(int i);
  void change_gamma(int i);
  void change_zfactor(int i);
  void change_fps(int i);
  void change_rotation_factor(int i);
  void change_centering(int i);
  void set_flight_marker(int i);

  QString getHelp();

private:

  void showHelp();

 protected:

  virtual void keyPressEvent( QKeyEvent * k );

  virtual void paintEvent( QPaintEvent* event );

  virtual void resizeEvent( QResizeEvent * event );

  virtual void mousePressEvent( QMouseEvent* event );

 private:
  /**
   * Parent dialog
   */
  Igc3DDialog* igc3DDialog;
	/**
   * Flight data to display
   */
  Igc3DFlightData* flight;
  /**
   * Flight data?
   */
  bool isFlight;
  /**
   * 3D view state
   */
  Igc3DViewState* state;
  /**
   * 3D box for flight
   */
  Igc3DPolyhedron* flightbox;

  int timerID;

public slots: // Public slots
  /** No descriptions */
  void slotRedraw();
  /** No descriptions */
  void slotShowFlight();
};

#endif
