/***********************************************************************
**
**   flightrecorder.h
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

#ifndef __FLIGHTRECORDER_H__
#define __FLIGHTRECORDER_H__

/**********************************************************************
**
** This file describes the API for the back-end of kio_logger.
**
***********************************************************************/

#include <termios.h>
//#include <sys/types.h>

#include <qstring.h>

//#include <vlapi2.h>
#include "garmin.h"

/**
 * @author Heiner Lamprecht
 * @version $Id$
 */
//class FlightRecorder
//{
//  public:
//    /**
//     * Constructor. Do not call this method outside the FR-backend!
//     */
//    FlightRecorder();
//    /**
//     * Destructor. Do not call this method outside the FR-backend!
//     */
//    ~FlightRecorder();
//    /**
//     * Returns the ID-string of the FR.
//     */
//    QString getFRName();
//    /**
//     * Returns the complete name of the backend.
//     */
//    static QString getLibName();
//    /**
//     * Initilizes the port used by the backend. Returns OK, if the
//     * port can be opened. port is the name of the device (f.e.
//     * "/dev/ttyS1"), maxSpeed is the maximum speed (bits per second)
//     * used for communication. The backend can use lower speeds without
//     * notifying the frontend.
//     *
//     * Note: The backend have to store the tty-settings of the port, so
//     * that they can be restored after closing the port.
//     *
//     * This method is called directly after opening the lib.
//     */
//    int initPort(const char* port, int maxSpeed);
//    /**
//     * Closes the port and restores the settings. In order to ensure,
//     * that the port is in a defined state after closing the lib, install
//     * a signal-handler in the calling application, which calls this method
//     * when caught a signal.
//     */
//    int closePort();
//    /**
//     * WIR BRAUCHEN NOCH MEHR FEHLERCODES ...
//     */
//    enum ErrorCode {OK = 0, NoPort, TimeOut, NoLogger};
//
//  private:
//    /**
//     * Contains the port-settings at start of the lib. Used to restore
//     * the settings after closing the port.
//     */
//    struct termios oldTerm;
//    /**
//     * Contains the current port-settings.
//     */
//    struct termios newTerm;
//    /**
//     */
//    bool isOpen;
//    /**
//     * The file-handle of the port.
//     */
//    int portID;
//    /**
//     * The maximum allowed speed for communication with the recorder.
//     */
//    int maxSpeed;
//    /**
//     */
//    VLAPI recorder;
//};

/**
 * This method is the used to initilize the FlightRecord-object of the
 * backend. Normally, this method does nothing but creating a new
 * FlightRecord-object and returning a pointer to it.
 */
//FlightRecorder* initFR();

#endif // __FLIGHTRECORDER_H__
