/***********************************************************************
**
**   projectionbase.h
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

#ifndef PROJECTIONBASE_H
#define PROJECTIONBASE_H

/**
 * This class is used as a base-class for the map-projection. It will
 * be inherited by all classes, which implement one type of map-projection.
 *
 * @author Heiner Lamprecht
 * @version $Id$
 */
class ProjectionBase
{
  public:
    /** */
    ProjectionBase(int projectionType = Unknown);
    /** */
    virtual ~ProjectionBase();
    /** */
    const int projectionType() const {  return projType;  };
    /** */
    virtual double projectX(double latitude, double longitude) const;
    /** */
    virtual double projectY(double latitude, double longitude) const;
    /** */
    virtual double invertLat(double x, double y) const;
    /** */
    virtual double invertLon(double x, double y) const;
    /** */
    virtual double getRotationArc(int x, int y) const;
    /** */
    virtual int getTranslationX(int width, int x) const;
    /** */
    virtual int getTranslationY(int height, int y) const;
    /**
     * List of possible projection-types.
     */
    enum ProjectionType {Unknown = 0, Lambert, Cylindric};

  protected:
    /** */
    ProjectionType projType;
};

#endif
