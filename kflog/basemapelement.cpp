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
  : name(n), typeID(tID)
{

}

BaseMapElement::~BaseMapElement()
{

}

bool BaseMapElement::__isVisible() const { return true; }

MapMatrix* BaseMapElement::glMapMatrix;

MapConfig* BaseMapElement::glConfig;

void BaseMapElement::initMapElement(MapMatrix* matrix, MapConfig* config)
{
  glMapMatrix = matrix;
  glConfig = config;
}

void BaseMapElement::printMapElement(QPainter* printP, bool isT)
{
//  warning("BaseMapElement::printMapElement");
}


void BaseMapElement::drawMapElement(QPainter* pP, QPainter* mP) {  }

QString BaseMapElement::getName() const { return name; }

unsigned int BaseMapElement::getTypeID() const { return typeID; }
