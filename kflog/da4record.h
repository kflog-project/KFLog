/***********************************************************************
**
**   da4record.h
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
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef DA4RECORD_H
#define DA4RECORD_H


/**used to support filser da4 format
  *@author Eggert Ehmke
  */

#include "basemapelement.h"
#include "airport.h"
#include "waypoint.h"

#define WAYPOINT_MAX  600
#define TASK_MAX      100
#define MAXTSKPNT      10
#define MAXTSKNAME     37

// need to be packed to ensure size 31
struct DA4WPStruct {
  char prg;
  char name[9];
  float latitude;
  float longitude;
  short altitude;
  float f;
  short rw;
  char rwdir;
  char rwtype;
  char tcdir;
  short tc;
} __attribute__ ((packed));
  
// need to be packed to ensure size 32
struct DA4TaskStruct
{
  char prg;
  char ctrlpnt;
  char pnttype[MAXTSKPNT];
  unsigned short pntind[MAXTSKPNT];
} __attribute__ ((packed));

struct DA4Buffer
{
  DA4WPStruct waypoints [WAYPOINT_MAX];
  DA4TaskStruct tasks [TASK_MAX];
} __attribute__ ((packed));

class DA4WPRecord {
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
  void setSurface (Airport::SurfaceType surface);
  Airport::SurfaceType surface () const;
  void setTC ();

  Waypoint* newWaypoint () const;
  void setWaypoint (Waypoint* wp);

private:
  DA4WPStruct* _buffer;
};

class DA4TaskRecord {
public:
	DA4TaskRecord(DA4TaskStruct* buffer);
	~DA4TaskRecord();
  void clear();
  char pnttype (char inx) { return _buffer->pnttype[inx]; };
  short int pntind (char inx);
  void setPrg (char prg) { _buffer->prg = prg; };
  void setInd (char wp, short inx);
private:
  DA4TaskStruct* _buffer;
};
#endif
