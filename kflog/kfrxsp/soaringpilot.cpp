/***********************************************************************
**
**   soaringpilot.cpp
**
**   This file is part of KFLog2.
**
************************************************************************
**
**   Copyright (c):  2003 by Harald Maier
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "soaringpilot.h"

#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>

extern int breakTransfer;

int portID = -1;
/**
 * holds the port-settings at start of the application
 */
struct termios oldTermEnv;
/**
 * is used to change the port-settings
 */
struct termios newTermEnv;

/**
 * Needed to reset the serial port in any case of unexpected exiting
 * of the programm. Called via signal-handler of the runtime-environment.
 */
void releaseTTY(int signal)
{
  tcsetattr(portID, TCSANOW, &oldTermEnv);
  exit(-1);
}

SoaringPilot::SoaringPilot()
{
  portID = -1;
}

SoaringPilot::~SoaringPilot()
{
}

/** No descriptions */
int SoaringPilot::open(char *port, int baud)
{
  speed_t speed;

  /* eventuell als Mode zusätzlich O_NONBLOCK ??? */
  portID = open(port, O_RDWR | O_NOCTTY);

  if(portID != -1) {
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

    //
    // NOTE: POSIX defines only speed-values up to B38400.
    // All other may not be present on other systems. Setting
    // the speed to more than 115200 bits/sec is not possible
    // for the volkslogger.
    //
    if(baud >= 115200) speed = B115200;
    else if(baud >= 57600) speed = B57600;
    else if(baud >= 38400) speed = B38400;
    else if(baud >= 19200) speed = B19200;
    else if(baud >=  9600) speed = B9600;
    else if(baud >=  4800) speed = B4800;
    else if(baud >=  2400) speed = B2400;
    else if(baud >=  1800) speed = B1800;
    else if(baud >=  1200) speed = B1200;
    else if(baud >=   600) speed = B600;
    else if(baud >=   300) speed = B300;
    else if(baud >=   200) speed = B200;
    else if(baud >=   150) speed = B150;
    else if(baud >=   110) speed = B110;
    else speed = B75;

    cfsetospeed(&newTermEnv, speed);
    cfsetispeed(&newTermEnv, speed);
  
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

    // Bytesize = 8
    newTermEnv.c_cflag |= CS8;

    // disable stop-bit
    newTermEnv.c_cflag |= CSTOPB;

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
    return 1;
  }
  else {
    return 0;
  }
}

/** No descriptions */
int SoaringPilot::close()
{
  if (portID != -1) {
    tcsetattr(portID, TCSANOW, &oldTermEnv);  
    return 1;
  }
  else {
    return 0;
  }
}

/** write a file like structure to the device */
int SoaringPilot::writeFile(QStringList &file)
{
  QStringList::Iterator line;
  for (line = file.begin(); line != file.end(); ++line) {
  }

  return 1;
}

/** read a file like structure to the device */
int SoaringPilot::readFile(QStringList &file)
{
  char inbyte;
  QString s;
  while (!breakTransfer) {
    if (read(portID, &inbyte, sizeof(inbyte))) {
      switch(inbyte) {
      case '\n':
        file.push_back(s);
        s = "";
        break;
      case '\r':
        continue;
      default:
        s += inbyte;
      }
    }
  }
  return 1;
}
