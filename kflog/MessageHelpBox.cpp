/***********************************************************************
**
**   MessageHelpBox.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014-2023 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>

#include "MessageHelpBox.h"

MessageHelpBox::MessageHelpBox( QWidget *parent, QString title ) :
  QDialog(parent)
{
  setWindowTitle( title );
  setModal( true );
  //setAttribute( Qt::WA_DeleteOnClose );
  setSizeGripEnabled( true );

  QGridLayout * grid = new QGridLayout(this);

  QLabel *icon = new QLabel(this);
  icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Information));
  icon->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
  grid->addWidget(icon, 0, 0, Qt::AlignTop);

  m_display = new QTextEdit(this);
  m_display->setReadOnly( true );
  grid->addWidget(m_display, 0, 1);

  m_showAgain = new QCheckBox(this);
  m_showAgain->setChecked(true);
  m_showAgain->setText(tr("&Show this message again"));
  grid->addWidget(m_showAgain, 1, 1, Qt::AlignTop);

  QPushButton *ok = new QPushButton(this);
  ok->setText(tr("&OK"));
  ok->setFocus();

  grid->addWidget(ok, 2, 0, 1, 2, Qt::AlignCenter);
  grid->setColumnStretch(1, 42);
  grid->setRowStretch(0, 42);

  connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
}

MessageHelpBox::~MessageHelpBox()
{
}

void MessageHelpBox::showMessage(const QString &message)
{
  m_display->setMinimumSize(500, 300);
  m_display->setHtml( message );
  show();
}

void MessageHelpBox::showEvent( QShowEvent* event )
{
  QDialog::showEvent( event );
}

void MessageHelpBox::done( int /* r */ )
{
  if( m_showAgain->checkState() == Qt::Checked )
    {
      QDialog::done( 1 );
    }
  else
    {
      QDialog::done( 0 );
    }
}

void MessageHelpBox::reject()
{
  // Prevent dialog closing by catching reject call in base class.
  return;
}
