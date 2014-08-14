/***********************************************************************
**
**   evaluationdialog.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                :  2008 by Constantijn Neeteson
**                :  2011-2013 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "evaluationdialog.h"
#include "evaluationframe.h"
#include "evaluationview.h"
#include "flight.h"
#include "mainwindow.h"
#include "mapcontents.h"

#if defined(_WIN32) && !defined(isnan)
#define isnan(x) ((x) != (x))
#endif

extern MapContents* _globalMapContents;
extern QSettings    _settings;

EvaluationDialog::EvaluationDialog( QWidget *parent ) : QWidget( parent )
{
  setWindowTitle( tr("Flight Evaluation") );

  // movable text display
  QSplitter* textSplitter = new QSplitter( Qt::Vertical, this );

  // upper diagram widget
  m_evalFrame = new EvaluationFrame( textSplitter, this );

  connect( this, SIGNAL(flightChanged(Flight *)),
           m_evalFrame, SLOT(slotShowFlight(Flight *)));

  connect( this, SIGNAL(textChanged(QString)),
           m_evalFrame, SLOT(slotUpdateCursorText(QString)));

  // lower text display
  m_textDisplay = new QTextBrowser( textSplitter );
  m_textDisplay->setMinimumHeight( 5 );

  QVBoxLayout* layout = new QVBoxLayout( this );
  layout->setMargin( 5 );
  layout->setSpacing( 1 );
  layout->addWidget(textSplitter);

  setMinimumSize( QSize(600, 200) );

  bool ok = restoreGeometry( _settings.value( "/EvaluationDialog/Geometry" ).toByteArray() );

  if( ! ok )
    {
      resize(800, 600);
    }
}

EvaluationDialog::~EvaluationDialog()
{
  _settings.setValue( "/EvaluationDialog/Geometry", saveGeometry() );
}

void EvaluationDialog::hideEvent( QHideEvent* event )
{
  Q_UNUSED(event)

  emit windowHidden();
}

void EvaluationDialog::showEvent( QShowEvent* event )
{
  QString help = QString("<HTML>") +

      tr( "You can move the begin and end flags in the flight diagram by using "
          "the left mouse button. Position the mouse pointer over the "
          "vertical bar under the flag, press the left mouse button and move the bar "
          "to a new position. Release the mouse button at the new position. "
          "The flag in the diagram is moved to this new position. In parallel "
          "the flag at the map is moved too. <p>Another move possibility is to use "
          "the middle or right mouse button. If you click in the diagram by "
          "using the middle mouse button, the begin flag is moved to this new "
          "position. Using the right mouse button will do the same with the end "
          "flag.</p><p>If you move the mouse pointer at the map along the drawn "
          "flight, an orange triangle shows you the position in the flight "
          "diagram.</p>"
         ) +

       "</HTML>";

  emit evaluationHelp( help );

  slotShowFlightData();
  event->accept();
}

void EvaluationDialog::updateText(int index1, int index2, bool updateAll)
{
  QString htmlText = "";
  QString text;
  FlightPoint p1;
  FlightPoint p2;

  if ( ! m_flight )
  {
    // Clear last displayed text
    m_textDisplay->clear();
    emit textChanged(htmlText);
    return;
  }

  switch(m_flight->getTypeID())
  {

  case BaseMapElement::Flight:

    p1 = m_flight->getPoint(index1);
    p2 = m_flight->getPoint(index2);

    htmlText = QString("<HTML><TABLE WIDTH=\"100%\" BORDER=0 CELLPADDING=3 CELLSPACING=0>")+
              "<TR><TD ALIGN=left><FONT COLOR=#00bb00>" +
              tr("Time=") +  printTime(p1.time, true) +
              tr(" MSL=") + QString::number(p1.height) + "m" +
              tr(" Vario=");

    if( isnan(getVario(p1)) )
      {
        text = "-m/s";
      }
    else
      {
        text.sprintf("%.1fm/s", getVario(p1));
      }

    htmlText += text + tr(" Speed=");

    if( isnan(getSpeed(p1)) )
      {
        text = "-km/h";
      }
    else
      {
        text.sprintf("%.1fkm/h", getSpeed(p1));
      }

    htmlText += text + "</TD>";

    htmlText += QString("<TD ALIGN=right><FONT COLOR=#bb0000>") +
              tr("Time=") + printTime(p2.time, true) +
              tr(" MSL=") + QString::number(p2.height) + "m" +
              tr(" Vario=");

    if( isnan(getVario(p2)) )
      {
        text = "-m/s";
      }
    else
      {
        text.sprintf("%.1fm/s", getVario(p2));
      }

    htmlText += text + tr(" Speed=");

    if( isnan(getSpeed(p2)) )
      {
        text = "-km/h";
      }
    else
      {
        text.sprintf("%.1fkm/h", getSpeed(p2));
      }

    htmlText += text + "</TD></TR></TABLE></HTML>";

    emit textChanged(htmlText);

    htmlText = "";

    if(updateAll)
      {
        QStringList erg = m_flight->getFlightValues(index1, index2);

        htmlText = QString( "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=0>" ) +
                            "<TR><TH align='left'>" + tr("Circling") + "&nbsp;</TH>" +
                            "<TH align='center'><I>" + tr("Time") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Vario") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Gain") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Loss") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Netto") + "</I></TH>" +
                            "<TH>&nbsp;</TH>" +
                            "<TH>&nbsp;</TH>" + "</TR>" +

                            "<TR><TD>" + tr("total:") + "</TD>" +
                            "<TD ALIGN=left>" + erg.at(3) +
                            "</TD><TD ALIGN=right>" + erg.at(7) +
                            "</TD><TD ALIGN=right>" + erg.at(11) +
                            "</TD><TD ALIGN=right>" + erg.at(15) +
                            "</TD><TD ALIGN=right>" + erg.at(19) +
                            "</TD>" +
                            "<TD>&nbsp;</TD><TD>&nbsp;</TD></TR>" +

                            "<TR><TD>" + tr("right:") + "</TD>" +
                            "<TD ALIGN=left>" + erg.at(0) +
                            "</TD><TD ALIGN=right>" + erg.at(4) +
                            "</TD><TD ALIGN=right>" + erg.at(8) +
                            "</TD><TD ALIGN=right>" + erg.at(12) +
                            "</TD><TD ALIGN=right>" + erg.at(16) +
                            "</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>" +

                            "<TR><TD>" + tr("left:") + "</TD>" +
                            "<TD ALIGN=left>" + erg.at(1) +
                            "</TD><TD ALIGN=right>" + erg.at(5) +
                            "</TD><TD ALIGN=right>" + erg.at(9) +
                            "</TD><TD ALIGN=right>" + erg.at(13) +
                            "</TD><TD ALIGN=right>" + erg.at(17) +
                            "</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>" +

                            "<TR><TD>" + tr("mixed:") + "</TD>" +
                            "<TD ALIGN=left>" + erg.at(2) +
                            "</TD><TD ALIGN=right>" + erg.at(6) +
                            "</TD><TD ALIGN=right>" + erg.at(10) +
                            "</TD><TD ALIGN=right>" + erg.at(14) +
                            "</TD><TD ALIGN=right>" + erg.at(18) +
                            "</TD><TD>&nbsp;</TD><TD>&nbsp;</TD></TR>" +

                            "<TR><TD colspan=\"8\">&nbsp;</TD></TR>" +

                            "<TR><TH align=left>" + tr("Straight") + "&nbsp;</TH>" +
                            "<TH align='center'><I>" + tr("Time") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Distance") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Gain") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Loss") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Alt. Netto") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("Speed") + "</I></TH>" +
                            "<TH align='center'><I>" + tr("L/D ratio") + "</I></TH></TR>" +

                            "<TR><TD>&nbsp;</TD>" +
                            "<TD ALIGN=left>" + erg.at(26) + "</TD>" +
                            "<TD ALIGN=center>" + erg.at(25) + "</TD>" +
                            "<TD ALIGN=right>" + erg.at(22) + "</TD>" +
                            "<TD ALIGN=right>" + erg.at(23) + "</TD>" +
                            "<TD ALIGN=right>" + erg.at(24) + "</TD>" +
                            "<TD ALIGN=center>" + erg.at(21) + "</TD>" +
                            "<TD ALIGN=center>" + erg.at(20) + "</TD>" +
                            "</TR>" +

                            "<TR><TD colspan=\"8\"></TD></TR>" +
                            "<TR><TH align=left>" + tr("Total") + "&nbsp;</TH>" +
                            "<TD ALIGN=left>" + erg.at(27) + "</TD>" +
                            "<TD>&nbsp;</TD>" +
                            "<TD ALIGN=right>" + erg.at(28) + "</TD>" +
                            "<TD ALIGN=right>" + erg.at(29) + "</TD>" +
                            "<TD>&nbsp;</TD>" +
                            "<TD>&nbsp;</TD>" +
                            "</TR></TABLE><BR><HR><BR>";

        QList<statePoint*> state_list;
        QString text = "";
        state_list = m_flight->getFlightStates(index1, index2);

        htmlText +=  "<TABLE border='0' cellpadding='3' cellspacing='0'> \
                      <TR><TH align=left colspan=11>" + tr("Flight sections") + "</TH></TR>";

        for(int n = 0; n < state_list.count(); n++)
        {
          if( n%10 == 0 )
            {
              if( n != 0 )
                {
                  // Add an empty line
                  htmlText += "<TR><TD colspan=11>&nbsp;</TD></TR>";
                }

              htmlText += "<TR> \
                          <TH align='left'><I>" + tr("Type") + "</I></TH> \
                          <TH align='center'><I>" + tr("Start") + "</I></TH> \
                          <TH align='center'><I>" + tr("End") + "</I></TH> \
                          <TH align='center'><I>" + tr("Time") + "</I></TH> \
                          <TH align='right'><I>" + tr("Alt. Gain") + "</I></TH> \
                          <TH align='right'><I>" + tr("Alt. Loss") + "</I></TH> \
                          <TH align='right'><I>" + tr("Alt. Netto") + "</I></TH> \
                          <TH align='right'><I>" + tr("Distance") + "</I></TH> \
                          <TH align='center'><I>" + tr("Speed") + "</I></TH> \
                          <TH align='right'><I>" + tr("L/D") + "</I></TH> \
                          <TH align='center'><I>" + tr("Vario") + "</I></TH></TR>";
            }

          switch(state_list.at(n)->f_state)
            {
              case Flight::RightTurn:
                text = QString( tr("Circling (%1 right)") ).arg(state_list.at(n)->circles, 0, 'f', 1 );
                break;

              case Flight::LeftTurn:
                text = QString( tr("Circling (%1 left)") ).arg(state_list.at(n)->circles, 0, 'f', 1 );
                break;

              case Flight::MixedTurn:
                text = QString( tr("Circling (%1 mixed)") ).arg(state_list.at(n)->circles, 0, 'f', 1 );
                break;

              case Flight::Straight:
                text = "Straight";
                break;
            }

          //text.sprintf("%i", state_list.at(n)->f_state);
          htmlText += (QString) "<TR><TD align=\"left\"><pre>" + text + "</pre></TD>";
          text.sprintf("%s", printTime(state_list.at(n)->start_time, true).toAscii().data());
          htmlText += (QString) "<TD align='right'>" + text + "</TD>";
          text.sprintf("%s", printTime(state_list.at(n)->end_time, true).toAscii().data());
          htmlText += (QString) "<TD align='right'>" + text + "</TD>";
          text.sprintf("%s", printTime(state_list.at(n)->duration, true, true, true).toAscii().data());
          htmlText += (QString) "<TD align='right'>" + text + "</TD>";
          text.sprintf("%i", state_list.at(n)->dH_pos);
          htmlText += (QString) "<TD align='right'>" + text + " m</TD>";
          text.sprintf("%i", state_list.at(n)->dH_neg);
          htmlText += (QString) "<TD align='right'>" + text + " m</TD>";
          text.sprintf("%i", (state_list.at(n)->dH_pos+state_list.at(n)->dH_neg));
          htmlText += (QString) "<TD align='right'>" + text + " m</TD>";
          text.sprintf("%.1f", state_list.at(n)->distance);
          htmlText += (QString) "<TD align='right'>" + text + " km</TD>";

          if(state_list.at(n)->f_state==Flight::Straight)
            {
              text.sprintf("%.1f %s", state_list.at(n)->speed, "km/h");
            }
          else
            {
            text = "";
            }

          htmlText += (QString) "<TD align='right'>" + text + "</TD>";

          if(state_list.at(n)->f_state==Flight::Straight)
            {
              if(abs((int) state_list.at(n)->L_D)>1000)
                  text = tr("inf");
              else
                  text.sprintf("%.0f", state_list.at(n)->L_D);
            }
          else
            {
              text = "";
            }

          htmlText += (QString) "<TD align='right'>" + text + "</TD>";
          text.sprintf("%.1f", state_list.at(n)->vario);
          htmlText += (QString) "<TD align='right'>" + text + " m/s</TD>";
          htmlText += (QString) "</TR>";
          }

        htmlText += "</TABLE>";
      }

    emit showCursor(p1.projP, p2.projP);
    break;

  case BaseMapElement::Task:
    emit textChanged(QString::null);
    htmlText = "<DIV ALIGN=CENTER>" + tr("Cannot evaluate task") + "</DIV>";
    break;

  case BaseMapElement::FlightGroup:
    emit textChanged(QString::null);
    htmlText = "<DIV ALIGN=CENTER>" + tr("Cannot (yet) evaluate a flight group") + "</DIV>";
    break;

  default:
    break;
  }

  m_textDisplay->setHtml(htmlText);
}


void EvaluationDialog::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  slotShowFlightData();
}

void EvaluationDialog::slotShowFlightData()
{
  m_flight = dynamic_cast<Flight *> (_globalMapContents->getFlight());

  QWidget* parent = parentWidget();

  if( m_flight != static_cast<Flight *> (0) )
    {
      if( m_flight->getTypeID() == BaseMapElement::Flight && parent )
        {
          parent->setWindowTitle( tr( "Flight Evaluation" ) + ": [" +
                                  QFileInfo(m_flight->getFileName()).fileName() + "] " +
                                  m_flight->getPilot() + ", " +
                                  m_flight->getDate() );
        }
      else
        {
          parent->setWindowTitle( tr( "Flight Evaluation" ) );
        }

      // set defaults
      updateText( 0, m_flight->getRouteLength() - 1, true );
    }
  else
    {
      parent->setWindowTitle( tr( "Flight Evaluation" ) );
      updateText( 0, 0, false );
    }

  emit flightChanged( m_flight );
}

void EvaluationDialog::slotSetCursors(time_t NewCursor1, time_t NewCursor2)
{
  if( getFlight() )
    {
      m_evalFrame->getEvalView()->slotSetCursors( m_flight, NewCursor1, NewCursor2 );
    }
}

/** No descriptions */
void EvaluationDialog::slotShowFlightPoint(const QPoint&, const FlightPoint& fp)
{
  emit(showFlightPoint(&fp));
}

/** No descriptions */
void EvaluationDialog::slotRemoveFlightPoint()
{
  emit(showFlightPoint(0));
}
