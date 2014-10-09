/***********************************************************************
 **
 **   radiopoint.cpp
 **
 **   This file is part of KFLog.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000      by Heiner Lamprecht, Florian Ehinger
 **                   2008-2014 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

#include <QtCore>

#include "distance.h"
#include "radiopoint.h"

RadioPoint::RadioPoint(const QString& name,
                       const QString& icao,
                       const QString& shortName,
                       BaseMapElement::objectType type,
                       const WGSPoint& wgsP,
                       const QPoint& pos,
                       const float frequency,
                       const QString channel,
                       const float elevation,
                       const QString country,
                       const float range,
                       const float declination,
                       const bool aligned2TrueNorth ) :
  SinglePoint( name, shortName, type, wgsP, pos, elevation, country ),
  m_frequency(frequency),
  m_channel(channel),
  m_icao(icao),
  m_range(range),
  m_declination(declination),
  m_aligned2TrueNorth(aligned2TrueNorth)
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

QString RadioPoint::getAdditionalText() const
{
  QString text;

  if( getChannel().isEmpty() == false )
    {
      text += QObject::tr("Channel ") + getChannel();
    }

  if( getRange() > 0.0 )
    {
      if( ! text.isEmpty() )
	{
	  text += ", ";
	}

      text += QObject::tr("Range ") + Distance::getText(getRange(), true, 0);
    }

  if( getDeclination() != SHRT_MIN )
    {
      if( ! text.isEmpty() )
	{
	  text += ", ";
	}

      text += QObject::tr("Declination %1%2")
		      .arg( getDeclination(), 0, 'f', 0 )
		      .arg( QChar(Qt::Key_degree) );
    }

  if( ! text.isEmpty() )
    {
      text += ", ";
    }

  if( isAligned2TrueNorth() )
    {
      text += QObject::tr("Not ");
    }

  text += QObject::tr("True N aligned");

  return text;
}
