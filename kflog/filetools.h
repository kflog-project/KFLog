/***********************************************************************
**
**   filetools.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <QDataStream>
#include <QByteArray>
#include <QString>
#include <QPolygon>

/**
 * This file contains a set of functions that can be used to write
 * and read more compact binary files than those written by default
 * by Qt. Reductions of over 50% can be reached.
 */

/**
 * Save a QByteArray  to a stream with only one leading byte instead of 4.
 */
void ShortSave (QDataStream &, const QByteArray&);

/**
 * Save a QString to a stream with only one leading byte instead of 4.
 */
void ShortSave (QDataStream &, const QString&);

/**
 * Save a QPolygon to a stream in a shorter form.
 */
void ShortSave (QDataStream &, const QPolygon&);

/**
 * Load a QByteArray from a stream with only one leading byte instead of 4.
 */
void ShortLoad (QDataStream &, QByteArray&);

/**
 * Load a QString from a stream with only one leading byte instead of 4.
 */
void ShortLoad (QDataStream &, QString&);

/**
 * Load a QPolygon in a short form from a stream.
 */
void ShortLoad (QDataStream &, QPolygon&);

