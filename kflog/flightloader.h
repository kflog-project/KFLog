/***********************************************************************
**
**   flightloader.h
**
**   This file is part of KFLog4.
**   This class reads the flight files into the memory.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
***********************************************************************/

#ifndef FLIGHTLOADER_H
#define FLIGHTLOADER_H

#include <QFile>
#include <QFileInfo>

class FlightLoader
{
  public:
   /**
   * Loads a new flight-file.
   *
   * @param  file  The path to the file
   * @return "true", when the file has successfully been loaded
   */
  bool openFlight(QFile&);
   /**
   * Loads a new flight-file.
   *
   * @param  file  The path to the igc-file
   * @return "true", when the file has successfully been loaded
   */
  bool openIGC(QFile&, QFileInfo&);
   /**
   * Imports a file downloaded with Gardown in DOS
   *
   * @param  file  The path to the Gardown-file
   * @return "true", when the file has successfully been loaded
   */
  bool openGardownFile(QFile&, QFileInfo&);

  private:

  // Short structure to handle the optional entries in an igc file
  class bOption {
    public:
      int begin, length;
      char mnemonic[4];
  };


};

#endif
