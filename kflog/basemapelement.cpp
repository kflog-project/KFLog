/***********************************************************************
**
**   basemapelement.cpp
**
**   This file is part of KFLog2.
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

#include "basemapelement.h"

BaseMapElement::BaseMapElement(const char* n, unsigned int tID)
  : name(n), typeID(tID), section(0)
{

}

BaseMapElement::~BaseMapElement()
{

}

bool BaseMapElement::__isVisible() const { return true; }

static MapMatrix* BaseMapElement::glMapMatrix;

static MapConfig* BaseMapElement::glConfig;

void BaseMapElement::initMapElement(MapMatrix* matrix, MapConfig* config)
{
  glMapMatrix = matrix;
  glConfig = config;
}

void BaseMapElement::printMapElement(QPainter* printP, const double dX,
      const double dY, const int mapCenterLon, const double scale,
      const struct elementBorder mapBorder)
{

}

void BaseMapElement::printMapElement(QPainter* printP)
{

}

void BaseMapElement::drawMapElement(QPainter* printP, QPainter* maskP)
{

}

QRegion* BaseMapElement::drawRegion(QPainter* targetP, QPainter* maskPainter)
{
  return (new QRegion(0,0,1,1));
}

QString BaseMapElement::getName() const { return name; }

unsigned int BaseMapElement::getTypeID() const { return typeID; }
