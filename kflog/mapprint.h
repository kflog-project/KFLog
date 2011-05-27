/***********************************************************************
**
**   mapprint.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAP_PRINT_H
#define MAP_PRINT_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QPainter>

/**
 * \class MapPrintDialogPage
 *
 * \author Heiner Lamprecht, Axel Pauli
 *
 * \brief Printer configuration dialog.
 *
 * This class provides a dialog page. The page is
 * used to set the scale for printing the map as well as the page header.
 *
 *
 * \date 2002-2011
 *
 * \version $Id$
 */
class MapPrintDialogPage : public QDialog
{
 public:

  MapPrintDialogPage( QStringList sList,
                      QWidget *parent=0,
                      bool printFlight = false );

  ~MapPrintDialogPage();

  /** */
  void getOptions( QString& printScale,
                   bool& printTitle,
                   bool& printText,
                   bool& printLegend );

 private:

  /** */
  QComboBox* scaleSelectBox;
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
