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

#define READ_PEN(A, C1, C2, C3, C4, P1, P2, P3, P4, S1, S2, S3, S4) \
  A.append(new QPen(config->readColorEntry("Color 1", new C1), \
        config->readNumEntry("Pen Size 1", P1), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 1", S1))); \
  A.append(new QPen(config->readColorEntry("Color 2", new C2), \
        config->readNumEntry("Pen Size 2", P2), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 2", S2))); \
  A.append(new QPen(config->readColorEntry("Color 3", new C3), \
        config->readNumEntry("Pen Size 3", P3), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 3", S3))); \
  A.append(new QPen(config->readColorEntry("Color 4", new C4), \
        config->readNumEntry("Pen Size 4", P4), \
        (Qt::PenStyle)config->readNumEntry("Pen Style 4", S4)));

#define READ_BRUSH(A, C1, C2, C3, C4, S1, S2, S3, S4) \
  A.append(new QBrush(config->readColorEntry("Brush Color 1", new C1), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 1", S1))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 2", new C2), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 2", S2))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 3", new C3), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 3", S3))); \
  A.append(new QBrush(config->readColorEntry("Brush Color 4", new C4), \
        (Qt::BrushStyle)config->readNumEntry("Brush Style 4", S4))); \

MapConfig::MapConfig(KConfig* cnf)
  : config(cnf), scaleIndex(0)
{
  airCBorder = new bool[4];
  airDBorder = new bool[4];
  airElBorder = new bool[4];
  airEhBorder = new bool[4];
  airFBorder = new bool[4];
  ctrCBorder = new bool[4];
  ctrDBorder = new bool[4];
  dangerBorder = new bool[4];
  lowFBorder = new bool[4];
  restrBorder = new bool[4];
  tmzBorder = new bool[4];

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
  while(airDPenList.remove());
  while(airDBrushList.remove());
  while(airElPenList.remove());
  while(airElBrushList.remove());
  while(airEhPenList.remove());
  while(airEhBrushList.remove());
  while(airFPenList.remove());
  while(airFBrushList.remove());
  while(ctrCPenList.remove());
  while(ctrCBrushList.remove());
  while(ctrDPenList.remove());
  while(ctrDBrushList.remove());
  while(dangerPenList.remove());
  while(dangerBrushList.remove());
  while(lowFPenList.remove());
  while(lowFBrushList.remove());
  while(restrPenList.remove());
  while(restrBrushList.remove());
  while(tmzPenList.remove());
  while(tmzBrushList.remove());

  while(roadPenList.remove());
  while(highwayPenList.remove());
  while(riverPenList.remove());
  while(railPenList.remove());
  while(cityPenList.remove());
  while(cityBrushList.remove());

  config->setGroup("Road");
  READ_PEN(roadPenList, ROAD_COLOR_1, ROAD_COLOR_2, ROAD_COLOR_3, ROAD_COLOR_4,
        ROAD_PEN_1, ROAD_PEN_2, ROAD_PEN_3, ROAD_PEN_4,
        ROAD_PEN_STYLE_1, ROAD_PEN_STYLE_2, ROAD_PEN_STYLE_3, ROAD_PEN_STYLE_4)
  READ_BORDER(roadBorder);

  config->setGroup("River");
  READ_PEN(riverPenList, RIVER_COLOR_1, RIVER_COLOR_2, RIVER_COLOR_2,
        RIVER_COLOR_4, RIVER_PEN_1, RIVER_PEN_2, RIVER_PEN_3, RIVER_PEN_4,
        RIVER_PEN_STYLE_1, RIVER_PEN_STYLE_2, RIVER_PEN_STYLE_3, RIVER_PEN_STYLE_4)
  READ_BORDER(riverBorder);

  config->setGroup("Rail");
  READ_PEN(railPenList, RAIL_COLOR_1, RAIL_COLOR_2, RAIL_COLOR_3, RAIL_COLOR_4,
        RAIL_PEN_1, RAIL_PEN_2, RAIL_PEN_3, RAIL_PEN_4,
        RAIL_PEN_STYLE_1, RAIL_PEN_STYLE_2, RAIL_PEN_STYLE_3, RAIL_PEN_STYLE_4)
  READ_BORDER(railBorder);

  config->setGroup("Highway");
  READ_PEN(highwayPenList, HIGH_COLOR_1, HIGH_COLOR_2, HIGH_COLOR_3, HIGH_COLOR_4,
        HIGH_PEN_1, HIGH_PEN_2, HIGH_PEN_3, HIGH_PEN_4,
        HIGH_PEN_STYLE_1, HIGH_PEN_STYLE_2, HIGH_PEN_STYLE_3, HIGH_PEN_STYLE_4)
  READ_BORDER(highwayBorder);
  /*
   * In version <= 2.0.1, the fillcolor of cities is called "Color" instead
   * of "Brush Color", so we must look, which version of configfile we read.
   */
  config->setGroup("General Options");
  if(config->hasKey("Version") && config->readEntry("Version") >= "2.0.2")
    {
      config->setGroup("City");
      // PenStyle and BrushStyle are not used for cities ...
      READ_PEN(cityPenList, CITY_COLOR_1, CITY_COLOR_2, CITY_COLOR_3,
            CITY_COLOR_4, CITY_PEN_1, CITY_PEN_2, CITY_PEN_3, CITY_PEN_4,
            Qt::SolidLine, Qt::SolidLine, Qt::SolidLine, Qt::SolidLine)
      READ_BRUSH(cityBrushList, CITY_BRUSH_COLOR_1, CITY_BRUSH_COLOR_2,
          CITY_BRUSH_COLOR_3, CITY_BRUSH_COLOR_4, Qt::SolidPattern,
          Qt::SolidPattern, Qt::SolidPattern, Qt::SolidPattern)
    }
  else
    {
      // We assume to have an old configfile ...
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
  READ_PEN(airCPenList, AIRC_COLOR_1, AIRC_COLOR_2, AIRC_COLOR_3, AIRC_COLOR_4,
        AIRC_PEN_1, AIRC_PEN_2, AIRC_PEN_3, AIRC_PEN_4,
        AIRC_PEN_STYLE_1, AIRC_PEN_STYLE_2, AIRC_PEN_STYLE_3, AIRC_PEN_STYLE_4)
  READ_BRUSH(airCBrushList, AIRC_BRUSH_COLOR_1, AIRC_BRUSH_COLOR_2,
        AIRC_BRUSH_COLOR_3, AIRC_BRUSH_COLOR_4, AIRC_BRUSH_STYLE_1,
        AIRC_BRUSH_STYLE_2, AIRC_BRUSH_STYLE_3, AIRC_BRUSH_STYLE_4)
  READ_BORDER(airCBorder);

  config->setGroup("Airspace D");
  READ_PEN(airDPenList, AIRD_COLOR_1, AIRD_COLOR_2, AIRD_COLOR_3, AIRD_COLOR_4,
        AIRD_PEN_1, AIRD_PEN_2, AIRD_PEN_3, AIRD_PEN_4,
        AIRD_PEN_STYLE_1, AIRD_PEN_STYLE_2, AIRD_PEN_STYLE_3, AIRD_PEN_STYLE_4)
  READ_BRUSH(airDBrushList, AIRD_BRUSH_COLOR_1, AIRD_BRUSH_COLOR_2,
        AIRD_BRUSH_COLOR_3, AIRD_BRUSH_COLOR_4, AIRD_BRUSH_STYLE_1,
        AIRD_BRUSH_STYLE_2, AIRD_BRUSH_STYLE_3, AIRD_BRUSH_STYLE_4)
  READ_BORDER(airDBorder);

  config->setGroup("Airspace E low");
  READ_PEN(airElPenList, AIREL_COLOR_1, AIREL_COLOR_2, AIREL_COLOR_3,
        AIREL_COLOR_4, AIREL_PEN_1, AIREL_PEN_2, AIREL_PEN_3, AIREL_PEN_4,
        AIREL_PEN_STYLE_1, AIREL_PEN_STYLE_2, AIREL_PEN_STYLE_3, AIREL_PEN_STYLE_4)
  READ_BRUSH(airElBrushList, AIREL_BRUSH_COLOR_1, AIREL_BRUSH_COLOR_2,
        AIREL_BRUSH_COLOR_3, AIREL_BRUSH_COLOR_4, AIREL_BRUSH_STYLE_1,
        AIREL_BRUSH_STYLE_2, AIREL_BRUSH_STYLE_3, AIREL_BRUSH_STYLE_4)
  READ_BORDER(airElBorder);

  config->setGroup("Airspace E high");
  READ_PEN(airEhPenList, AIREH_COLOR_1, AIREH_COLOR_2, AIREH_COLOR_3,
        AIREH_COLOR_4, AIREH_PEN_1, AIREH_PEN_2, AIREH_PEN_3, AIREH_PEN_4,
        AIREH_PEN_STYLE_1, AIREH_PEN_STYLE_2, AIREH_PEN_STYLE_3, AIREH_PEN_STYLE_4)
  READ_BRUSH(airEhBrushList, AIREH_BRUSH_COLOR_1, AIREH_BRUSH_COLOR_2,
        AIREH_BRUSH_COLOR_3, AIREH_BRUSH_COLOR_4, AIREH_BRUSH_STYLE_1,
        AIREH_BRUSH_STYLE_2, AIREH_BRUSH_STYLE_3, AIREH_BRUSH_STYLE_4)
  READ_BORDER(airEhBorder);

  config->setGroup("Airspace F");
  READ_PEN(airFPenList, AIRF_COLOR_1, AIRF_COLOR_2, AIRF_COLOR_3,
        AIRF_COLOR_4, AIRF_PEN_1, AIRF_PEN_2, AIRF_PEN_3, AIRF_PEN_4,
        AIRF_PEN_STYLE_1, AIRF_PEN_STYLE_2, AIRF_PEN_STYLE_3, AIRF_PEN_STYLE_4)
  READ_BRUSH(airFBrushList, AIRF_BRUSH_COLOR_1, AIRF_BRUSH_COLOR_2,
        AIRF_BRUSH_COLOR_3, AIRF_BRUSH_COLOR_4, AIRF_BRUSH_STYLE_1,
        AIRF_BRUSH_STYLE_2, AIRF_BRUSH_STYLE_3, AIRF_BRUSH_STYLE_4)
  READ_BORDER(airFBorder);

  config->setGroup("Control C");
  READ_PEN(ctrCPenList, CTRC_COLOR_1, CTRC_COLOR_2, CTRC_COLOR_3, CTRC_COLOR_4,
        CTRC_PEN_1, CTRC_PEN_2, CTRC_PEN_3, CTRC_PEN_4,
        CTRC_PEN_STYLE_1, CTRC_PEN_STYLE_2, CTRC_PEN_STYLE_3, CTRC_PEN_STYLE_4)
  READ_BRUSH(ctrCBrushList, CTRC_BRUSH_COLOR_1, CTRC_BRUSH_COLOR_2,
        CTRC_BRUSH_COLOR_3, CTRC_BRUSH_COLOR_4, CTRC_BRUSH_STYLE_1,
        CTRC_BRUSH_STYLE_2, CTRC_BRUSH_STYLE_3, CTRC_BRUSH_STYLE_4)
  READ_BORDER(ctrCBorder);

  config->setGroup("Control D");
  READ_PEN(ctrDPenList, CTRD_COLOR_1, CTRD_COLOR_2, CTRD_COLOR_3, CTRD_COLOR_4,
        CTRD_PEN_1, CTRD_PEN_2, CTRD_PEN_3, CTRD_PEN_4,
        CTRD_PEN_STYLE_1, CTRD_PEN_STYLE_2, CTRD_PEN_STYLE_3, CTRD_PEN_STYLE_4)
  READ_BRUSH(ctrDBrushList, CTRD_BRUSH_COLOR_1, CTRD_BRUSH_COLOR_2,
        CTRD_BRUSH_COLOR_3, CTRD_BRUSH_COLOR_4, CTRD_BRUSH_STYLE_1,
        CTRD_BRUSH_STYLE_2, CTRD_BRUSH_STYLE_3, CTRD_BRUSH_STYLE_4)
  READ_BORDER(ctrDBorder);

  config->setGroup("Danger");
  READ_PEN(dangerPenList, DNG_COLOR_1, DNG_COLOR_2, DNG_COLOR_3, DNG_COLOR_4,
        DNG_PEN_1, DNG_PEN_2, DNG_PEN_3, DNG_PEN_4,
        DNG_PEN_STYLE_1, DNG_PEN_STYLE_2, DNG_PEN_STYLE_3, DNG_PEN_STYLE_4)
  READ_BRUSH(dangerBrushList, DNG_BRUSH_COLOR_1, DNG_BRUSH_COLOR_2,
        DNG_BRUSH_COLOR_3, DNG_BRUSH_COLOR_4, DNG_BRUSH_STYLE_1,
        DNG_BRUSH_STYLE_2, DNG_BRUSH_STYLE_3, DNG_BRUSH_STYLE_4)
  READ_BORDER(dangerBorder);

  config->setGroup("Low Flight");
  READ_PEN(lowFPenList, LOWF_COLOR_1, LOWF_COLOR_2, LOWF_COLOR_3, LOWF_COLOR_4,
        LOWF_PEN_1, LOWF_PEN_2, LOWF_PEN_3, LOWF_PEN_4,
        LOWF_PEN_STYLE_1, LOWF_PEN_STYLE_2, LOWF_PEN_STYLE_3, LOWF_PEN_STYLE_4)
  READ_BRUSH(lowFBrushList, LOWF_BRUSH_COLOR_1, LOWF_BRUSH_COLOR_2,
        LOWF_BRUSH_COLOR_3, LOWF_BRUSH_COLOR_4, LOWF_BRUSH_STYLE_1,
        LOWF_BRUSH_STYLE_2, LOWF_BRUSH_STYLE_3, LOWF_BRUSH_STYLE_4)
  READ_BORDER(lowFBorder);

  config->setGroup("Restricted Area");
  READ_PEN(restrPenList, RES_COLOR_1, RES_COLOR_2, RES_COLOR_3, RES_COLOR_4,
        RES_PEN_1, RES_PEN_2, RES_PEN_3, RES_PEN_4,
        RES_PEN_STYLE_1, RES_PEN_STYLE_2, RES_PEN_STYLE_3, RES_PEN_STYLE_4)
  READ_BRUSH(restrBrushList, RES_BRUSH_COLOR_1, RES_BRUSH_COLOR_2,
        RES_BRUSH_COLOR_3, RES_BRUSH_COLOR_4, RES_BRUSH_STYLE_1,
        RES_BRUSH_STYLE_2, RES_BRUSH_STYLE_3, RES_BRUSH_STYLE_4)
  READ_BORDER(restrBorder);

  config->setGroup("TMZ");
  READ_PEN(tmzPenList, TMZ_COLOR_1, TMZ_COLOR_2, TMZ_COLOR_3, TMZ_COLOR_4,
        TMZ_PEN_1, TMZ_PEN_2, TMZ_PEN_3, TMZ_PEN_4,
        TMZ_PEN_STYLE_1, TMZ_PEN_STYLE_2, TMZ_PEN_STYLE_3, TMZ_PEN_STYLE_4)
  READ_BRUSH(tmzBrushList, TMZ_BRUSH_COLOR_1, TMZ_BRUSH_COLOR_2,
        TMZ_BRUSH_COLOR_3, TMZ_BRUSH_COLOR_4, TMZ_BRUSH_STYLE_1,
        TMZ_BRUSH_STYLE_2, TMZ_BRUSH_STYLE_3, TMZ_BRUSH_STYLE_4)
  READ_BORDER(tmzBorder);

  config->setGroup(0);
}

void MapConfig::setMatrixValues(int index, bool sw)
{
  isSwitch = sw;
  scaleIndex = index;
}

QPen MapConfig::getPrintPen(unsigned int typeID)
{
  return getDrawPen(typeID);
}

QPen MapConfig::getDrawPen(unsigned int typeID)
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
      case BaseMapElement::AirC:
        return *airCPenList.at(scaleIndex);
      case BaseMapElement::AirD:
        return *airDPenList.at(scaleIndex);
      case BaseMapElement::AirElow:
        return *airElPenList.at(scaleIndex);
      case BaseMapElement::AirEhigh:
        return *airEhPenList.at(scaleIndex);
      case BaseMapElement::AirF:
        return *airFPenList.at(scaleIndex);
      case BaseMapElement::ControlC:
        return *ctrCPenList.at(scaleIndex);
      case BaseMapElement::ControlD:
        return *ctrDPenList.at(scaleIndex);
      case BaseMapElement::Danger:
        return *dangerPenList.at(scaleIndex);
      case BaseMapElement::LowFlight:
        return *lowFPenList.at(scaleIndex);
      case BaseMapElement::Restricted:
        return *restrPenList.at(scaleIndex);
      case BaseMapElement::TMZ:
        return *tmzPenList.at(scaleIndex);
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

QBrush MapConfig::getPrintBrush(unsigned int typeID)
{
  return getDrawBrush(typeID);
}

QBrush MapConfig::getDrawBrush(unsigned int typeID)
{
  switch(typeID)
    {
      case BaseMapElement::City:
        return *cityBrushList.at(scaleIndex);
      case BaseMapElement::Lake:
        return QBrush(riverPenList.at(scaleIndex)->color(), Qt::SolidPattern);
      case BaseMapElement::AirC:
        return *airCBrushList.at(scaleIndex);
      case BaseMapElement::AirD:
        return *airDBrushList.at(scaleIndex);
      case BaseMapElement::AirElow:
        return *airElBrushList.at(scaleIndex);
      case BaseMapElement::AirEhigh:
        return *airEhBrushList.at(scaleIndex);
      case BaseMapElement::AirF:
        return *airFBrushList.at(scaleIndex);
      case BaseMapElement::ControlC:
        return *ctrCBrushList.at(scaleIndex);
      case BaseMapElement::ControlD:
        return *ctrDBrushList.at(scaleIndex);
      case BaseMapElement::Danger:
        return *dangerBrushList.at(scaleIndex);
      case BaseMapElement::LowFlight:
        return *lowFBrushList.at(scaleIndex);
      case BaseMapElement::Restricted:
        return *restrBrushList.at(scaleIndex);
      case BaseMapElement::TMZ:
        return *tmzBrushList.at(scaleIndex);
    }
  return QBrush();
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
        if(isWinch)
            iconName = "glider.xpm";
        else
            iconName = "glider2.xpm";
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
      case BaseMapElement::VOR:
        iconName = "vor.xpm";
        break;
      case BaseMapElement::VORDME:
        iconName = "vordme.xpm";
        break;
      case BaseMapElement::VORTAC:
        iconName = "vortac.xpm";
        break;
      case BaseMapElement::NDB:
        iconName = "ndb.xpm";
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
