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

/**
 * @author Heiner Lamprecht, Florian Ehinger
 */
class Isohypse : public LineElement
{
  public:
    /** */
    Isohypse(QPointArray pA, unsigned int elev, bool isValley, int s_ID);
    /** */
    ~Isohypse();
   /** */
    virtual void drawMapElement(QPainter* targetP, QPainter* maskP);
    /** */
    int getElevation() const;
    /** */
    int sortID() const;

  private:
    /** */
    int elevation;
    /** true, wenn Element ein Tal ist */
    bool valley;
    /** Enthält den Index der Liste, in die das Element gehört ... */
    int sort_ID;
};

#endif
