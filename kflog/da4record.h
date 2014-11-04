/***********************************************************************
**
**   da4record.h
**
**   This file is part of KFLog.
**
************************************************************************
**
**   Copyright (c):  2003 by Eggert Ehmke
**
**   Parts are derived from LoggerFil
**
**   Copyright (C) 2003 Christian Fughe
**                                                                     
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DA4_RECORD_H
#define DA4_RECORD_H


/**
 * used to support filser da4 format
 *
 * @author Eggert Ehmke
 */

#include "basemapelement.h"
#include "runway.h"
#include "waypoint.h"

#define WAYPOINT_MAX  600
#define TASK_MAX      100
#define MAXTSKPNT      10
#define MAXTSKNAME     37

// need to be packed to ensure size 31
#if defined WIN32 && defined _MSC_VER
#pragma pack(push, 1)
#endif
struct DA4WPStruct
{
  char prg;
  char name[9];
  float latitude;
  float longitude;
  short altitude;
  float f;
  short rw;
  unsigned char rwdir;
  unsigned char rwtype;
  unsigned char tcdir;
  short tc;
}
#if defined WIN32 && defined _MSC_VER
;
#pragma pack(pop)
#else
__attribute__ ((packed));
#endif

// need to be packed to ensure size 32
#if defined WIN32 && defined _MSC_VER
#pragma pack(push, 1)
#endif
struct DA4TaskStruct
{
  char prg;
  char ctrlpnt;
  char pnttype[MAXTSKPNT];
  unsigned short pntind[MAXTSKPNT];
}
#if defined WIN32 && defined _MSC_VER
;
#pragma pack(pop)
#else
__attribute__ ((packed));
#endif

#if defined WIN32 && defined _MSC_VER
#pragma pack(push, 1)
#endif
struct DA4Buffer
{
  DA4WPStruct waypoints [WAYPOINT_MAX];
  DA4TaskStruct tasks [TASK_MAX];
}
#if defined WIN32 && defined _MSC_VER
;
#pragma pack(pop)
#else
__attribute__ ((packed));
#endif

class DA4WPRecord
{
public: 
	DA4WPRecord(DA4WPStruct* buffer);
	~DA4WPRecord();
  void clear();

  void setType (BaseMapElement::objectType type);
  BaseMapElement::objectType type () const;
  void setName (const QString& name);
  QString name () const;
  void setLat (float lat);
  void setLon (float lon);
  float lat () const;
  float lon () const;
  void setElev (short int elev);
  short int elev () const;
  void setFreq (float freq);
  float freq () const;
  void setLen (short int len);
  short int len () const;
  void setDir (short int dir);
  short int dir () const;
  void setSurface (Runway::SurfaceType surface);
  Runway::SurfaceType surface () const;
  void setTC ();

  Waypoint* newWaypoint () const;
  void setWaypoint (Waypoint* wp);

private:
  DA4WPStruct* _buffer;
};

class DA4TaskRecord
{
public:
	DA4TaskRecord(DA4TaskStruct* buffer);
	~DA4TaskRecord();
  void clear();
  char pnttype (char inx) { return _buffer->pnttype[(int)inx]; };
  short int pntind (char inx);
  void setPrg (char prg) { _buffer->prg = prg; };
  void setInd (char wp, short inx);
private:
  DA4TaskStruct* _buffer;
};
#endif
