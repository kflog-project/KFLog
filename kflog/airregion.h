/***********************************************************************
**
**   airregion.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2004      by André Somers
**                   2008-2010 by Axel pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: airregion.h 4502 2010-12-09 22:32:02Z axel $
**
***********************************************************************/

/**
 * \class AirRegion
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Container for \ref Airspace objects in a QPainterPath.
 *
 * \see Airspace
 *
 * Contains the projected region of an \ref airspace onto the map.
 * The Map class maintains a list to find the airspace data when
 * the users selects an airspace in the map or to check the nearness
 * to an airspace during the drawing and after a position change.
 *
 * This class overtakes the ownership of the region, but not of
 * the airspace!
 *
 * Due to the cross pointer reference to the airspace this class do not
 * allow copies and assignments of an existing instance.
 *
 * \date 2004-2010
 *
 */

#ifndef AirRegion_h
#define AirRegion_h

#include <QPainterPath>
#include <QPoint>

#include "airspace.h"

class AirRegion
{

private:

  /**
   * Don't allow copies and assignments.
   */
  AirRegion(const AirRegion& ) {};
  AirRegion& operator=(const AirRegion& ) {return *this;};

public:

    QPainterPath* region;
    Airspace* airspace;

    AirRegion( QPainterPath* reg, Airspace* air );
    virtual ~AirRegion();

    /**
     * Returns true if the given horizontal position conflicts with
     * the airspace properties
     * @param pos position in WSG coordinates
     * @param awd collection of distances to use for the warnings
     * @param changed set to true if the warning is different from the
     *                last one issued for this airspace
     * @returns a conflict type
     */
    Airspace::ConflictType conflicts( const QPoint& pos,
                                      const AirspaceWarningDistance& awd,
                                      bool* changed = 0 );

    /**
     * @returns the last known horizontal conflict for this airspace
     */
    Airspace::ConflictType currentConflict() const
    {
      return m_lastResult;
    }

private:
    /**
     * Create new regions around the current position to check for collisions
     */
    void createRegions();
    /**
     * @returns true if the position, the airspace warning distances or the
     * scale has changed. If so, the static parameters that contain these
     * values as well as ms_smallPostionChange are set to reflect the
     * new situation.
     */
    bool parametersChanged( const QPoint& pos, const AirspaceWarningDistance& awd );

    /** the result returned last time */
    Airspace::ConflictType m_lastResult;

    /** true if this is the first time this region is used */
    bool m_isNew;

    /**
     * Projected pos *this* instance has last been used with.
     * ms_lastProjPos is the static value, used for the whole class.
     * This value is used to check if we need to re-check for
     * horizontal conflicts.
     */
    QPoint m_lastProjPos;

private:
    // given values
    /** contains the last known position */
    static QPoint ms_lastPos;
    /** contains the last known scale */
    static double ms_lastScale;
    /** contains the last known set of warning distances */
    static AirspaceWarningDistance ms_lastAwd;

    // calculated values based on the values above
    /** contains a large circular region around ms_lastProjPos */
    static QPainterPath ms_regNear;
    /** contains a small circular region around ms_lastProjPos */
    static QPainterPath ms_regVeryNear;
    /** contains the last position projected */
    static QPoint ms_lastProjPos;
    /** true if the last position change was a small change */
    static bool ms_smallPositionChange;

};

#endif
