/***********************************************************************
**
**   baseflightelement.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "baseflightelement.h"

BaseFlightElement::BaseFlightElement(const QString& name, unsigned int typeID,
      const QString& fName)
  : BaseMapElement(name, typeID),
    sourceFileName(fName)
{

}

BaseFlightElement::~BaseFlightElement(){
}

int BaseFlightElement::searchPoint(const QPoint&, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetPrevPoint(int, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetNextPoint(int, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchStepNextPoint(int, flightPoint&, int) {  return -1;  }

int BaseFlightElement::searchStepPrevPoint(int,  flightPoint&, int) {  return -1;  }

/*
void BaseFlightElement::printMapElement(QPainter* printP, bool isT)
{
  warning("BaseFlightElement::printMapElement");
}
*/

void BaseFlightElement::reProject() {
  warning("BaseFlightElement::reProject()");
}
