/***********************************************************************
**
**   cumuluswaypointfile.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "cumuluswaypointfile.h"
#include <qfile.h>

#define KFLOG_FILE_MAGIC    0x404b464c
#define FILE_TYPE_WAYPOINTS 0x50
#define FILE_FORMAT_ID      100
#define FILE_FORMAT_ID_2    101

CumulusWaypointFile::CumulusWaypointFile(){
}

CumulusWaypointFile::~CumulusWaypointFile(){
}

/** loads the waypoints into the QList, and returns the number of loaded waypoints. */
int CumulusWaypointFile::load(const QString& file, QPtrList<Waypoint> *waypoints){
  bool ok = false;
  int cnt=0;
  int ret=FO_ERROR;

  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  Q_INT8 wpType;
  Q_INT32 wpLatitude;
  Q_INT32 wpLongitude;
  Q_INT16 wpElevation;
  double wpFrequency;
  Q_INT8 wpLandable;
  Q_INT16 wpRunway;
  Q_INT16 wpLength;
  Q_INT8 wpSurface;
  QString wpComment="";
  Q_UINT8 wpImportance;

  Q_UINT32 fileMagic;
  Q_INT8 fileType;
  Q_UINT16 fileFormat;

  QFile f(file);
  if (f.exists()) {
    if (f.open(IO_ReadOnly)) {

      QDataStream in(&f);
      in.setVersion(2);

      //check if the file has the correct format
      in >> fileMagic;
      if (fileMagic != KFLOG_FILE_MAGIC) {
        qDebug("Waypoint file not recognized as KFLog filetype.");
        return ret;
      }

      in >> fileType;
      if (fileType != FILE_TYPE_WAYPOINTS) {
        qDebug("Waypoint file is a KFLog file, but not for waypoints.");
        return ret;
      }

      in >> fileFormat;
      if (fileFormat != FILE_FORMAT_ID && fileFormat != FILE_FORMAT_ID_2) {
        qDebug("Waypoint file does not have the correct format. It returned %d, where %d was expected.", fileFormat, FILE_FORMAT_ID);
        return ret;
      }
    //from here on, we assume that the file has the correct format.

      while(!in.eof()) {
        // read values from file
          in >> wpName;
          in >> wpDescription;
          in >> wpICAO;
          in >> wpType;
          in >> wpLatitude;
          in >> wpLongitude;
          in >> wpElevation;
          in >> wpFrequency;
          in >> wpLandable;
          in >> wpRunway;
          in >> wpLength;
          in >> wpSurface;
          in >> wpComment;
          if (fileFormat>=FILE_FORMAT_ID_2) {
            in >> wpImportance;
          } else {
            wpImportance=1; //normal importance
          };

       //create new waypoint object and set the correct properties
          Waypoint *w = new Waypoint;

          w->name = wpName;
          w->description = wpDescription;
          w->icao = wpICAO;
          w->type = wpType;
          w->origP.setLat(wpLatitude);
          w->origP.setLon(wpLongitude);
          w->elevation = wpElevation;
          w->frequency = wpFrequency;
          w->isLandable = wpLandable;
          w->runway =wpRunway;
          w->length = wpLength;
          w->surface = wpSurface;
          w->comment = wpComment;
          w->importance = wpImportance;
          //qDebug("Waypoint read: %s (%s - %s)",w->name.latin1(),w->description.latin1(),w->icao.latin1());

          waypoints->append(w);
          cnt++;
          
          
        }
        ret=cnt;

    }
    else {
      warning("Access to waypointfile denied.");
      ret=FO_ERROR;
    }
  }
  else {
    warning("Waypoint catalog not found.");
    ret=FO_ERROR;
  }

  return ret;
}

/** Tries to save the waypoints in the list, and returns NOTIMPLEMENTED (-2), ERROR (-1) or OK (1). */
int CumulusWaypointFile::save(const QString& file, QPtrList<Waypoint> *waypoints){
  QString wpName="";
  QString wpDescription="";
  QString wpICAO="";
  Q_INT8 wpType;
  Q_INT32 wpLatitude;
  Q_INT32 wpLongitude;
  Q_INT16 wpElevation;
  double wpFrequency;
  Q_INT8 wpLandable;
  Q_INT16 wpRunway;
  Q_INT16 wpLength;
  Q_INT8 wpSurface;
  QString wpComment="";
  Q_UINT8 wpImportance;

  Waypoint *w;
  QFile f;
  QListIterator<Waypoint> it(*waypoints);
  int ret=FO_NOTSUPPORTED;

  f.setName(file);
  if (f.open(IO_WriteOnly)) {
    QDataStream out(& f);

    //write fileheader
    out << Q_UINT32(KFLOG_FILE_MAGIC);
    out << Q_INT8(FILE_TYPE_WAYPOINTS);
    out << Q_UINT16(FILE_FORMAT_ID_2); //use the new format with importance field.

    for (w = it.current(); w != 0; w = ++it) {
      wpName=w->name;
      wpDescription=w->description;
      wpICAO=w->icao;
      wpType=w->type;
      wpLatitude=w->origP.lat();
      wpLongitude=w->origP.lon();
      wpElevation=w->elevation;
      wpFrequency=w->frequency;
      wpLandable=w->isLandable;
      wpRunway=w->runway;
      wpLength=w->length;
      wpSurface=w->surface;
      wpComment=w->comment;
      wpImportance=w->importance;

      out << wpName;
      out << wpDescription;
      out << wpICAO;
      out << wpType;
      out << wpLatitude;
      out << wpLongitude;
      out << wpElevation;
      out << wpFrequency;
      out << wpLandable;
      out << wpRunway;
      out << wpLength;
      out << wpSurface;
      out << wpComment;
      out << wpImportance;
    }
    f.close();
    ret=FO_OK;
  }
  else {
    warning("Permission denied!");
    ret=FO_ERROR;
  }
  return ret;

}
