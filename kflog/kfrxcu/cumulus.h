/***********************************************************************
**
**   cumulus.h
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

#ifndef CUMULUS_H
#define CUMULUS_H


/**
 * This class presents a plugin to interface with the Zaurus as if it
 * were a flightrecorder.
 * @author André Somers
 */

#include <qstring.h>
#include <../flightrecorderpluginbase.h>

class Cumulus:public FlightRecorderPluginBase
{
public: 
  Cumulus();
  ~Cumulus();

  /**
   * Returns the name of the lib.
   */
  virtual QString getLibName();
  /**
   * Returns the transfermode this plugin supports.
   */
  virtual TransferMode getTransferMode();
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QList<FRDirEntry>*);
  /**
   *
   */
  virtual int downloadFlight(int flightID, int secMode, QString fileName);
  /**
   * get recorder info serial id
   */
  virtual QString getRecorderSerialNo();
  /**
   * Opens the recorder for serial communication.
   */
  virtual int openRecorder(const QString portName, int baud);
  /**
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(QString URL);
  /**
   * Closes the connection with the flightrecorder.
   */
  virtual int closeRecorder();
  /**
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint> *taskPoints); 
  /**
   * Read tasks from recorder
   */
  virtual int readTasks(QList<FlightTask> *tasks);
  /**
   * Write tasks to recorder
   */
  virtual int writeTasks(QList<FlightTask> *tasks);
  /**
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QList<Waypoint> *waypoints);
  /**
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QList<Waypoint> *waypoints);

protected:
  QString _tmpWaypointFile;
  QString _URL;  
};

#endif
