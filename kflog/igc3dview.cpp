/***********************************************************************
**
**   igc3dview.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by the KFLog-Team
**                   2011 by Axel Pauli
**
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "igc3dview.h"
#include "mapcontents.h"

#define X_ABSTAND 100
#define Y_ABSTAND 30

static const QString HelpMessageTitle( QObject::tr("Help for IGC 3D view" ) );

static const QString HelpMessageText( "<PRE>" +
                                      QObject::tr(
                                         "Basic key functions:\n\n"
                                         "Left/Right: rotate\n"
                                         "Up/Down:    tilt\n"
                                         "+/-:        zoom\n"
                                         "S:          toggle shadow\n"
                                         "B:          toggle back\n"
                                         "F:          toggle front\n") +
                                      "</PRE>" );

class Igc3DViewState;
class Igc3DPolyhedron;

extern MapContents *_globalMapContents;

Igc3DView::Igc3DView( Igc3DDialog* dialog ) :
  QWidget(dialog),
  igc3DDialog(dialog)
{
  setObjectName("Igc3DView");
  setFocusPolicy(Qt::StrongFocus);

  QPalette p = palette();
  p.setColor( QPalette::Window, Qt::white );
  setPalette(p);
  setAutoFillBackground( true );

  isFlight = false;

  // create members
  state     = new Igc3DViewState();
  flightbox = new Igc3DPolyhedron(state);
  flight    = new Igc3DFlightData(state);

  // set size
  state->height    = height();
  state->width     = width();
  state->timerflag = 0;

  slotShowFlight();
}

Igc3DView::~Igc3DView()
{
  delete state;
  delete flightbox;
  delete flight;
}

void Igc3DView::showHelp()
{
  QMessageBox::information( this, HelpMessageTitle, HelpMessageText );
}

QString Igc3DView::getHelp()
{
  QString help = QString("<HTML><DIV align=\"center\">") +
                 HelpMessageTitle +
                 "</DIV>" +
                 HelpMessageText +
                 "</HTML>";
  return help;
}

void Igc3DView::resizeEvent( QResizeEvent* event )
{
  qDebug() << "Igc3DView::resizeEvent" << event->size();

  QSize qs = event->size();
  state->height = qs.height();
  state->width  = qs.width();
}

QSize Igc3DView::sizeHint()
{
  qDebug() << "Igc3DView::sizeHint()" << QWidget::sizeHint();

  QSize qs = QWidget::sizeHint();
  state->height = qs.height();
  state->width = qs.width();

  return qs;
}

void Igc3DView::paintEvent( QPaintEvent *event )
{
  qDebug() << "Igc3DView::paintEvent Rein" << event->rect()
           << "state->height" << state->height
           << "state->width" << state->width;

  QPainter painter( this );

  flightbox->calculate();

  if( flight->flight_opened_flag )
    {
      if( state->flight_trace )
        {
          flight->calculate_flight();
        }

      if( state->flight_shadow )
        {
          flight->calculate_shadow();
        }
    }

  if( state->polyhedron_back )
    {
      flightbox->draw_back(&painter);
    }

  if( flight->flight_opened_flag )
    {
      if( flightbox->is_front( 4 ) )
        {
          if( state->flight_trace )
            {
              flight->draw_flight(&painter);
              flight->draw_marker(&painter);
            }
          if( state->flight_shadow )
            {
              flight->draw_shadow(&painter);
            }
        }
      else
        {
          if( state->flight_shadow )
            {
              flight->draw_shadow(&painter);
            }
          if( state->flight_trace )
            {
              flight->draw_flight(&painter);
              flight->draw_marker(&painter);
            }
        }
    }

  if( state->polyhedron_front )
    {
      flightbox->draw_front(&painter);
    }

  qDebug() << "Igc3DView::paintEvent Raus";
}

/** No descriptions */
void Igc3DView::slotRedraw()
{
  update();
}

void Igc3DView::keyPressEvent ( QKeyEvent * k )
{
  int n;

  switch ( k->key() )
    {
      case Qt::Key_R:
              reset();
              break;
//                case Key_T:
//                        state->flight_trace = (state->flight_trace + 1)%2;
//                        break;
      case Qt::Key_S:
              state->flight_shadow = (state->flight_shadow + 1)%2;
              break;
      case Qt::Key_B:
              state->polyhedron_back = (state->polyhedron_back + 1)%2;
              break;
      case Qt::Key_F:
              state->polyhedron_front = (state->polyhedron_front + 1)%2;
              break;
      case Qt::Key_Down:
              n = (int)state->alpha-5;
              change_alpha(n);
              break;
      case Qt::Key_Up:
              n = (int)state->alpha+5;
              change_alpha(n);
              break;
      case Qt::Key_Left:
              n = (int)state->gamma+5;
              change_gamma(n);
              break;
      case Qt::Key_Right:
              n = (int)state->gamma-5;
              change_gamma(n);
              break;
      case Qt::Key_Plus:
              n = (int)state->mag+2;
              change_mag(n);
              break;
      case Qt::Key_Minus:
              n = (int)state->mag-2;
              change_mag(n);
              break;
      case Qt::Key_F1:
              showHelp();
              break;
  }

  repaint();
}

void Igc3DView::reset()
{
  change_centering( 0 );
  state->reset();
}

Igc3DViewState* Igc3DView::setState(Igc3DViewState* vs)
{
  Igc3DViewState* rs = new Igc3DViewState();
  rs = state;

  state = vs;
  return rs;
}

void Igc3DView::change_mag(int i)
{
  state->mag = i;
}

void Igc3DView::change_dist(int i)
{
  // Make sure delay will never be less than offset (-> display would look funny)
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

  if( flight->flight_opened_flag )
    {
      flight->change_zfactor();
      flightbox->adjust_size();
    }

  if( state->centering )
    {
      flight->centre_data_to_marker();
      flight->calculate_min_max();
      flightbox->adjust_size();
    }

  repaint();
}

void Igc3DView::change_fps(int i)
{
  state->ms_timer = (int) (1000.0 / i);

  if( state->timerflag == 1 )
    {
      killTimer( timerID );
      timerID = startTimer( state->ms_timer );
    }
}

void Igc3DView::change_rotation_factor(int i)
{
  state->rotate_fract = i;
  repaint();
}

void Igc3DView::change_centering(int i)
{
  state->centering = i;

  if( i == 0 && flight->flight_opened_flag )
    {
      flight->calculate_min_max();
      flight->flatten_data();
      flight->calculate_min_max();
      flightbox->adjust_size();

    }
  else if( flight->flight_opened_flag )
    {
      flight->centre_data_to_marker();
      flight->calculate_min_max();
      flightbox->adjust_size();
    }

  repaint();
}

void Igc3DView::set_flight_marker(int i)
{
  state->flight_marker_position = i;

  if( state->centering && flight->flight_opened_flag )
    {
      flight->centre_data_to_marker();
      flight->calculate_min_max();
      flightbox->adjust_size();
    }

  if( state->timerflag == 0 )
    {
      repaint();
    }
}

void Igc3DView::slotShowFlight()
{
  /**
  * Now we would read our igc-file, but it is already open and
  * is accessed by member flight
  */

  // load the igc3dflightdata list from the original list
  // Note, that this function does not only delivers Flight objects!!!
  Flight *cf = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  if( cf == static_cast<Flight *> (0) )
    {
      return;
    }

  flight->load(cf);
  flight->koord2dist();

  change_zfactor(state->zfactor);

  state->deltay = state->deltay + state->deltayoffset;
  state->flight_trace = 1;
  state->flight_shadow = 1;
  reset();
}

void Igc3DView::mousePressEvent(QMouseEvent* event)
{
  Q_UNUSED( event )

  showHelp();
}
