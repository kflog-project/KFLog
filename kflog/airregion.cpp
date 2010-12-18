/***********************************************************************
**
**   airregion.cpp
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2004      by André Somers
**                  2008-2010 by Axel pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <math.h>

#include "airregion.h"
#include "mapmatrix.h"
#include "mapcalc.h"

// Initialize the static class variables
bool   AirRegion::ms_smallPositionChange = false;
QPoint AirRegion::ms_lastPos = QPoint();
double AirRegion::ms_lastScale = -1.0;
AirspaceWarningDistance AirRegion::ms_lastAwd;
QPainterPath AirRegion::ms_regNear = QPainterPath();
QPainterPath AirRegion::ms_regVeryNear = QPainterPath();
QPoint  AirRegion::ms_lastProjPos = QPoint();


/**
 * @short Container for @ref Airspace objects with a QPainterPath
 * @author André Somers
 *
 * Contains the projected region of an airspace onto the map.
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
 */

AirRegion::AirRegion( QPainterPath* reg, Airspace* air )
{
  region   = reg;
  airspace = air;
  m_isNew = true;
  m_lastResult = Airspace::none;
  m_lastProjPos = QPoint(0,0);

  // set a reference to the related airspace instance
  airspace->setAirRegion(this);
}

AirRegion::~AirRegion()
{
  if ( region )
    {
      delete region;
    }

  // @AP: Remove the class pointer in related AirSpace object. We
  // have a cross reference here. First deleted object will reset
  // its pointer in the other object. Check is necessary to avoid
  // usage of null pointer.

  if ( airspace )
    {
      airspace->setAirRegion( static_cast<AirRegion*> (0) );
    }
}

/**
 * Returns true if the given horizontal position conflicts with the
 * airspace properties
 */
Airspace::ConflictType AirRegion::conflicts( const QPoint& pos,
                                             const AirspaceWarningDistance& awd,
                                             bool* changed )
  {
    //if the object is new, the last position is invalid, so we need a
    //full check
    bool fullCheck = m_isNew;

    if ( parametersChanged(pos, awd) )
      {
        //the parameters changed, so we need to re-create the regions.
        createRegions();
      }

    if ( ! m_isNew && ms_lastProjPos == m_lastProjPos )
      {
        //The parameters have not changed, so we can use what we have.
        //The last result must still be valid!
        if (changed)
          {
            *changed = false;
          }

        return m_lastResult;
      }

    //We need a full check if the position change is large.
    fullCheck |= !ms_smallPositionChange;

    // dont know why this happens, but it does.
    if ( ms_lastProjPos.x() > 10000 || ms_lastProjPos.x() < -10000 )
      {
        m_lastResult = Airspace::none;

        if (changed)
          {
            *changed = true;
          }
        //qDebug("returning no conflict because of invalid position");
        return Airspace::none;
      }

    Airspace::ConflictType hConflict = Airspace::none;
    m_lastProjPos = ms_lastProjPos;

    // check for horizontal conflicts
    if (fullCheck)
      {
        //we need to do a full check
        if ( region->contains(ms_lastProjPos) )
          {
            hConflict=Airspace::inside;
          }
        else if ( region->intersects(ms_regVeryNear) )
          {
            hConflict=Airspace::veryNear;
          }
        else if ( region->intersects(ms_regNear) )
          {
            hConflict=Airspace::near;
          }
        else
          {
            hConflict=Airspace::none;
          }
      }
    else
      {
        // we only need to check around the last result. This is a potentially
        // significant save, as intersecting regions is quite expensive.
        switch (m_lastResult)
          {
          case Airspace::inside:

            if (region->contains(ms_lastProjPos))
              {
                hConflict=Airspace::inside;
              }
            else if ( region->intersects(ms_regVeryNear) )
              {
                hConflict=Airspace::veryNear;
              };
            break;

          case Airspace::veryNear:

            if (region->contains(ms_lastProjPos))
              {
                hConflict=Airspace::inside;
              }
            else if ( region->intersects(ms_regVeryNear) )
              {
                hConflict=Airspace::veryNear;
              }
            else if ( region->intersects(ms_regNear) )
              {
                hConflict=Airspace::near;
              }
            break;

          case Airspace::near:

            if ( region->intersects(ms_regVeryNear) )
              {
                hConflict=Airspace::veryNear;
              }
            else if ( region->intersects(ms_regNear) )
              {
                hConflict=Airspace::near;
              }
            else
              {
                hConflict=Airspace::none;
              }
            break;

          case Airspace::none:

            if ( region->intersects(ms_regNear) )
              {
                hConflict=Airspace::near;
              }
            else
              {
                hConflict=Airspace::none;
              }
            break;

          default:
            break;
          }
      }

    if (changed)
      {
        *changed = (m_lastResult == hConflict);
      }

    m_lastResult = hConflict;
    m_isNew = false;
    // qDebug("horizontal conflict: %d, airspace: %s", hConflict, airspace->getName().latin1());
    return hConflict;
  }


bool AirRegion::parametersChanged(const QPoint& pos,
                                  const AirspaceWarningDistance& awd)
  {
    extern MapMatrix* _globalMapMatrix;

    if (pos == ms_lastPos &&
        ms_lastAwd == awd &&
        ms_lastScale == _globalMapMatrix->getScale(MapMatrix::CurrentScale))
      {
        return false;
      }

    // 30 meters is a small position change
    QPoint* p = &(const_cast<QPoint&>(pos));
    ms_smallPositionChange = (dist(p, &ms_lastPos) < 0.03);
    ms_lastPos = pos;
    ms_lastAwd = awd;
    ms_lastScale = _globalMapMatrix->getScale(MapMatrix::CurrentScale);

    //qDebug("parametersChanged, smallPositionChange: %d", ms_smallPositionChange);
    return true;
  }


void AirRegion::createRegions()
  {
    extern MapMatrix* _globalMapMatrix;

    ms_lastProjPos = _globalMapMatrix->map( _globalMapMatrix->wgsToMap(ms_lastPos) );

    Distance dist = ms_lastAwd.horClose * 2;
    int projDist = (int) rint(dist.getMeters()/ms_lastScale);

    ms_regNear = QPainterPath();
    ms_regNear.addEllipse( ms_lastProjPos.x()-projDist/2,
                           ms_lastProjPos.y()-projDist/2,
                           projDist, projDist );

    dist = ms_lastAwd.horVeryClose * 2;
    projDist = (int) rint(dist.getMeters()/ms_lastScale);

    ms_regVeryNear = QPainterPath();
    ms_regVeryNear.addEllipse( ms_lastProjPos.x()-projDist/2,
                               ms_lastProjPos.y()-projDist/2,
                               projDist, projDist );
  }
