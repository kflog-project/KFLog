/***********************************************************************
**
**   baseflightelement.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtCore>

#include "baseflightelement.h"

BaseFlightElement::BaseFlightElement( const QString& name,
                                      const BaseMapElement::objectType type,
                                      const QString& fName) :
  BaseMapElement( name, type ),
  sourceFileName( fName )
{
}

BaseFlightElement::~BaseFlightElement()
{
}

int BaseFlightElement::searchPoint(const QPoint&, FlightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetPrevPoint(int, FlightPoint&)  {  return -1;  }

int BaseFlightElement::searchGetNextPoint(int, FlightPoint&)  {  return -1;  }

int BaseFlightElement::searchStepNextPoint(int, FlightPoint&, int) {  return -1;  }

int BaseFlightElement::searchStepPrevPoint(int,  FlightPoint&, int) {  return -1;  }
