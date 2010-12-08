/***********************************************************************
**
**   da4record.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Eggert Ehmke
**
**   Parts are derived from LoggerFil
**   Copyright (C) 2003 Christian Fughe
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cmath>

#include "da4record.h"

Q_UINT16 U16Swap (Q_UINT16 u16)
{
  union
  {
    Q_UINT16 u16;
    Q_UINT8 bytes [2];
  } u;
  u.u16 = u16;
  Q_UINT8 tmp = u.bytes[0];
  u.bytes[0] = u.bytes[1];
  u.bytes[1] = tmp;
  return u.u16;
}

DA4WPRecord::DA4WPRecord(DA4WPStruct* buffer)
{
  _buffer = buffer;
}

DA4WPRecord::~DA4WPRecord()
{
}

DA4TaskRecord::DA4TaskRecord(DA4TaskStruct* buffer)
{
  _buffer = buffer;
}

DA4TaskRecord::~DA4TaskRecord()
{
}

void DA4TaskRecord::clear()
{
  memset (_buffer, 0, sizeof (DA4TaskStruct));
}

void DA4WPRecord::clear()
{
  memset (_buffer, 0, sizeof (DA4WPStruct));
}

void DA4WPRecord::setType (BaseMapElement::objectType type)
{
  switch (type)
  {
    case BaseMapElement::Landmark:
      _buffer->prg = 1;
      break;
    case BaseMapElement::Airfield:
      _buffer->prg = 2;
      break;
    case BaseMapElement::Outlanding:
      _buffer->prg = 3;
      break;
    default:
      _buffer->prg = 4;
  }
}

BaseMapElement::objectType DA4WPRecord::type () const
{
  switch (_buffer->prg)
  {
    case 1:  return BaseMapElement::Landmark; // should be turnpoint
             break;
    case 2:  return BaseMapElement::Airfield;
             break;
    case 3:  return BaseMapElement::Outlanding;
             break;
    case 4:  return BaseMapElement::Landmark;
             break;
    default: return BaseMapElement::NotSelected;
  }
}

void DA4WPRecord::setName (const QString& name)
{
  qstrncpy (_buffer->name, name.upper().leftJustify (8, ' ', true), 9);
}

QString DA4WPRecord::name () const
{
  return _buffer->name;
}

void DA4WPRecord::setLat (float lat)
{
  _buffer->latitude = lat;
}

void DA4WPRecord::setLon (float lon)
{
  _buffer->longitude = lon;
}

float DA4WPRecord::lat () const
{
  return _buffer->latitude;
}

float DA4WPRecord::lon () const
{
  return _buffer->longitude;
}

void DA4WPRecord::setElev (short int elev)
{
  _buffer->altitude = U16Swap (elev);
}

short int DA4WPRecord::elev () const
{
  return U16Swap (_buffer->altitude);
}

void DA4WPRecord::setFreq (float freq)
{
  _buffer->f = freq;
}

float DA4WPRecord::freq () const
{
  return _buffer->f;
}

void DA4WPRecord::setLen (short int len)
{
  Q_UINT16 ulen;
  if (len == -1)
    ulen = 0;
  else
    ulen = U16Swap (len);
  _buffer->rw = ulen;
}

short int DA4WPRecord::len () const
{
  return U16Swap (_buffer->rw);
}

void DA4WPRecord::setDir (short int dir)
{
  if (dir == -1)
    _buffer->rwdir = 0;
  else
    _buffer->rwdir = dir;
}

short int DA4WPRecord::dir () const
{
  return _buffer->rwdir;
}

void DA4WPRecord::setSurface (Airport::SurfaceType surface)
{
  switch (surface)
  {
    case Airport::Grass:
      _buffer->rwtype = 'G';
      break;
    case Airport::Concrete:
      _buffer->rwtype = 'C';
      break;
    default:
      _buffer->rwtype = 'U';
  }
}

Airport::SurfaceType DA4WPRecord::surface () const
{
  switch (_buffer->rwtype)
  {
    case 'G': return Airport::Grass;
          break;
    case 'C': return Airport::Concrete;
          break;
    default:  return Airport::Unknown;
  }
}

void DA4WPRecord::setTC ()
{
  _buffer->tcdir = 'I';
  _buffer->tc = 0;
}

Waypoint* DA4WPRecord::newWaypoint () const
{
  Waypoint* wp = new Waypoint;

  wp->type = type();
  wp->name = name();
  wp->description = "";
  wp->icao = "";
  wp->origP.setLat((int)(lat() * 600000.0));
  wp->origP.setLon((int)(lon() * 600000.0));
  wp->elevation = (int)(elev() * 0.3048); // don't we have conversion constants ?
  wp->frequency = freq();
  wp->isLandable = false;
  wp->length = len(); // length ?!
  wp->runway = dir(); // direction ?!
  wp->surface = surface();
  wp->comment = QObject::tr("Imported from Filser");
  wp->importance = 3;

  return wp;
}

void DA4WPRecord::setWaypoint (Waypoint* wp)
{
  setType ((BaseMapElement::objectType)wp->type);
  setName (wp->name);
  setLat (wp->origP.lat()/600000.0);
  setLon (wp->origP.lon()/600000.0);
  setElev ((short int)round(wp->elevation/0.3048));
  setFreq(wp->frequency);
  setLen(wp->length);
  setDir(wp->runway);
  setSurface((Airport::SurfaceType)wp->surface);
  setTC();
}

short DA4TaskRecord::pntind (char inx)
{
  return U16Swap (_buffer->pntind [(int)inx]);
}

void DA4TaskRecord::setInd (char wp, short inx)
{
  _buffer->pnttype [(int)wp] = 1;
  _buffer->pntind [(int)wp] = U16Swap (inx);
}
