/***********************************************************************
**
**   airspace.cpp
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "airspace.h"

#include <mapcalc.h>
#include <mapmatrix.h>

#include <kapp.h>
#include <kiconloader.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

Airspace::Airspace(QString n, unsigned int t, QPointArray pA)
  : LineElement(n, t, pA)
{
  switch(typeID)
    {
      case AirC:
        PEN_THICKNESS(3,3,3,3,3,3,3,3,3)
        drawColor = QColor(0,120,0);
        fillBrushStyle = QBrush::NoBrush;
        break;
      case AirCtemp:
        PEN_THICKNESS(3,3,3,3,3,3,3,3,3)
        drawColor = QColor(0,120,0);
        fillColor = QColor(0,200,0);
        fillBrushStyle = QBrush::Dense6Pattern;
        break;
      case AirD:
        PEN_THICKNESS(3,3,3,3,3,3,3,3,3)
        drawPenStyle = QPen::DashLine;
        drawColor = QColor(0,180,0);
        fillBrushStyle = QBrush::NoBrush;
        break;
      case AirDtemp:
        PEN_THICKNESS(3,3,3,3,3,3,3,3,3)
        drawPenStyle = QPen::DashLine;
        drawColor = QColor(0,180,0);
        fillColor = QColor(120,255,120);
        fillBrushStyle = QBrush::FDiagPattern;
        break;
      case AirElow:
        PEN_THICKNESS(3,3,3,3,2,2,2,2,2)
        drawColor = QColor(200,100,100);
        fillBrushStyle = QBrush::NoBrush;
        break;
      case AirEhigh:
        PEN_THICKNESS(3,3,3,3,2,2,2,2,2)
        drawColor = QColor(100,100,175);
        fillBrushStyle = QBrush::NoBrush;
        break;
      case AirF:
        PEN_THICKNESS(2,2,2,2,2,2,2,2,2)
        drawColor = QColor(50,50,125);
        fillColor = QColor(100,100,175);
        fillBrushStyle = QBrush::Dense5Pattern;
        break;
      case ControlD:
        PEN_THICKNESS(2,2,2,2,2,2,2,2,2)
        drawPenStyle = QPen::DashLine;
        drawColor = QColor(0,0,150);
        fillColor = QColor(200,50,50);
        fillBrushStyle = QBrush::Dense5Pattern;
        break;
      case Restricted:
        PEN_THICKNESS(2,2,2,2,2,2,2,2,2)
        drawColor = QColor(50,50,125);
        fillColor = QColor(100,100,175);
        fillBrushStyle = QBrush::BDiagPattern;
        break;
      case Danger:
        PEN_THICKNESS(2,2,2,2,2,2,2,2,2)
        drawColor = QColor(50,50,125);
        fillColor = QColor(100,100,175);
        fillBrushStyle = QBrush::BDiagPattern;
        break;
      case LowFlight:
        PEN_THICKNESS(3,3,3,3,3,3,3,3,3)
        drawPenStyle = QPen::DashLine;
        drawColor = QColor(150,0,0);
        fillColor = QColor(255,150,150);
        fillBrushStyle = QBrush::FDiagPattern;
        break;
    }
}

Airspace::~Airspace()
{

}

unsigned int Airspace::getUpperL() const { return uLimit; }

unsigned int Airspace::getLowerL() const { return lLimit; }

unsigned int Airspace::getUpperT() const { return uLimitType; }

unsigned int Airspace::getLowerT() const { return lLimitType; }

QString Airspace::getInfoString()
{
  QString text, tempL, tempU;

  switch(lLimitType)
    {
      case MSL:
        tempL.sprintf("%d MSL", lLimit);
        break;
      case GND:
        if(lLimit)
            tempL.sprintf("%d GND", lLimit);
        else
            tempL = "GND";
        break;
      case FL:
        tempL.sprintf("FL %d", lLimit);
        break;
      default: ;
    }

  switch(uLimitType)
    {
      case MSL:
        if(uLimit >= 99999)
            tempU = "unlimited";
        else
            tempU.sprintf("%d MSL", uLimit);
        break;
      case GND:
        tempU.sprintf("%d GND", uLimit);
        break;
      case FL:
        tempU.sprintf("FL %d", uLimit);
        break;
      default: ;
    }

  switch(typeID)
    {
      case BaseMapElement::AirC:
      case BaseMapElement::AirCtemp:
        text = "C";
        break;
      case BaseMapElement::AirD:
      case BaseMapElement::AirDtemp:
        text = "D";
        break;
      case BaseMapElement::AirElow:
        text = "E (low)";
        break;
      case BaseMapElement::AirEhigh:
        text = "E (high)";
        break;
      case BaseMapElement::AirF:
        text = "F";
        break;
      case BaseMapElement::Restricted:
        text = "Restricted";
        break;
      case BaseMapElement::Danger:
        text = "Danger";
        break;
      case BaseMapElement::ControlD:
        text = "Control";
        break;
      case BaseMapElement::LowFlight:
        text = "LowFlight";
        break;
      default:
        text = "<B><EM>unknown</EM></B>";
        break;
    }

  text = text + " " + name + "<BR>" +
      "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}

void Airspace::setValues(unsigned int upper, unsigned int upperType,
            unsigned int lower, unsigned int lowerType)
{
  lLimit = lower;
  lLimitType = lowerType;
  uLimit = upper;
  uLimitType = upperType;
}

void Airspace::printMapElement(QPainter* printPainter)
{
//  if(!__isPrintable()) return;
warning("Drucke Luftraum ...");

  extern const MapMatrix _globalMapMatrix;
  QPointArray temp = _globalMapMatrix.print(projPointArray);
  printPainter->setPen(QPen(QColor(255,0,0), 4));
//  printPainter->drawPolygon(_globalMapMatrix.print(projPointArray));
  printPainter->drawPolygon(temp);

  warning("1. Position: ( %d / %d )",
    temp.at(0).x(), temp.at(0).y());
}

void Airspace::printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        struct elementBorder mapBorder)
{
  if(!__isVisible()) return;

  switch(typeID)
    {
      case AirC:
        printPainter->setBrush(QBrush::NoBrush);
        printPainter->setPen(QPen(QColor(0,120,0), 3));
        break;
      case AirCtemp:
        printPainter->setBrush(QBrush(QColor(150,150,150), QBrush::HorPattern));
        printPainter->setPen(QPen(QColor(0,120,0), 3));
        break;
      case AirD:
        printPainter->setBrush(QBrush::NoBrush);
        printPainter->setPen(QPen(QColor(0,180,0), 3, QPen::DashLine));
        break;
      case AirDtemp:
        printPainter->setBrush(QBrush(QColor(150,150,150), QBrush::VerPattern));
        printPainter->setPen(QPen(QColor(0,180,0), 3, QPen::DashLine));
        break;
      case AirElow:
        printPainter->setBrush(QBrush::NoBrush);
        printPainter->setPen(QPen(QColor(200,100,100), 4));
        break;
      case AirEhigh:
        printPainter->setBrush(QBrush::NoBrush);
        printPainter->setPen(QPen(QColor(100,100,175), 4));
        break;
      case AirF:
        printPainter->setBrush(QBrush(QColor(100,100,100), QBrush::DiagCrossPattern));
        printPainter->setPen(QPen(QColor(50,50,125), 2));
        break;
      case ControlD:
        printPainter->setBrush(QBrush(QColor(100,100,100), QBrush::DiagCrossPattern));
        printPainter->setPen(QPen(QColor(0,0,150), 2, QPen::DashLine));
        break;
      case Restricted:
        printPainter->setBrush(QBrush(QColor(150,150,150), QBrush::BDiagPattern));
        printPainter->setPen(QPen(QColor(50,50,125), 2));
        break;
      case Danger:
        printPainter->setBrush(QBrush(QColor(100,100,100), QBrush::BDiagPattern));
        printPainter->setPen(QPen(QColor(50,50,125), 2));
        break;
      case LowFlight:
        printPainter->setBrush(QBrush(QColor(100,100,100), QBrush::FDiagPattern));
        printPainter->setPen(QPen(QColor(150,0,0), 3, QPen::DashLine));
        break;
    }
//  printPainter->drawPolygon(__projectElement(dX, dY, mapCenterLon, scale));
}
