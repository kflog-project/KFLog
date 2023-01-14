/***********************************************************************
 **
 **   ThermalPoint.h
 **
 **   This file is part of KFlog
 **
 ************************************************************************
 **
 **   Copyright (c): 2023 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   License. See the file COPYING for more information.
 **
 ***********************************************************************/

/**
 * \class ThermalPoint
 *
 * \author Axel Pauli
 *
 * \brief Map element used for thermal points.
 *
 * Map element used for thermal point objects.
 *
 * \see BaseMapElement#objectType
 *
 * \date 2022-2023
 *
 * \version 1.0
 */

#pragma once

#include "singlepoint.h"

class ThermalPoint : public SinglePoint
{
 public:

  enum type {
    // The thermal type. Possible values:
    unknown=255,
    natural=0,
    artificial=1
  };

  enum reliability {
    // The thermal reliability. Possible values:
    unknownReliability=255,
    poor=0,
    fair=1,
    high=2,
    veryHigh=3
  };

  enum occurrence {
    // The thermal occurrence. Possible values:
    unknownOccurrence=255,
    irregularIntervals=0,
    scheduledInterval=1,
    nearlyConstant=2
  };

  enum category {
    // If set, the thermal is only suitable for the specified aircraft category.
    notUsed=255,
    Glider=0,
    HangGlider=1,
    Paraglider=2
  };

  /**
   * Default constructor.
   */
  ThermalPoint() :
    SinglePoint(),
    m_type(255),
    m_reliability(255),
    m_occurrence(255),
    m_category(255)
    {
      setTypeID( BaseMapElement::Thermal );
    }

 /**
   * Destructor
   */
  virtual ~ThermalPoint()
  {
  }

  void setType( const short type )
    {
      m_type = type;
    }

  short getType() const
    {
      return m_type;
    }

  void setReliability( const short reliability )
    {
      m_reliability = reliability;
    }

  short getReliability() const
    {
      return m_reliability;
    }

  void setOccurrence( const short occurrence )
    {
    m_occurrence = occurrence;
    }

  short getOccurrence() const
    {
      return m_occurrence;
    }

  void setCategory( const short category )
    {
      m_category = category;
    }

  short getCategory() const
    {
      return m_category;
    }

 protected:
  /**
   * The thermal type,
   */
  short m_type;

  /**
   * The thermal reliability.
   */
  short m_reliability;

  /**
   * The thermal occurrence.
   */
  short m_occurrence;

  /**
   *  If set, the thermal is only suitable for the specified aircraft category.
   */
  short m_category;
};

