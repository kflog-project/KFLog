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
 * @short Flightrecorder plugin for Cumulus
 *
 * This class presents a plugin to interface with the Zaurus as if it
 * were a flightrecorder.
 * @author André Somers
 */

#include <qstring.h>
#include <qmemarray.h>
#include "../flightrecorderpluginbase.h"
class KTempDir;

class Cumulus:public FlightRecorderPluginBase
{
public: 
  Cumulus();
  ~Cumulus();

  /**
   * Returns the name of the lib.
   */
  virtual QString getLibName() const;
  /**
   * Returns the transfermode this plugin supports.
   * @returns FlightRecorderPluginBase::URL
   */
  virtual TransferMode getTransferMode() const;
  /**
   * Returns a list of recorded flights in this device. (Not implemented.)
   */
  virtual int getFlightDir(QPtrList<FRDirEntry>*);
  /**
   * Downloads the indicated flight from the recorder.
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
    * get basic flight recorder data
    */
  int getBasicData(FR_BasicData&);
  /**
   * Opens the recorder for serial communication. (Not implemented.)
   */
  virtual int openRecorder(const QString& portName, int baud);
  /**
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(const QString& URL);
  /**
   * Closes the connection with the flightrecorder.
   */
  virtual int closeRecorder();
  /**
   * Write flight declaration to recorder. (Not implemented.)
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints); 
  /**
   * Read tasks from recorder. (Not implemented.)
   */
  virtual int readTasks(QPtrList<FlightTask> *tasks);
  /**
   * Write tasks to recorder. (Not implemented.)
   */
  virtual int writeTasks(QPtrList<FlightTask> *tasks);
  /**
   * Read waypoints from recorder.
   */
  virtual int readWaypoints(QPtrList<Waypoint> *waypoints);
  /**
   * Write waypoints to recorder.
   */
  virtual int writeWaypoints(QPtrList<Waypoint> *waypoints);

protected:
  /**
   * Contains the path of the temporary local copy of the waypoint file from/for Cumulus
   */
  QString _tmpWaypointFile;
  /**
   * Contains the path of the temporary local copy of the tasks file from/for Cumulus
   */
  QString _tmpTasksFile;
  /**
   * Contains the base URL for the communication with Cumulus
   */
  QString _URL; 
  QString _home;
  
  /**
   * Returns the home path on the PDA
   */
  QString homePath(); 
  
  KTempDir* _tmpFlightDir;
  KTempDir* getTmpFlightDir();
  
  /**
   * Returns an FRDirEntry struct for the igc file
   */
  FRDirEntry* getFlightInfo(QString filename);
  
  QMemArray<QString> flightList;
};

#endif
