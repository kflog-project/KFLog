/***********************************************************************
**
**   mapconfig.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2001 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "mapconfig.h"

#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>

#include <basemapelement.h>
#include <mapdefaults.h>

#define READ_BORDER(a) \
    a[0] = config->readBoolEntry("Border 1", true); \
    a[1] = config->readBoolEntry("Border 2", true); \
    a[2] = config->readBoolEntry("Border 3", true); \
    a[3] = config->readBoolEntry("Border 4", true);

MapConfig::MapConfig(KConfig* cnf)
  : config(cnf), scaleIndex(0)
{
  airCBorder = new bool[4];
  roadBorder = new bool[4];
  highwayBorder = new bool[4];
  railBorder = new bool[4];
  riverBorder = new bool[4];
  cityBorder = new bool[4];

  readConfig();
}

MapConfig::~MapConfig()
{

}

void MapConfig::readConfig()
{
  /* Unfortunately, a ~QList() doesn't work, so we must remove all
   * item manualy ;-(
   */
  while(airCPenList.remove());
  while(airCBrushList.remove());
  while(roadPenList.remove());
  while(highwayPenList.remove());
  while(riverPenList.remove());
  while(railPenList.remove());
  while(cityPenList.remove());
  while(cityBrushList.remove());

  config->setGroup("Road");
  roadPenList.append(new QPen(config->readColorEntry("Color 1",
        new ROAD_COLOR_1), config->readNumEntry("Pen Size 1", ROAD_PEN_1),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", ROAD_PEN_STYLE_1)));
  roadPenList.append( new QPen(config->readColorEntry("Color 2",
        new ROAD_COLOR_2), config->readNumEntry("Pen Size 2", ROAD_PEN_2),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", ROAD_PEN_STYLE_2)));
  roadPenList.append(new QPen(config->readColorEntry("Color 3",
        new ROAD_COLOR_3), config->readNumEntry("Pen Size 3", ROAD_PEN_3),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", ROAD_PEN_STYLE_3)));
  roadPenList.append(new QPen(config->readColorEntry("Color 4",
        new ROAD_COLOR_4), config->readNumEntry("Pen Size 4", ROAD_PEN_4),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", ROAD_PEN_STYLE_4)));
  READ_BORDER(roadBorder);

  config->setGroup("River");
  riverPenList.append(new QPen(config->readColorEntry("Color 1",
        new RIVER_COLOR_1), config->readNumEntry("Pen Size 1", RIVER_PEN_1),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RIVER_PEN_STYLE_1)));
  riverPenList.append(new QPen(config->readColorEntry("Color 2",
        new RIVER_COLOR_2), config->readNumEntry("Pen Size 2", RIVER_PEN_2),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RIVER_PEN_STYLE_2)));
  riverPenList.append(new QPen(config->readColorEntry("Color 3",
        new RIVER_COLOR_3), config->readNumEntry("Pen Size 3", RIVER_PEN_3),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RIVER_PEN_STYLE_3)));
  riverPenList.append(new QPen(config->readColorEntry("Color 4",
        new RIVER_COLOR_4), config->readNumEntry("Pen Size 4", RIVER_PEN_4),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RIVER_PEN_STYLE_4)));
  READ_BORDER(riverBorder);

  config->setGroup("Rail");
  railPenList.append(new QPen(config->readColorEntry("Color 1",
        new RAIL_COLOR_1), config->readNumEntry("Pen Size 1", RAIL_PEN_1),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RAIL_PEN_STYLE_1)));
  railPenList.append(new QPen(config->readColorEntry("Color 2",
        new RAIL_COLOR_2), config->readNumEntry("Pen Size 2", RAIL_PEN_2),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RAIL_PEN_STYLE_2)));
  railPenList.append(new QPen(config->readColorEntry("Color 3",
        new RAIL_COLOR_3), config->readNumEntry("Pen Size 3", RAIL_PEN_3),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RAIL_PEN_STYLE_3)));
  railPenList.append(new QPen(config->readColorEntry("Color 4",
        new RAIL_COLOR_4), config->readNumEntry("Pen Size 4", RAIL_PEN_4),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", RAIL_PEN_STYLE_4)));
  READ_BORDER(railBorder);

  config->setGroup("Highway");
  highwayPenList.append(new QPen(config->readColorEntry("Color 1",
        new HIGH_COLOR_1), config->readNumEntry("Pen Size 1", HIGH_PEN_1),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", HIGH_PEN_STYLE_1)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 2",
        new HIGH_COLOR_2), config->readNumEntry("Pen Size 2", HIGH_PEN_2),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", HIGH_PEN_STYLE_2)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 3",
        new HIGH_COLOR_3), config->readNumEntry("Pen Size 3", HIGH_PEN_3),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", HIGH_PEN_STYLE_3)));
  highwayPenList.append(new QPen(config->readColorEntry("Color 4",
        new HIGH_COLOR_4), config->readNumEntry("Pen Size 4", HIGH_PEN_4),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", HIGH_PEN_STYLE_4)));
  READ_BORDER(highwayBorder);

  /*
   * In version <= 2.0.1, the fillcolor of cities is called "Color" instead
   * of "Brush Color", so we must lokk, wich version of configfile we read.
   */
  config->setGroup("General Options");
  if(config->hasKey("Version") && config->readEntry("Version") == "2.0.2")
    {
      config->setGroup("City");
      cityPenList.append(new QPen(config->readColorEntry("Color 1",
            new CITY_COLOR_1),
            config->readNumEntry("Pen Size 1", 1)));
      cityPenList.append(new QPen(config->readColorEntry("Color 2",
            new CITY_COLOR_2),
            config->readNumEntry("Pen Size 2", 1)));
      cityPenList.append(new QPen(config->readColorEntry("Color 3",
            new CITY_COLOR_3),
            config->readNumEntry("Pen Size 3", 1)));
      cityPenList.append(new QPen(config->readColorEntry("Color 4",
            new CITY_COLOR_4),
            config->readNumEntry("Pen Size 4", 1)));
      // PenStyle is not used for cities ...
      cityBrushList.append(new QBrush(config->readColorEntry("Brush Color 1",
          new CITY_BRUSH_COLOR_1), Qt::SolidPattern));
      cityBrushList.append(new QBrush(config->readColorEntry("Brush Color 2",
          new CITY_BRUSH_COLOR_2), Qt::SolidPattern));
      cityBrushList.append(new QBrush(config->readColorEntry("Brush Color 3",
          new CITY_BRUSH_COLOR_3), Qt::SolidPattern));
      cityBrushList.append(new QBrush(config->readColorEntry("Brush Color 4",
          new CITY_BRUSH_COLOR_4), Qt::SolidPattern));
    }
  else
    {
      // We assume to have an old configfile ...
      debug("KFLog: old config-file found. (Version <= 2.0.1)");
      config->setGroup("City");
      cityPenList.append(new QPen(CITY_COLOR_1, 1));
      cityPenList.append(new QPen(CITY_COLOR_2, 1));
      cityPenList.append(new QPen(CITY_COLOR_3, 1));
      cityPenList.append(new QPen(CITY_COLOR_4, 1));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_1, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_2, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_3, Qt::SolidPattern));
      cityBrushList.append(new QBrush(CITY_BRUSH_COLOR_4, Qt::SolidPattern));
    }
  READ_BORDER(cityBorder);

  config->setGroup("Airspace C");
  airCPenList.append(new QPen(config->readColorEntry("Color 1",
        new AIRC_COLOR_1), config->readNumEntry("Pen Size 1", AIRC_PEN_1),
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", AIRC_PEN_STYLE_1)));
  airCPenList.append(new QPen(config->readColorEntry("Color 2",
        new AIRC_COLOR_2), config->readNumEntry("Pen Size 2", AIRC_PEN_2),
        (Qt::PenStyle)config->readNumEntry("Pen Style 2", AIRC_PEN_STYLE_2)));
  airCPenList.append(new QPen(config->readColorEntry("Color 3",
        new AIRC_COLOR_3), config->readNumEntry("Pen Size 3", AIRC_PEN_3),
        (Qt::PenStyle)config->readNumEntry("Pen Style 3", AIRC_PEN_STYLE_3)));
  airCPenList.append(new QPen(config->readColorEntry("Color 4",
        new AIRC_COLOR_4), config->readNumEntry("Pen Size 4", AIRC_PEN_4),
        (Qt::PenStyle)config->readNumEntry("Pen Style 4", AIRC_PEN_STYLE_4)));
  airCBrushList.append(new QBrush(config->readColorEntry("Brush Color 1",
        new AIRC_BRUSH_COLOR_1),
        (Qt::BrushStyle)config->readNumEntry("Brush Style 1",
        AIRC_BRUSH_STYLE_1)));
  airCBrushList.append(new QBrush(config->readColorEntry("Brush Color 2",
        new AIRC_BRUSH_COLOR_2),
        (Qt::BrushStyle)config->readNumEntry("Brush Style 2",
        AIRC_BRUSH_STYLE_2)));
  airCBrushList.append(new QBrush(config->readColorEntry("Brush Color 3",
        new AIRC_BRUSH_COLOR_3),
        (Qt::BrushStyle)config->readNumEntry("Brush Style 3",
        AIRC_BRUSH_STYLE_3)));
  airCBrushList.append(new QBrush(config->readColorEntry("Brush Color 4",
        new AIRC_BRUSH_COLOR_4),
        (Qt::BrushStyle)config->readNumEntry("Brush Style 4",
        AIRC_BRUSH_STYLE_4)));
  READ_BORDER(airCBorder);
}

QList<QPen> MapConfig::getPenList(unsigned int listID)
{
  switch(listID)
    {
      case RoadList:
        return roadPenList;
      case HighwayList:
        return highwayPenList;
      case RiverList:
        return riverPenList;
      case RailwayList:
        return railPenList;
      case CityList:
        return cityPenList;
    }
  return roadPenList;
}

bool* MapConfig::getPenList(QList<QPen>* list, unsigned int listID)
{
  switch(listID)
    {
      case RoadList:
        list = new QList<QPen>(roadPenList);
        return roadBorder;
      case HighwayList:
        list = &highwayPenList;
        return highwayBorder;
      case RiverList:
        list = &riverPenList;
        return riverBorder;
      case RailwayList:
        list = &railPenList;
        return railBorder;
      case CityList:
        list = &cityPenList;
        return cityBorder;
    }
  return roadBorder;
}

void MapConfig::setMatrixValues(int index, bool sw)
{
  isSwitch = sw;
  scaleIndex = index;
}

QPen MapConfig::getPen(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Road:
        return *roadPenList.at(scaleIndex);
      case BaseMapElement::Highway:
        return *highwayPenList.at(scaleIndex);
      case BaseMapElement::Railway:
        return *railPenList.at(scaleIndex);
      case BaseMapElement::River:
      case BaseMapElement::Lake:
        return *riverPenList.at(scaleIndex);
      case BaseMapElement::City:
        return *cityPenList.at(scaleIndex);
      default:
        return *roadPenList.at(scaleIndex);
    }
}

bool MapConfig::isBorder(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::Road:
        return roadBorder[scaleIndex];
      case BaseMapElement::Highway:
        return highwayBorder[scaleIndex];
      case BaseMapElement::Railway:
        return railBorder[scaleIndex];
      case BaseMapElement::River:
      case BaseMapElement::Lake:
        return riverBorder[scaleIndex];
      case BaseMapElement::City:
        return cityBorder[scaleIndex];
    }

  /* Should never happen ... */
  return true;
}

QPen MapConfig::getOutlinePen(unsigned int typeID)
{
  /**
   * OutlinePen hier immer auf "4" fest eingestellt. Wir brauchen ein enum!
   *
   * Die Dicke des Pen sollte fest eingestellt werden ...
   */
  switch(typeID)
    {
      case BaseMapElement::City:
        return QPen(cityPenList.at(4)->color(),
            cityPenList.at(4)->width() * 3);
    }
}

QBrush MapConfig::getBrush(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::City:
        return *cityBrushList.at(scaleIndex);
    }
  return QBrush();
}

QColor MapConfig::getDrawColor(unsigned int typeID)
{
  return QColor();
}

QColor MapConfig::getFillColor(unsigned int typeID)
{
  return QColor();
}

QPixmap MapConfig::getPixmap(unsigned int typID, bool isWinch)
{
  QString iconName;

  switch(typID)
    {
      case BaseMapElement::BaseMapElement::Airport:
      case BaseMapElement::BaseMapElement::IntAirport:
        iconName = "airport.xpm";
        break;
      case BaseMapElement::MilAirport:
        iconName = "milairport.xpm";
        break;
      case BaseMapElement::CivMilAirport:
        iconName = "civmilair.xpm";
        break;
      case BaseMapElement::Airfield:
        iconName = "airfield_c.xpm";
        break;
      case BaseMapElement::ClosedAirfield:
        iconName = "closed.xpm";
        break;
      case BaseMapElement::CivHeliport:
        iconName = "civheliport.xpm";
        break;
      case BaseMapElement::MilHeliport:
        iconName = "milheliport.xpm";
        break;
      case BaseMapElement::AmbHeliport:
        iconName = "ambheliport.xpm";
        break;
      case BaseMapElement::Glidersite:
        iconName = "glider.xpm";
        break;
      case BaseMapElement::UltraLight:
        iconName = "ul.xpm";
        break;
      case BaseMapElement::HangGlider:
        iconName = "paraglider.xpm";
        break;
      case BaseMapElement::Parachute:
        iconName = "jump.xpm";
        break;
      case BaseMapElement::Ballon:
        iconName = "ballon.xpm";
        break;
      case BaseMapElement::CompPoint:
        iconName = "compoint.xpm";
        break;
      case BaseMapElement::Landmark:
        iconName = "landmark.xpm";
        break;
      default:
        iconName = "";
        break;
    }

  if(isSwitch)
      return QPixmap(KGlobal::dirs()->findResource("appdata",
          "mapicons/small/" + iconName));

  return QPixmap(KGlobal::dirs()->findResource("appdata",
      "mapicons/" + iconName));
}
