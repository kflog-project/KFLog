/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of kio-logger.
**
************************************************************************
**
**   Copyright (c):  2002 by Thomas Nielsen
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <unistd.h>
#include <ctype.h>

#include <klocale.h>

#include "garminfr.h"

extern "C"
{
  /** returns a reference to the flightrecorder object */
  FlightRecorderPluginBase * getRecorder();
}


/*************************************************************************
**
** begin of logger-specific implementation
**
*************************************************************************/
FlightRecorderPluginBase * getRecorder() {
  return new GarminFR();
}


