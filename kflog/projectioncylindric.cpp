/***********************************************************************
 **
 **   projectioncylindric.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2002 by Heiner Lamprecht, 2007 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <cmath>
#include "projectioncylindric.h"

#define NUM_TO_RAD(num) ( M_PI  / 108000000.0 * (double)(num) )

ProjectionCylindric::ProjectionCylindric(int v1_new)
{
  v1 = 0.0;
  i_v1 = 0;
  initProjection(v1_new);
}

ProjectionCylindric::ProjectionCylindric(QDataStream & s)
{
  v1 = 0.0;
  i_v1 = 0;
  loadParameters(s);
}

ProjectionCylindric::~ProjectionCylindric()
{
}


bool ProjectionCylindric::initProjection(int v1_new)
{
  bool changed(false);

  if(v1_new > 54000000 || v1_new < -54000000) {
    // this is >90° or <-90°: take default of 45°;
    // better check in input dlg and not here???
    changed = ( v1 != NUM_TO_RAD(27000000) );
    v1 = NUM_TO_RAD(27000000);
    i_v1=27000000;
  } else {
    changed = ( i_v1 != v1_new );
    v1 = NUM_TO_RAD(v1_new);
    i_v1=v1_new;
  }

  cos_v1=cos(v1);

  return changed;
}


/**
 * Saves the parameters specific to this projection to a stream
 */
void ProjectionCylindric::saveParameters(QDataStream & s)
{
  s << Q_INT32(i_v1);
}


/**
 * Loads the parameters specific to this projection from a stream
 */
void ProjectionCylindric::loadParameters(QDataStream & s)
{
  Q_INT32 i=0;
  s >> i;
  initProjection(i);
}

