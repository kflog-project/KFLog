/***********************************************************************
**
**   filser.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Christian Fughe, Harald Maier, Eggert Ehmke
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FILSER_H
#define FILSER_H

#include <termios.h>
#include <stdio.h>
#include <qtimer.h>

#include <qglobal.h>
#if QT_VERSION < 0x030000

#include <qlist.h>
#include "waypoint.h"
#include "frstructs.h"
#include "flighttask.h"
#include "flightrecorderpluginbase.h"
#include "da4record.h"

#else

#include <qptrlist.h>
#include "../waypoint.h"
#include "../frstructs.h"
#include "../flighttask.h"
#include "../flightrecorderpluginbase.h"
#include "../da4record.h"

#endif


/**
  *@author Christian Fughe, Harald Maier
  */

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

#define FLIGHT_INDEX_WIDTH 0x60 /* Bytes per record of the flight  */
                                /* index. (96)                     */
#define LX_MEM_RET 7            /* Number of bytes returned by     */
                                /* wb(STX), wb(Q).                 */
                              
#define INFO_SIZE       118     /* Size of the s_hdrinfo struct */
#define TASK_SIZE       230     /* Size of the s_hdrtask struct */
#define BASIC_LENGTH    (1 + INFO_SIZE + TASK_SIZE)
                                /* 1 undocumented byte ahead    */
#define EXTENDED_LENGTH BASIC_LENGTH + 9
                                /* 9 characters class name      */

struct flightTable {
  unsigned char record[FLIGHT_INDEX_WIDTH];
};

class Filser : public FlightRecorderPluginBase
{
  Q_OBJECT

public: 
  Filser();
  virtual ~Filser();
  /**
   * Returns the name of the lib.
   */
  virtual QString getLibName() const {  return "libkfrfil";  };
  /**
   * Returns the transfermode this plugin supports.
   */
  virtual TransferMode getTransferMode() const;
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QPtrList<FRDirEntry>*);
  /**
   *
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName);
  /**
    * get basic flight recorder data
    */
  virtual int getBasicData(FR_BasicData&);
  /*
   * get recorder config data
   */
  virtual int getConfigData(FR_ConfigData&);
  /**
   * Opens the recorder for serial communication.
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
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints);
  /**
   * Read waypoint and flight declaration form from recorder into mem
   */
  virtual int readDatabase();
  /**
   * Read tasks from recorder
   */
  virtual int readTasks(QPtrList<FlightTask> *tasks);
  /**
   * Write tasks to recorder
   */
  virtual int writeTasks(QPtrList<FlightTask> *tasks);
  /**
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QPtrList<Waypoint> *waypoints);
  /**
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QPtrList<Waypoint> *waypoints);

  static unsigned char calcCrcBuf(const void* buf, unsigned int count);

public slots:
  void slotTimeout();

private:
  /**
   * try to find a filser device
   */
  bool check4Device();
  /**
   * try to sync the transfer rate with connected recorder
   */
  bool AutoBaud();
  /**
   * write byte
   */
  int wb(unsigned char c);
  /**
   * read byte
   */
  unsigned char rb();
  /**
   * Calculate the check sum
   */
  static unsigned char calcCrc(unsigned char d, unsigned char crc);
  /**
   * Calculate the check sum on a buffer of bytes
   */
  bool readMemSetting();
  bool defMem(struct flightTable *ft);
  bool getMemSection(unsigned char *memSection, int size);
  bool getLoggerData(unsigned char *memSection, int sectionSize);
  bool convFil2Igc(FILE *figc,  unsigned char *fil_p, unsigned char *fil_p_last);
  unsigned char *readData(unsigned char *buf_p, int count);
  unsigned char *writeData(unsigned char *buf_p, int count);
  QPtrList <flightTable> flightIndex;
  char *wordtoserno(unsigned int Binaer);
  // true if the da4buffer is identical to the logger content
  bool _da4BufferValid;
  DA4Buffer _da4Buffer;
  int readDA4Buffer();
  int writeDA4Buffer();
  int findWaypoint (Waypoint* wp);
  QTimer* _keepalive;
  speed_t _speed;
};

#endif
