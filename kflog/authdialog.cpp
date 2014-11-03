/***********************************************************************
**
**   authdialog.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/*
 * This widget ask for a user name and a password. It is used to get these
 * data on request.
 *
 */
#ifdef QT_5
    #include <QtWidgets>
#else
    #include <QtGui>
#endif

#include "authdialog.h"

AuthDialog::AuthDialog( QString &user, QString &password,
                        QString title, QWidget *parent ) :
  QDialog(parent),
  _user(user),
  _password(password)
{
  _user = "";
  _password = "";

  setObjectName( "AuthDialog" );
  setWindowTitle( title );
  setAttribute( Qt::WA_DeleteOnClose );
  setSizeGripEnabled( true );

  userEdit     = new QLineEdit;
  passwordEdit = new QLineEdit;
  passwordEdit->setEchoMode(QLineEdit::Password);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( new QLabel(tr("User:")), userEdit );
  formLayout->addRow( new QLabel(tr("Password:")), passwordEdit );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Cancel |
                                                      QDialogButtonBox::Ok );
  buttonBox->setCenterButtons( true );
  QPushButton *ok =  buttonBox->button( QDialogButtonBox::Ok );
  ok->setDefault( true );

  QPushButton *cancel =  buttonBox->button( QDialogButtonBox::Cancel );
  cancel->setAutoDefault(false);

  QVBoxLayout *vBox = new QVBoxLayout;

  vBox->addLayout( formLayout );
  vBox->addWidget( buttonBox );

  setLayout( vBox );

  connect( buttonBox, SIGNAL(accepted()), this, SLOT(accept()) );
  connect( buttonBox, SIGNAL(rejected()), this, SLOT(reject()) );

  // Set minimum size of input line
  QFontMetrics fm( font() );
  int strWidth = fm.width(QString("***************"));
  int strWidthTiltle = fm.width( windowTitle() );

  if( strWidth >= strWidthTiltle )
    {
      userEdit->setMinimumWidth( strWidth );
    }
  else
    {
      userEdit->setMinimumWidth( strWidthTiltle );
    }

  //userEdit->setMaximumWidth( strWidth );
}

AuthDialog::~AuthDialog()
{
  // qDebug("AuthDialog::~AuthDialog()");
}

/** User has pressed Ok button */
void AuthDialog::accept()
{
  // Set results to input text
  _user     = userEdit->text();
  _password = passwordEdit->text();

  // close and destroy dialog
  QDialog::done(QDialog::Accepted);
}

/** User has pressed Cancel button */
void AuthDialog::reject()
{
  // close and destroy dialog
  QDialog::done(QDialog::Rejected);
}
