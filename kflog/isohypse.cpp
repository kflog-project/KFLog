/***********************************************************************
**
**   isohypse.cpp
**
**   This file is part of KFLog.
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

//#include "iostream.h"

#include "isohypse.h"

#include <mapmatrix.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qtextstream.h>

Isohypse::Isohypse()
: LineElement(0, BaseMapElement::Isohypse, 0)
{

}

Isohypse::Isohypse(QPointArray pA, unsigned int elev, bool isV, int s_ID)
: LineElement(0, BaseMapElement::Isohypse, pA),
  elevation(elev), valley(isV), sort_ID(s_ID)
{
  int tempElev = elevation;
  int delta = 0;

  /* Bestimmen der Layers: */
  if(elevation == 0)  delta = 100;
  else if(elevation == 10)   delta = 10;
  else if(elevation <= 100)  delta = 25;
  else if(elevation <= 1000) delta = 100;
  else delta = 250;

  if(!valley) tempElev -= delta;

  /* Welche Höhenlinien sollen letztlich angezeigt werden ??? */
  double rot = 0, gruen = 0, blau = 0;
  if(tempElev < 0)
    {
      rot = 96;
      gruen = 128;
      blau = 248;
    }
  else if(tempElev <= 10)
    {
      rot = 174 + ( tempElev / 10.0 * 27 );
      gruen = 208 + ( tempElev / 10.0 * 22 );
      blau = 129 + ( tempElev / 10.0 * 49 );
    }
  else if(tempElev <= 50)
    {
      rot = 201 + ( ( tempElev - 10 ) / 40.0 * 30 );
      gruen = 230 + ( ( tempElev - 10 ) / 40.0 * 25 );
      blau = 178 + ( ( tempElev - 10 ) / 40.0 * 53 );
    }
  else if(tempElev <= 100)
    {
      rot = 231 + ( ( tempElev - 50 ) / 50.0 * -10 );
      gruen = 255 + ( ( tempElev - 50) / 50.0 * -10 );
      blau = 231 + ( ( tempElev - 50 ) / 50.0 * -48 );
    }
  else if(tempElev <= 200)
    {
      rot = 221 + ( ( tempElev - 100 ) / 100.0 * 19 );
      gruen = 245 + ( ( tempElev - 100 ) / 100.0 * -5 );
      blau = 221 + ( ( tempElev - 100 ) / 100.0 * -15 );
    }
  else if(tempElev <= 1000)
    {
      rot = 240 + ( ( tempElev - 200 ) / 800.0 * -5 );
      gruen = 240 + ( ( tempElev - 200 ) / 800.0 * -85 );
      blau = 168 + ( ( tempElev - 200 ) / 800.0 * -70 );
    }
  else if(tempElev <= 4000)
    {
      rot = 235 + ( ( tempElev - 1000 ) / 3000.0 * -105 );
      gruen = 155 + ( ( tempElev - 1000 ) / 3000.0 * -90 );
      blau = 98 + ( ( tempElev - 1000 ) / 3000.0 * -78 );
    }
  else if(tempElev <= 9000)
    {
      rot = 130 + ( ( tempElev - 4000 ) / 5000.0 * -34 );
      gruen = 65 + ( ( tempElev - 4000 ) / 5000.0 * -22 );
      blau = 20 + ( ( tempElev - 4000 ) / 5000.0 * -4 );
    }

  fillColor.setRgb(rot, gruen, blau);
  /*

  tempElev <    0 ( 96, 128, 248);
  tempElev <   10 (174, 208, 129);
  tempElev <   50 (201, 230, 178);
  tempElev <  100 (231, 255, 231);
  tempElev <  250 (221, 245, 183);
  tempElev <  500 (240, 240, 168);
  tempElev <  750 (240, 223, 140);
  tempElev < 1000 (235, 185, 128);
  tempElev < 2000 (235, 155,  98);
  tempElev < 3000 (210, 115,  50);
  tempElev < 4000 (180,  75,  25);
  tempElev > 4000 (130,  65,  20);

   * Definierte Farbwerte bei (dazwischen linear ändern):
   *
   *   <0 m :    96 / 128 / 248
   *    0 m :   174 / 208 / 129 *
   *   10 m :   201 / 230 / 178 *
   *   50 m :   231 / 255 / 231 *
   *  100 m :   221 / 245 / 183 *
   *  250 m :   240 / 240 / 168 *
   * 1000 m :   235 / 155 /  98 *
   * 4000 m :   130 /  65 /  20 *
   * 9000 m :    96 /  43 /  16 *
   */
}

Isohypse::~Isohypse()
{

}

void Isohypse::setValley(bool isV)
{
  valley = isV;
}

void Isohypse::setElevation(int elev)
{
  elevation = elev;
}

void Isohypse::setSortID(int sort)
{
  sort_ID = sort;
}

void Isohypse::setValues(int h, int sort, bool valley)
{
  elevation = h;
  sort_ID = sort;
  valley = valley;

  int tempElev = elevation;
  int delta = 0;

  /* Bestimmen der Layers: */
  if(elevation == 0)
      /* Senken werden als "-100" dargestellt. */
      delta = 100;
  else if(elevation == 10)
      delta = 10;
  else if(elevation <= 100)
      delta = 25;
  else if(elevation <= 1000)
      delta = 100;
  else
      delta = 250;

  if(valley)
    {
      tempElev -= delta;
      /* Anscheinend gibt es zur Zeit keine Täler ???? */
      fatal("Höhe: %d / delta: %d / valley: %d", elevation, delta, valley);
    }
  /* Welche Höhenlinien sollen letztlich angezeigt werden ??? */
  double rot = 0, gruen = 0, blau = 0;
  if(tempElev < 0)
    {
      rot = 96;
      gruen = 128;
      blau = 248;
    }
  else if(tempElev <= 10)
    {
      rot = 174 + ( tempElev / 10.0 * 27 );
      gruen = 208 + ( tempElev / 10.0 * 22 );
      blau = 129 + ( tempElev / 10.0 * 49 );
    }
  else if(tempElev <= 50)
    {
      rot = 201 + ( ( tempElev - 10 ) / 40.0 * 30 );
      gruen = 230 + ( ( tempElev - 10 ) / 40.0 * 25 );
      blau = 178 + ( ( tempElev - 10 ) / 40.0 * 53 );
    }
  else if(tempElev <= 100)
    {
      rot = 231 + ( ( tempElev - 50 ) / 50.0 * -10 );
      gruen = 255 + ( ( tempElev - 50) / 50.0 * -10 );
      blau = 231 + ( ( tempElev - 50 ) / 50.0 * -48 );
    }
  else if(tempElev <= 200)
    {
      rot = 221 + ( ( tempElev - 100 ) / 100.0 * 19 );
      gruen = 245 + ( ( tempElev - 100 ) / 100.0 * -5 );
      blau = 221 + ( ( tempElev - 100 ) / 100.0 * -15 );
    }
  else if(tempElev <= 1000)
    {
      rot = 240 + ( ( tempElev - 200 ) / 800.0 * -5 );
      gruen = 240 + ( ( tempElev - 200 ) / 800.0 * -85 );
      blau = 168 + ( ( tempElev - 200 ) / 800.0 * -70 );
    }
  else if(tempElev <= 4000)
    {
      rot = 235 + ( ( tempElev - 1000 ) / 3000.0 * -105 );
      gruen = 155 + ( ( tempElev - 1000 ) / 3000.0 * -90 );
      blau = 98 + ( ( tempElev - 1000 ) / 3000.0 * -78 );
    }
  else if(tempElev <= 9000)
    {
      rot = 130 + ( ( tempElev - 4000 ) / 5000.0 * -34 );
      gruen = 65 + ( ( tempElev - 4000 ) / 5000.0 * -22 );
      blau = 20 + ( ( tempElev - 4000 ) / 5000.0 * -4 );
    }

  fillColor.setRgb(rot, gruen, blau);
}

void Isohypse::drawMapElement(QPainter* targetPainter)
{
  extern const MapMatrix _globalMapMatrix;

  targetPainter->drawPolygon(_globalMapMatrix.map(projPointArray));
}

bool Isohypse::isValley() const { return valley; }

int Isohypse::getElevation() const { return elevation; }

int Isohypse::sortID() const { return sort_ID; }
