/***********************************************************************
**
**   topolegend.h
**
**   This file is part of KFLog4.
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
 * Generates a legend for the topography levels used on the map.
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
  ~TopoLegend();

private:

  /**
   * Index of the currently highlighted elevation label
   */
   int currentHighlight;

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
   * Makes sure the indicated level is visible.
   */
  void highlightLevel( const int elevation );

};

#endif
