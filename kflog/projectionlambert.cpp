/***********************************************************************
**
**   projectionlambert.cpp
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

#include <iostream.h>

#include "projectionlambert.h"

#define PI 3.141592654
#define NUM_TO_RAD(num) ( ( PI * (double)(num) ) / 108000000.0 )

ProjectionLambert::ProjectionLambert(int v1_new, int v2_new, int orig_new)
  : ProjectionBase(ProjectionBase::Lambert)
{
  initProjection(v1_new, v2_new, orig_new);
}

ProjectionLambert::~ProjectionLambert()
{

}

bool ProjectionLambert::initProjection(int v1_new, int v2_new, int orig_new)
{
  bool changed(false);

  if(v1_new > 54000000.0 || v1_new < -54000000.0 ||
      v2_new > 54000000.0 || v2_new < -54000000.0)
    {
      // values out of range, resetting to default
      changed = (v1 == NUM_TO_RAD(32400000.0) || v2 == NUM_TO_RAD(30000000.0));

      v1 = NUM_TO_RAD(32400000.0);
      v2 = NUM_TO_RAD(30000000.0);
    }
  else
    {
      changed = (v1 == NUM_TO_RAD(v1_new) || v2 == NUM_TO_RAD(v2_new));

      v1 = NUM_TO_RAD(v1_new);
      v2 = NUM_TO_RAD(v2_new);
    }

  var1 = cos(v1) * cos(v1);
  var2 = sin(v1) + sin(v2);

  changed = changed || ( origin == NUM_TO_RAD(orig_new) );
  origin = NUM_TO_RAD(orig_new);

  return changed;
}

double ProjectionLambert::projectX(double latitude, double longitude) const
{
  longitude -= origin;

  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
             * sin( var2 * longitude / 2 ) );
}

double ProjectionLambert::projectY(double latitude, double longitude) const
{
  longitude -= origin;

  return ( 2 * ( sqrt( var1 + ( sin(v1) - sin(latitude) ) * var2 ) / var2 )
             * cos( var2 * longitude / 2 ) );
}

double ProjectionLambert::invertLat(double x, double y) const
{
  double lat = -asin(
              ( -4.0 * pow(cos(v1), 2.0) - 4.0 * pow(sin(v1), 2.0)
                -4.0 * sin(v1) * sin(v2)
                + x * x * pow(sin(v1), 2.0) + pow(sin(v1), 2.0)* y * y
                + 2.0 * x * x * sin(v1) * sin(v2) + 2.0 * sin(v1)
                * sin(v2) * y * y + x * x * pow(sin(v2), 2.0)
                + pow(sin(v2), 2.0) * y * y
                ) /
              ( sin(v1) + sin(v2) ) / 4 );

  return lat;
}

double ProjectionLambert::invertLon(double x, double y) const
{
  double lon = 2.0 * atan( x / y ) / ( sin(v1) + sin(v2) );

  return lon + origin;
}

double ProjectionLambert::getRotationArc(int x, int y) const
{
  return atan(x * 1.0 / y * 1.0);
}

int ProjectionLambert::getTranslationX(int width, int x) const
{
  return width / 2;
}

int ProjectionLambert::getTranslationY(int height, int y) const
{
  return (height / 2) - y;
}
