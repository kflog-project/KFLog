/***********************************************************************
**
**   radiopoint.cpp
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "radiopoint.h"

RadioPoint::RadioPoint(QString n, QString i, QString g, unsigned int t,
    QPoint pos, const char* f)
  : SinglePoint(n, g, t, pos),
    frequency(f), icao(i)
{

}

RadioPoint::~RadioPoint()
{

}

void RadioPoint::printMapElement(QPainter* printPainter) const
{

}
