/***********************************************************************
**
**   projectionlambert.h
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

#ifndef PROJECTIONLAMBERT_H
#define PROJECTIONLAMBERT_H

#include <projectionbase.h>

/**
 * This class provides a conical orthomorphic projection ("Lambert-projection").
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ProjectionLambert : public ProjectionBase
{
  public:
    /**
     * Creates a new object and sets the two standard parallels. The
     * default for the parallels are 54°N and 50°N.
     */
    ProjectionLambert(int v1 = 32400000, int v2 = 30000000, int origin = 0);
    /** */
    ~ProjectionLambert();
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
     * Sets the two standard parallels. If one of the parallels is
     * too large, the default values (54°N and 50°N) are used for both
     * of them.
     */
    void initProjection(int  v1, int v2, int origin);

  private:
    /**
     * First standard parallel.
     */
    double v1;
    /**
     * Second standard parallel.
     */
    double v2;
    /**
     * Value used for fasten up projecting.
     */
    double var1;
    /**
     * Value used for fasten up projecting.
     */
    double var2;
    /** */
    double origin;
};

#endif
