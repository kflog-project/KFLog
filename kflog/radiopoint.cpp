/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2011 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>

#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& n, const QString& i,
                       const QString& g, BaseMapElement::objectType t,
                       const WGSPoint& wgsP, const QPoint& pos,
                       const QString& f, int elev, const QString& comment )
  : SinglePoint(n, g, t, wgsP, pos, elev, comment),
    frequency(f), icao(i)
{
}

RadioPoint::~RadioPoint()
{
}

void RadioPoint::printMapElement( QPainter* printPainter, bool isText )
{
  Q_UNUSED( printPainter )
  Q_UNUSED( isText )
}
