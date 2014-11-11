/***********************************************************************
**
**   singlepoint.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
**                   2011-2014 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef SINGLE_POINT_H
#define SINGLE_POINT_H

#include "basemapelement.h"

/**
 * \class SinglePoint
 *
 * \brief Map element used for a point object definition.
 *
 * Map element used for point objects. The object can be one of:
 * UltraLight, HangGlider, Parachute, Balloon, Village
 * or Landmark.
 *
 * \see BaseMapElement#objectType
 * \see Airfield
 * \see RadioPoint
 *
 * \author Heiner Lamprecht, Florian Ehinger, Axel Pauli
 *
 * \date 2000-2014
 *
 * \version 11.11.2014
 */

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
   * Additional field
   */
  unsigned int lm_typ;
  /**
   * Comment related to the single point.
   */
  QString comment;
};

#endif
