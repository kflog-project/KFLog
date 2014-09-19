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

#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "topolegend.h"
#include "mapconfig.h"

extern MapConfig* _globalMapConfig;

TopoLegend::TopoLegend( QWidget* parent ) :
  QWidget(parent),
  selectedItem(-1)
{
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

  /*
   * AP: It was a longer fight to get running the widget in a scroll area. The
   * trick is, to put the scroll area into an own layout and add this layout
   * to the parent widget. The elevation bar itself must be put into an own,
   * widget which is passed to the scroll area widget. That ensures that the
   * scroll area fills out the parent widget. A good example was to find here:
   *
   * http://wiki.forum.nokia.com/index.php/Shows_the_use_of_QScrollArea
   */

  // Layout used by scroll area
  QHBoxLayout *hbox = new QHBoxLayout(this);

  // new widget as elevation display
  QWidget* widget = new QWidget(this);

  // layout used by elevation display
  QVBoxLayout* levelBox = new QVBoxLayout(widget);
  levelBox->setMargin(0);
  levelBox->setSpacing(0);

  // set font size
  QFont labelFont = QFont( font() );
  labelFont.setPointSizeF( 9.5 );
  setFont( labelFont );

  QFontMetrics fm = fontMetrics();

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

      label->setMinimumWidth( fm.width(label->text()) + 10 );

      label->setAutoFillBackground( true );
      label->setBackgroundRole( QPalette::Window );

      // get the appropriate color from the mapconfig
      label->setPalette( QPalette(_globalMapConfig->getIsoColor(i)) );

      // Add label to layout box
      levelBox->addWidget( label );

      // Add the label to our label list
      labelList.append( label );
    }

  // Set the size limits of the widget
  setMinimumWidth( 180 );
  setMaximumWidth( 200 );
  setMinimumHeight( 180 );

  scrollArea = new QScrollArea(this);

  // Pass elevation widget to scroll area
  scrollArea->setWidget( widget );
  scrollArea->setWidgetResizable( true );
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  // Add scroll area to an own layout
  hbox->addWidget( scrollArea );

  // Pass scroll area layout to the parent widget.
  setLayout( hbox );
}

TopoLegend::~TopoLegend()
{
}

void TopoLegend::slotSelectElevation( int elevation )
{
  if( ! isVisible() )
    {
      return;
    }

  int index = isoHash.value( elevation );

  if( index == selectedItem )
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

  if( selectedItem >= 0 && selectedItem < 51 )
    {
      labelList.at( 50 - selectedItem )->setFrameStyle( QFrame::NoFrame );
    }

  selectedItem = index;
}

void TopoLegend::slotUpdateElevationColors()
{
  for( int i = 50; i >= 0 && i < labelList.size(); --i )
    {
      QLabel* label = labelList.at( i );

      label->setPalette( QPalette(_globalMapConfig->getIsoColor(i)) );
    }
}
