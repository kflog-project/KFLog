/***********************************************************************
**
**   dataview.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "dataview.h"

#include <klocale.h>
#include <qlayout.h>
#include <qstrlist.h>

#include <flight.h>
#include <mapcalc.h>
#include <flighttask.h>
#include <mapcontents.h>

#include <kmessagebox.h>
#include "iostream.h"

DataView::DataView(QWidget* parent)
: QFrame(parent, "FlightData")
{
  flightDataText = new KTextBrowser(this, "flightDataBrowser", true);

  QHBoxLayout* flightLayout = new QHBoxLayout(this, 5);
  flightLayout->addWidget(flightDataText);

  connect(flightDataText, SIGNAL(urlClick(const QString &)), this,
      SLOT(slotWPSelected(const QString &)));
}

DataView::~DataView()
{

}

void DataView::slotShowTaskText( FlightTask* task, QPoint current)
{
  QList<wayPoint> taskPointList = task->getWPList();
  QString htmlText = "";
  QString tmp;
  double distance = 0;

  if(taskPointList.count() > 0)
  {
    QPoint pre_position, position;

    pre_position = taskPointList.at(0)->origP;
    QString name;

    for(unsigned int n = 0; n < taskPointList.count(); n++)
    	{
  	  	position = taskPointList.at(n)->origP;
		 	  distance += dist(pre_position.y(),pre_position.x(),position.y(),position.x());			
			
  		 	name = taskPointList.at(n)->name;
	  		htmlText += (QString)"<b>" + name + "</b>" + "<br>" +
		  							printPos(position.x(), false) + " / " + printPos(position.y()) + "<br>";
			  pre_position = position;
			
    	}

   	distance += dist(pre_position.y(),pre_position.x(),current.y(),current.x());
  }

  // Frage
  if(taskPointList.count() == 0)
    {
      htmlText += "Bitte wählen Sie den <b>Starort</b> der Aufgabe in der Karte<br>";
    }
  else if(taskPointList.count() == 1)
    {
      htmlText += "<br><b>Abflugpunkt?</b><br>";
    }
  else
    {
      htmlText += "<br><b>Nächster WendePunkt (End-/LandePunkt)?</b><br>";
    }

  tmp.sprintf("<hline><br><br><b>Entfernung: %.2f km<b>",distance);
  htmlText += tmp;

  htmlText += "<hline><br><br><b>Entfernung: " + task->getTotalDistanceString();
  htmlText += "<hline><br><br><b>Entfernung Wertung: " + task->getTaskDistanceString();

  htmlText += "<br><br>Aufgabenart: ";
  switch(task->getTaskType())
    {
      case FlightTask::ZielS:
        htmlText += i18n("Zielstrecke");
        break;
      case FlightTask::ZielR:
        htmlText += i18n("ZielRückkehrstrecke");
        break;
      case FlightTask::FAI:
        htmlText += i18n("FAI Dreieck");
        break;
      case FlightTask::Dreieck:
        htmlText += i18n("Dreieck");
        break;
      case FlightTask::FAI_S:
        htmlText += i18n("FAI Dreieck Start auf Schenkel");
        break;
      case FlightTask::Dreieck_S:
        htmlText += i18n("Dreieck Start auf Schenkel");
        break;
      default:
        htmlText += i18n("Unbekannt");
    }
  htmlText += "<br>";


  htmlText += "<br><hline><br><b>pos:<b>" + printPos(current.y()) + " / "
                                          + printPos(current.x(),true);
  flightDataText->setText(htmlText);
}

void DataView::setFlightData()
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();

  slotClearView();
  if (e) {
    flightDataText->setText(e->getFlightInfoString());
  }
}

void DataView::slotWPSelected(const QString &url)
{
  extern MapContents _globalMapContents;
  BaseFlightElement* e = _globalMapContents.getFlight();

  switch(e->getTypeID()) {
    case BaseMapElement::Flight:
      emit wpSelected(url.toUInt());
      break;
    case BaseMapElement::Task:
      if (url == "EDITTASK") {
        KMessageBox::information(0, "This will bring up the task editing dialog");
      }
      else {
        emit wpSelected(url.toUInt());
      }
      break;
    case BaseMapElement::FlightGroup:
      if (url == "EDITGROUP") {
        KMessageBox::information(0, "This will bring up the flight group editing dialog");
      }
      else {
        emit flightSelected((BaseFlightElement *)url.toUInt());
      }
      break;
  }
}

void DataView::slotClearView()
{
  QString htmlText = "";
  flightDataText->setText(htmlText);
}

