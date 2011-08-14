/***********************************************************************
**
**   flarm.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c): 2011 by Eggert Ehmke
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id: flarm.h 1177 2011-05-19 19:32:33Z axel $
**
***********************************************************************/

#ifndef FLARM_H
#define FLARM_H

#include <termios.h>
#include <stdio.h>

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QList>
#include <QTextStream>

#include "../waypoint.h"
#include "../frstructs.h"
#include "../flighttask.h"
#include "../flightrecorderpluginbase.h"
#include "../da4record.h"

/**
  *@author Christian Fughe, Harald Maier, Eggert Ehmke
  * parts of this file are based on the kfrfil plugin for LX devices
  * adapted for Flarm devices
  */

#define FLIGHT_INDEX_WIDTH 0x60 /* Bytes per record of the flight  */
                                /* index. (96)                     */

#define INFO_SIZE       118     /* Size of the s_hdrinfo struct */
#define TASK_SIZE       230     /* Size of the s_hdrtask struct */

struct flightTable
{
    unsigned char record[FLIGHT_INDEX_WIDTH];
};

class Flarm : public FlightRecorderPluginBase
{
  Q_OBJECT

public:

  Flarm( QObject *parent = 0 );

  virtual ~Flarm();
  /**
   * Returns the name of the library.
   */
  virtual QString getLibName() const {  return "libkfrfla";  };
  /**
   * Returns the transfer mode this plugin supports.
   */
  virtual TransferMode getTransferMode() const;
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QList<FRDirEntry*> *);
  /**
   *
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
    * get basic flight recorder data
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
   * Reads waypoint and flight declaration form from recorder into memory.
   */
  virtual int readDatabase();
  /**
   * Reads tasks from recorder
   */
  virtual int readTasks(QList<FlightTask*> *tasks);
  /**
   * Writes tasks to recorder
   */
  virtual int writeTasks(QList<FlightTask*> *tasks);
  /**
   * Reads waypoints from recorder
   */
  virtual int readWaypoints(QList<Waypoint*> *waypoints);
  /**
   * Writes waypoints to recorder
   */
  virtual int writeWaypoints(QList<Waypoint*> *waypoints);

  static unsigned char calcCrcBuf(const void* buf, unsigned int count);

private:

  /**
   * try to find a Flarm device
   */
  bool check4Device();
  /**
   * try to sync the transfer rate with connected recorder
   */
  bool AutoBaud();
  ushort calcCheckSum (int pos, const QString& sentence);
  bool checkCheckSum (int pos, const QString& sentence);
  void sendStreamComment (QTextStream& stream, const QString& comment);
  void sendStreamData (QTextStream& stream, const QString& sentence);
  int sendStreamData (QTextStream& stream, FRTaskDeclaration* decl, QList<Waypoint*>* wpList);
  QString getFlarmDebug(QFile&);
  QString getFlarmData(QFile&, const QString&, const QString&);
  bool putFlarmData(QFile&, const QString& cmd, const QString& key, const QString& data1=NULL, const QString& data2=NULL, const QString& data3=NULL);
  QString lat2flarm (int);
  QString lon2flarm (int);

  QList <flightTable *> flightIndex;

  char *wordtoserno(unsigned int Binaer);
  speed_t _speed;
};

#endif
