/***********************************************************************
**
**   waypointelement.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "waypointelement.h"

WaypointElement::WaypointElement()
  : type(-1), surface(-1), runway(0), length(0), elevation(0), frequency(0.0)
{
}
WaypointElement::~WaypointElement(){
}
