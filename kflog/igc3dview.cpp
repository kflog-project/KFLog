/***********************************************************************
**
**   igc3dview.cpp
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

#include "igc3dview.h"

// Qt headers
#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qmessagebox.h>

// KDE headers
#include <klocale.h>

// Application headers
#include "igc3ddialog.h"
#include "flight.h"
#include "resource.h"
#include "mapcalc.h"
#include "igc3dpolyhedron.h"
#include "igc3dviewstate.h"
#include "mapcontents.h"


#define X_ABSTAND 100
#define Y_ABSTAND 30

#define DISPLAY_HELP_MESSAGE QMessageBox::information( this, i18n("Help for 3D view"), \
                                      i18n("Basic key functions:\n\n" \
												"Left/Right :\t rotate \n"  \
												"Up/Down : \t tilt \n"        \
												"+/- : \t zoom \n"               \
												"S : \t \t toggle shadow \n" \
												"B : \t \t toggle back \n"       \
												"F : \t \t toggle front \n"),     \
                                      i18n("&Ok"), 0,  0 );



class Igc3DViewState;
class Igc3DPolyhedron;

Igc3DView::Igc3DView(Igc3DDialog* dialog)
 : QWidget(dialog, "Igc3DView", 0),
  igc3DDialog(dialog)
{
  isFlight = false;
  setBackgroundColor(QColor(white));

  setFocusPolicy(QWidget::StrongFocus);

  // create members
  this->state = new Igc3DViewState();
  this->flightbox = new Igc3DPolyhedron(state);
  this->flight = new Igc3DFlightData(state);

  // set size
  state->height = this->height();
  state->width = this->width();
  state->timerflag = 0;

  slotShowFlight();
}

Igc3DView::~Igc3DView()
{
  delete state;
  delete flightbox;
  delete flight;
}

void Igc3DView::resizeEvent( QResizeEvent * event )
{
	QSize qs;

	qs = event->size();
	state->height = qs.height();
	state->width = qs.width();
}


QSize Igc3DView::sizeHint()
{
	QSize qs;
	qs = QWidget::sizeHint();

	state->height = qs.height();
	state->width = qs.width();

  return qs;
}

void Igc3DView::paintEvent(QPaintEvent* event = 0)
{
	/**
   * call the drawing function for the view
   */	
	__draw();
}

/** No descriptions */
void Igc3DView::__draw(void)
{
  QPainter paint;
	QPainter *p;
	p = &paint;
	
	QPixmap pm(state->width, state->height);
	pm.fill(); // clears the pixmap (white)
	
	p->begin(&pm, this);
	
	flightbox->calculate();
	
	if(flight->flight_opened_flag){
		if(state->flight_trace){
			flight->calculate_flight();
		}
	
		if(state->flight_shadow){
			flight->calculate_shadow();
		}
	}
	
	if(state->polyhedron_back){
		flightbox->draw_back(p);
	}
	
	if(flight->flight_opened_flag){
		if(flightbox->is_front(4)){
			if(state->flight_trace){
				flight->draw_flight(p);
				flight->draw_marker(p);
			}
			if(state->flight_shadow){
				flight->draw_shadow(p);
			}
		} else {
			if(state->flight_shadow){
				flight->draw_shadow(p);
			}
			if(state->flight_trace){
				flight->draw_flight(p);
				flight->draw_marker(p);
			}
		}
	}
	
	if(state->polyhedron_front){
		flightbox->draw_front(p);
	}
	
	p->end();
	bitBlt(this, 0, 0, &pm);
}

/** No descriptions */
void Igc3DView::slotRedraw(){
	this->__draw();
}

void Igc3DView::keyPressEvent ( QKeyEvent * k )
{
	int n;

	switch ( k->key() ) {
		case Key_R:
			reset();
			break;
//		case Key_T:
//			state->flight_trace = (state->flight_trace + 1)%2;
//			break;
		case Key_S:
			state->flight_shadow = (state->flight_shadow + 1)%2;
			break;
		case Key_B:
			state->polyhedron_back = (state->polyhedron_back + 1)%2;
			break;
		case Key_F:
			state->polyhedron_front = (state->polyhedron_front + 1)%2;
			break;
		case Key_Down:
			n = (int)state->alpha-5;
			change_alpha(n);
			break;
		case Key_Up:
			n = (int)state->alpha+5;
			change_alpha(n);
			break;
		case Key_Left:
			n = (int)state->gamma+5;
			change_gamma(n);
			break;
		case Key_Right:
			n = (int)state->gamma-5;
			change_gamma(n);
			break;
		case Key_Plus:
			n = (int)state->mag+2;
			change_mag(n);
			break;
		case Key_Minus:
			n = (int)state->mag-2;
			change_mag(n);
			break;
		case Key_F1:
			DISPLAY_HELP_MESSAGE
			break;
	}
	__draw();
}

void Igc3DView::reset()
{
	change_centering(0);
	state->reset();

	//startTimer( state->ms_timer );
}

Igc3DViewState* Igc3DView::getState()
{
	return this->state;
}

Igc3DViewState* Igc3DView::setState(Igc3DViewState* vs)
{
  Igc3DViewState* rs = new Igc3DViewState();
	rs = this->state;

  this->state = vs;

  return rs;
}

void Igc3DView::change_mag(int i)
{
	state->mag = i;
}

void Igc3DView::change_dist(int i)
{
	// Make sure deltay will never be less than offset (-> display would look funny)
	state->deltay = state->deltayoffset - i;
}

void Igc3DView::change_alpha(int i)
{
	state->alpha = i;
}

void Igc3DView::change_beta(int i)
{
	state->beta = i;
}

void Igc3DView::change_gamma(int i)
{
	state->gamma = i;
}

void Igc3DView::change_zfactor(int i)
{
	state->zfactor = i;
	if(flight->flight_opened_flag){
		flight->change_zfactor();
		//state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
	if(state->centering){
		flight->centre_data_to_marker();
		flight->calculate_min_max();
		//state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
	__draw();
}

void Igc3DView::change_fps(int i)
{
	state->ms_timer = (int) (1000.0/i);
	if(state->timerflag == 1){
		killTimers();
		startTimer( state->ms_timer );
	}
}

void Igc3DView::change_rotation_factor(int i)
{
	state->rotate_fract = i;
	__draw();
}

void Igc3DView::change_centering(int i)
{
	state->centering = i;
	if(i == 0 && flight->flight_opened_flag){
		flight->calculate_min_max();
		flight->flatten_data();
		flight->calculate_min_max();
//		state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	
	} else if(flight->flight_opened_flag){
		flight->centre_data_to_marker();
		flight->calculate_min_max();
//		state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
	__draw();
}

void Igc3DView::set_flight_marker(int i)
{
	state->flight_marker_position = i;
	if(state->centering && flight->flight_opened_flag){
		flight->centre_data_to_marker();
		flight->calculate_min_max();
//		state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
	if(state->timerflag == 0){
		__draw();
	}
}
/** No descriptions */
void Igc3DView::slotShowFlight()
{
  /**
  * Now we would read our igc-file, but it is already open and
  * is accessed by member flight
  */

  // load the igc3dflightdata list from the original list
  extern MapContents _globalMapContents;
  flight->load((Flight*)_globalMapContents.getFlight());

  flight->koord2dist();
  	
  change_zfactor(state->zfactor);
  	
  //flight->add_shadow();
  state->deltay = state->deltay + state->deltayoffset;
  state->flight_trace = 1;
  state->flight_shadow = 1;
  this->reset();
}
/** No descriptions */
void Igc3DView::mousePressEvent(QMouseEvent* event){
	DISPLAY_HELP_MESSAGE
}
