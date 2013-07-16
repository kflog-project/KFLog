/***********************************************************************
**
**   flightloader.h
**
**   This file is part of KFLog4.
**
**   This class reads a flight file into the memory.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**                   2011 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#ifndef FLIGHT_LOADER_H
#define FLIGHT_LOADER_H

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
  /**
  * Imports a file downloaded with Gardown in DOS
  *
  * @param  OriginalFileName  The name of the original file
  * @return "true", when the QNH has sucessfully been reset
  */
  bool resetQNH(QString OriginalFileName);

  private:

  bool loadQNH(QString OriginalFileName, int & result);
  bool saveQNH(QString OriginalFileName, int QNH);
  bool getQNHFromUser(QString OriginalFileName, int & result, int startValue = 1013);

  // Short structure to handle the optional entries in an igc file
  class bOption
  {
    public:
      int begin, length;
      char mnemonic[4];
  };
};

#endif
