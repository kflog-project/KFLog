/***********************************************************************
 **
 **   projectionlambert.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Heiner Lamprecht
 **                   2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include "projectionlambert.h"

#define NUM_TO_RAD(num) ( (M_PI / 108000000.0) * (double)(num) )

ProjectionLambert::ProjectionLambert(int v1_new, int v2_new, int orig_new)
{
  i_v1=v1_new;
  i_v2=v2_new;
  i_origin=orig_new;
  last_lat=0;
  last_lon=0;
  project_XY_arg_lat=0.0;
  project_XY_arg_lon=0.0;

  initProjection(v1_new, v2_new, orig_new);
}

ProjectionLambert::ProjectionLambert(QDataStream & s)
{
  // @AP: valgrind said, do initialize internal variables :-))
  i_v1 = 0;
  i_v2 = 0;
  i_origin = 0;

  loadParameters(s);
}

ProjectionLambert::~ProjectionLambert()
{}

bool ProjectionLambert::initProjection(int v1_new, int v2_new, int orig_new)
{
  bool changed(false);

  /**
   * If either of the standard parallels > 90 degrees or < -90 degrees use default
   */
  if(v1_new > 54000000.0 || v1_new < -54000000.0 ||
     v2_new > 54000000.0 || v2_new < -54000000.0) {
    // values out of range, resetting to default, v1 = 54 degrees, v2 = 50 degrees
    // better check in input dlg and not here???
    changed = (v1 != NUM_TO_RAD(32400000.0) || v2 != NUM_TO_RAD(30000000.0));

    v1 = NUM_TO_RAD(32400000.0);
    v2 = NUM_TO_RAD(30000000.0);
    i_v1=32400000;
    i_v2=30000000;
  } else {
    changed = ((i_v1 != v1_new) || (i_v2 != v2_new));

    v1 = NUM_TO_RAD(v1_new);
    v2 = NUM_TO_RAD(v2_new);
    i_v1=v1_new;
    i_v2=v2_new;
  }

  sinv1 = sin(v1);
  sinv1_2 = sinv1 * sinv1;
  sinv2 = sin(v2);
  sinv2_2 = sinv2 * sinv2;
  cosv1 = cos(v1);
  cosv1_2 = cosv1 * cosv1;
  cosv2 = cos(v2);
  //var1 = cosv1 * cosv1;
  //var2 = sinv1 + sinv2;
  var1 = sinv1 + sinv2;
  var2 = -(1 + sinv1*sinv2)/var1;
  var3 = var1/4;
  var4 = 1.0/(2.0*var3);

  last_lat =0;
  last_lon=0;

  changed = changed || ( i_origin != orig_new );
  origin = NUM_TO_RAD(orig_new);
  i_origin=orig_new;

  return changed;
}

double ProjectionLambert::projectX(const double& latitude, const double& longitude)
{
  if (last_lat!=latitude) {
    last_lat=latitude;
    project_XY_arg_lat= var4*sqrt(cosv1_2 + (sinv1 - sin(latitude))*var1); //store result, we'll probably need it again soon!
  }
  if (last_lon!=longitude) {
    last_lon=longitude;
    project_XY_arg_lon=  2.0 * var3 * (longitude - origin);
  }

  return ( project_XY_arg_lat )
    * sin( project_XY_arg_lon );
}

double ProjectionLambert::projectY(const double& latitude, const double& longitude)
{
  if (last_lat!=latitude) {
    last_lat=latitude;
    project_XY_arg_lat= var4*(sqrt(cosv1_2 + (sinv1 - sin(latitude))*var1)); //store result, we'll probably need it again soon!
  }
  if (last_lon!=longitude) {
    last_lon=longitude;
    project_XY_arg_lon= 2.0*var3*(longitude - origin);
  }

  return ( project_XY_arg_lat)
    * cos( project_XY_arg_lon );
}

double ProjectionLambert::invertLat(const double& x, const double& y) const
{
  //    double lat =
  //              -asin(
  //                ( -4.0 * cosv1_2 - 4.0 * sinv1_2 -4.0 * sinv1 * sinv2
  //                 + x * x * sinv1_2 + sinv1_2 * y * y
  //                 + 2.0 * x * x * sinv1 * sinv2 + 2.0 * sinv1 * sinv2 * y * y
  //                 + x * x * sinv2_2 + sinv2_2 * y * y ) / ( sinv1 + sinv2 ) / 4 );

  return -asin(var2 + var3*(x*x + y*y));
}

double ProjectionLambert::invertLon(const double& x, const double& y) const
{
  double lon = 2.0 * atan( x / y ) / ( sinv1 + sinv2 );

  return lon + origin;
}

double ProjectionLambert::getRotationArc(const int x, const int y) const
{
  return atan(x * 1.0 / y * 1.0);
}

int ProjectionLambert::getTranslationX(const int width, const int ) const
{
  return width / 2;
}


int ProjectionLambert::getTranslationY(const int height, const int y) const
{
  return (height / 2) - y;
}

/**
 * Saves the parameters specific to this projection to a stream
 */
void ProjectionLambert::saveParameters(QDataStream & s)
{
  s << qint32(i_v1);
  s << qint32(i_v2);
  s << qint32(i_origin);
}

/**
 * Loads the parameters specific to this projection from a stream
 */
void ProjectionLambert::loadParameters(QDataStream & s)
{
  qint32 i1=0;
  qint32 i2=0;
  qint32 i3=0;
  s >> i1;
  s >> i2;
  s >> i3;

  initProjection(i1, i2, i3);
}
