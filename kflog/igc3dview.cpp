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

// Application headers
#include "igc3ddialog.h"
#include <flight.h>
#include <resource.h>
#include <mapcalc.h>
#include <igc3dpolyhedron.h>
#include <igc3dviewstate.h>


#define X_ABSTAND 100
#define Y_ABSTAND 30

class Igc3DViewState;
class Igc3DPolyhedron;

Igc3DView::Igc3DView(Igc3DDialog* dialog, QList<Flight>* fList)
 : QWidget(dialog, "Igc3DView", 0),
  igc3DDialog(dialog), originalflightlist(fList)
{
  isFlight = false;
  setBackgroundColor(QColor(white));

  // create members
	this->state = new Igc3DViewState();
	this->flightbox = new Igc3DPolyhedron(state);
	this->flight = new Igc3DFlightData(state);

  // set size
	state->height = this->height();
	state->width = this->width();
	state->timerflag = 0;

  /**
	 * Now we would read our igc-file, but it is already open and
	 * is accessed by member flight
	 */

  // load the igc3dflightdata list from the original list
	flight->load(originalflightlist);

  flight->koord2dist();
	
	change_zfactor(state->zfactor);
	
	//flight->add_shadow();
	state->deltay = state->deltay + state->deltayoffset;
	state->flight_trace = 1;
	state->flight_shadow = 1;
	
}

Igc3DView::~Igc3DView()
{
//  delete pixBuffer;
  delete state;
  delete flightbox;
  delete flight;
}

QSize Igc3DView::sizeHint()
{
  return QWidget::sizeHint();
}

void Igc3DView::paintEvent(QPaintEvent* event = 0)
{
	/**
   * call the drawing function for the view
   */	
	__draw();

//  warning("paintEvent");
}

void Igc3DView::change_zfactor(int i)
{
	state->zfactor = i;

	if(flight->flight_opened_flag){
		flight->change_zfactor();
//		state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
	if(state->centering){
		flight->centre_data_to_marker();
		flight->calculate_min_max();
//		state->deltay = state->deltayoffset - Dial2->value();
		flightbox->adjust_size();
	}
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

/*
	if(flight->flight_opened_flag){
		dataline = getflightpointdata();
		LineEdit1->setText(dataline);
	}
*/
}
/** No descriptions */
void Igc3DView::slotRedraw(){
	this->__draw();
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


