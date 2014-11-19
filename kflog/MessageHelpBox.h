/***********************************************************************
**
**   MessageHelpBox.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2014 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class MessageHelpBox
 *
 * \author Axel Pauli
 *
 * \brief A dialog box for display of a help message.
 *
 * A dialog box for display of a help message with a checkbox to suppress
 * the display in the future, if the checkbox is checked by the user.
 *
 * The returned result code of the exec method of the dialog is the state of
 * the checkbox. 1 means checked, 0 means unchecked.
 *
 * \date 2014
 *
 * \version 1.0
 */

#ifndef MessageHelpBox_h
#define MessageHelpBox_h

#include <QCheckBox>
#include <QDialog>
#include <QString>
#include <QTextEdit>

class MessageHelpBox : public QDialog
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY ( MessageHelpBox )

 public:

  /**
   * Constructor.
   *
   * \param parent Pointer to parent widget.
   *
   * \param title The title of the dialog.
   */
  MessageHelpBox( QWidget *parent=0, QString title=""  );

  virtual ~MessageHelpBox();

  /**
   * Sets a title for the dialog.
   */
  void setTitle( QString title )
  {
    setWindowTitle( title );
  };

  /**
   * Shows the passed message in the dialog.
   *
   * \param message Message to be shown in the dialog.
   */
  void showMessage(const QString &message);

 protected:

  void showEvent( QShowEvent* event );

  /**
   * Overwriting of done method to give back an own return code. The return
   * code is reflecting the checkbox state. 1 means checked, 0 means unchecked.
   */
  void	done( int r );

  /**
   * Overwriting of reject method to prevent an unwanted dialog close.
   */
  void reject();

 private:

  QTextEdit *m_display;
  QCheckBox *m_showAgain;
};

#endif /* MessageHelpBox_h */
