/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrxsp.
**
************************************************************************
**
**   Copyright (c):  2002 by André Somers
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

#include "cumulus.h"

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
  return new Cumulus ();
}

