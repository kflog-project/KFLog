/***********************************************************************
**
**   helpwindow.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Florian Ehinger
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**  $Id$
**
***********************************************************************/

/**
 * \class HelpWindow
 *
 * \author Florian Ehinger, Axel Pauli
 *
 * \brief This class provides a widget usable as help browser.
 *
 * Creates a help browser widget as single window and displays
 * the help text into it.
 *
 * \date 2008-2011
 *
 * \version $Id$
 */

#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <QWidget>

class QString;
class QTextBrowser;

class HelpWindow : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( HelpWindow )

 public:

  HelpWindow( QWidget *parent=0 );

  virtual ~HelpWindow();

 public slots:

  /**
    * Shows the help text.
    */
  void slotShowHelpText( QString& text );

  /**
    * Clears the contents of the HelpWindow.
    */
  void slotClearView();

 private:
  /**
   * The text browser
   */
  QTextBrowser* browser;
};

#endif
