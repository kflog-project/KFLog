/***********************************************************************
**
**   recorderdialog.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef RECORDERDIALOG_H
#define RECORDERDIALOG_H

#include <frstructs.h>

#include <kcombobox.h>
#include <kconfig.h>
#include <kdialogbase.h>
#include <klistview.h>

#include <qcheckbox.h>
#include <qstringlist.h>
#include <qwidget.h>

/**
 * Provides a dialog-window for accessing the flightrecorder.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class RecorderDialog : public KDialogBase
{
  Q_OBJECT

  public:
    /** */
    RecorderDialog(QWidget *parent, KConfig* cnf, const char *name);
    /** */
    ~RecorderDialog();

  public slots:
    /** */
    void slotConnectRecorder();
    /** */
    void slotReadFlightList();
    /** */
    void slotDownloadFlight();

  private:
    /** */
    int __fillDirList();
    /** */
    int __openLib(QString libN);
    /** */
    void __addSettingsPage();
    /** */
    void __addFlightPage();
    /** */
    void __addTaskPage();
    /** */
    QFrame* flightPage;
    /** */
    QFrame* settingsPage;
    /** */
    QFrame* taskPage;
    /** */
    KConfig* config;
    /** */
    KConfig* loggerConf;
    /** */
    KComboBox* selectType;
    KComboBox* selectPort;
    /** */
    QLabel* serID;
    QLabel* apiID;
    /** */
    KListView* flightList;
    /** */
    KListView* taskList;
    /** */
    QCheckBox* useFastDownload;
    /** */
    QCheckBox* useLongNames;
    /** */
    void* libHandle;
    /** */
    QString libName;
    /** */
    QString portName;
    /** */
    bool isOpen;
    /** */
    QList<FRDirEntry> dirList;
    /** */
    QStringList libNameList;
    /** */
    int colID;
    int colDate;
    int colPilot;
    int colGlider;
    int colFirstPoint;
    int colLastPoint;
    /** */
    int taskColID;
    int taskColName;
    int taskColPosition;
};

#endif
