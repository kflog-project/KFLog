/***********************************************************************
**
**   filser.h
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Christian Fughe, Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FILSER_H
#define FILSER_H

#include <qptrlist.h>

#include "../frstructs.h"
#include "../flighttask.h"
#include "../flightrecorderpluginbase.h"

/**
  *@author Christian Fughe, Harald Maier
  */

#define FLIGHT_INDEX_WIDTH 0x60 /* Bytes per record of the flight  */
                                /* index. (96)                          */
#define LX_MEM_RET 7            /* Number of bytes returned by     */
                                /* wb(STX), wb(Q).                 */
                              
struct flightTable {
  unsigned char record[FLIGHT_INDEX_WIDTH];
};

class Filser : public FlightRecorderPluginBase
{
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

  static char calcCrcBuf(const char *buf, unsigned int count);

private:
  /**
   * try to find a filser device
   */
  bool check4Device();
  /**
   * write byte
   */
  int wb(unsigned char c);
  /**
   * read byte
   */
  char rb();
  /**
   * Calculate the check sum
   */
  static char calcCrc(char d, char crc);
  /**
   * Calculate the check sum on a buffer of bytes
   */
  bool readMemSetting();
  bool defMem(struct flightTable *ft);
  bool getMemSection(char *memSection, int size);
  bool getLoggerData(char *memSection, int sectionSize, char **memContents, int *contentSize);
  bool convFil2Igc(FILE *figc,  unsigned char *fil_p, unsigned char *fil_p_last);
  char *readData(char *buf_p, int count);
  QPtrList <flightTable> flightIndex;
  char *wordtoserno(unsigned int Binaer);
};

#endif
