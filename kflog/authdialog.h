/***********************************************************************
**
**   authdialog.h
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

/**
 * \class AuthDialog
 *
 * \author Axel Pauli
 *
 * \brief User and password query dialog.
 *
 * This widget ask for a user name and a password. It is used to get these
 * data on request.
 *
 * \date 2010
 */

#ifndef AUTH_DIALOG_H
#define AUTH_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QString>

class AuthDialog : public QDialog
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AuthDialog )

 public:

  AuthDialog( QString &user, QString &password, QString title="", QWidget *parent = 0 );

  virtual ~AuthDialog();

 protected:

  /** standard slots */
  virtual void accept();
  virtual void reject();

 private:

  /** Return instance for the user name */
  QString &_user;

  /** Return instance for the user password */
  QString &_password;

  /** Editor for user input */
  QLineEdit *userEdit;

  /** Editor for password input */
  QLineEdit *passwordEdit;

};

#endif /* AUTH_DIALOG_H */
