/***********************************************************************
**
**   projectionbase.cpp
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

#include "projectionbase.h"

ProjectionBase::ProjectionBase(int projectionType)
  : projType((ProjectionType)projectionType)
{

}

ProjectionBase::~ProjectionBase()
{

}

// We implement these methods not in the header, because otherwise
// we would get warning "unsused parameters", whenever an other
// file includes the header ...
double ProjectionBase::projectX(double latitude, double longitude) const
{
  return 0;
}

double ProjectionBase::projectY(double latitude, double longitude) const
{
  return 0;
}

double ProjectionBase::invertLat(double x, double y) const  {  return 0;  }

double ProjectionBase::invertLon(double x, double y) const  {  return 0;  }

double ProjectionBase::getRotationArc(int x, int y) const  {  return 0;  }

int ProjectionBase::getTranslationX(int width, int x) const  {  return 0;  }

int ProjectionBase::getTranslationY(int height, int y) const  {  return 0;  }
