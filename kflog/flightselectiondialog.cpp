/***********************************************************************
**
**   flightselectiondialog.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Harald Maier
**                   2011 by Axel Pauli
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

#include "flightselectiondialog.h"
#include "mainwindow.h"

extern MainWindow *_mainWindow;

FlightSelectionDialog::FlightSelectionDialog(QWidget *parent) :
  QDialog(parent)
{
  setObjectName("FlightSelectionDialog");
  setWindowTitle( tr("Flight Selection") );
  setModal(true);

  __initDialog();

  setMinimumWidth(400);
  setMinimumHeight(300);
}

FlightSelectionDialog::~FlightSelectionDialog()
{
}

/** No descriptions */
void FlightSelectionDialog::__initDialog()
{
  QLabel *l;
  QPushButton *b;

  QVBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setMargin(10);

  QVBoxLayout *leftLayout = new QVBoxLayout;
  leftLayout->setSpacing(5);
  QVBoxLayout *middleLayout = new QVBoxLayout;
  middleLayout->setSpacing(5);
  QVBoxLayout *rightLayout = new QVBoxLayout;
  rightLayout->setSpacing(5);

  QHBoxLayout *topGroup = new QHBoxLayout;
  topGroup->setSpacing(10);
  QHBoxLayout *buttons = new QHBoxLayout;
  buttons->setSpacing(10);
  QHBoxLayout *smallButtons = new QHBoxLayout;
  smallButtons->setSpacing(5);

  buttons->addStretch(5);

  b = new QPushButton(tr("&Ok"), this);
  b->setDefault(true);
  connect(b, SIGNAL(clicked()), SLOT(slotAccept()));
  buttons->addWidget(b);
  b = new QPushButton(tr("&Cancel"), this);
  connect(b, SIGNAL(clicked()), SLOT(reject()));
  buttons->addWidget(b);

  smallButtons->addStretch();
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_up_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveUp()));
  smallButtons->addWidget(b);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_down_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveDown()));
  smallButtons->addWidget(b);
  smallButtons->addStretch();

  aFlights = new QListWidget;
  l = new QLabel(tr("&available Flights"));
  l->setBuddy(aFlights);
  leftLayout->addWidget(l);
  leftLayout->addWidget(aFlights);

  middleLayout->addStretch();
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_forward_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotAddOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_2rightarrow_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotAddAll()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_back_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveOne()));
  middleLayout->addWidget(b);
  b = new QPushButton(this);
  b->setIcon(_mainWindow->getPixmap("kde_2leftarrow_16.png"));
  connect(b, SIGNAL(clicked()), SLOT(slotMoveAll()));
  middleLayout->addWidget(b);
  middleLayout->addStretch();

  sFlights = new QListWidget;
  l = new QLabel(tr("&selected Flights"));
  l->setBuddy(sFlights);
  rightLayout->addWidget(l);
  rightLayout->addWidget(sFlights);
  rightLayout->addLayout(smallButtons);

  topGroup->addLayout(leftLayout);
  topGroup->addLayout(middleLayout);
  topGroup->addLayout(rightLayout);
  topLayout->addLayout(topGroup);
  topLayout->addLayout(buttons);
}

void FlightSelectionDialog::slotAccept()
{
  if( selectedFlights.count() > 0 )
    {
      accept();
    }
  else
    {
       if( QMessageBox::Yes == QMessageBox::warning( this,
                                  tr("No flights selected"),
                                  "<html>" +
                                  tr("No flights are selected, so a flight<br>"
                                     "group cannot be created or modified!<br><br>"
                                     "Exit dialog?") +
                                  "</html>",
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes ) )
        reject();
    }
}

/** No descriptions */
void FlightSelectionDialog::slotAddOne()
{
  QListWidgetItem *item = aFlights->currentItem();
  int row = aFlights->currentRow();

  if( item )
    {
      QListWidgetItem * it = aFlights->takeItem( row );
      sFlights->addItem(it);
      selectedFlights.append(availableFlights.takeAt(row));
    }
}

/** No descriptions */
void FlightSelectionDialog::slotAddAll()
{
  for( int i = 0; i < availableFlights.count(); i++ )
    {
      BaseFlightElement *bfe = availableFlights.at(i);
      selectedFlights.append(bfe);
      sFlights->addItem( new QListWidgetItem( bfe->getFileName() ) );
    }

  aFlights->clear();
  availableFlights.clear();
}

/** No descriptions */
void FlightSelectionDialog::slotMoveOne()
{
  QListWidgetItem *item = sFlights->currentItem();
  int row = sFlights->currentRow();

  if( item )
    {
      QListWidgetItem * it = sFlights->takeItem( row );
      aFlights->addItem(it);
      availableFlights.append(selectedFlights.takeAt(row));
    }
}

/** No descriptions */
void FlightSelectionDialog::slotMoveAll()
{
  for( int i = 0; i < selectedFlights.count(); i++)
    {
      BaseFlightElement *bfe = selectedFlights.at(i);
      availableFlights.append(bfe);
      aFlights->addItem( new QListWidgetItem( bfe->getFileName() ) );
    }

  sFlights->clear();
  selectedFlights.clear();
}

/** No descriptions */
void FlightSelectionDialog::slotMoveUp()
{
  QListWidgetItem *item = sFlights->currentItem();
  int row = sFlights->currentRow();

  if( item && row != 0 )
    {
      QListWidgetItem *it = sFlights->takeItem( row );
      sFlights->insertItem( row - 1, it );
      sFlights->setCurrentItem(it);

      BaseFlightElement *bfe = selectedFlights.takeAt(row);
      selectedFlights.insert(row - 1, bfe);
    }
}

/** No descriptions */
void FlightSelectionDialog::slotMoveDown()
{
  QListWidgetItem *item = sFlights->currentItem();
  int row = sFlights->currentRow();

  if( item && row < (sFlights->count() - 1) )
    {
      QListWidgetItem *it = sFlights->takeItem( row );
      sFlights->insertItem( row + 1, it );
      sFlights->setCurrentItem(it);

      BaseFlightElement *bfe = selectedFlights.takeAt(row);
      selectedFlights.insert(row + 1, bfe);
    }
}

/** Loads the data of the list boxes before every show. */
void FlightSelectionDialog::showEvent( QShowEvent *event )
{
  Q_UNUSED( event )

  BaseFlightElement *bfe;

  aFlights->clear();
  sFlights->clear();

  for (int i = 0; i < availableFlights.count(); i++)
    {
      bfe = availableFlights.at(i);
      aFlights->addItem( new QListWidgetItem( bfe->getFileName() ) );
    }

  for (int i = 0; i < selectedFlights.count(); i++)
    {
      bfe = selectedFlights.at(i);
      sFlights->addItem(new QListWidgetItem( bfe->getFileName() ) );
    }
}
