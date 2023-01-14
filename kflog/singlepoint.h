/***********************************************************************
**
**   singlepoint.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

/**
 * \class SinglePoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \brief Map element used for small point objects.
 *
 * Map element used for small point objects like obstacles and reporting points.
 *
 * \date 2000-2023
 *
 * \version 1.2
 */

#pragma once

#include "basemapelement.h"
#include "wgspoint.h"

class SinglePoint : public BaseMapElement
{
  public:

  /**
   * Default constructor.
   */
  SinglePoint();

  /**
   * Creates a new "SinglePoint".
   *
   * @param  name  The name
   * @param  shortName An alias-name, used for the gps-logger
   * @param  typeID  The typeid
   * @param  pos  The projected position
   * @param  wgsPos  The original WGS-position
   * @param  elevation The elevation of the point when available
   * @param  comment An additional comment related to the single point
   * @param  country The country where the point is located.
   * @param  secID  The map section ID
   * @param compulsory The compulsory of the single point
   * @param  lmTyp Additional field (eg. for the population index for cities or the lm_typ)
   */
  SinglePoint( const QString& name,
               const QString& shortName,
               const BaseMapElement::objectType typeID,
               const WGSPoint& wgsPos,
               const QPoint& pos,
               const float elevation = 0.0,
               const QString& comment = "",
               const QString& country = "",
               const unsigned short secID = 0,
               const bool compulsory = false,
               unsigned int lmType = 0 );
  /**
   * Destructor
   */
  virtual ~SinglePoint();

  /**
   * Draws the element into the given painter. Reimplemented from
   * BaseMapElement.
   *
   * @param  targetP  The painter to draw the element into.
   */
  virtual bool drawMapElement(QPainter* targetP);

  /**
   * Prints the element. Reimplemented from BaseMapElement.
   *
   * @param  printP  The painter to draw the element into.
   *
   * @param  isText  Shows, if the text of some mapelements should
   *                 be printed.
   */
  virtual void printMapElement(QPainter* printP, bool isText);

  /**
  * Return A short html-info-string about the point, containing the
  * name, the alias and the elevation.
  *
  * @return the info string
  */
  virtual QString getInfoString();

  /**
   * @return the projected position of the element.
   */
  virtual QPoint getPosition() const
    {
      return position;
    };

  /**
   * Set the projected position of the element.
   */
  virtual void setPosition( const QPoint& newPos )
    {
      position = newPos;
    };

  /**
   * @return the WGSposition of the element. (normales Lat/Lon System)
   */
  virtual WGSPoint getWGSPosition() const
    {
      return wgsPosition;
    };

  /**
   * @return The WGS position of the element in kflog format.
   */
  virtual WGSPoint* getWGSPositionPtr()
    {
      return &wgsPosition;
    };

  /**
   * @return The WGS position of the element in kflog format.
   */
  virtual WGSPoint& getWGSPositionRef()
    {
      return wgsPosition;
    };

  /**
   * @param newPos The new WGS position of the element in kflog format.
   */
  virtual void setWGSPosition( const WGSPoint& value )
    {
      wgsPosition = value;
    };

  /**
   * @return the  short name of the element.
   */
  virtual QString getShortName() const
    {
      return shortName;
    };

  /**
   * @param newName The new short name of the element.
   */
  virtual void setShortName( const QString& newName )
    {
      shortName = newName;
    };

  /**
   * @return the  short name of the element.
   */
  virtual QString getWPName() const
    {
      return shortName;
    };

  /**
   * @param newName The new short name of the element.
   */
  virtual void setWPName( const QString& newName )
    {
      shortName = newName;
    };

  /**
   * @return the position in the current map.
   */
  virtual QPoint getMapPosition() const
    {
      return curPos;
    };

  /**
   * @param newPos The new position in the current map.
   */
  virtual void setMapPosition( const QPoint& newPos )
    {
      curPos = newPos;
    };

  /**
   * @return the elevation of the element.
   */
  virtual float getElevation() const
    {
      return elevation;
    };

  /**
   * @param newElevation The new elevation of the element.
   */
  virtual void setElevation( const float value )
    {
      elevation = value;
    };

  /**
   * @return the comment text of the single point
   */
  const QString& getComment()
    {
      return comment;
    };

  /**
   * Sets the comment text of the single point.
   *
   * @param newValue New country code of the element.
   */
  virtual void setComment( QString value )
    {
      comment = value;
    };

  /**
   * @return The compulsory of the single point.
   */
  virtual bool getCompulsory() const
    {
      return compulsory;
    }

  /**
   * Sets the compulsory of the single point.
   *
   * @param newValue New country code of the element.
   */
  virtual void setCompulsory( bool value )
    {
      compulsory = value;
    }

  /**
   * Reimplemented from BaseMapElement.
   *
   * Proofs, if the object is in the drawing-area of the map.
   *
   * @return "true", if the element is in the drawing-area of the map.
   */
  virtual bool isVisible() const
    {
      return glMapMatrix->isVisible(position);
    };

protected:

  /**
   * Position coordinates as WGS84.
   */
  WGSPoint wgsPosition;
  /**
   * The projected lat/lon-position of the element
   */
  QPoint position;
  /**
   * The abbreviation used for the GPS-logger.
   */
  QString shortName;
  /**
   * The current draw-position of the element.
   */
  QPoint curPos;
  /**
   * The elevation.
   */
  float elevation;
 /**
   * Comment related to the single point.
   */
  QString comment;
  /*
   * Feature of single point.
   */
  bool compulsory;
  /**
   * Additional field
   */
  unsigned int lm_typ;
};
