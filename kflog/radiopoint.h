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

#include "singlepoint.h"

/**
 * This struct contains the data of one frequency;
 */
struct radioContact
{
  /** */
  QString frequency;
  /** */
  QString callSign;
  /**
   * @see KFLog#ContactType
   */
  unsigned int type;
};

/**
 * This class provides a mapelement for radio-navigation-facilities. It is
 * derived from SinglePoint. This class is used for: VOR, VORDME, VORTAC,
 * NDB and CompPoint.
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
     * Creates a new radio-point.
     *
     * @param  name  The name
     * @param  icao  The icao-name
     * @param  gps  The abbreviation, used for the gps-logger
     * @param  typeID  The typeid
     * @param  pos  The projected position
     * @param  wgsPos  The original WGS-position
     * @param  frequency  The frequency
     */
    RadioPoint(QString name, QString icao, QString gps, unsigned int typeID,
        WGSPoint wgsP, QPoint pos, const char* frequency, int elevation = 0);
    /**
     * Destructor
     */
    ~RadioPoint();
    /**
     * Prints the element. Reimplemented from BaseMapElement.
     *
     * @param  printP  The painter to draw the element into.
     *
     * @param  isText  Shows, if the text of some mapelements should
     *                 be printed.
     */
    virtual void printMapElement(QPainter* printPainter, bool isText);

    /* return frequency */
    virtual QString getFrequency() const;
    /* return ICAO name */
    virtual QString getICAO() const;
  protected:
    /**
     * The frequency
     */
    QString frequency;
    /**
     * The icao-name
     */
    QString icao;
};

#endif
