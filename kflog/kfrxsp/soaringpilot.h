/***********************************************************************
**
**   soaringpilot.h
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

#ifndef SOARINGPILOT_H
#define SOARINGPILOT_H

#include <qstring.h>
#include <qstringlist.h>

#include <termios.h>

/**Implementation for SoaringPilot.

  *@author Harald Maier
  */

class SoaringPilot
{
public: 
  SoaringPilot();
  ~SoaringPilot();
  /** No descriptions */
  int close();
  /** No descriptions */
  int open(char *port, int baud);
private: // Private methods
  /** write a file like structure to the device */
  int writeFile(QStringList &file);
  /** read a file like structure to the device */
  int readFile(QStringList &file);
};

#endif
