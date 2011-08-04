/***********************************************************************
**
**   soaringpilot.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Harald Maier
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SOARING_PILOT_H
#define SOARING_PILOT_H

#include <QObject>
#include <QString>

#include "../frstructs.h"
#include "../flighttask.h"
#include "../flightrecorderpluginbase.h"

/**Implementation for SoaringPilot.

  *@author Harald Maier
  */

class SoaringPilot : public FlightRecorderPluginBase
{
 public:

  SoaringPilot( QObject *parent = 0 );

  ~SoaringPilot();

  /**
   * Returns the name of the library.
   */
  virtual QString getLibName() const;
  /**
   * Returns the transfer mode this plugin supports.
   */
  virtual TransferMode getTransferMode() const;
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QList<FRDirEntry*>*);
  /**
   *
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
    * get basic flight recorder data
    */
  int getBasicData(FR_BasicData&);
  /*
   * get recorder config data
   */
  virtual int getConfigData(FR_ConfigData&);
  /**
   * write recorder basic and config data
   */
  virtual int writeConfigData(FR_BasicData&, FR_ConfigData&);
  /**
   * Opens the recorder for serial communication.
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
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints);
  /**
   * Export flight declaration to file
   */
  virtual int exportDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints);
  /**
   * Read waypoint and flight declaration form from recorder into mem
   */
  virtual int readDatabase();
  /**
   * Read tasks from recorder
   */
  virtual int readTasks(QList<FlightTask*> *tasks);
  /**
   * Write tasks to recorder
   */
  virtual int writeTasks(QList<FlightTask*> *tasks);
  /**
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QList<Waypoint*> *waypoints);
  /**
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QList<Waypoint*> *waypoints);

private:

  /** write a file like structure to the device */
  int writeFile(QStringList &file);
  /** read a file like structure from the device */
  int readFile(QStringList &file);
  int coordToDegree(QString &s);
  QString degreeToDegMin(int d, bool isLat);
  QString degreeToDegMinSec(int d, bool isLat);
  int feetToMeter(QString &s);
  QString meterToFeet(int m);
};

#endif
