/***********************************************************************
**
**   projectionbase.cpp
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

#include "projectionbase.h"
#include "projectionlambert.h"
#include "projectioncylindric.h"

ProjectionBase::ProjectionBase()
{}


ProjectionBase::~ProjectionBase()
{}


void SaveProjection(QDataStream & s, ProjectionBase * p)
{
  s << Q_INT8( p->projectionType() );
  p->saveParameters(s);
}


ProjectionBase * LoadProjection(QDataStream & s)
{
  ProjectionBase * result;
  result=0;
  Q_INT8 i=-1;;
  s >> i;
  switch (ProjectionBase::ProjectionType(i)) {
  case ProjectionBase::Lambert:
    result=new ProjectionLambert(s);
    break;
  case ProjectionBase::Cylindric:
    result=new ProjectionCylindric(s);
    break;
  default:
    qWarning("Unknown projection type!");
    break;
  }
  return result;
}
