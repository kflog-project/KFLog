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

#include "altitude.h"
#include "distance.h"
#include "mapconfig.h"
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

QString RadioPoint::getInfoString()
{
  QString path = glConfig->getIconPath();
  QString text;
  QString text1;

  text = QString("<html><center><b>") +
         "<IMG SRC=" + path + "/" +
         glConfig->getPixmapName(typeID, false, false) + "> " +
         name;

  if( !m_icao.isEmpty() )
    {
      text1 = " (" + m_icao;
    }

  if( !country.isEmpty() )
    {
      if( text1.isEmpty() )
        {
          text1 = " (";
        }
      else
        {
          text1 += ", ";
        }

      text1 += country + ")";
    }
  else
    {
      text1 += ")";
    }

  text += text1;

  text += " " + BaseMapElement::item2Text( typeID, QObject::tr("(unknown object)") ) +
           "</b></center>";

  text += "<table cellpadding=5 width=100%>";

  text += "<tr><td>" + QObject::tr("Frequency:") + "</td><td>";

  if( m_frequency > 0.0 )
    {
      text += "<b>" + frequencyAsString() + " " + QObject::tr("MHz") + "</b></td>";
    }
  else
    {
      text += "<b>" + QObject::tr("unknown") +"</b></td>";
    }

  text += "<td>" + QObject::tr("Channel:") + "</td><td>";

  if( m_channel.isEmpty() )
    {
      text += "<b>" + QObject::tr("unknown") +"</b></td>";
    }
  else
    {
      text += "<b>" + m_channel + "</b></td></tr>";
    }

  text += "<tr><td>" + QObject::tr("Declination:") + "</td><td>";

  if( m_declination != SHRT_MIN )
    {
      text += "<b>" + QString("%1%2")
			      .arg( getDeclination(), 0, 'f', 0 )
			      .arg( QChar(Qt::Key_degree) ) +
	      "</b></td>";
    }
  else
    {
      text += "<b>" + QObject::tr("unknown") +"</b></td>";
    }

  text += "<td>" + QObject::tr("True N aligned:") + "</td><td>";

  if( isAligned2TrueNorth() )
    {
      text += "<b>" + QObject::tr("Yes") +"</b></td></tr>";
    }
  else
    {
      text += "<b>" + QObject::tr("No") +"</b></td></tr>";
    }

  text += "<tr><td>" + QObject::tr("Range:") + "</td><td>";

  if( getRange() > 0.0  )
    {
      text += "<b>" + Distance::getText(getRange(), true, 0) + "</b></td>";
    }
  else
    {
      text += "<b>" + QObject::tr("unknown") +"</b></td>";
    }

  // save current unit
  Altitude::altitudeUnit currentUnit = Altitude::getUnit();

  Altitude::setUnit(Altitude::meters);
  QString meters = Altitude::getText( elevation, true, 0 );

  Altitude::setUnit(Altitude::feet);
  QString feet = Altitude::getText( elevation, true, 0 );

  // restore save unit
  Altitude::setUnit(currentUnit);

  if( currentUnit == Altitude::meters )
   {
     text += "<td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + meters + " / " + feet +
             "</b></td></tr>";
   }
  else
   {
     text += "<td>" + QObject::tr("Elevation:") +
             "</td><td><b>" + feet + " / " + meters +
             "</b></td></tr>";
   }

  text += "</table></html>";

  return text;
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
