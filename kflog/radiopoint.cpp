/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of KFLog4.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QtCore>

#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& name,
                       const QString& icao,
                       const QString& shortName,
                       BaseMapElement::objectType type,
                       const WGSPoint& wgsP,
                       const QPoint& pos,
                       const float frequency,
                       const QString channel,
                       float elevation,
                       const QString& country ) :
  SinglePoint(name, shortName, type, wgsP, pos, elevation, country),
  m_frequency(frequency),
  m_channel(channel),
  m_icao(icao)
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
