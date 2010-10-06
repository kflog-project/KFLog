/***********************************************************************
**
**   mapprint.h
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

#ifndef MAPPRINT_H
#define MAPPRINT_H

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpainter.h>

/**
 * This class provides a dialog-page in a kprinterdialog. The page is
 * used to set the scale for printing the map as well as the page-header.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class MapPrintDialogPage : public QDialog
{
  public:
    /** */
    MapPrintDialogPage(QStringList sList, QWidget *parent=0,
        const char *name=0, bool printFlight = false);
    /** */
    ~MapPrintDialogPage();
    /** */
    void getOptions(QString *printScale, bool *printTitle, bool *printText, bool *printLegend);

  private:
    /** */
    QComboBox* scaleSelect;
    /** */
    QCheckBox* printLegend;
    /** */
    QCheckBox* printText;
    /** */
    QCheckBox* printTitle;
    /** */
    QLineEdit* titleInput;
    /** */
    QStringList scaleList;
};

/**
 * @author Heiner Lamprecht
 * @version $Id$
 */
class MapPrint
{
  public:
    /** */
    MapPrint(bool flightLoaded);
    /** */
    ~MapPrint();

  private:
    /** */
    MapPrintDialogPage* dialogPage;
    /** */
    double* scaleRange;
    /** */
    void __drawGrid(const double scale, QPainter* gridP, const QSize pS,
        const QRect mapBorder, const int mapCenterLon,
        const double dX, const double dY,
        const double gridLeft, const double gridRight,
        const double gridTop, const double gridBot);
    /** Prints the waypointlist to the supplied QPainter */
    void __drawWaypoints(const double selectedScale, QPainter* wpP, const QSize pS,
		const QRect mapBorder, const int mapCenterLon,
		const double dX, const double dY,
		const double gridLeft, const double gridRight,
		const double gridTop, const double gridBot);
    /** Prints the task, if defined, to the supplied QPainter */
/*    void __drawTask(const double selectedScale, QPainter* taskP, const QSize pS,
		const QRect mapBorder, const int mapCenterLon,
		const double dX, const double dY,
		const double gridLeft, const double gridRight,
		const double gridTop, const double gridBot);
 */
};

#endif
