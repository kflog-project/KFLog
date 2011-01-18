/***********************************************************************
**
**   topolegend.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QtGui>

#include "topolegend.h"
#include "mapconfig.h"
#include "resource.h"

TopoLegend::TopoLegend(QWidget *parent) :
  QWidget(parent),
  currentHighlight(-1)
{
  extern MapConfig *_globalMapConfig;

  //These are the levels used, as defined in mapconfig.cpp.
  //For internal reasons, -1 and 10000 are added to the list.
  int levels[] = {
      -1,0,10,25,50,75,100,150,200,250,300,350,400,450,500,600,700,800,900,1000,
      1250,1500,1750,2000,2250,2500,2750,3000,3250,3500,3750,4000,4250,4500,4750,
      5000,5250,5500,5750,6000,6250,6500,6750,7000,7250,7500,7750,8000,8250,8500,
      8750,10000
  };

  // Setup a hash used as reverse mapping from isoLine elevation value
  // to elevation level array index.
  for( ushort i = 0; i <= 50; i++ )
    {
      isoHash.insert( levels[i], i );
    }

  QWidget* widget = new QWidget;

  QVBoxLayout* levelBox = new QVBoxLayout(widget);
  levelBox->setMargin(0);
  levelBox->setSpacing(0);

  // set font size
  QFont labelFont = QFont( font() );
  labelFont.setPointSizeF( 9.5 );
  setFont( labelFont );

  QFontMetrics fm = fontMetrics();
  int minwidth = 0;

  for( int i = 50; i >= 0; --i )
    {
      //loop over levels defined above in reversed order
      QLabel *label = new QLabel( this );
      label->setAlignment( Qt::AlignHCenter );

      if( i == 50 )
        { //set the text. Normally, this is "xx - yy m", but the first and last get a different one
          label->setText( QString(">= %1 m").arg(levels[i]) );
        }
      else if( i == 0 )
        {
          label->setText( QString(tr("< 0 m (water)")) );
        }
      else
        {
          label->setText( QString("%1 - %2 m").arg(levels[i]).arg(levels[i + 1]) );
        }

      //label->setMinimumSize( fm.size( 0, label->text() ).width(), 0 );

      minwidth = qMax( minwidth, fm.width(label->text()) );

      label->setAutoFillBackground( true );
      label->setBackgroundRole( QPalette::Window );

      // get the appropriate color from the mapconfig
      label->setPalette( QPalette(_globalMapConfig->getIsoColor(i)) );

      // Add label to layout box
      levelBox->addWidget( label );

      // Add the label to our label list
      labelList.append( label );
    }

  int wts = windowTitle().size();

  if( wts > minwidth )
    {
      //setMaximumWidth( wts + 30 );
    }
  else
    {
      //setMaximumWidth( minwidth + 30 );
    }

  //widget->resize( size() );

  scrollArea = new QScrollArea(this);
  scrollArea->setWidget( widget );
  scrollArea->setWidgetResizable( true );
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  scrollArea->setMinimumWidth( minwidth + 40 );
}

TopoLegend::~TopoLegend()
{
}

/** Makes sure the indicated level is visible. */
void TopoLegend::highlightLevel( const int elevation )
{
  if( ! isVisible() )
    {
      return;
    }

  int index = isoHash.value( elevation );

  if( index == currentHighlight )
    {
      return;
    }

  // make sure it's visible, but only if this is a valid level!
  if( index >= 0 && index < 51 )
    {
      QLabel *label = labelList.at( 50 - index );

      int y = label->y();

      scrollArea->ensureVisible( 10, y );

      label->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    }

  if( currentHighlight >= 0 && currentHighlight < 51 )
    {
      labelList.at( 50 - currentHighlight )->setFrameStyle( QFrame::NoFrame );
    }

  currentHighlight = index;
}
