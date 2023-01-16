/***********************************************************************
**
**   AirfieldSelectionList.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>

#include "AirfieldSelectionList.h"
#include "mapcontents.h"

extern MapContents *_globalMapContents;

AirfieldSelectionList::AirfieldSelectionList( QWidget *parent ) :
 QWidget(parent)
{
  setObjectName( "AirfieldSelectionList" );

  m_searchEntry  = new QLineEdit;
  m_searchEntry->setToolTip( tr("Enter a search string, to navigate to a certain list entry.") );

  connect( m_searchEntry, SIGNAL(returnPressed ()), SLOT(slotReturnPressed()) );
  connect( m_searchEntry, SIGNAL(textEdited(const QString&)),
           this, SLOT(slotTextEdited(const QString&)) );

  m_selectionBox = new QComboBox;

  QPushButton* clearButton = new QPushButton(tr("Clear"));
  clearButton->setToolTip( tr("Click Clear to remove the search string.") );

  connect( clearButton, SIGNAL(clicked()),
           SLOT(slotClearSearchEntry()));

  QPushButton* setButton = new QPushButton(tr("Set"));
  setButton->setToolTip( tr("Click Set to take over the selected list entry.") );

  connect( setButton, SIGNAL(clicked()),
           SLOT(slotSetSelectedEntry()));

  QGridLayout* groupLayout = new QGridLayout;

  m_groupBox = new QGroupBox;
  m_groupBox->setLayout( groupLayout );

  int row = 0;
  groupLayout->addWidget( m_searchEntry, row, 0 );
  groupLayout->addWidget( clearButton, row, 1 );
  row++;
  groupLayout->addWidget( m_selectionBox, row, 0 );
  groupLayout->addWidget( setButton, row, 1 );
  groupLayout->setColumnStretch( 0, 5 );

  QHBoxLayout* hbox = new QHBoxLayout( this );
  hbox->setSpacing(0);
  hbox->addWidget( m_groupBox );
}

AirfieldSelectionList::~AirfieldSelectionList()
{
}

void AirfieldSelectionList::showEvent( QShowEvent *event )
{
  fillSelectionBox();
  QWidget::showEvent( event );
}

void AirfieldSelectionList::fillSelectionBox()
{
  int searchList[] = { MapContents::GliderfieldList, MapContents::AirfieldList };

  QString selectedItemText = m_selectionBox->currentText();

  m_airfieldDict.clear();
  m_searchEntry->clear();
  m_selectionBox->clear();

  QStringList airfieldList;

  for( int l = 0; l < 2; l++ )
    {
      for( int loop = 0; loop < _globalMapContents->getListLength(searchList[l]); loop++ )
      {
        SinglePoint *hitElement = (SinglePoint *) _globalMapContents->getElement(searchList[l], loop );
        airfieldList.append( hitElement->getName() );
        m_airfieldDict.insert( hitElement->getName(), hitElement );
      }
  }

  airfieldList.sort();
  m_selectionBox->addItems( airfieldList );

  // try to find the last selection in the new content.
  int newIndex = m_selectionBox->findText( selectedItemText );

  if( newIndex != -1 )
    {
      // Try to find the last selection.
      m_selectionBox->setCurrentIndex( newIndex );
    }
  else
    {
      m_selectionBox->setCurrentIndex( 0 );
    }
}

void AirfieldSelectionList::slotClearSearchEntry()
{
  m_searchEntry->clear();
  m_selectionBox->setCurrentIndex( 0 );
}

void AirfieldSelectionList::slotSetSelectedEntry()
{
  QString itemText = m_selectionBox->currentText();

  if( itemText.isEmpty() )
    {
      return;
    }

  const SinglePoint *sp = 0;

  if( m_airfieldDict.contains(itemText) )
    {
      sp = m_airfieldDict.value(itemText);

      if( sp != 0 )
	{
	  emit takeThisPoint( sp );
	}
    }
}

void AirfieldSelectionList::slotReturnPressed()
{
  slotSetSelectedEntry();
}

void AirfieldSelectionList::slotTextEdited( const QString& text )
{
  int idx = m_selectionBox->findText( text, Qt::MatchStartsWith );

   if( idx != -1 )
      {
	m_selectionBox->setCurrentIndex( idx );
      }
}
