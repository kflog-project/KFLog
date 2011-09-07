/***********************************************************************
**
**   garminfr.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Thomas Nielsen, André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GARMIN_FR_H
#define GARMIN_FR_H

#include <QObject>

#include "../flightrecorderpluginbase.h"

#include "garmin.h"

/**
 * @class GarminFR
 *
 * @author André Somers, Axel Pauli
 *
 * @short Encapsulation of old Garmin code into new plugin system
 *
 * This class is a rework of the original Garmin plugin to the new
 * plugin architecture introduced by André Somers. It can be further
 * optimized by merging the @ref GarminGPS class with this GarminFR class.
 */

class GarminFR : public FlightRecorderPluginBase
{
 public:
  /**
   * Constructor
   */
  GarminFR( QObject *parent = 0 );
  /**
   * Destructor
   */
  virtual ~GarminFR();

  /**
   * Returns the name of the library.
   */
  virtual QString getLibName() const;
  /**
   * Returns the transfer mode this plugin supports.
   * @returns FlightRecorderPluginBase::URL
   */
  virtual TransferMode getTransferMode() const;
  /**
   * Returns a list of recorded flights in this device. (Not implemented.)
   */
  virtual int getFlightDir(QList<FRDirEntry*>*);
  /**
   * Downloads the indicated flight from the recorder.
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
   * get recorder basic data
   */
  virtual int getBasicData(FR_BasicData&);
  /*
   * get recorder configuration data
   */
  virtual int getConfigData(FR_ConfigData&);
  /**
   * write recorder basic and configuration data
   */
  virtual int writeConfigData(FR_BasicData&, FR_ConfigData&);
  /**
   * Opens the recorder for serial communication. (Not implemented.)
   */
  virtual int openRecorder(const QString& portName, int baud);
  /**
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(const QString& URL);
  /**
   * Closes the connection with the flight recorder.
   */
  virtual int closeRecorder();
  /**
   * Write flight declaration to recorder. (Not implemented.)
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints, const QString&);
  /**
   * Export flight declaration to file. (Not implemented.)
   */
  virtual int exportDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints, const QString&);
  /**
   * Read tasks from recorder. (Not implemented.)
   */
  virtual int readTasks(QList<FlightTask*> *tasks);
  /**
   * Write tasks to recorder. (Not implemented.)
   */
  virtual int writeTasks(QList<FlightTask*> *tasks);
  /**
   * Read waypoints from recorder.
   */
  virtual int readWaypoints(QList<Waypoint*> *waypoints);
  /**
   * Write waypoints to recorder.
   */
  virtual int writeWaypoints(QList<Waypoint*> *waypoints);

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
