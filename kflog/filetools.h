/***********************************************************************
**
**   filetools.h
**
**   This file is part of KFlog2.
**
************************************************************************
**
**   Copyright (c):  2004 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <qdatastream.h>
#include <qcstring.h>
#include <qstring.h>
#include <qpointarray.h>

/**
 * This file contains a set of functions that can be used to write
 * and read more compact binary files than those written by default
 * by Qt. Reductions of over 50% can be reached.
 */

/**
 * Save a QCString to a stream with only one leading byte instead of 4.
 */
void ShortSave (QDataStream &, const QCString&);

/**
 * Save a QString to a stream with only one leading byte instead of 4.
 */
void ShortSave (QDataStream &, const QString&);

/**
 * Save a QPointArray to a stream in a shorter form.
 */
void ShortSave (QDataStream &, const QPointArray&);

/**
 * Load a QCString from a stream with only one leading byte instead of 4.
 */
void ShortLoad (QDataStream &, QCString&);

/**
 * Load a QString from a stream with only one leading byte instead of 4.
 */
void ShortLoad (QDataStream &, QString&);

/**
 * Load a QPointArray in a short form from a stream.
 */
void ShortLoad (QDataStream &, QPointArray&);

