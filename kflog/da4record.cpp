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
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "da4record.h"

/**
  * This helper function converts floating point numbers that are stored in filser binary files
  * into normal float numbers.
  */
float FloatFromU32(Q_UINT32 u32)
{
  union
  {
    float    f32;
    Q_UINT32 u32;
  } u;

  u.u32 = u32;
  // read out the float value
  return u.f32;
}

/**
  * This helper function converts floating point numbers
  * into float numbers to be stored in filser binary files.
  */
Q_UINT32 U32fromFloat (float f32)
{
  union
  {
    float    f32;
    Q_UINT32 u32;
  } u;
  u.f32 = f32;
  return u.u32;
}

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

DA4Record::DA4Record()
{
}

DA4Record::~DA4Record()
{
}

void DA4Record::setType (BaseMapElement::objectType type)
{
  switch (type)
  {
    case BaseMapElement::Landmark:
      _buffer[0] = 1;
      break;
    case BaseMapElement::Airfield:
      _buffer[0] = 2;
      break;
    case BaseMapElement::Outlanding:
      _buffer[0] = 3;
      break;
    default:
      _buffer[0] = 4;
  }
}

BaseMapElement::objectType DA4Record::type ()
{
  switch (_buffer[0])
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

void DA4Record::setName (const QString& name)
{
  QCString sName (name);
  qstrncpy (&_buffer[1], sName.leftJustify (8, ' ', true), 9);
}

QString DA4Record::name () const
{
  char pName [9];
  memcpy (pName, &_buffer[1], 9);
  return pName;
}

void DA4Record::setLat (float lat)
{
  Q_UINT32 ulat = U32fromFloat (lat);  
  memcpy (&_buffer[10], &ulat, 4);
}

void DA4Record::setLon (float lon)
{
  Q_UINT32 ulon = U32fromFloat (lon);
  memcpy (&_buffer[14], &ulon, 4);
}

float DA4Record::lat () const
{
  Q_UINT32 ulat;
  memcpy (&ulat, &_buffer[10], 4);
  return FloatFromU32(ulat);
}

float DA4Record::lon () const
{
  Q_UINT32 ulon;
  memcpy (&ulon, &_buffer[14], 4);
  return FloatFromU32(ulon);
}

void DA4Record::setElev (short int elev)
{
  Q_UINT16 uelev = U16Swap (elev);
  memcpy (&_buffer[18], &uelev, 2);
}

short int DA4Record::elev () const
{
  Q_UINT16 uelev;
  memcpy (&uelev, &_buffer[18], 2);
  return U16Swap (uelev);  
}

void DA4Record::setFreq (float freq)
{
  Q_UINT32 ufreq = U32fromFloat (freq);
  memcpy (&_buffer[20], &ufreq, 4);
}

float DA4Record::freq () const
{
  Q_UINT32 ufreq;
  memcpy (&ufreq, &_buffer[20], 4);
  return FloatFromU32(ufreq);
}

void DA4Record::setLen (short int len)
{
  Q_UINT16 ulen;
  if (len == -1)
    ulen = 0;
  else
    ulen = U16Swap (len);
  memcpy (&_buffer[24], &ulen, 2);
}

short int DA4Record::len () const
{
  Q_UINT16 ulen;
  memcpy (&ulen, &_buffer[24], 2);
  return U16Swap (ulen);
}

void DA4Record::setDir (short int dir)
{
  if (dir == -1)
    _buffer [26] = 0;
  else
    _buffer [26] = dir;
}

short int DA4Record::dir () const
{
  return _buffer[26];
}

void DA4Record::setSurface (Airport::SurfaceType surface)
{
  switch (surface)
  {
    case Airport::Grass:
      _buffer[27] = 'G';
      break;
    case Airport::Concrete:
      _buffer[27] = 'C';
      break;
    default:
      _buffer[27] = 'U';
  }
}

Airport::SurfaceType DA4Record::surface () const
{
  switch (_buffer [27])
  {
    case 'G': return Airport::Grass;
          break;
    case 'C': return Airport::Concrete;
          break;
    default:  return Airport::Unknown;
  }
}

void DA4Record::setFiller ()
{
  _buffer[28] = 'I';
  _buffer[29] = 0;
  _buffer[30] = 0;
}
