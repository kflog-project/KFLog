/***********************************************************************
**
**   isohypse.h
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

#ifndef ISOHYPSE_H
#define ISOHYPSE_H

#include <lineelement.h>

#include <qdatastream.h>
#include <qtextstream.h>

/**
  *@author Heiner Lamprecht, Florian Ehinger
  */
class Isohypse : public LineElement
{
  public:
    /** */
    Isohypse();
    /** */
    Isohypse(QPointArray pA, unsigned int elev, bool isValley, int s_ID);
    /** */
    ~Isohypse();
   /** */
    virtual void drawMapElement(QPainter* targetPainter);
    /** */
    int getElevation() const;
    /** */
    bool isValley() const;
    /** */
    void setValley(bool isV);
    /** */
    int sortID() const;
    /** */
    void setElevation(int elevation);
    /** */
    void setSortID(int sort);
    /** */
    void setValues(int height, int sort, bool isV);

  protected:
    /** */
    int elevation;
    /** true, wenn Element ein Tal ist */
    bool valley;
    /** Enthält den Index der Liste, in die das Element gehört ... */
    int sort_ID;
};

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const Isohypse& iso);
//QTextStream& operator<<(QTextStream& outStream, const Isohypse& iso);
//QDataStream& operator>>(QDataStream& outStream, Isohypse& iso);

#endif
