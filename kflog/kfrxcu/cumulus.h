/***********************************************************************
**
**   cumulus.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Andr� Somers
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
 * @short Flightrecorder plugin for Cumulus
 *
 * This class presents a plugin to interface with the Zaurus as if it
 * were a flightrecorder.
 * @author Andr� Somers
 */

#include <qstring.h>
#include "../flightrecorderpluginbase.h"

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
   * @returns FlightRecorderPluginBase::URL
   */
  virtual TransferMode getTransferMode();
  /**
   * Returns a list of recorded flights in this device. (Not implemented.)
   */
  virtual int getFlightDir(QList<FRDirEntry>*);
  /**
   * Downloads the indicated flight from the recorder.
   */
  virtual int downloadFlight(int flightID, int secMode, QString fileName);
  /**
   * get recorder info serial id
   */
  virtual QString getRecorderSerialNo();
  /**
   * Opens the recorder for serial communication. (Not implemented.)
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
   * Write flight declaration to recorder. (Not implemented.)
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint> *taskPoints); 
  /**
   * Read tasks from recorder. (Not implemented.)
   */
  virtual int readTasks(QList<FlightTask> *tasks);
  /**
   * Write tasks to recorder. (Not implemented.)
   */
  virtual int writeTasks(QList<FlightTask> *tasks);
  /**
   * Read waypoints from recorder.
   */
  virtual int readWaypoints(QList<Waypoint> *waypoints);
  /**
   * Write waypoints to recorder.
   */
  virtual int writeWaypoints(QList<Waypoint> *waypoints);

protected:
  /**
   * Contains the path of the temporary local copy of the waypoint file from/for Cumulus
   */
  QString _tmpWaypointFile;
  /**
   * Contains the base URL for the communication with Cumulus
   */
  QString _URL;  
};

#endif
