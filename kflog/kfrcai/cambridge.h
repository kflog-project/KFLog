/***********************************************************************
**
**   cambridge.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2007 by Hendrik Hoeth
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef CAMBRIDGE_H
#define CAMBRIDGE_H

#include "../frstructs.h"
#include "../flighttask.h"
#include "../flightrecorderpluginbase.h"

/**
  * This plugin should work under QtEmbedded (Version 2.3.3) and Qt (Version 3.x)
  * we take care of this by some defines
  */
#if QT_VERSION < 0x030000
#define i18n tr
#define QPtrList QList
#else
#include <klocale.h>
#endif


class Cambridge : public FlightRecorderPluginBase
{
public: 
  Cambridge();
  ~Cambridge();
  /*
   * Returns the name of the lib.
   */
  virtual QString getLibName() const;
  /*
   * Returns the transfermode this plugin supports.
   */
  virtual TransferMode getTransferMode() const;
  /*
   * get recorder basic data
   */
  virtual int getBasicData(FR_BasicData&);
  /*
   * get recorder config data
   */
  virtual int getConfigData(FR_ConfigData&);
  /*
   * Opens the recorder for serial communication.
   */
  virtual int openRecorder(const QString& portName, int baud);
  /*
   * Closes the connection with the flightrecorder.
   */
  virtual int closeRecorder();
  /*
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QPtrList<FRDirEntry>*);
  /*
   * Download flights from the recorder
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /*
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(const QString& URL);
  /*
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints);
  /*
   * Read waypoint and flight declaration form from recorder into mem
   */
  virtual int readDatabase();
  /*
   * Read tasks from recorder
   */
  virtual int readTasks(QPtrList<FlightTask> *tasks);
  /*
   * Write tasks to recorder
   */
  virtual int writeTasks(QPtrList<FlightTask> *tasks);
  /*
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QPtrList<Waypoint> *waypoints);
  /*
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QPtrList<Waypoint> *waypoints);
private:
  bool haveDatabase;

  /*
   * wait some time in ms
   */
  void wait_ms(const int);

  /*
   * write byte to logger
   */
  int wb(unsigned char c);

  /*
   * send a command to the logger
   */
  int sendCommand(QString cmd);

  /*
   * read data from logger
   */
  unsigned char *readData(unsigned char *buf_p, int count);
  int readReply(QString cmd, int mode, unsigned char *reply);

  /*
   * calculate checksums to check the data received from the logger
   */
  int calcChecksum8(unsigned char *buf, int count);
  int calcChecksum16(unsigned char *buf, int count);

  /*
   * convert latitude and longitude to the format needed for writing waypoints to the logger
   */
  QString lat2cai(int lat);
  QString lon2cai(int lon);
};

#endif

