/***********************************************************************
**
**   flightrecorderpluginbase.h
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

#ifndef FLIGHTRECORDERPLUGINBASE_H
#define FLIGHTRECORDERPLUGINBASE_H


/**
 * This class funtions as a baseclass for all flightrecorder plugins.
 * It features default implementation of all functions that return
 * 'not available' values. Just re-implement the functions you want
 * to support.
 * @author André Somers
 */

#include <termios.h>

#include <qstring.h>
#include <qptrlist.h>
#include <qobject.h>

#include "frstructs.h"
#include "waypoint.h"
#include "flighttask.h"

// Standard FlightRecorder return values
#define FR_ERROR -1          // Error
#define FR_NOTSUPPORTED -2   // Function not supported
#define FR_OK 1              // OK, no problem.

/**
 * @short Baseclass for all flightrecorder plugins
 *
 * This class funtions as a baseclass for all flightrecorderplugins. The
 * decending classes must re-implement all memberfuntions. Because
 * different flightrecorders have different capabilities, the capabilities
 * of a decendant class must be set in the constructor using a
 * @ref FR_Capabilities struct. Also, you need use
 * <pre>
 *    return FR_NOTSUPPORTED;
 * </pre>
 * as implementation of the not supported memberfunctions.
 */
    
class FlightRecorderPluginBase:public QObject {
Q_OBJECT
public: 
  enum TransferMode{none, serial, URL};
  /* The none option is pretty useless, but is included for completeness
     sake. It _is_ used in the flightrecorder dialog though! */

  enum TransferBps  {bps00000 = 0x0000,
                     bps00075 = 0x0001,
                     bps00150 = 0x0002,
                     pbs00200 = 0x0004,
                     bps00300 = 0x0008,
                     bps00600 = 0x0010,
                     bps01200 = 0x0020,
                     bps01800 = 0x0040,
                     bps02400 = 0x0080,
                     bps04800 = 0x0100,
                     bps09600 = 0x0200,
                     bps19200 = 0x0400,
                     bps38400 = 0x0800,
                     bps57600 = 0x1000,
                     bps115200= 0x2000};

  struct transferStruct
  {
    TransferBps _bps;
    int         _speed;
    speed_t     _tspeed;
  };

  static transferStruct transferData[];
  static int transferDataMax;

  // FlightRecorder capabilities
  struct FR_Capabilities
  {
    unsigned int maxNrTasks;             //maximum number of tasks
    unsigned int maxNrWaypoints;         //maximum number of waypoints
    unsigned int maxNrWaypointsPerTask;  //maximum number of waypoints per task
    unsigned int maxNrPilots;            //maximum number of pilots
    unsigned int transferSpeeds;         //the set of actual supported speeds

    bool supDlWaypoint;      //supports downloading of waypoints?
    bool supUlWaypoint;      //supports uploading of waypoints?
    bool supDlFlight;        //supports downloading of flights?
    bool supUlFlight;        //supports uploading of flights?
    bool supSignedFlight;    //supports downloading in of signed flights?
    bool supDlTask;          //supports downloading of tasks?
    bool supUlTask;          //supports uploading of tasks?
    bool supUlDeclaration;   //supports uploading of declarations?
    bool supDspPilotName;    //supports display of pilot name
    bool supDspRecorderType; //supports display of logger type
    bool supDspSerialNumber; //supports display of serial number
    bool supDspGliderID;     //supports display of glider ID
    bool supDspGliderType;   //supports display of glider type
    bool supDspCompetitionID;//supports display of competition ID
    bool supAutoSpeed;       //supports automatic transfer speed detection
  };

  struct FR_BasicData
  {
    QString serialNumber;
    QString recorderType;
    QString pilotName;
    QString gliderType;
    QString gliderID;
    QString competitionID;
  };
  
  FlightRecorderPluginBase();
  virtual ~FlightRecorderPluginBase();

  /**
   * Returns the name of the lib.
   */
  virtual QString getLibName() const = 0;
  /**
   * Returns the transfermode this plugin supports.
   */
  virtual TransferMode getTransferMode() const = 0;
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QPtrList<FRDirEntry>*)=0;
  /**
   * Downloads a specific flight.
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName)=0;
  /**
   * get recorder basic data
   */
  virtual int getBasicData(FR_BasicData&) = 0;
  /**
   * Opens the recorder for serial communication.
   */
  virtual int openRecorder(const QString& portName, int baud)=0;
  /**
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(const QString& URL)=0;
  /**
   * Closes the connection with the flightrecorder.
   */
  virtual int closeRecorder()=0;
  /**
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QPtrList<Waypoint> *taskPoints)=0; 
  /**
   * Read tasks from recorder
   */
  virtual int readTasks(QPtrList<FlightTask> *tasks)=0;
  /**
   * Write tasks to recorder
   */
  virtual int writeTasks(QPtrList<FlightTask> *tasks)=0;
  /**
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QPtrList<Waypoint> *waypoints)=0;
  /**
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QPtrList<Waypoint> *waypoints)=0;
  /**
   * Returns whether the flighrecorder is connected.
   */
  inline bool isConnected(){return _isConnected;};

  /**
   * Returns the flightrecorders capabilities.
   */
  inline FR_Capabilities capabilities(){return _capabilities;};

  /**
   * Returns additional info about an error that occured (optional).
   * _errorinfo is reset afterwards.
   */
  QString lastError();
    
protected:
  /**
   * Is the flightrecorder connected?
   */
  bool _isConnected;

  /**
   * The flightrecorders capabilities.
   * To be set in the constructor of depending classes.
   */
  FR_Capabilities _capabilities;
  /**
   * The flightrecorders basic data.
   * To be set in the constructor of depending classes.
   */
  FR_BasicData _basicData;

  /**
   * Optionally contains additional info about an error.
   */
  QString _errorinfo; 
   
signals:
  /**
   * May be emitted when transfering data
   *  bool finished is true if the datatransfer is complete
   *  int progress is a measure how much progress has been made, relative to total
   *  int total is the total progress to be made, for instance the total number of bytes to transfer
   * May be used to display a progress dialog.
   */
  virtual void progress(bool finished, int progress, int total);
  /**
   * May be emited when the actual transfer speed has changed
   */
  virtual void newSpeed (int speed);
};

#endif
