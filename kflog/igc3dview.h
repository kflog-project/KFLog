/***********************************************************************
**
**   igc3dview.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFLog-Team
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef IGC3DVIEW_H
#define IGC3DVIEW_H

// Qt headers
#include <qpixmap.h>
#include <qscrollview.h>
#include <qwidget.h>
#include <qframe.h>

// Application headers
#include <igc3dviewstate.h>
#include <igc3dpolyhedron.h>
#include <igc3ddialog.h>
#include <igc3dflightdata.h>

class Flight;
class Igc3DDialog;
class Igc3DViewState;
class Igc3DPolyhedron;
class Igc3DFlightData;


/**
  * @author Thomas Nielsen
  * @version $Id$
  *
  * Based on Igc3D by Jan Max Krueger <Jan.Krueger@uni-konstanz.de>
  *
  */
class Igc3DView : public QWidget
{
  Q_OBJECT

 public:
  /** */
  Igc3DView(Igc3DDialog* dialog);
  /** */
  ~Igc3DView();
  /** */
  QSize sizeHint();
  /** */
  Igc3DViewState* getState(void);	
  /** */
  Igc3DViewState* setState(Igc3DViewState* vs);	
  /** */
	void keyPressEvent ( QKeyEvent * k );
	/** */
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

 protected:
  /**
   * Redefinition of the paintEvent.
   */
  virtual void paintEvent(QPaintEvent* event);

	/**
	 * Redefinition of the resize event
	 */
	virtual void resizeEvent(QResizeEvent * event );


  /**
	 * No descriptions
	 */
  void __draw(void);

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

public slots: // Public slots
  /** No descriptions */
  void slotRedraw();
};

#endif
