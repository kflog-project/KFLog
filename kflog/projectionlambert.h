/***********************************************************************
**
**   projectionlambert.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2007 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef PROJECTIONLAMBERT_H
#define PROJECTIONLAMBERT_H

#include "projectionbase.h"

/**
 * This class provides a conical orthomorphic projection ("Lambert-projection").
 *
 */

class ProjectionLambert : public ProjectionBase
{

public:

  /**
   * Creates a new object and sets the two standard parallels.
   */
  ProjectionLambert(int, int, int);
  ProjectionLambert(QDataStream &);

  /** */
  virtual ~ProjectionLambert();

  /**
   * @Returns the initial value
   */
  const int getStandardParallel1() const
  {
    return i_v1;
  };

  /**
   * @Returns the initial value
   */
  const int getStandardParallel2() const
  {
    return i_v2;
  };

  /**
   * @Returns the initial value
   */
  const int getOrigin() const
  {
    return i_origin;
  };

  /** */
  virtual const ProjectionType projectionType() const
  {
    return Lambert;
  };
    
  /**
   * returns the x-position.
   *
   * @param  latitude  The latitude of the position, given in radiant.
   * @param  longitude  The longitude of the position, given in radiant.
   */
  virtual double projectX(const double& latitude, const double& longitude) ;

  /**
   * Returns the y-position.
   *
   * @param  latitude  The latitude of the position, given in radiant.
   * @param  longitude  The longitude of the position, given in radiant.
   */
  virtual double projectY(const double& latitude, const double& longitude) ;

  /**
   * Returns the latitude of a given projected position in radiant.
   */
  virtual double invertLat(const double& x, const double& y) const;

  /**
   * Returns the longitude of a given projected position in radiant.
   */
  virtual double invertLon(const double& x, const double& y) const;

  /** */
  virtual double getRotationArc(const int x, const int y) const;

  /** */
  virtual int getTranslationX(const int width, const int x) const;

  /** */
  virtual int getTranslationY(const int height, const int y) const;

  /**
   * Sets the two standard parallels. If one of the parallels is
   * too large, the default values (54°N and 50°N) are used for both
   * of them.
   */
  bool initProjection(int  v1, int v2, int origin);

  /**
   * Saves the parameters specific to this projection to a stream
   */
  virtual void saveParameters(QDataStream &);

  /**
   * Loads the parameters specific to this projection from a stream
   */
  virtual void loadParameters(QDataStream &);

 private:
  /**
   * First standard parallel.
   */
  double v1;

  int i_v1;

  /**
   * Second standard parallel.
   */
  double v2;

  int i_v2;

  /**
   * Value used to make projection inversion quicker.
   */

  double var1; // = sinv1 + sinv2 = 2 * projection scale factor
  /**
   * Value used to make projection inversion quicker.
   */
  double var2; // = -(1 + sinv1sinv2)/var1

  /**
   * Value used to make projection inversion quicker.
   */
  double var3; // = var1/4

  /**
   * Value used to make projection inversion quicker.
   */
  double var4; // = 1.0/(2.0*var3)

  /**
   * Trig values for standard parallels.
   */
  double sinv1, sinv2, cosv1, cosv2;

  /**
   * Squares of trig values to use in calculations
   */
  double sinv1_2, sinv2_2, cosv1_2;

  /**
   * auxiliary parameters used in inverse projection equations
   */
  double project_XY_arg_lat, last_lat, project_XY_arg_lon, last_lon;

  /**
   * Projection origin longitude - helps keep it looking vertical
   */
  double origin;

  int i_origin;
};

#endif
