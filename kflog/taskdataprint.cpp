/***********************************************************************
**
**   taskdataprint.cpp
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

#include "taskdataprint.h"

#include <flighttask.h>

#include <config.h>
#include <mapcalc.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kprinter.h>

// p_w and p_h given in mm
#define CALC_FORMAT(p_w, p_h) \
  width = (int)(( p_w / 25.4 ) * 72.0); \
  height = (int)(( p_h / 25.4 ) * 72.0);

#define PRINT_POSITION(yPos) \
  painter.drawText(50, yPos, cPoint->name); \
  painter.drawText(125, yPos, printPos(cPoint->origP.lat(), true)); \
  painter.drawText(190, yPos, "/"); \
  painter.drawText(200, yPos, printPos(cPoint->origP.lon(), false));

#define PRINT_POSITION_TEXT(yPos, text) \
  painter.drawText(50, yPos, text); \
  painter.drawText(125, yPos, printPos(cPoint->origP.lat(), true)); \
  painter.drawText(190, yPos, "/"); \
  painter.drawText(200, yPos, printPos(cPoint->origP.lon(), false));

TaskDataPrint::TaskDataPrint(FlightTask* task)
{
  KPrinter printer;

  if(!printer.setup())  return;

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

  QString temp;
  wayPoint* cPoint;

  QPainter painter(&printer);

  painter.setFont(QFont("helvetica", 18, QFont::Bold));
  painter.drawText(50, 50, i18n("Flightplanning") + ":");
  painter.setPen(QPen(QColor(0, 0, 0), 2));
  painter.drawLine(50, 56, 545, 56);

  painter.setFont(QFont("helvetica", 10));
  painter.drawText(50, 100, i18n("Task-Type") + ":");
  painter.drawText(125, 100, task->getTastTypeString());
  painter.drawText(50, 115, i18n("total Distance") + ":");
  painter.drawText(125, 115, task->getTotalDistanceString());
  painter.drawText(50, 130, i18n("Task-Distance") + ":");
  painter.drawText(125, 130, task->getTaskDistanceString());

  painter.setPen(QPen(QColor(0,0,0), 1));
  painter.drawLine(50, 175, 545, 175);

  painter.setFont(QFont("helvetica", 12, QFont::Bold));
  painter.drawText(50, 170, i18n("Task") + ":");

  painter.setFont(QFont("helvetica", 10));

  int yPos = 210;
  for(unsigned int loop = 0; loop < task->getWPList().count(); loop++)
    {
      cPoint = task->getWPList().at(loop);
      PRINT_POSITION(yPos);
      yPos += 13;
    }
}

TaskDataPrint::~TaskDataPrint()
{

}
