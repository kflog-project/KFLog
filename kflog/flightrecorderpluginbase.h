/***********************************************************************
**
**   flight recorderpluginbase.h
**
**   This file is part of KFLog4.
**
************************************************************************
**
**   Copyright (c):  2003 by André Somers
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef FLIGHT_RECORDER_PLUGIN_BASE_H
#define FLIGHT_RECORDER_PLUGIN_BASE_H

#ifndef _WIN32
#include <termios.h>
#endif

#include <QObject>
#include <QList>
#include <QString>

#include "flighttask.h"
#include "frstructs.h"
#include "waypoint.h"

// Standard flight recorder return values
#define FR_ERROR        -1   // Error
#define FR_NOTSUPPORTED -2   // Function not supported
#define FR_OK            1   // OK, no problem.

/**
 * \class FlightRecorderPluginBase
 *
 * \author André Somers, Axel Pauli
 *
 * \brief Base class for all flight recorder plugins
 *
 * This class functions as a base class for all flight recorder plugins. The
 * derived classes must re-implement all member methods. Because
 * different flight recorders have different capabilities, the capabilities
 * of a derived class must be set in the constructor using a
 * \ref FR_Capabilities structure. Also, you need use
 * <pre>
 *    return FR_NOTSUPPORTED;
 * </pre>
 * as implementation of the not supported member functions.
 *
 * \date 2003-2011
 *
 * \version $Id$
 */
#ifdef _WIN32
typedef unsigned int speed_t;
#endif // ifdef (_WIN32)
class FlightRecorderPluginBase : public QObject
{
  Q_OBJECT

public:

  enum TransferMode{ none, serial, URL };
  /* The none option is pretty useless, but is included for completeness
     sake. It _is_ used in the flight recorder dialog though! */

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

  /** flight recorder capabilities */
  struct FR_Capabilities
  {
    int maxNrTasks;             //maximum number of tasks
    int maxNrWaypoints;         //maximum number of waypoints
    int maxNrWaypointsPerTask;  //maximum number of waypoints per task
    int maxNrPilots;            //maximum number of pilots
    int transferSpeeds;         //the set of actual supported speeds

    bool supDlWaypoint;      //supports downloading of waypoints?
    bool supUlWaypoint;      //supports uploading of waypoints?
    bool supDlFlight;        //supports downloading of flights?
    bool supUlFlight;        //supports uploading of flights?
    bool supSignedFlight;    //supports downloading in of signed flights?
    bool supDlTask;          //supports downloading of tasks?
    bool supUlTask;          //supports uploading of tasks?
    bool supExportDeclaration;//supports export of declaration?
    bool supUlDeclaration;   //supports uploading of declarations?
    bool supDspPilotName;    //supports display of pilot name
    bool supDspCoPilotName;  //supports display of pilot name
    bool supDspRecorderType; //supports display of logger type
    bool supDspSerialNumber; //supports display of serial number
    bool supDspDvcID;        //supports display of device ID
    bool supDspGliderID;     //supports display of glider ID
    bool supDspGliderType;   //supports display of glider type
    bool supDspCompetitionID;//supports display of competition ID
    bool supAutoSpeed;       //supports automatic transfer speed detection

    bool supEditPilotName;   //supports changing the pilot name
    bool supEditCoPilotName; //supports changing the copilot name
    bool supEditGliderID;    //supports changing the glider ID
    bool supEditGliderType;  //supports changing the glider type
    bool supEditGliderPolar; //supports editing the glider polar
    bool supEditUnits;       //supports changing units (speed, altitude, distance, ...)
    bool supEditGoalAlt;     //supports changing arrival altitude
    bool supEditArvRadius;   //supports changing arrival radius
    bool supEditAudio;       //supports changing vario audio settings
    bool supEditLogInterval; //supports changing logging intervals
  };

  struct FR_BasicData
  {
    QString serialNumber;
    QString recorderType;
    QString dvcID;
    QString pilotName;
    QString copilotName;
    QString gliderType;
    QString gliderID;
    QString competitionID;
  };

  enum FR_Units {
    FR_Unit_Vario_kts =   1,
    FR_Unit_Alt_ft    =   2,
    FR_Unit_Temp_F    =   4,
    FR_Unit_Baro_inHg =   8,
    FR_Unit_Dist_nm   =  16,
    FR_Unit_Dist_sm   =  32,
    FR_Unit_Spd_kts   =  64,
    FR_Unit_Spd_mph   = 128
  };

  struct FR_ConfigData
  {
    /* polar data: */
    int LD;              // best L/D
    int speedLD;         // speed at best L/D in km/h
    int speedV2;         // speed at 2 m/s sink in km/h
    int dryweight;       // dry weight of glider in kg
    int maxwater;        // max water ballast in liters

    /* instrument settings: */
    bool sinktone;       // sink tone
    bool totalenergyfg;  // total energy final glide
    bool fgdiffalt;      // final glide altitude difference
    int approachradius;  // approach radius in meters
    int arrivalradius;   // arrival radius in meters
    int sloginterval;    // enroute logging interval in seconds
    int floginterval;    // close logging interval in seconds
    int gaptime;         // time between flights in minutes
    int minloggingspd;   // min speed to force flight logging in 10ths of knots
    int stfdeadband;     // speed-to-fly dead band in 10ths of m/s
    unsigned char units; // units (vario, altitude, temperatur, pressure, distance, speed)
    int goalalt;         // goal altitude in 10ths of meters
    int compensation;    // total energy, super netto, netto
  };

  FlightRecorderPluginBase( QObject *parent = 0 );

  virtual ~FlightRecorderPluginBase();

  /**
   * Returns the name of the library.
   */
  virtual QString getLibName() const = 0;
  /**
   * Returns the transfer mode this plugin supports.
   */
  virtual TransferMode getTransferMode() const = 0;
  /**
   * Returns a list of recorded flights in this device.
   */
  virtual int getFlightDir(QList<FRDirEntry*>*)=0;
  /**
   * Downloads a specific flight.
   */
  virtual int downloadFlight(int flightID, int secMode, const QString& fileName)=0;
  /**
   * get recorder basic data
   */
  virtual int getBasicData(FR_BasicData&)=0;
  /**
   * get recorder config data
   */
  virtual int getConfigData(FR_ConfigData&)=0;
  /**
   * write recorder basic and config data
   */
  virtual int writeConfigData(FR_BasicData&, FR_ConfigData&)=0;
  /**
   * Opens the recorder for serial communication.
   */
  virtual int openRecorder(const QString& portName, int baud)=0;
  /**
   * Opens the recorder for other communication.
   */
  virtual int openRecorder(const QString& URL)=0;
  /**
   * Closes the connection with the flight recorder.
   */
  virtual int closeRecorder()=0;
  /**
   * Write flight declaration to recorder
   */
  virtual int writeDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints, const QString& name)=0;
  /**
   * Export flight declaration to file
   */
  virtual int exportDeclaration(FRTaskDeclaration *taskDecl, QList<Waypoint*> *taskPoints, const QString& name)=0;
  /**
   * Read tasks from recorder
   */
  virtual int readTasks(QList<FlightTask*> *tasks)=0;
  /**
   * Write tasks to recorder
   */
  virtual int writeTasks(QList<FlightTask*> *tasks)=0;
  /**
   * Read waypoints from recorder
   */
  virtual int readWaypoints(QList<Waypoint*> *waypoints)=0;
  /**
   * Write waypoints to recorder
   */
  virtual int writeWaypoints(QList<Waypoint*> *waypoints)=0;
  /**
   * Returns whether the flight recorder is connected or not.
   */
  bool isConnected()
  {
    return _isConnected;
  };

  /**
   * Returns the flight recorders capabilities.
   */
  FR_Capabilities capabilities(){return _capabilities;};

  /**
   * Returns additional info about an error that occurred (optional).
   * _errorinfo is reset afterwards.
   */
  QString lastError();

  /**
   * Sets a pointer to the parent widget object (recorder dialog).
   */
  void setParent(QObject* _parent);

protected:
  /**
   * Is the flight recorder connected?
   */
  bool _isConnected;

  /**
   * The flight recorders capabilities.
   * To be set in the constructor of depending classes.
   */
  FR_Capabilities _capabilities;
  /**
   * The flight recorders basic data.
   * To be set in the constructor of depending classes.
   */
  FR_BasicData _basicData;

  /**
   * The flight recorders configuration data.
   */
  FR_ConfigData _configData;

  /**
   * Optionally contains additional info about an error.
   */
  QString _errorinfo; 

  /**
   * Contains a reference to the parent widget. This is needed to make KDE's KIO mechanism
   * work properly.
   */
  QObject* _parent;

signals:

  /**
   * May be emitted when transferring data
   *
   *  \param finished is true if the data transfer is complete
   *  \param progress is a measure how much progress has been made, relative to total
   *  \param total is the total progress to be made, for instance the total
   *              number of bytes to transfer. May be used to display a
   *              progress dialog.
   */
  void progress(bool finished, int progress, int total);

  /**
   * May be emitted when the actual transfer speed has changed
   */
  void newSpeed (int speed);
};

#endif
