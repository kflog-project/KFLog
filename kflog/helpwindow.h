/***********************************************************************
**
**   helpwindow.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**
***********************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <qframe.h>
#include <qtextbrowser.h>

/**Generates a HelpWindow.
  *@author Florian Ehinger
  */



class HelpWindow : public QFrame
{
   Q_OBJECT
   
   public: 
    HelpWindow(QWidget *parent);
    ~HelpWindow();

  protected:
  
  public slots:
    /**
      * Show the helptext
    	*/
    void slotShowHelpText(QString text);
    /**
      * Clears the contents of the HelpWindow
      */
    void slotClearView();
    
  private:
    /**
     * The textwidget
     */
    QTextBrowser* helpText;

};

#endif
