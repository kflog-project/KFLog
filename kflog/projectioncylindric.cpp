/***********************************************************************
**
**   projectioncylindric.cpp
**
**   This file is part of KFLog2.
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

#include <cmath>

#include "projectioncylindric.h"

#define PI 3.141592654
#define NUM_TO_RAD(num) ( ( PI * (double)(num) ) / 108000000.0 )

ProjectionCylindric::ProjectionCylindric(double v1_new)
{
  initProjection(v1_new);
}

ProjectionCylindric::~ProjectionCylindric()
{

}

bool ProjectionCylindric::initProjection(int v1_new)
{
  bool changed(false);

  if(v1_new > 54000000 || v1_new < -54000000)
    {
      changed = ( v1 == NUM_TO_RAD(27000000) );
      v1 = NUM_TO_RAD(27000000);
    }
  else
    {
      changed = ( v1 == NUM_TO_RAD(v1_new) );
      v1 = NUM_TO_RAD(v1_new);
    }

  return true;
}

double ProjectionCylindric::projectX(double latitude, double longitude) const
{
  return longitude * cos(v1);
}

double ProjectionCylindric::projectY(double latitude, double longitude) const
{
  return -latitude;
}

double ProjectionCylindric::invertLat(double x, double y) const
{
  return -y;
}

double ProjectionCylindric::invertLon(double x, double y) const
{
  return x / cos(v1);
}

// We do not rotate the map at all ...
double ProjectionCylindric::getRotationArc(int x, int y) const  {  return 0;  }

int ProjectionCylindric::getTranslationX(int width, int x) const
{
  return width / 2 - x;
}

int ProjectionCylindric::getTranslationY(int height, int y) const
{
  return (height / 2) - y;
}
