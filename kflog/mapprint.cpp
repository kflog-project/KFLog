/***********************************************************************
**
**   mapprint.cpp
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

#include "mapprint.h"

#include <config.h>
#include <flight.h>
#include <mapcontents.h>
#include <mapmatrix.h>
//#include <mapprintdialogpage.h>

#include <klocale.h>
#include <kprinter.h>

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>

#define TOP_LEFT_X   ( ( 0 + leftMargin ) * 2 )
#define TOP_LEFT_Y   ( ( 0 + topMargin ) * 2 )
#define TOP_RIGHT_X  ( ( pS.width() - rightMargin ) * 2 )
#define TOP_RIGHT_Y  ( ( 0 + topMargin ) * 2 )
#define BOT_LEFT_X   ( ( 0 + leftMargin ) * 2 )
#define BOT_LEFT_Y   ( ( pS.height() - bottomMargin ) * 2 )
#define BOT_RIGHT_X  ( ( pS.width() - rightMargin ) * 2 )
#define BOT_RIGHT_Y  ( ( pS.height() - bottomMargin ) * 2 )

#define GRID_LEFT_X  ( TOP_LEFT_X + 30 )
#define GRID_RIGHT_X ( TOP_RIGHT_X - 30 )
#define GRID_TOP_Y   ( TOP_LEFT_Y + 30 )
#define GRID_BOT_Y   ( BOT_LEFT_Y - 95 )

// p_w and p_h given in mm
#define CALC_FORMAT(p_w, p_h) \
  width = (int)(( p_w / 25.4 ) * 72.0); \
  height = (int)(( p_h / 25.4 ) * 72.0);

MapPrintDialogPage::MapPrintDialogPage(QStringList sList, QWidget *parent,
    const char *name, bool printFlight)
  : KPrintDialogPage(parent,name),
    scaleList(sList)
{
  setTitle(i18n("Map"));

  QGroupBox* scaleBox = new QGroupBox(i18n("Map print"), this);

  scaleSelect = new QComboBox(this);
  scaleSelect->insertStringList(scaleList);

  printTitle = new QCheckBox(i18n("Print Pagetitle"), this);
  titleInput = new QLineEdit(this);

  connect(printTitle, SIGNAL(toggled(bool)), titleInput,
      SLOT(setEnabled(bool)));

  printLegend = new QCheckBox(i18n("Print Legend"), this);
  printText = new QCheckBox(i18n("Print Text"), this);
  printText->setChecked(true);

  if(printFlight)
    {
      // Hier müssen noch Infos über den Flug hin!!!
      titleInput->setText(i18n("Flight Track") + ":");
    }

  QGridLayout* pageLayout = new QGridLayout(this, 5, 7);
  pageLayout->addMultiCellWidget(scaleBox, 0, 6, 0, 4);
  pageLayout->addWidget(new QLabel(i18n("Map scale") + ":", this), 1, 1);
  pageLayout->addWidget(scaleSelect, 1, 3);
  pageLayout->addWidget(printTitle, 3, 1);
  pageLayout->addWidget(titleInput, 3, 3);
  pageLayout->addWidget(printLegend, 5, 1);
  pageLayout->addWidget(printText, 5, 3);

  pageLayout->addColSpacing(0, 10);
  pageLayout->addColSpacing(2, 5);
  pageLayout->setColStretch(3, 1);
  pageLayout->addColSpacing(4, 10);

  pageLayout->addRowSpacing(0, 25);
  pageLayout->addRowSpacing(2, 15);
  pageLayout->addRowSpacing(4, 15);
  pageLayout->setRowStretch(6, 2);
  pageLayout->addRowSpacing(6, 10);

  // Unused widgets disabled:

  printTitle->setEnabled(false);
  printLegend->setEnabled(false);
}

MapPrintDialogPage::~MapPrintDialogPage()
{

}

void MapPrintDialogPage::getOptions(QMap<QString,QString>& opts, bool incldef)
{
  QString temp;

  opts["kde-kflog-printscale"] = scaleSelect->currentText();

  opts["kde-kflog-printtitle"] = temp.sprintf("%d",
      printTitle->isChecked());

  opts["kde-kflog-printtext"] = temp.sprintf("%d",
      printText->isChecked());

  opts["kde-kflog-printlegend"] = temp.sprintf("%d",
      printLegend->isChecked());
}

void MapPrintDialogPage::setOptions( const QMap<QString,QString>& opts )
{
  if(opts["kde-kflog-printscale"] == NULL)
      scaleSelect->setCurrentItem(0);
  else
      scaleSelect->setCurrentItem(scaleList.findIndex(
          opts["kde-kflog-printscale"]));

  if(opts["kde-kflog-printtitle"] == NULL)
    {
      printTitle->setChecked(false);
      titleInput->setEnabled(false);
    }
  else
    {
      printTitle->setChecked((opts["kde-kflog-printtitle"]).toInt());
      titleInput->setEnabled((opts["kde-kflog-printtitle"]).toInt());
    }

  if(opts["kde-kflog-printlegend"] == NULL)
      printLegend->setChecked(false);
  else
      printLegend->setChecked((opts["kde-kflog-printlegend"]).toInt());

  if(opts["kde-kflog-printtext"] == NULL)
      printText->setChecked(false);
  else
      printText->setChecked((opts["kde-kflog-printtext"]).toInt());
}

////////////////////////////////////////////////////////////////////////////
//
// MapPrint
//
////////////////////////////////////////////////////////////////////////////

MapPrint::MapPrint(bool flightLoaded)
{
  QStringList scaleList;

  scaleList.append("1:1.000.000");
  scaleList.append("1:500.000");
  scaleList.append("1:200.000");
  scaleList.append("1:100.000");
  scaleList.append("1:50.000");
  scaleList.append("1:25.000");

  if(flightLoaded)
    {
      scaleList.append(i18n("Center to Flight"));
      scaleList.append(i18n("Center to Task"));
    }

  dialogPage = new MapPrintDialogPage(scaleList, 0, "MapPrintDialogPage", false);

  KPrinter printer;
  printer.addDialogPage(dialogPage);

  if(!printer.setup())  return;

  scaleRange = new double[6];
  scaleRange[0] = 1000.0 / 72 * 25.4;          /* 1:1.000.000 */
  scaleRange[1] =  500.0 / 72 * 25.4;          /* 1:500.000   */
  scaleRange[2] =  200.0 / 72 * 25.4;          /* 1:200.000   */
  scaleRange[3] =  100.0 / 72 * 25.4;          /* 1:100.000   */
  scaleRange[4] =   50.0 / 72 * 25.4;          /* 1:50.000    */
  scaleRange[5] =   25.0 / 72 * 25.4;          /* 1:25.000    */

  QMap<QString,QString> opts = printer.options();

  printer.setDocName("kflog-map.ps");
  printer.setCreator((QString)"KFLog " + VERSION);

  // We have to set the real page size. KPrinter knows the
  // pageformat, but reports a wrong pagesize ...
  int width = 0, height = 0;

  switch (printer.pageSize())
    {
      case KPrinter::A0: // (841 x 1189 mm)
        CALC_FORMAT(841, 1189)
        break;
      case KPrinter::A1: // (594 x 841 mm)
        CALC_FORMAT(594, 841)
        break;
      case KPrinter::A2: // (420 x 594 mm)
        CALC_FORMAT(420, 594)
        break;
      case KPrinter::A3: // (297 x 420 mm)
        CALC_FORMAT(297, 420)
        break;
      case KPrinter::A4: // (210x297 mm, 8.26x11.7 inches)
        CALC_FORMAT(210, 297)
        break;
      case KPrinter::A5: // (148 x 210 mm)
        CALC_FORMAT(148, 210)
        break;
      case KPrinter::A6: // (105 x 148 mm)
        CALC_FORMAT(105, 148)
        break;
      case KPrinter::A7: // (74 x 105 mm)
        CALC_FORMAT(74, 105)
        break;
      case KPrinter::A8: // (52 x 74 mm)
        CALC_FORMAT(52, 74)
        break;
      case KPrinter::A9: // (37 x 52 mm)
        CALC_FORMAT(37, 52)
        break;
      case KPrinter::B0: // (1030 x 1456 mm)
        CALC_FORMAT(1030, 1456)
        break;
      case KPrinter::B1: // (728 x 1030 mm)
        CALC_FORMAT(728, 1030)
        break;
      case KPrinter::B10: // (32 x 45 mm)
        CALC_FORMAT(32, 45)
        break;
      case KPrinter::B2: // (515 x 728 mm)
        CALC_FORMAT(515, 728)
        break;
      case KPrinter::B3: // (364 x 515 mm)
        CALC_FORMAT(364, 515)
        break;
      case KPrinter::B4: // (257 x 364 mm)
        CALC_FORMAT(257, 364)
        break;
      case KPrinter::B5: // (182 x 257 mm, 7.17x10.13 inches)
        CALC_FORMAT(182, 257)
        break;
      case KPrinter::B6: // (128 x 182 mm)
        CALC_FORMAT(128, 182)
        break;
      case KPrinter::B7: // (91 x 128 mm)
        CALC_FORMAT(91, 128)
        break;
      case KPrinter::B8: // (64 x 91 mm)
        CALC_FORMAT(64, 91)
        break;
      case KPrinter::B9: // (45 x 64 mm)
        CALC_FORMAT(45, 64)
        break;
      case KPrinter::C5E: // (163 x 229 mm)
        CALC_FORMAT(163, 229)
        break;
      case KPrinter::Comm10E: // (105 x 241 mm, US Common #10 Envelope)
        CALC_FORMAT(105, 241)
        break;
      case KPrinter::DLE: // (110 x 220 mm)
        CALC_FORMAT(110, 220)
        break;
      case KPrinter::Executive: // (7.5x10 inches, 191x254 mm)
        CALC_FORMAT(191, 254)
        break;
      case KPrinter::Folio: // (210 x 330 mm)
        CALC_FORMAT(210, 330)
        break;
      case KPrinter::Ledger: // (432 x 279 mm)
        CALC_FORMAT(432, 279)
        break;
      case KPrinter::Legal: // (8.5x14 inches, 216x356 mm)
        CALC_FORMAT(216, 356)
        break;
      case KPrinter::Letter: // (8.5x11 inches, 216x279 mm)
        CALC_FORMAT(216, 279)
        break;
      case KPrinter::Tabloid: // (279 x 432 mm)
        CALC_FORMAT(279, 432)
        break;
      case KPrinter::NPageSize: // "Custom"
      default:
        // Until we find a better solution, fallback is DIN-A4 ...
        CALC_FORMAT(210, 297)
        break;
    }

  QSize pS;

  if(printer.orientation() == KPrinter::Portrait)
       pS = QSize(width, height);
  else
       pS = QSize(height, width);

  printer.setRealPageSize(pS);
  printer.setFullPage(true);

  // Okay, now lets start creating the printout ...
  extern MapMatrix _globalMapMatrix;

  extern MapContents _globalMapContents;

  // Store the current center and scale:
  const QPoint cMapCenter = _globalMapMatrix.getMapCenter();
  const double cMapScale = _globalMapMatrix.getScale(MapMatrix::CurrentScale);

  QPainter printPainter(&printer);

  const QWMatrix oldMatrix = printPainter.worldMatrix();
  QWMatrix newMatrix = printPainter.worldMatrix();

  double selectedScale;
  QPoint mapCenter;

  if(scaleList.findIndex(opts["kde-kflog-printscale"]) == 6)
      selectedScale = _globalMapMatrix.centerToRect(
          ((Flight *)_globalMapContents.getFlight())->getFlightRect(), pS - QSize(100,163));
  else if(scaleList.findIndex(opts["kde-kflog-printscale"]) == 7)
      selectedScale = _globalMapMatrix.centerToRect(
          ((Flight *)_globalMapContents.getFlight())->getTaskRect(), pS - QSize(100,163));
  else
      selectedScale = scaleRange[scaleList.findIndex(
          opts["kde-kflog-printscale"])];

  _globalMapMatrix.createPrintMatrix(selectedScale, pS);
  mapCenter = _globalMapMatrix.getMapCenter();

  int leftMargin = 35, rightMargin = 35, topMargin = 35, bottomMargin = 35;

  QPoint delta(_globalMapMatrix.print(mapCenter.x(), mapCenter.y(), 0, 0));

  double dX = ( leftMargin + ( pS.width() - leftMargin - rightMargin ) / 2 )
        - delta.x();
  double dY = ( topMargin + ( pS.height() - topMargin - bottomMargin ) / 2 )
        - delta.y();

  const QRect mapBorder = _globalMapMatrix.getPrintBorder(topMargin - dY, 0,
      BOT_LEFT_Y / 2 - dY, leftMargin - dX, topMargin - dY,
      pS.width() - rightMargin - dX, topMargin - dY, leftMargin - dX);

  selectedScale /= 2.0;
  dX *= 2;
  dY *= 2;

  newMatrix.scale(0.5, 0.5);

  printPainter.setWorldMatrix(newMatrix);

  printPainter.setClipRect(leftMargin, topMargin,
      pS.width() - leftMargin - rightMargin,
      pS.height() - topMargin - bottomMargin - 33);

  __drawGrid(selectedScale, &printPainter, pS, mapBorder, mapCenter.y(),
      dX, dY, TOP_LEFT_X + 30, TOP_RIGHT_X - 30, TOP_LEFT_Y + 30,
      BOT_LEFT_Y - 95);

  // draw waypoints
  __drawWaypoints(selectedScale, &printPainter, pS, mapBorder, mapCenter.y(),
      dX, dY, TOP_LEFT_X + 30, TOP_RIGHT_X - 30, TOP_LEFT_Y + 30,
      BOT_LEFT_Y - 95);

  // draw task
  __drawTask(selectedScale, &printPainter, pS, mapBorder, mapCenter.y(),
      dX, dY, TOP_LEFT_X + 30, TOP_RIGHT_X - 30, TOP_LEFT_Y + 30,
      BOT_LEFT_Y - 95);

  printPainter.setClipRect(leftMargin + 15, topMargin + 15,
      pS.width() - leftMargin - rightMargin - 30,
      pS.height() - topMargin - bottomMargin - 63);

  // Workaround. It moves the map slightly upwards ...
  if(scaleList.findIndex(opts["kde-kflog-printscale"]) == 6 ||
          scaleList.findIndex(opts["kde-kflog-printscale"]) == 7)
      dY -= 32;

  _globalMapMatrix.createPrintMatrix(selectedScale, pS * 2, dX, dY);

  _globalMapContents.printContents(&printPainter,
      ((QString)opts["kde-kflog-printtext"]).toInt());

  printPainter.setClipRect(0, 0, pS.width(), pS.height());

  QString scaleText;
  bool show1 = false;
  bool show5 = false;
  unsigned int stop10, stop1 = 10, stop_small10 = 0, stop_small1 = 10;

  switch(scaleList.findIndex(opts["kde-kflog-printscale"]))
    {
      case 0:
        scaleText = "1:1.000.000";
        stop10 = 10;
        stop_small10 = 5;
        stop1 = 0;
        break;
      case 1:
        scaleText = "1:500.000";
        stop10 = 5;
        stop_small10 = 3;
        break;
      case 2:
        scaleText = "1:200.000";
        stop10 = 2;
        stop_small10 = 1;
        show5 = true;
        break;
      case 3:
        scaleText = "1:100.000";
        stop10 = 1;
        stop_small10 = 0;
        show5 = true;
        break;
      case 4:
        scaleText = "1:50.000";
        stop10 = 0;
        stop1 = 5;
        stop_small1 = 3;
        show1 = true;
        break;
      case 5:
        scaleText = "1:25.000";
        stop10 = 0;
        stop1 = 2;
        stop_small1 = 1;
        show1 = true;
        break;
      default:
        /* Hier muß jetzt zunächst der verwendete Maßstab berechnet und dann
         * entschieden werden, welche Art der Balken-Darstellung verwendet wird.
         */
        int tempScale = (int)(selectedScale / 25.4 * 72 * 2);

        scaleText.sprintf("1:%d.000", tempScale);

        if(tempScale >= 1000)          // >= 1:1.000.000
          {
            scaleText.sprintf("1:%d.%3d.000",
                (int)(tempScale / 1000.0), tempScale - 1000);
            stop10 = 10;
            stop_small10 = 5;
            stop1 = 0;
          }
        else if(tempScale >= 500)      // >= 1:500.000
          {
            stop10 = 5;
            stop_small10 = 3;
          }
        else if(tempScale >= 200)      // >= 1:200.000
          {
            stop10 = 2;
            stop_small10 = 1;
            show5 = true;
          }
        else if(tempScale >= 100)      // >= 1:100.000
          {
            stop10 = 1;
            stop_small10 = 0;
            show5 = true;
          }
        else if(tempScale >= 50)      // >= 1:50.000
          {
            stop10 = 0;
            stop1 = 5;
            stop_small1 = 3;
            show1 = true;
          }
        else                           // < 1:50.000
          {
            stop10 = 0;
            stop1 = 2;
            stop_small1 = 1;
            show1 = true;
          }
    }

  // 370 Punkte =^ ca. 13 cm
  if(pS.width() - leftMargin - rightMargin < 370)
    {
      stop10 = stop_small10;
      stop1 = stop_small1;
    }

  int scaleX = BOT_LEFT_X + 15;
  int scaleY = BOT_LEFT_Y - 25;

  int scale1Y = scaleY - 5;
  int scale5Y = scaleY - 8;
  int scale10Y = scaleY - 10;

  /*
   * Rahmen der Karte und der Legende.
   */
  printPainter.setPen(QPen(QColor(0, 0, 0), 2));
  printPainter.setBrush(Qt::NoBrush);
  printPainter.drawRect(TOP_LEFT_X, TOP_LEFT_Y,
      BOT_RIGHT_X - TOP_LEFT_X, BOT_RIGHT_Y - TOP_LEFT_Y);
  printPainter.drawLine(BOT_LEFT_X, BOT_LEFT_Y - 65,
      BOT_RIGHT_X, BOT_RIGHT_Y - 65);
  printPainter.setPen(QPen(QColor(0, 0, 0), 1));
  printPainter.drawRect(GRID_LEFT_X, GRID_TOP_Y,
      BOT_RIGHT_X - TOP_LEFT_X - 60, BOT_RIGHT_Y - TOP_LEFT_Y - 125);

  /*
   * Wenn die Boxen hier zu klein sind, wird die gesamte Ausgabe zerstört.
   * Anscheinend ein Bug in QPainter :-(
   */
  printPainter.setFont(QFont("helvetica", 20, QFont::Bold));
  printPainter.drawText(BOT_RIGHT_X - 310, BOT_RIGHT_Y - 65, 300, 50,
          Qt::AlignTop | Qt::AlignRight, scaleText);
  printPainter.setFont(QFont("helvetica", 12, QFont::Normal, true));
  printPainter.drawText(BOT_RIGHT_X - 310, BOT_RIGHT_Y - 35, 296, 30,
          Qt::AlignBottom | Qt::AlignRight,
          (QString)"printed by KFLog " + VERSION);

  /*
   * Wenn die druckbare Fläche zu klein ist, wird der Maßstabsbalken nicht
   * angepaßt. Der Balken ist bei den festen Maßstäben immer 8 bzw. 10cm breit!
   */
  printPainter.setPen(QPen(QColor(0, 0, 0), 2));
  printPainter.setFont(QFont("helvetica", 15));

  if(stop10 > 0)
      printPainter.drawLine(scaleX, scaleY,
            scaleX + (10000 * stop10 / selectedScale), scaleY);
  else
      printPainter.drawLine(scaleX, scaleY,
            scaleX + (1000 * stop1 / selectedScale), scaleY);

  printPainter.setPen(QPen(QColor(0, 0, 0), 1));
  printPainter.drawLine(scaleX, scaleY, scaleX, scale10Y);
  printPainter.drawText(scaleX - 4, scale10Y - 5, "0");
  printPainter.drawText(scaleX + 2, scaleY + 15, "[km]");

  /* 5- und 10-km Striche zeichnen */
  for(unsigned int loop = 1; loop <= stop10; loop++)
    {
      printPainter.drawLine(scaleX + (10000 * loop / selectedScale), scaleY,
          scaleX + (10000 * loop / selectedScale), scale10Y);
      scaleText.sprintf("%d", loop * 10);
      printPainter.drawText(scaleX - 9 + (10000 * loop / selectedScale),
              scale10Y - 5, scaleText);
      printPainter.drawLine(scaleX + ((10000 * loop - 5000) / selectedScale),
          scaleY, scaleX + ((10000 * loop - 5000) / selectedScale), scale5Y);
      if(show5)
        {
          int dx = 4;
          if(loop > 1) dx = 9;
          scaleText.sprintf("%d", (loop * 10) - 5);
          printPainter.drawText(scaleX - dx +
                ((10000 * loop - 5000) / selectedScale),
              scale10Y - 5, scaleText);
        }
    }

  /* 1-km Striche zeichnen */
  for(unsigned int loop = 1; loop <= stop1; loop++)
    {
      printPainter.drawLine(scaleX + (1000 * loop / selectedScale), scaleY,
            scaleX + (1000 * loop / selectedScale), scale1Y);
      if(show1)
        {
          scaleText.sprintf("%d", loop);
          printPainter.drawText(scaleX - 4 + (1000 * loop / selectedScale),
                scale10Y - 5, scaleText);
        }
    }

  /* Zurücksetzen der Darstellung   */
  printPainter.setWorldMatrix(oldMatrix);
  printPainter.end();

  // restoring the MapMatrix-values:
  _globalMapMatrix.centerToLatLon(cMapCenter);
  _globalMapMatrix.slotSetScale(cMapScale);
}

MapPrint::~MapPrint()
{

}

void MapPrint::__drawGrid(const double selectedScale, QPainter* gridP,
    const QSize pS, const QRect mapBorder, const int mapCenterLon,
    const double dX, const double dY, const double gridLeft,
    const double gridRight, const double gridTop, const double gridBot)
{
  extern const MapMatrix _globalMapMatrix;

  gridP->setBrush(Qt::NoBrush);

  QString text;
  if(mapCenterLon > 0)
    {
      const int lon1 = mapBorder.left() / 600000 - 1;
      const int lon2 = mapBorder.right() / 600000 + 1;
      const int lat1 = mapBorder.top() / 600000 + 1;
      const int lat2 = mapBorder.bottom() / 600000 - 1;

      /* Abstand zwischen zwei Linien in Minuten
       * Wenn __drawGrid() aufgerufen wird, ist der Maßstab bereits
       * skaliert worden.
       */
      int step = 1;
      if(selectedScale * 2.0 >= scaleRange[0]) step = 30;
      else if(selectedScale * 2.0 >= scaleRange[1]) step = 10;
      else if(selectedScale * 2.0 >= scaleRange[2]) step = 5;
      else if(selectedScale * 2.0 >= scaleRange[3]) step = 2;

      QPoint cP, cP2, cP3, cP4;

      /* Zunächst die Längengrade: */
      for(int loop = lon1; loop <= lon2; loop++)
        {
          cP = _globalMapMatrix.print(mapBorder.top(), (loop * 600000),
              dX, dY);
          cP2 = _globalMapMatrix.print(mapBorder.bottom(), (loop * 600000),
              dX, dY);

          /* Die Hauptlinien */
          gridP->setPen(QPen(QColor(0,0,0), 1));
          gridP->drawLine(cP.x(), cP.y(), cP2.x(), cP2.y());

          // Hier könnte mal 'ne Abfrage hin, ob wir zu nah am Rand sind ...
          gridP->setFont(QFont("helvetica", 14, QFont::Bold));
          text.sprintf("%d°", loop);
          if(cP.x() > gridLeft && cP.x() < gridRight)
              gridP->drawText(cP.x() - 101, gridTop - 28, 100, 50,
                  Qt::AlignTop | Qt::AlignRight, text);
          if(cP2.x() > gridLeft && cP2.x() < gridRight)
              gridP->drawText(cP2.x() - 101, gridBot + 2, 100, 50,
                  Qt::AlignTop | Qt::AlignRight, text);

          /* Die kleineren Linien */
          gridP->setFont(QFont("helvetica", 12));
          int number = (int) (60.0 / step);
          for(int loop2 = 1; loop2 < number; loop2++)
            {
              cP = _globalMapMatrix.print(mapBorder.top(),
                  ((loop + (loop2 * step / 60.0)) * 600000), dX, dY);
              cP2 = _globalMapMatrix.print(mapBorder.bottom(),
                  ((loop + (loop2 * step / 60.0)) * 600000), dX, dY);

              if(loop2 == (number / 2.0))
                  gridP->setPen(QPen(QColor(0,0,0), 1, Qt::DashLine));
              else
                  gridP->setPen(QPen(QColor(0,0,0), 1, Qt::DotLine));

              /* Die Linie wird nur gezeichnet, wenn das obere Ende innerhalb
               * der Breite des Gitternetzes liegt. Das klappt so allerdings
               * nur auf der Nordhalbkugel ...
               */
              if(cP.x() > gridLeft && cP.x() < gridRight)
                {
                  gridP->drawLine(cP.x(), cP.y(), cP2.x(), cP2.y());
                  text.sprintf("%d°", loop);
                  gridP->drawText(cP.x() - 101, gridTop - 27, 100, 50,
                      Qt::AlignTop | Qt::AlignRight, text);
                  text.sprintf("%d'", loop2 * step);
                  gridP->drawText(cP.x() + 3, gridTop - 27, 100, 50,
                      Qt::AlignTop | Qt::AlignLeft, text);
                  if(cP2.x() > gridLeft && cP2.x() < gridRight)
                    {
                      text.sprintf("%d°", loop);
                      gridP->drawText(cP2.x() - 101, gridBot + 3, 100, 50,
                          Qt::AlignTop | Qt::AlignRight, text);
                      text.sprintf("%d'", loop2 * step);
                      gridP->drawText(cP2.x() + 3, gridBot + 3, 100, 50,
                          Qt::AlignTop | Qt::AlignLeft, text);
                    }
                }
            }
        }

      /* Damit keine Längengrade in den Rand ragen, wird links und rechts je
       * ein weißer Strich gezogen. Damit wird eventuell auch der Text des
       * östlichsten Längengrades überdeckt.
       */
      gridP->setPen(QPen(QColor(255, 255, 255)));
      gridP->setBrush(QBrush(QColor(255, 255, 255), Qt::SolidPattern));
      gridP->drawRect(gridLeft - 30, gridTop, 30, pS.height() * 2);
      gridP->drawRect(gridRight, gridTop, 30, pS.height() * 2);

      cP2 = _globalMapMatrix.print(mapBorder.top(), mapBorder.left(),
          dX, dY);
      cP3 = _globalMapMatrix.print(mapBorder.bottom(), mapBorder.right(),
          dX, dY);
      /* Hier könnte es noch passieren, dass die Breitengrade in den Rand
       * hineinragen ...
       */
      for(int loop = 0; loop < (lat1 - lat2 + 1) ; loop++)
        {
          int size = (lon2 - lon1 + 1) * 10;
          QPointArray pointArray(size);

          for(int lonloop = 0; lonloop < size; lonloop++)
            {
              cP = _globalMapMatrix.print((lat2 + loop) * 600000,
                        (lon1 + (lonloop * 0.1)) * 600000, dX, dY);
              pointArray.setPoint(lonloop, cP.x(), cP.y());
              if(cP.x() < gridLeft)
                {
                  // Wir sind noch westlich der Westgrenze der Karte ...
                  cP4.setX(cP2.x());
                  cP4.setY(cP.y());
                }
            }

          if(cP.y() > gridTop && cP.y() < gridBot)
            {
              /* Die Hauptlinien */
              gridP->setPen(QPen(QColor(0,0,0), 1));
              gridP->drawPolyline(pointArray);
              gridP->setFont(QFont("helvetica", 14, QFont::Bold));
              text.sprintf("%d°", lat2 + loop);
              gridP->drawText(gridLeft - 27, cP4.y() - 52, 100, 50,
                  Qt::AlignBottom | Qt::AlignLeft, text);
              gridP->drawText(gridRight + 3, cP4.y() - 52, 100, 50,
                  Qt::AlignBottom | Qt::AlignLeft, text);
            }

          /* Die kleineren Linien */
          gridP->setFont(QFont("helvetica", 12));
          int number = (int) (60.0 / step);
          for(int loop2 = 1; loop2 < number; loop2++)
            {
              QPointArray pointArraySmall(size);

              for(int lonloop = 0; lonloop < size; lonloop++)
                {
                  cP = _globalMapMatrix.print((lat2 + loop +
                      (loop2 * (step / 60.0))) * 600000,
                      (lon1 + (lonloop * 0.1)) * 600000,
                      dX, dY);

                  pointArraySmall.setPoint(lonloop, cP.x(), cP.y());
                  if(cP.x() < gridLeft)
                    {
                      // Wir sind noch westlich der Westgrenze der Karte ...
                      cP4.setX(cP2.x());
                      cP4.setY(cP.y());
                    }
                }

              if(cP.y() > gridTop && cP.y() < gridBot)
                {
                  if(loop2 == (number / 2.0))
                    {
                      gridP->setPen(QPen(QColor(0,0,0), 1, Qt::DashLine));
                      gridP->drawPolyline(pointArraySmall);
                    }
                  else
                    {
                      gridP->setPen(QPen(QColor(0,0,0), 1, Qt::DotLine));
                      gridP->drawPolyline(pointArraySmall);
                    }
                  text.sprintf("%d°", lat2 + loop);
                  gridP->drawText(gridLeft - 27, cP4.y() - 52, 100, 50,
                      Qt::AlignBottom | Qt::AlignLeft, text);
                  gridP->drawText(gridRight + 3, cP4.y() - 52, 100, 50,
                      Qt::AlignBottom | Qt::AlignLeft, text);
                  text.sprintf("%d'", loop2 * step);
                  gridP->drawText(gridLeft - 27, cP4.y() + 2, 100, 50,
                      Qt::AlignTop | Qt::AlignLeft, text);
                  gridP->drawText(gridRight + 3, cP4.y() + 2, 100, 50,
                      Qt::AlignTop | Qt::AlignLeft, text);
                }
            }
        }
    }
}
/** Prints the waypointlist to the supplied QPainter */
void MapPrint::__drawWaypoints(const double selectedScale, QPainter* wpP, const QSize pS, const QRect mapBorder, const int mapCenterLon,     const double dX, const double dY, const double gridLeft,     const double gridRight, const double gridTop, const double gridBot){
  extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  QList<wayPoint> *wpList;
  wayPoint *wp;
  int i,n;
  QPoint p;

  wpP->setBrush(Qt::NoBrush);
  wpP->setPen(QPen(QColor(0,0,0), 1, Qt::SolidLine));

 wpList = _globalMapContents.getWaypointList() ;

  n = wpList->count();
  for (i=0; i < n; i++){
     wp = (wayPoint*)wpList->at(i);
     p = _globalMapMatrix.print(wp->origP.lat(), wp->origP.lon(), dX, dY);
	 // draw marker and name
     wpP->drawRect(p.x() - 4,p.y() - 4, 8, 8);
     wpP->drawText(p.x()+6, p.y(), wp->name, -1);
  }
}

/** Prints the task, if defined, to the supplied QPainter */
void MapPrint::__drawTask(const double selectedScale, QPainter* taskP, const QSize pS, const QRect mapBorder, const int mapCenterLon,     const double dX, const double dY, const double gridLeft,     const double gridRight, const double gridTop, const double gridBot){
 extern const MapMatrix _globalMapMatrix;
  extern MapContents _globalMapContents;
  QList<wayPoint> *wpList;
  wayPoint *wp;
  int i,n;
  QPoint p;
  FlightTask* task;

  task = (FlightTask*)_globalMapContents.getFlight();

  if(task && task->getTypeID() == BaseMapElement::Task) {
    taskP->setBrush(Qt::NoBrush);
    taskP->setPen(QPen(QColor(170,0,0), 5, Qt::SolidLine));
    task->printMapElement( taskP, false, dX, dY);
   }
}
