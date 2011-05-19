/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrxsp.
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
#include <unistd.h>
#include <ctype.h>

#include "soaringpilot.h"

int breakTransfer = 0;

extern "C"
{
  /** returns a reference to the flight recorder object */
  FlightRecorderPluginBase * getRecorder();
}

/*************************************************************************
**
** begin of logger-specific implementation
**
*************************************************************************/
FlightRecorderPluginBase * getRecorder()
{
  return new SoaringPilot ();
}


