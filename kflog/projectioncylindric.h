/***********************************************************************
**
**   projectioncylindric.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PROJECTIONCYLINDRIC_H
#define PROJECTIONCYLINDRIC_H

#include "projectionbase.h"

/**
 * This class provides a cylindrical projection.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ProjectionCylindric : public ProjectionBase
{
  public:
    /** */
    ProjectionCylindric(double v1 = 27000000.0);
    /** */
    ~ProjectionCylindric();
    /**
     * returns the x-position.
     *
     * @param  latitude  The latitude of the position, given in radiant.
     * @param  longitude  The longitude of the position, given in radiant.
     */
    virtual double projectX(double latitude, double longitude) const;
    /**
     * Returns the y-position.
     *
     * @param  latitude  The latitude of the position, given in radiant.
     * @param  longitude  The longitude of the position, given in radiant.
     */
    virtual double projectY(double latitude, double longitude) const;
    /**
     * Returns the latitude of a given projected position in radiant.
     */
    virtual double invertLat(double x, double y) const;
    /**
     * Returns the longitude of a given projected position in radiant.
     */
    virtual double invertLon(double x, double y) const;
    /** */
    virtual double getRotationArc(int x, int y) const;
    /** */
    virtual int getTranslationX(int width, int x) const;
    /** */
    virtual int getTranslationY(int height, int y) const;
    /**
     * Sets the standard parallel. If it is too large,
     * the default value (45°N) is used.
     */
    bool initProjection(int v1);

  private:
    /**
     * The standard parallel.
     */
    double v1;
};

#endif
