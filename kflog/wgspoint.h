/***********************************************************************
**
**   wgspoint.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/
/**
 * This class is used to handle WGS-coordinates. It inherits QPoint.
 * The only difference is, that the methods to access the coordinates
 * are called "lat" and "lon".
 */

#ifndef WGSPOINT_H
#define WGSPOINT_H

#include <QPoint>

class WGSPoint : public QPoint
{
  public:
    /**
     * Creates a new, empty WGSPoint.
     */
    WGSPoint();
    /**
     * Creates a new WGSPoint with the given position-data.
     */
    WGSPoint(int lat, int lon);
    /**
     * Returns the latitude in the internal format.
     */
    inline int lat() const  {  return x();  };
    /**
     * Returns the longitude in the internal format.
     */
    inline int lon() const  {  return y();  };
    /**
     * Sets the latitude.
     */
    inline void setLat(int lat)  {  setX(lat);  };
    /**
     * Sets the longitude.
     */
    inline void setLon(int lon)  {  setY(lon);  };
    /**
     * Sets the position.
     */
    inline void setPos(int lat, int lon)  {  setX(lat);  setY(lon);  };
    /**
     *
     */
    WGSPoint &operator=( const QPoint &p );
    /**
     *
     */
    friend inline bool operator==( const WGSPoint &pA, const WGSPoint &pB )
        {  return ( (QPoint)pA == (QPoint)pB );  };
    /**
     *
     */
    friend inline bool operator!=( const WGSPoint &pA, const WGSPoint &pB )
        {  return ( (QPoint)pA != (QPoint)pB );  };;
};

#endif
