/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrgcs.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/
#include "volkslogger.h"

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
  return new Volkslogger ();
}
