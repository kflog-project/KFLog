/***********************************************************************
 **
 **   vlapisys_linux.cpp
 **
 **   This file is part of libkfrgcs.
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

#include "vla_support.h"

#include <stdio.h>
#include <cstdlib>
#include <iostream>

// Datei-handling:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

using namespace std; 

int noninteractive;

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int signal)
{
  extern int portID;
  extern struct termios oldTermEnv;

  cerr << "Logger-Connection: releaseTTY(" << signal << ")\n";
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  exit(-1);
}

/***********************************************************************
 *
 * vlapi_sys
 *
 **********************************************************************/

/** wait a specified amount of milliseconds (t) */
void VLA_SYS::wait_ms(const int32 t)  {  usleep(t * 1000);  }

/** read value of a continous running seconds-timer */
int32 VLA_SYS::get_timer_s()  {  return time(NULL);  }

/**
 * acquire serial port for communication with VL
 * returns 0 if port was successfully opened
 * otherwise != 0
 */
VLA_ERROR VLA_SYS::serial_open_port()
{
  extern char* portName;
  extern int portID;
  extern struct termios newTermEnv;
  extern struct termios oldTermEnv;

  /* eventuell als Mode zusätzlich O_NONBLOCK ??? */
  portID = open(portName, O_RDWR | O_NOCTTY);

  if(portID == -1) {
    return VLA_ERR_COMM;
  }

  //
  // Before we change any port-settings, we must establish a
  // signal-handler, which is used to restore the port-settings
  // after terminating the programm.
  //    Because a SIGKILL-signal removes the programm immediately,
  // the status of the port will be undefined.
  //
  struct sigaction sact;

  sact.sa_handler = releaseTTY;
  sigaction(SIGHUP, &sact, NULL);
  sigaction(SIGINT, &sact, NULL);
  sigaction(SIGPIPE, &sact, NULL);
  sigaction(SIGTERM, &sact, NULL);

  ////////////////////////////////////////////////////////////////////////
  //
  // port-configuration
  //

  // reading the current port-settings
  tcgetattr(portID, &newTermEnv);

  // storing the port-settings to restore them ...
  oldTermEnv = newTermEnv;

  newTermEnv.c_cflag |= HUPCL; // (egal)

  newTermEnv.c_iflag |= CREAD;   // (wichtig!)
  newTermEnv.c_iflag |= CLOCAL;  // (wichtig!)
  newTermEnv.c_iflag &= ~IGNCR;  // (wichtig!)
  newTermEnv.c_iflag &= ~INLCR;  // (wichtig!)
  newTermEnv.c_iflag &= ~ISTRIP; // (scheint egal zu sein ...)

  newTermEnv.c_iflag |= CRTSCTS; // (scheint egal zu sein ...)

  newTermEnv.c_iflag &= ~IGNBRK; //
  newTermEnv.c_iflag &= ~BRKINT; // (Kombination der drei wichtig!)
  newTermEnv.c_lflag &= ~IEXTEN; //

  newTermEnv.c_iflag &= ~IGNPAR;  // (scheint egal zu sein ...)

  newTermEnv.c_iflag &= ~IXOFF;   // (scheint egal zu sein ...)
  newTermEnv.c_iflag &= ~IXANY;   // (scheint egal zu sein ...)

  // Bytesize = 8
  newTermEnv.c_cflag |= CS8;

  // disable stop-bit
  newTermEnv.c_cflag &= ~CSTOPB;

  // enable raw-mode (diable canonical-mode)
  newTermEnv.c_lflag &= ~ICANON;

  // don't wait for a character
  newTermEnv.c_cc[VMIN] = 0;

  // wait at least 1 msec.
  newTermEnv.c_cc[VTIME] = 1;

  // no parity
  newTermEnv.c_cflag &= ~PARENB;

  // don't change newline (\n) in carriage-return (\r)
  newTermEnv.c_iflag &= ~ICRNL;

  // disable XON flow-control on output
  newTermEnv.c_iflag &= ~IXON;

  // disable implementation-defined output-processing
  newTermEnv.c_oflag &= ~OPOST;

  // don't interpret INTR, QUIT, SUSP or DSUSP
  newTermEnv.c_lflag &= ~ISIG;

  //
  newTermEnv.c_lflag |= IEXTEN;

  // echo input characters
  newTermEnv.c_lflag &= ~ECHO;

  // Activating the port-settings
  tcsetattr(portID, TCSANOW, &newTermEnv);

  serial_set_baudrate(9600);
  return VLA_ERR_NOERR;
}

/** release serial port on normal exit */
VLA_ERROR VLA_SYS::serial_close_port()
{
  extern int portID;
  extern struct termios oldTermEnv;

  if (portID == -1) {
    return VLA_ERR_COMM;
  }

  tcsetattr(portID, TCSANOW, &oldTermEnv);

  return VLA_ERR_NOERR;
}

/** serial output of single character to the VL */
VLA_ERROR VLA_SYS::serial_out(const byte outbyte)
{
  extern int portID;
  if (portID == -1) {
    return VLA_ERR_COMM;
  }

  write(portID, &outbyte, sizeof(outbyte));
  return VLA_ERR_NOERR;
}

/**
 * serial input of single character from the VL
 * returns 0 if character has been received, and -1 when no character
 * was in the receive buffer
 */
VLA_ERROR VLA_SYS::serial_in(byte *inbyte)
{
  extern int portID;
  if (portID == -1) {
    return VLA_ERR_COMM;
  }

  int res = read(portID, inbyte, sizeof(*inbyte));

  if(res == 0) {
    //  	  *inbyte = 0x03;
    // Kein Zeichen empfangen!!!
    //		  cerr << "\n Nichts gelesen !!!\n\n";
    //      cerr << "Fehlercode: " << VLA_ERR_NOCHAR << " (sonst: " << VLA_ERR_NOERR << ")\n";
    // Trotz dieses Rückgabe-Codes bricht die aufrufende Funktion nicht ab!
    // Eine "-1" kann die Funktion nicht zurückliefern, da dies kein
    // Element von VLA_ERROR ist :-(
    return VLA_ERR_NOCHAR;		
  }

  return VLA_ERR_NOERR;
}

/** clear serial input- and output-buffers */
VLA_ERROR VLA_SYS::serial_empty_io_buffers()
{
  extern int portID;

  if (portID == -1) {
    return VLA_ERR_COMM;
  }
	
  if(tcflush(portID, TCIOFLUSH) == -1) {
    cerr << "\nERROR: could not empty buffer!\n";
  }

  return VLA_ERR_NOERR;
}

/** set communication parameters */
VLA_ERROR VLA_SYS::serial_set_baudrate(const int32 baudrate)
{
  extern struct termios newTermEnv;
  extern int portID;

  if (portID == -1) {
    return VLA_ERR_COMM;
  }

  //
  ////////////////////////////////////////////////////////////////////////

  // Setting up the speed of the connection
  if(baudrate) {
    speed_t speed;
    //
    // NOTE: POSIX defines only speed-values up to B38400.
    // All other may not be present on other systems. Setting
    // the speed to more than 115200 bits/sec is not possible
    // for the volkslogger.
    //
    if(baudrate >= 115200) speed = B115200;
    else if(baudrate >= 57600) speed = B57600;
    else if(baudrate >= 38400) speed = B38400;
    else if(baudrate >= 19200) speed = B19200;
    else if(baudrate >=  9600) speed = B9600;
    else if(baudrate >=  4800) speed = B4800;
    else if(baudrate >=  2400) speed = B2400;
    else if(baudrate >=  1800) speed = B1800;
    else if(baudrate >=  1200) speed = B1200;
    else if(baudrate >=   600) speed = B600;
    else if(baudrate >=   300) speed = B300;
    else if(baudrate >=   200) speed = B200;
    else if(baudrate >=   150) speed = B150;
    else if(baudrate >=   110) speed = B110;
    else speed = B75;
    
    cfsetospeed(&newTermEnv, speed);
    cfsetispeed(&newTermEnv, speed);
    // Activating the port-settings
    tcsetattr(portID, TCSANOW, &newTermEnv);
  }

  return VLA_ERR_NOERR;
}

// interaction - input functions
//
boolean VLA_SYS::test_user_break()
{
  if(noninteractive)  return 0;

  return 0;
  /*
    if you want the datatransfer to be user-interruptable,
    implement testing a flag or anything which can
    indicate that the user wants to interrupt the datatransfer
  */
}

int16 VLA_SYS::clear_user_break()
{
  cerr << "VLA_SYS::clear_user_break()\n";
  if(noninteractive)  return 0;

  return 0;
}

// text for the status line
//
char *statustext[] = {
  "sending command to FR",
  "command is being processed",
  "datatransfer in progress ... (press <Q> to abort)",
  "writing database & FDF to FR",
  "data transfer interrupted by user - press any key",
  "data transfer unsuccessful, try lower baudrate - press any key",
  "no data was received from FR - press any key",
  "user-interrupt !",
  "please connect the VOLKSLOGGER and press OK",
  "connection established - press OK",
  "connection not established - press OK",
  "intentionally left blank ...",
  "error: command not implemented",
  "error: you've the wrong FR connected",
  "error: no response from FR"
};


void VLA_SYS::progress_reset()
{
  if(noninteractive)  return;
}

void VLA_SYS::progress_set(VLS_TXT_ID txtid)
{
  if(noninteractive)  return;
}

void VLA_SYS::show(VLS_TXT_ID txtid)
{
  cout << statustext[txtid] << endl;

  if(noninteractive)  return;
  if (txtid == VLS_TXT_NIL)  return;
}


void VLA_SYS::showwait(VLS_TXT_ID txtid)
{
  cout << statustext[txtid] << endl;

  if(noninteractive)  return;
}
