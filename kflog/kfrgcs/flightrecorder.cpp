/***********************************************************************
**
**   flightrecorder.cpp
**
**   This file is part of kio-logger.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include <iostream.h>

#include <unistd.h>

#include <vlapi2.h>
#include <vlapihlp.h>

#include <klocale.h>
#include <qarray.h>
#include <qfile.h>
#include <qlist.h>
#include <qstring.h>
#include <qtextstream.h>

#include <../frstructs.h>
#include <termios.h>

#define MAX(a,b)   ( ( a > b ) ? a : b )
#define MIN(a,b)   ( ( a < b ) ? a : b )

extern "C"
{
  /**
   * Returns the name of the lib.
   */
  QString getLibName();
  /**
   * Reads the list of flight in the logger. The lib has to take care
   * that all fields of the FRDirEntry are filled with valuable content.
   *
   * Return -1, if there is any problem with the logger, -2, if the
   * method is not implemented, 1, if reading was okay.
   */
  int getFlightDir(char* portName, QList<FRDirEntry>*);
  /**
   * Used to download a flight from the FR. The flight is returned as a
   * QByteArray.
   */
  QByteArray getFlight(char* portName, int flightID, char* tmpFileName,
      int* ret);
  /** */
  int downloadFlight(char* pName, int flightID, int secMode, char* fileName);
  /** */
  QString getRecorderName(char* portName);
  /** */
  int openRecorder(char* portName);
  /** */
  int writeTask(FRTaskDeclaration* taskDecl, QList<FRTaskPoint> taskPoints,
      char* portName);
}

/*************************************************************************
**
** begin of logger-specific implementation
**
*************************************************************************/

/**
 * The device-name of the port.
 */
char* portName = '\0';
/**
 * The file-handle
 */
int portID = -1;

/**
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;

/**
 * is used to change the port-settings
 */
struct termios newTermEnv;

VLAPI vl;

QString getLibName()  {  return "libkfrgcs";  }

int getFlightDir(char* portN, QList<FRDirEntry>* dirList)
{
  extern char* portName;

  portName = portN;

  dirList->clear();

  if(vl.open(0,5,0) != VLA_ERR_NOERR)
    {
      // Restoring the old port-settings
      tcsetattr(portID, TCSANOW, &oldTermEnv);

      warning(i18n("No logger found!"));

      return -1;
    }

  if(vl.read_directory() == VLA_ERR_NOERR)
    {
      tm lastDate;
      lastDate.tm_year = 0;
      lastDate.tm_mon = 0;
      lastDate.tm_mday = 1;
      int flightCount = 0;

      for(int loop = 0; loop < vl.directory.nflights; loop++)
        {
          DIRENTRY flight = vl.directory.flights[loop];

          if(lastDate.tm_year == flight.firsttime.tm_year &&
                lastDate.tm_mon == flight.firsttime.tm_mon &&
                lastDate.tm_mday == flight.firsttime.tm_mday)
            {
              flightCount++;
            }
          else
              flightCount = 1;

          FRDirEntry* entry = new FRDirEntry;

          entry->pilotName = flight.pilot;
          entry->gliderID = flight.gliderid;
          entry->firstTime = flight.firsttime;
          entry->lastTime = flight.lasttime;
          entry->duration = flight.recordingtime;
          entry->shortFileName = flight.filename;
//cerr << flight.filename << endl;
          entry->longFileName.sprintf("%d-%.2d-%.2d-GCS-%s-%.2d.igc",
              flight.firsttime.tm_year + 1900, flight.firsttime.tm_mon + 1,
              flight.firsttime.tm_mday, wordtoserno(flight.serno),
              flightCount);

          dirList->append(entry);

          lastDate = flight.firsttime;

//      		  vl.read_igcfile(fileName, loop, 0);
	      }
		}

  vl.close();

  return 1;
}

QByteArray getFlight(char* pName, int flightID, char* tmpFileName, int* ret)
{
  extern char* portName;

  cerr << "getFlight(" << pName << ", " << flightID << ", "
       << tmpFileName << ")\n";

  QByteArray bArray;

  portName = pName;

  if(vl.open(0,5,0) != VLA_ERR_NOERR)
    {
      // Restoring the old port-settings
      tcsetattr(portID, TCSANOW, &oldTermEnv);

      warning(i18n("No logger found!"));

      *ret = -1;
      return bArray;
    }

  vl.read_igcfile(tmpFileName, 2, 0);

  vl.close();

  QFile tmpFile(tmpFileName);
  QTextStream outStream(bArray, IO_WriteOnly);
  QTextStream inStream(&tmpFile);

  while(!inStream.eof())
      outStream << inStream.readLine();

  *ret = 1;
  return bArray;
}

int downloadFlight(char* pName, int flightID, int secMode, char* fileName)
{
  extern char* portName;

  portName = pName;

  if(vl.open(0, 5, 0) != VLA_ERR_NOERR)
    {
      // Restoring the old port-settings
      tcsetattr(portID, TCSANOW, &oldTermEnv);

      warning(i18n("No logger found!"));

      return -1;
    }

  vl.read_igcfile(fileName, flightID, secMode);
  vl.close();

  return 1;
}

QString getRecorderName(char* pName)
{
  extern char* portName;
  portName = pName;

  if(vl.open(0,5,0) != VLA_ERR_NOERR)
    {
      // Restoring the old port-settings
      tcsetattr(portID, TCSANOW, &oldTermEnv);

      warning(i18n("No logger found!"));

//      *ret = -1;
      return 0;
    }

  vl.read_info();

  vl.close();

	// Aufbau der Versions- und sonstigen Nummern
//	vlinfo.sessionid = 256*buffer[0] + buffer[1];
//	vlinfo.vlserno = 256*buffer[2] + buffer[3];
//	vlinfo.fwmajor = buffer[4] / 16;
//	vlinfo.fwminor = buffer[4] % 16;
//	vlinfo.fwbuild = buffer[7];

  return wordtoserno(vl.vlinfo.vlserno);
}

int openRecorder(char* pName)
{
  extern char* portName;

  portName = pName;

  if(vl.open(0,5,0) != VLA_ERR_NOERR)
    {
      warning(i18n("No logger found!"));
      return -1;
    }

  return 0;
}

int writeTask(FRTaskDeclaration* taskDecl, QList<FRTaskPoint> taskPoints,
      char* pName)
{
  extern char* portName;
  portName = pName;

  if(vl.open(0,5,0) != VLA_ERR_NOERR)
    {
      // Restoring the old port-settings
      tcsetattr(portID, TCSANOW, &oldTermEnv);

      warning(i18n("No logger found!"));

      return -1;
    }

  vl.read_db_and_declaration();

  // Filling the strings with whitespaces
  QString pilotA(taskDecl->pilotA.left(32));
  QString pilotB(taskDecl->pilotB.left(32));
  pilotA += "                                 ";
  pilotB += "                                 ";

  sprintf(vl.declaration.flightinfo.pilot, "%s%s",
      (const char*)pilotA.left(32), (const char*)pilotB.left(32));

warning(taskDecl->pilotA);
warning(taskDecl->pilotB);
warning(vl.declaration.flightinfo.pilot);

  strcpy(vl.declaration.flightinfo.gliderid,
      taskDecl->gliderID.left(7));
  strcpy(vl.declaration.flightinfo.glidertype,
      taskDecl->gliderType.left(12));
  strcpy(vl.declaration.flightinfo.competitionid,
      taskDecl->compID.left(3));
  strcpy(vl.declaration.flightinfo.competitionclass,
      taskDecl->compClass.left(12));

  // TakeOff (same ans landing ...)
  strcpy(vl.declaration.flightinfo.homepoint.name,
      taskPoints.first()->name.left(6));

  vl.declaration.flightinfo.homepoint.lon =
      taskPoints.first()->lonPos / 60.0;

  vl.declaration.flightinfo.homepoint.lat =
      taskPoints.first()->latPos / 100.0;

  // Begin of Task
  strcpy(vl.declaration.task.startpoint.name,
      taskPoints.at(1)->name.left(6));

  vl.declaration.task.startpoint.lat =
      taskPoints.at(1)->latPos / 600.0;

  vl.declaration.task.startpoint.lon =
      taskPoints.at(1)->lonPos / 1000.0;

  for(unsigned int loop = 2; loop < MIN(taskPoints.count() - 2, 12); loop++)
    {
      strcpy(vl.declaration.task.turnpoints[loop - 2].name,
          taskPoints.at(loop)->name.left(6));

      vl.declaration.task.turnpoints[loop - 2].lat =
          taskPoints.at(loop)->latPos / 10.0;

      vl.declaration.task.turnpoints[loop - 2].lon =
          taskPoints.at(loop)->lonPos / 10.0;
    }

  vl.declaration.task.nturnpoints = MAX(MIN((int)taskPoints.count() - 4, 12), 0);

  // End of Task
  strcpy(vl.declaration.task.finishpoint.name,
      taskPoints.at(taskPoints.count() - 2)->name.left(6));

  vl.declaration.task.finishpoint.lat =
      taskPoints.at(taskPoints.count() - 2)->latPos / 10.0;

  vl.declaration.task.finishpoint.lon =
      taskPoints.at(taskPoints.count() - 2)->lonPos / 10.0;

  vl.write_db_and_declaration();
  vl.close(0);

  return 0;
}
