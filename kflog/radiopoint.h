/***********************************************************************
**
**   radiopoint.h
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

#ifndef RADIOPOINT_H
#define RADIOPOINT_H

#include <singlepoint.h>

/**
  * This class provides a mapelement for radio-navigation-facilities. It is
  * derived from SinglePoint. This class is used for: VOR, VORDME, VORTAC
  * and NDB.
  *
  * @see BaseMapElement#objectType
  *
  * @author Heiner Lamprecht, Florian Ehinger
  * @version $Id$
  */
class RadioPoint : public SinglePoint
{
 public:
  /**
    * Creates a new radio-point. n is the name of the element, t is the
    * typeID, latPos and lonPos give the position of the element, f is the
	  * frequency and a is the alias.
    */
  RadioPoint(QString n, QString abbr, unsigned int t, QPoint pos,
      const char* f, const char* a, bool wP=false);
  /**
    * Destructor, does nothing special.
    */
  ~RadioPoint();
  /** */
  virtual void printMapElement(QPainter* printPainter, const double dX,
        const double dY, const int mapCenterLon, const double scale,
        const struct elementBorder mapBorder);

 protected:
  /** The frequency */
  QString frequency;
  /** The alias */
  QString alias;
};

/*************************************************************************
 *
 * Die Ein- und Ausgabeoperatoren
 *
 *************************************************************************/
//QDataStream& operator<<(QDataStream& outStream, const RadioPoint& rPoint);
//QTextStream& operator<<(QTextStream& outStream, const RadioPoint& rPoint);
//QDataStream& operator>>(QDataStream& outStream, RadioPoint& rPoint);

#endif
