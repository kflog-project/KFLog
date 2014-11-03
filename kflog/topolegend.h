/***********************************************************************
**
**   topolegend.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class TopoLegend
 *
 * \short Elevation profile
 *
 * Generates a legend for the elevation profile used on the map.
 *
 * \author André Somers, Axel Pauli
 *
 * \date 2003-2011
 *
 * $Id$
 */

#ifndef TOPO_LEGEND_H
#define TOPO_LEGEND_H

#include <QHash>
#include <QLabel>
#include <QList>
#include <QScrollArea>
#include <QWidget>

class TopoLegend : public QWidget
{
   Q_OBJECT

private:

  Q_DISABLE_COPY ( TopoLegend )

public:
  /**
   * Constructor. Normal QWidget parameters are used.
   */
  TopoLegend( QWidget *parent = 0 );

  /**
   * Destructor
   */
  virtual ~TopoLegend();

private:

  /**
   * Index of the currently selected elevation item.
   */
   int selectedItem;

 /**
  * Contains a list of the labels that have been created
  */
  QList<QLabel *> labelList;

  /**
   * Contains the scrollable widget.
   */
  QScrollArea* scrollArea;

  /** Hash table with elevation in meters as key and related elevation
   * index as value
   */
  QHash<short, short> isoHash;

public slots:
  /**
   * Makes sure the selected elevation level is visible.
   *
   * \param elevation Elevation in unit meter.
   */
  void slotSelectElevation( int elevation );

  /**
   * Makes an update of the colors of the elevation bar.
   */
  void slotUpdateElevationColors();

};

#endif
