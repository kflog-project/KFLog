/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>

#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& n, const QString& i,
                       const QString& g, BaseMapElement::objectType t,
                       const WGSPoint& wgsP, const QPoint& pos,
                       const QString& f, int elev)
  : SinglePoint(n, g, t, wgsP, pos, elev),
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
