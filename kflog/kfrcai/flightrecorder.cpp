/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of libkfrcai.
**
************************************************************************
**
**   Copyright (c):  2007 by Hendrik Hoeth
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/
#include "cambridge.h"

int breakTransfer = 0;

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
FlightRecorderPluginBase * getRecorder()
{
  return new Cambridge();
}
