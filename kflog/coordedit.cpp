/***********************************************************************
**
**   coordedit.cpp - Editor for WGS84 coordinates, supports three formats.
**
**   This file is part of KFLog
**
************************************************************************
**
**   Copyright (c):  2001 by Harald Maier
**                   2009 by Axel Pauli complete redesign done
**                   2011 by Axel Pauli Bug fixing
**
************************************************************************
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <cmath>

#include <QHBoxLayout>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QLabel>
#include <QFontMetrics>

#include "coordedit.h"
#include "wgspoint.h"

CoordEdit::CoordEdit(QWidget *parent) : QWidget( parent )
{
  iniKflogDegree = 0;
  iniDegree      = "";
  iniMinute      = "";
  iniSecond      = "";
  iniDirection   = "";

  setObjectName("CoordEdit");

  const int spaceItem1 = 5;
  const int spaceItem2 = 10;

  QLabel *label;
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->setSpacing(0);
  hbox->setContentsMargins ( 0, 0, 0, 0 );

  // In dependency of the selected coordinate format three different layouts
  // are provided by this widget.
  minuteBox = static_cast<QLineEdit *> (0);
  secondBox = static_cast<QLineEdit *> (0);

  // Degree input is always needed
  degreeBox = new QLineEdit( this );
  degreeBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  hbox->addWidget( degreeBox );
  hbox->addSpacing( spaceItem1 );
  label = new QLabel( "°", this );
  hbox->addWidget( label );
  hbox->addSpacing( spaceItem2 );

  if ( WGSPoint::getFormat() == WGSPoint::DDM ||
       WGSPoint::getFormat() == WGSPoint::DMS )
    {
      minuteBox = new QLineEdit( this );
      minuteBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      hbox->addWidget( minuteBox );
      hbox->addSpacing( spaceItem1 );
      label = new QLabel( "'", this );
      hbox->addWidget( label );
      hbox->addSpacing( spaceItem2 );
    }

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      secondBox = new QLineEdit( this );
      secondBox->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      hbox->addWidget( secondBox );
      hbox->addSpacing( spaceItem1 );
      label = new QLabel( "\"", this );
      hbox->addWidget( label );
      hbox->addSpacing( spaceItem2 );
    }

  // add combo box for sky directions
  directionBox = new QComboBox( this );
  hbox->addWidget( directionBox );
  hbox->addStretch( 10 );

  setLayout( hbox );

  if ( WGSPoint::getFormat() != WGSPoint::DDD )
    {
      // If the coordinate format is not equal to decimal degree, we have
      // to check all the input boxes to prevent senseless values there.
      connect( degreeBox, SIGNAL(textEdited( const QString&)),
               this, SLOT(slot_textEdited( const QString& )) );

      connect( minuteBox, SIGNAL(textEdited( const QString&)),
               this, SLOT(slot_textEdited( const QString& )) );

      if( WGSPoint::getFormat() == WGSPoint::DMS )
        {
          connect( secondBox, SIGNAL(textEdited( const QString&)),
                   this, SLOT(slot_textEdited( const QString& )) );
        }
    }
}

CoordEdit::~CoordEdit()
{
}

/**
 * Catch show events in this class to set a uniform width for different
 * widgets depending on the used font.
 *
 */
void CoordEdit::showEvent( QShowEvent * )
{
  QFontMetrics fm( font() );

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      int strWidth = fm.width(QString("00000"));

      degreeBox->setMinimumWidth( strWidth );
      degreeBox->setMaximumWidth( strWidth );
      minuteBox->setMinimumWidth( strWidth );
      minuteBox->setMaximumWidth( strWidth );
      secondBox->setMinimumWidth( strWidth );
      secondBox->setMaximumWidth( strWidth );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      int strWidth1 = fm.width(QString("00000"));
      int strWidth2 = fm.width(QString("00.00000"));

      degreeBox->setMinimumWidth( strWidth1 );
      degreeBox->setMaximumWidth( strWidth1 );
      minuteBox->setMinimumWidth( strWidth2 );
      minuteBox->setMaximumWidth( strWidth2 );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      int strWidth = fm.width(QString("000.0000000"));

      degreeBox->setMinimumWidth( strWidth );
      degreeBox->setMaximumWidth( strWidth );
    }

  directionBox->setMinimumContentsLength( 2 );
}

/**
 * Returns true, if initial input values have been modified by the user.
 */
bool CoordEdit::isInputChanged()
{
  bool changed = false;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      changed |= iniDegree != degreeBox->text();
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      changed |= iniDegree != degreeBox->text();
      changed |= iniMinute != minuteBox->text();
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      changed |= iniDegree != degreeBox->text();
      changed |= iniMinute != minuteBox->text();
      changed |= iniSecond != secondBox->text();
    }

  changed |= iniDirection != directionBox->currentText();

  return changed;
}

/**
 * Sets the controls for the latitude editor.
 */
LatEdit::LatEdit(QWidget *parent, const int base) : CoordEdit(parent)
{
  setObjectName("LatEdit");

  QRegExpValidator *eValidator;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      degreeBox->setInputMask( "99.99999" );
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9]\\.[0-9]{5})|(90\\.00000)" ), this );
      degreeBox->setValidator( eValidator );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      degreeBox->setInputMask( "99" );
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9])|90" ), this );
      degreeBox->setValidator( eValidator );

      minuteBox->setInputMask( "99.999" );
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]\\.[0-9]{3}" ), this );
      minuteBox->setValidator( eValidator );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      degreeBox->setInputMask( "99");
      eValidator = new QRegExpValidator( QRegExp( "([0-8][0-9])|90" ), this );
      degreeBox->setValidator( eValidator );

      minuteBox->setInputMask( "99");
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      minuteBox->setValidator( eValidator );

      secondBox->setInputMask( "99");
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      secondBox->setValidator( eValidator );
    }

  directionBox->addItem( QString(" N") );
  directionBox->addItem( QString(" S") );

  // Set all edit fields to zero.
  setKFLogDegree(0);

  // Set default sky direction.
  if (base >= 0)
    {
      directionBox->setCurrentIndex( 0 );
    }
  else
    {
      directionBox->setCurrentIndex( 1 );
    }
}

/**
 * Sets the controls for the longitude editor.
 */

LongEdit::LongEdit(QWidget *parent, const int base) : CoordEdit(parent)
{
  setObjectName("LongEdit");

  QRegExpValidator *eValidator;

  if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      degreeBox->setInputMask( "999.99999" );
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9]\\.[0-9]{5})|([0-1][0-7][0-9]\\.[0-9]{5})|(180\\.00000)" ), this );
      degreeBox->setValidator( eValidator );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      degreeBox->setInputMask( "999" );
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9])|(1[0-7][0-9])|180" ), this );
      degreeBox->setValidator( eValidator );

      minuteBox->setInputMask( "99.999" );
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]\\.[0-9]{3}" ), this );
      minuteBox->setValidator( eValidator );
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      degreeBox->setInputMask( "999");
      eValidator = new QRegExpValidator( QRegExp( "(0[0-9][0-9])|(1[0-7][0-9])|180" ), this );
      degreeBox->setValidator( eValidator );

      minuteBox->setInputMask( "99");
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      minuteBox->setValidator( eValidator );

      secondBox->setInputMask( "99");
      eValidator = new QRegExpValidator( QRegExp( "[0-5][0-9]" ), this );
      secondBox->setValidator( eValidator );
    }

  directionBox->addItem( QString(" E") );
  directionBox->addItem( QString(" W") );

  // Set all edit fields to zero
  setKFLogDegree(0);

  // Set the default sky direction.
  if (base >= 0)
    {
      directionBox->setCurrentIndex( 0 );
    }
  else
    {
      directionBox->setCurrentIndex( 1 );
    }
}

/** Used to check the user input in the editor fields. */
void CoordEdit::slot_textEdited( const QString& )
{
  if( degreeBox->text() == "90" || degreeBox->text() == "180" )
    {
      // If the degree box is set to the possible maximum, the other
      // boxes must be set to zero to prevent senseless results.
      if ( WGSPoint::getFormat() == WGSPoint::DDM )
          {
            minuteBox->setText( "00.000" );
            return;
         }

      if ( WGSPoint::getFormat() == WGSPoint::DMS )
        {
          minuteBox->setText( "00");
          secondBox->setText( "00");
          return;
        }
    }
}

/**
 * Calculates a degree value in the KFLog internal format for degrees from
 * the input data fields.
 */
int CoordEdit::KFLogDegree()
{
  if( isInputChanged() == false )
    {
      // Nothing was modified, return initial value to avoid rounding
      // errors during conversions.
      return iniKflogDegree;
    }

  QString input = "";
  QChar degreeChar(Qt::Key_degree);

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      input = degreeBox->text() + degreeChar + " " +
              minuteBox->text() + "' " +
              secondBox->text() + "\"";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      input = degreeBox->text() + degreeChar + " " +
              minuteBox->text() + "'";
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      input = degreeBox->text() + degreeChar;
    }

  input += " " + directionBox->currentText().trimmed();

  // This method make the conversion to the internal KFLog degree format.
  return WGSPoint::degreeToNum( input );
}

/**
 * Sets all edit fields according to the passed coordinate value.
 * The coordinate value is encoded in the KFLog internal format for degrees.
 */
void CoordEdit::setKFLogDegree( const int coord, const bool isLat )
{
  QString posDeg, posMin, posSec;
  int degree, min, sec;
  double decDegree, decMin;

  iniKflogDegree = coord; // save initial coordinate value

  if ( WGSPoint::getFormat() == WGSPoint::DMS )
    {
      // degrees, minutes, seconds is used as format
      WGSPoint::calcPos (coord, degree, min, sec);
      degree = (degree < 0)  ? -degree : degree;

      if (isLat)
        {
          posDeg.sprintf("%02d", degree);
        }
      else
        {
          posDeg.sprintf("%03d", degree);
        }

      min = abs(min);
      posMin.sprintf("%02d", min);

      sec = abs(sec);
      posSec.sprintf("%02d", sec);

      degreeBox->setText( posDeg );
      minuteBox->setText( posMin );
      secondBox->setText( posSec );

      // save initial values
      iniDegree = posDeg;
      iniMinute = posMin;
      iniSecond = posSec;
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDM )
    {
      // degrees and decimal minutes is used as format
      WGSPoint::calcPos (coord, degree, decMin);
      degree = (degree < 0)  ? -degree : degree;

      if (isLat)
        {
          posDeg.sprintf("%02d", degree);
        }
      else
        {
          posDeg.sprintf("%03d", degree);
        }

      decMin = fabs(decMin);

      posMin.sprintf("%.3f", decMin);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if ( decMin < 10.0 )
        {
          posMin.insert(0, "0");
        }

      degreeBox->setText( posDeg );
      minuteBox->setText( posMin );

      // save initial values
      iniDegree = posDeg;
      iniMinute = posMin;
    }
  else if ( WGSPoint::getFormat() == WGSPoint::DDD )
    {
      // decimal degrees is used as format
      WGSPoint::calcPos (coord, decDegree);
      decDegree = (decDegree < 0)  ? -decDegree : decDegree;

      posDeg.sprintf("%.5f", decDegree);

      // Unfortunately sprintf does not support leading zero in float
      // formating. So we must do it alone.
      if (isLat)
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "0");
            }
        }
      else
        {
          if ( decDegree < 10.0 )
            {
              posDeg.insert(0, "00");
            }
          else if ( decDegree < 100.0 )
            {
              posDeg.insert(0, "0");
            }
        }

      degreeBox->setText( posDeg );

      // save initial value
      iniDegree = posDeg;
    }

  // Set sky direction in combo box
  if (coord < 0)
    {
      directionBox->setCurrentIndex(1);
    }
  else
    {
      directionBox->setCurrentIndex(0);
    }

  // Save initial value of sky direction.
  iniDirection = directionBox->currentText();
}

/** Sets all edit fields to the passed coordinate value in KFLog format. */
void LatEdit::setKFLogDegree( const int coord )
{
  CoordEdit::setKFLogDegree( coord, true );
}

/** Sets all edit fields to the passed coordinate value in KFLog format. */
void LongEdit::setKFLogDegree( const int coord )
{
  CoordEdit::setKFLogDegree( coord, false );
}
