/***********************************************************************
**
**   olcdialog.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "olcdialog.h"

#include <config.h>
#include <mapcalc.h>

#include <kconfig.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kseparator.h>

#include <qfile.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qregexp.h>

#define POS_STRINGS(point) \
  latitude = point.lat(); \
  longitude = point.lon(); \
  if(latitude < 0) latH = "S"; \
  if(longitude < 0) lonH = "W"; \
  degree = latitude / 600000; \
  min = (latitude - (degree * 600000)) / 10000; \
  min_deg = (latitude - (degree * 600000) - (min * 10000)); \
  min_deg = min_deg / 1000; \
  latG.sprintf("%d", degree); \
  latM.sprintf("%d", min); \
  latMD.sprintf("%d", min_deg); \
  degree = longitude / 600000; \
  min = (longitude - (degree * 600000)) / 10000; \
  min_deg = (longitude - (degree * 600000) - (min * 10000)); \
  min_deg = min_deg / 1000; \
  lonG.sprintf("%d", degree); \
  lonM.sprintf("%d", min); \
  lonMD.sprintf("%d", min_deg);

OLCDialog::OLCDialog(QWidget* parent, const char* name, Flight* cF)
  : QDialog(parent, name), currentFlight(cF)
{
  setCaption(i18n("OLC-Declaration"));

  QGridLayout* mainLayout = new QGridLayout(this, 7, 5, 8, 1);
  mainLayout->addMultiCellWidget(new QLabel(i18n("OLC-Declaration"), this),
      0, 0, 0, 6);
  mainLayout->addMultiCellWidget(new KSeparator(this), 1, 1, 0, 6);
  mainLayout->addMultiCellWidget(new KSeparator(this), 3, 3, 0, 6);

  olcName = new KComboBox(this);
  olcName->setEditable(false);
  olcName->insertItem("OLC-" + i18n("International"));
  olcName->insertItem("OLC-" + i18n("Australia"));
  olcName->insertItem("OLC-" + i18n("Austria"));
  olcName->insertItem("OLC-" + i18n("Africa"));
  olcName->insertItem("OLC-" + i18n("Brasil"));
  olcName->insertItem("OLC-" + i18n("Canada"));
  olcName->insertItem("OLC-" + i18n("CZ/SK"));
  olcName->insertItem("OLC-" + i18n("France"));
  olcName->insertItem("OLC-" + i18n("Germany"));
  olcName->insertItem("OLC-" + i18n("Italy"));
  // to be continued ...

  QPushButton* okButton = new QPushButton(i18n("Send"), this);
  QPushButton* saveButton = new QPushButton(i18n("Save as"), this);
  QPushButton* cancelButton = new QPushButton(i18n("Cancel"), this);

  QFrame* midFrame = new QFrame(this, "midFrame");

  mainLayout->addMultiCellWidget(midFrame, 2, 2, 0, 6);
  mainLayout->addWidget(olcName, 4, 0);
  mainLayout->addWidget(okButton, 4, 2);
  mainLayout->addWidget(saveButton, 4, 4);
  mainLayout->addWidget(cancelButton, 4, 6);

  mainLayout->setColStretch(1, 1);
  mainLayout->addColSpacing(3, 5);
  mainLayout->addColSpacing(5, 5);

  mainLayout->addRowSpacing(1, 10);
  mainLayout->setRowStretch(2, 1);
  mainLayout->addRowSpacing(3, 10);

  QGridLayout* dlgLayout = new QGridLayout(midFrame, 13, 14);

  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Pilot"), midFrame),
      0, 5, 0, 14);
  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Glider"), midFrame),
      7, 13, 0, 14);
  dlgLayout->addMultiCellWidget(new QGroupBox(i18n("Task"), midFrame),
      15, 21, 0, 14);

  preName = new KLineEdit(midFrame);
  surName = new KLineEdit(midFrame);
  birthday = new KRestrictedLine(midFrame, "birthday", "0123456789.-");

  QPushButton* pilotB = new QPushButton(midFrame);
  pilotB->setPixmap(BarIcon("find"));
  pilotB->setMaximumWidth(pilotB->sizeHint().width() + 10);
  pilotB->setMaximumHeight(pilotB->sizeHint().height() + 10);

  QPushButton* gliderB = new QPushButton(midFrame);
  gliderB->setPixmap(BarIcon("find"));
  gliderB->setMaximumWidth(pilotB->sizeHint().width() + 10);
  gliderB->setMaximumHeight(pilotB->sizeHint().height() + 10);

  gliderType = new KLineEdit(midFrame);
  gliderID = new KLineEdit(midFrame);
  daec = new KRestrictedLine(midFrame, "daec", "0123456789");

  pureGlider = new QCheckBox(i18n("pure glider"), midFrame);
  classSelect = new KComboBox(midFrame);
  classSelect->setEditable(false);
  classSelect->insertItem(i18n("not set"));
  classSelect->insertItem(i18n("Club"));
  classSelect->insertItem(i18n("Standard"));
  classSelect->insertItem(i18n("15 Meter"));
  classSelect->insertItem(i18n("18 Meter"));
  classSelect->insertItem(i18n("double sitter"));
  classSelect->insertItem(i18n("open"));

  startPoint = new KLineEdit(midFrame);
  startPos = new QLabel(midFrame);
  startPos->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  startPos->setBackgroundMode( PaletteLight );

  taskList = new KListView(midFrame);
  taskColID = taskList->addColumn(i18n("ID"));
  taskColWP = taskList->addColumn(i18n("Waypoint"));
  taskColLat = taskList->addColumn(i18n("Latitude"));
  taskColLon = taskList->addColumn(i18n("Longitude"));
  taskColDist = taskList->addColumn(i18n("Distance"));
  taskColTime = taskList->addColumn(i18n("Time"));

  routeLength = new QLabel(midFrame);
  routeLength->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  routeLength->setBackgroundMode( PaletteLight );

  routePoints = new QLabel(midFrame);
  routePoints->setFrameStyle( QFrame::Panel | QFrame::Sunken );
  routePoints->setBackgroundMode( PaletteLight );

  dlgLayout->addWidget(new QLabel(i18n("Prename"), midFrame), 1, 1);
  dlgLayout->addWidget(new QLabel(i18n("Surname"), midFrame), 1, 5);
  dlgLayout->addWidget(new QLabel(i18n("Birthday"), midFrame), 1, 9);
  dlgLayout->addWidget(preName, 1, 3);
  dlgLayout->addWidget(surName, 1, 7);
  dlgLayout->addWidget(birthday, 1, 11);
  dlgLayout->addWidget(pilotB, 1, 13, Qt::AlignLeft);

  dlgLayout->addWidget(new QLabel(i18n("Glidertype"), midFrame), 8, 1);
  dlgLayout->addWidget(new QLabel(i18n("Glider-ID"), midFrame), 8, 5);
  // Was machen wir mit diesem Eintrag ???
  dlgLayout->addWidget(new QLabel(i18n("DAEC-Index"), midFrame), 8, 9);
  dlgLayout->addWidget(gliderType, 8, 3);
  dlgLayout->addWidget(gliderID, 8, 7);
  dlgLayout->addWidget(daec, 8, 11);
  dlgLayout->addWidget(gliderB, 8, 13, Qt::AlignLeft);
  dlgLayout->addWidget(new QLabel(i18n("Class"), midFrame), 10, 1);
  dlgLayout->addMultiCellWidget(classSelect, 10, 10, 3, 5);
  dlgLayout->addMultiCellWidget(pureGlider, 10, 10, 9, 11);

  dlgLayout->addWidget(new QLabel(i18n("Startpoint"), midFrame), 16, 1);
  dlgLayout->addMultiCellWidget(startPoint, 16, 16, 3, 5);
  dlgLayout->addMultiCellWidget(startPos, 16, 16, 7, 13);
  dlgLayout->addMultiCellWidget(taskList, 18, 18, 1, 13);
  dlgLayout->addMultiCellWidget(new QLabel(i18n("Task-Length"), midFrame),
      20, 20, 3, 5, Qt::AlignRight);
  dlgLayout->addWidget(routeLength, 20, 7);
  dlgLayout->addWidget(new QLabel(i18n("Points"), midFrame), 20, 9, Qt::AlignRight);
  dlgLayout->addMultiCellWidget(routePoints, 20, 20, 11, 13);

  dlgLayout->addColSpacing(0, 10);
  dlgLayout->addColSpacing(2, 5);
  dlgLayout->addColSpacing(4, 5);
  dlgLayout->addColSpacing(6, 5);
  dlgLayout->addColSpacing(8, 5);
  dlgLayout->addColSpacing(10, 5);
  dlgLayout->addColSpacing(12, 5);
  dlgLayout->addColSpacing(14, 10);

  dlgLayout->setColStretch(1, 1);
  dlgLayout->setColStretch(3, 1);
  dlgLayout->setColStretch(5, 0);

  dlgLayout->addRowSpacing(0, 20);
  dlgLayout->addRowSpacing(2, 5);
  dlgLayout->addRowSpacing(4, 5);
  dlgLayout->addRowSpacing(5, 8);

  dlgLayout->addRowSpacing(6, 5);

  dlgLayout->addRowSpacing(7, 20);
  dlgLayout->addRowSpacing(9, 5);
  dlgLayout->addRowSpacing(11, 5);
  dlgLayout->addRowSpacing(13, 8);

  dlgLayout->addRowSpacing(14, 5);

  dlgLayout->addRowSpacing(15, 20);
  dlgLayout->addRowSpacing(17, 5);
  dlgLayout->addRowSpacing(19, 5);
  dlgLayout->addRowSpacing(21, 8);

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okButton, SIGNAL(clicked()), this, SLOT(slotSend()));

  __fillDataFields();
}

OLCDialog::~OLCDialog()
{

}

void OLCDialog::__fillDataFields()
{
  gliderID->setText(currentFlight->getID());
  gliderType->setText(currentFlight->getType());
  routePoints->setText(currentFlight->getPoints());
  routeLength->setText(currentFlight->getTaskDistance());

  QList<Waypoint> wpList = currentFlight->getWPList();

  startPoint->setText(wpList.at(0)->name);
  startPos->setText(printPos(wpList.at(0)->origP.lat(), true) + " / " +
    printPos(wpList.at(0)->origP.lon(), false));

  QString temp;
  QListViewItem* item;
  for(unsigned int loop = 1; loop < wpList.count(); loop++)
    {
      temp.sprintf("%d", loop);

      item = new QListViewItem(taskList);
      item->setText(taskColID, temp);
      item->setText(taskColWP, wpList.at(loop)->name);
      item->setText(taskColLat, printPos(wpList.at(loop)->origP.lat()));
      item->setText(taskColLon, printPos(wpList.at(loop)->origP.lon(), false));
      temp.sprintf("%.2f km", wpList.at(loop)->distance);
      item->setText(taskColDist, temp);

      unsigned int time;
      if(wpList.at(loop)->sectorFAI != 0)
        {
          time = wpList.at(loop)->sectorFAI;
        }
      else if(wpList.at(loop)->sector1 != 0)
        {
          time = wpList.at(loop)->sector1;
        }
      else if(wpList.at(loop)->sector2 != 0)
        {
          time = wpList.at(loop)->sector2;
        }
      else
        {
          // should never happen ...
          time = 0;
        }

      item->setText(taskColTime, printTime(time));
    }

  switch(currentFlight->getCompetitionClass())
    {
      case Flight::PW5:
        classSelect->setCurrentItem(1);
        break;
      case Flight::Club:
        classSelect->setCurrentItem(2);
        break;
      case Flight::Standard:
        classSelect->setCurrentItem(3);
        break;
      case Flight::FifteenMeter:
        classSelect->setCurrentItem(4);
        break;
      case Flight::EightteenMeter:
        classSelect->setCurrentItem(5);
        break;
      case Flight::DoubleSitter:
        classSelect->setCurrentItem(6);
        break;
      case Flight::OpenClass:
        classSelect->setCurrentItem(7);
        break;
      case Flight::HGFlexWing:
        classSelect->setCurrentItem(1);
        break;
      case Flight::HGRigidWing:
        classSelect->setCurrentItem(2);
        break;
      case Flight::ParaGlider:
        classSelect->setCurrentItem(10);
        break;
      case Flight::ParaOpen:
        classSelect->setCurrentItem(3);
        break;
      case Flight::ParaSport:
        classSelect->setCurrentItem(4);
        break;
      case Flight::ParaTandem:
        classSelect->setCurrentItem(5);
        break;
    }
}

void OLCDialog::slotSend()
{
  FlightTask t = currentFlight->getTask();

  KConfig* config = KGlobal::config();

  config->setGroup("Personal Data");

  QString link;

  // ungeklärte Felder:
  QString index("100");     // Sollte sowieso mal in eine Config-Datei ...
  QString glider("0");
  if(pureGlider->isChecked()) {  glider = "1";  }    // "1" for pure glider

  QString dateString;
  dateString.sprintf("%d", currentFlight->getDate().year() +
    currentFlight->getDate().dayOfYear());

  QString compClass("0");
  switch(currentFlight->getCompetitionClass())
    {
      case Flight::PW5:
        compClass = "1";
        break;
      case Flight::Club:
        compClass = "2";
        break;
      case Flight::Standard:
        compClass = "3";
        break;
      case Flight::FifteenMeter:
        compClass = "4";
        break;
      case Flight::EightteenMeter:
        compClass = "5";
        break;
      case Flight::DoubleSitter:
        compClass = "6";
        break;
      case Flight::OpenClass:
        compClass = "7";
        break;
      case Flight::HGFlexWing:
        compClass = "1";
        break;
      case Flight::HGRigidWing:
        compClass = "2";
        break;
      case Flight::ParaGlider:
        compClass = "10";
        break;
      case Flight::ParaOpen:
        compClass = "3";
        break;
      case Flight::ParaSport:
        compClass = "4";
        break;
      case Flight::ParaTandem:
        compClass = "5";
        break;
    }

  // personal info
  link = "OLCvnolc=" + config->readEntry("PreName", "")
      + "&na=" + config->readEntry("SurName", "")
      + "&geb=" + config->readEntry("Birthday", "");

  // glider info
  link = link + "&gty=" + currentFlight->getHeader().at(2)
      + "&gid=" + currentFlight->getHeader().at(1)
      + "&ind=" + index
      + "&klasse=" + compClass + "&flugzeug=" + glider;

  // The olc need an "offical" filename. So we have to create it here ...
  link += "&igcfn=" + currentFlight->getFileName() +
          "&sta=" + t.getWPList().first()->name +
          "&ft=" + dateString +
          "&s0=" + printTime(currentFlight->getStartTime(), true);

  QString latH("N"), latG, latM, latMD;
  QString lonH("E"), lonG, lonM, lonMD;

  int latitude, longitude;
  int degree, min, min_deg;

  // the beginning of the task should allways be the second point ...
  POS_STRINGS(t.getWPList().at(1)->origP)

  // Abflugpunkt
  link += "&w0bh=" + latH + "&w0bg=" + latG + "&w0bm=" + latM + "&w0bmd=" + latMD
      + "&w0lh=" + lonH + "&w0lg=" + lonG + "&w0lm=" + lonM + "&w0lmd=" + lonMD;

  if(t.getTaskType() == FlightTask::FAI || t.getTaskType() == FlightTask::Dreieck ||
      t.getTaskType() == FlightTask::FAI_S || t.getTaskType() == FlightTask::Dreieck_S)
    {
      // we have a triangle ...
      POS_STRINGS(t.getWPList().at(2)->origP)

      link += "&w1bh=" + latH + "&w1bg=" + latG + "&w1bm=" + latM + "&w1bmd=" + latMD
          + "&w1lh=" + lonH + "&w1lg=" + lonG + "&w1lm=" + lonM + "&w1lmd=" + lonMD;

      POS_STRINGS(t.getWPList().at(3)->origP)
      link += "&w2bh=" + latH + "&w2bg=" + latG + "&w2bm=" + latM + "&w2bmd=" + latMD
          + "&w2lh=" + lonH + "&w2lg=" + lonG + "&w2lm=" + lonM + "&w2lmd=" + lonMD;

      POS_STRINGS(t.getWPList().at(4)->origP)
      link += "&w3bh=" + latH + "&w3bg=" + latG + "&w3bm=" + latM + "&w3bmd=" + latMD
          + "&w3lh=" + lonH + "&w3lg=" + lonG + "&w3lm=" + lonM + "&w3lmd=" + lonMD;
    }

  // Endpunkt
  POS_STRINGS(t.getWPList().at(t.getWPList().count() - 2)->origP)
  link += "&w4bh=" + latH + "&w4bg=" + latG + "&w4bm=" + latM + "&w4bmd=" + latMD
      + "&w4lh=" + lonH + "&w4lg=" + lonG + "&w4lm=" + lonM + "&w4lmd=" + lonMD;

  link += "&s4=" + printTime(currentFlight->getLandTime(), true);

  QFile igcFile(currentFlight->getFileName());

//  if(!igcFile.open(IO_ReadOnly))
//    {
//      KMessageBox::error(0,
//          i18n("You don't have permission to access file<BR><B>%1</B>").arg(igcFile.name()),
//          i18n("No permission"));
//      return;
//    }

//  QTextStream igcStream(&igcFile);
  QString igcString;

//  while(!igcStream.eof())
//    {
//      igcString += igcStream.readLine();
//    }

  // IGC File
  link = link + "&software=" + "kflog-" + VERSION + "&IGCigcIGC=" + igcString;

  // Link für Hängegleiter:
  //   http://www.segelflugszene.de/olc-cgi/holc-d/olc
  // Link für Segelflüge:
  //   http://www.segelflugszene.de/olc-cgi/olc-d/olc
  // Ausserdem muss das Land noch konfigurierbar sein.
  link = "http://www.segelflugszene.de/olc-cgi/olc-d/olc?" + link;

  warning(link);

//    KProcess browser;

  // Because "%" is used as a placeholder in a string, we have to add it this way ...
  char prozent = 0x25;
  QString spaceString = QString(QChar(prozent)) + "20";
  link.replace(QRegExp("[ ]"), spaceString);

//  browser.clearArguments();
//  browser << "konqueror" << link;
//  browser.start();

}
