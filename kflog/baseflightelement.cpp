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

BaseFlightElement::BaseFlightElement(QString name, unsigned int typeID,
      QString fName)
  : BaseMapElement(name, typeID),
    sourceFileName(fName)
{

}

BaseFlightElement::~BaseFlightElement(){
}

int BaseFlightElement::searchPoint(QPoint, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetPrevPoint(int, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetNextPoint(int, flightPoint&)  {  return -1;  }

int BaseFlightElement::searchStepNextPoint(int, flightPoint&, int) {  return -1;  }

int BaseFlightElement::searchStepPrevPoint(int,  flightPoint&, int) {  return -1;  }

//QStrList BaseFlightElement::getHeader()  {  return header;  }
