/***********************************************************************
**
**   garminfr.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Thomas Nielsen, André Somers
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GARMINFR_H
#define GARMINFR_H

#include "../flightrecorderpluginbase.h"
#include "garmin.h"

/**
 * @author André Somers
 * @short Encapsulation of old Garmin code into new plugin system
 *
 * This class is a rework of the original Garmin plugin to the new
 * plugin architecture introduced by André Somers. It can be further
 * optimized by merging the @ref GarminGPS class with this GarminFR class.
 */

class GarminFR : public FlightRecorderPluginBase  {
public: 
  /**
   * Constructor
   */
  GarminFR();
  /**
   * Destructor
   */
  ~GarminFR();

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
  virtual int getFlightDir(QList<FRDirEntry>*);
  /**
   * Downloads the indicated flight from the recorder.
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
   * get recorder info serial id
   */
  virtual QString getRecorderSerialNo();
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
   * garmin class instance
   */
  GarminGPS gmn;

  /**
   * The device-name of the port.
   */
  char* portName;

  /**
   * The file-handle
   */
  int portID;

  /**
   * holds the port-settings at start of the application
   */
  struct termios oldTermEnv;

  /**
   * is used to change the port-settings
   */
  struct termios newTermEnv;


};

#endif
