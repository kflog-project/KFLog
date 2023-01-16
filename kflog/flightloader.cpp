/***********************************************************************
**
**   flightloader.cpp
**
**   This file is part of KFLog.
**
**   This class reads a flight file into the memory.
**
************************************************************************
**
**   Copyright (c):  2008 by Constantijn Neeteson
**                   2011-2023 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
***********************************************************************/

#include <QtWidgets>

#include "altitude.h"
#include "elevationfinder.h"
#include "flight.h"
#include "flightloader.h"
#include "mainwindow.h"
#include "mapcontents.h"
#include "mapmatrix.h"

extern MainWindow* _mainWindow;

QHash<QString, QString> FlightLoader::m_manufactures;

FlightLoader::FlightLoader( QObject *parent ) : QObject(parent)
{
  if( m_manufactures.size() == 0 )
    {
      m_manufactures.insert( "GCS", "Garrecht" );
      m_manufactures.insert( "CAM", "Cambridge Aero Instruments" );
      m_manufactures.insert( "DSX", "Data Swan/DSX" );
      m_manufactures.insert( "EWA", "EW Avionics" );
      m_manufactures.insert( "FIL", "Filser" );
      m_manufactures.insert( "FLA", "Flarm" );
      m_manufactures.insert( "SCH", "Scheffel" );
      m_manufactures.insert( "ACT", "Aircotec" );
      m_manufactures.insert( "NKL", "Nielsen Kellerman" );
      m_manufactures.insert( "LXN", "LX Navigation" );
      m_manufactures.insert( "IMI", "IMI Gliding Equipment" );
      m_manufactures.insert( "NTE", "New Technologies s.r.l." );
      m_manufactures.insert( "PES", "Peschges" );
      m_manufactures.insert( "PRT", "Print Technik" );
      m_manufactures.insert( "SDI", "Streamline Data Instruments" );
      m_manufactures.insert( "TRI", "Triadis Engineering GmbH" );
      m_manufactures.insert( "LXV", "LXNAV d.o.o." );
      m_manufactures.insert( "WES", "Westerboer" );
      m_manufactures.insert( "ZAN", "Zander" );
      m_manufactures.insert( "XXX", QObject::tr("unknow manufacture") );
    }
}

FlightLoader::~FlightLoader()
{
}

bool FlightLoader::openFlight(QFile& flightFile)
{
  QFileInfo fInfo(flightFile);

  if(! flightFile.exists() )
    {
      QMessageBox::warning(_mainWindow, QObject::tr("File does not exist"),
          "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>does not exist!").arg(flightFile.fileName()) + "</html>", QMessageBox::Ok);
      return false;
    }

  if( fInfo.suffix().toLower() == "kfp")
  {
      QMessageBox::warning(_mainWindow, QObject::tr("File is empty"),
          "<html>" + QObject::tr("Cannot open the selected file<BR><B>%1</B><BR>directly. Please open the flight file instead.").arg(flightFile.fileName()) + "</html>", QMessageBox::Ok);
      return false;
  }

  if(!flightFile.size())
    {
      QMessageBox::warning(_mainWindow, QObject::tr("File is empty"),
          "<html>" + QObject::tr("The selected file<BR><B>%1</B><BR>is empty!").arg(flightFile.fileName()) + "</html>", QMessageBox::Ok);
      return false;
    }

  if(!flightFile.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning(_mainWindow, QObject::tr("No permission to file"),
          "<html>" + QObject::tr("You don't have permission to access file<BR><B>%1</B>").arg(flightFile.fileName() + "</html>"), QMessageBox::Ok);
      return false;
    }

  //
  // We need a better format-identification then only the extension ...
  //
  if( fInfo.suffix().toLower() == "igc")
    {
      return openIGC(flightFile, fInfo);
    }
  else if( fInfo.suffix().toLower() == "gdn" || fInfo.suffix().toLower() == "trk")
    {
      return openGardownFile(flightFile, fInfo);
    }
  else
    {
      QMessageBox::warning(_mainWindow, QObject::tr("Unknown file extension"),
          "<html>" + QObject::tr("Couldn't open the file, because it has an unknown file extension") + "</html>", QMessageBox::Ok);
      return false;
    }

  return false;
}

/** Parses an igc-file */
bool FlightLoader::openIGC(QFile& igcFile, QFileInfo& fInfo)
{
  QProgressDialog importProgress( _mainWindow );
  importProgress.setWindowModality(Qt::WindowModal);
  importProgress.setWindowTitle(QObject::tr("Loading flight..."));
  importProgress.setLabelText(
      "<html>" + QObject::tr("Please wait while loading file<BR><B>%1</B>").arg(igcFile.fileName()) + "</html>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setRange(0, 100);
  importProgress.setVisible(true);
  //importProgress.setMinimumDuration(0);
  importProgress.setValue(0);

  // The cancel method of QProgressDialog did not work, if signal canceled is
  // not catched!
  connect(&importProgress, SIGNAL(canceled()), this, SLOT(slot_CancelLoad()));

  unsigned int fileLength = fInfo.size();
  QTextStream stream(&igcFile);

  Flight::FlightStaticData fsd;

  char latChar, lonChar;
  bool isFirstWP = true;
  int lat, latmin, latTemp, lon, lonmin, lonTemp, baroAltTemp, gpsAltTemp;
  int hh = 0, mm = 0, ss = 0;
  time_t curTime = 0, preTime = 0, timeOfFlightDay = 0;

  FlightPoint newPoint;
  QList<FlightPoint*> flightRoute;
  Waypoint* newWP = 0;
  Waypoint* preWP = 0;

  QList<bOption> options;

  //
  // This regexp is used to check the syntax of the position-lines in
  // the igc-file.
  //
  // BHHMMSSDDMMMMMNDDDMMMMMEVPPPPPGGGGGAAASSNNN
  //
  // HHMMSS       : Time of Fix, given in UTC                 6 byte
  //
  // DDMMmmmN/S   : Latitude (degree, minutes,                8 byte
  //                decimal of minutes)
  //
  //
  //                       quality
  //    time   lat      lon   |   h   GPS   ???
  //   |----||------||-------|||---||----||----?
  // ^B0944584832663N00856771EA0037700400100004
  //
  QRegExp bRecord("^B[0-2][0-9][0-6][0-9][0-6][0-9][0-9][0-9][0-6][0-9][0-9][0-9][0-9][NS][0-1][0-9][0-9][0-6][0-9][0-9][0-9][0-9][EW][AV][0-9,-][0-9][0-9][0-9][0-9][0-9,-][0-9][0-9][0-9][0-9]");

  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  ElevationFinder * ef=ElevationFinder::instance();

  int lineCount = 0;
  unsigned int wp_count = 0;
  int last0 = -1;
  bool isHeader = true;

  //
  // Sequence of records in the igc-file:
  //
  // A : FR manufacturer (single line, required)
  //
  // H : Header (multiple lines, required)
  //
  // I : Fix extension (single line, optional)
  //
  // J : Extension of K-record (single line, optional but required,
  //     if the K-record is used)
  //
  // C : Task-Definition (multiple lines, optional)
  //     the first C-record contains the UTC-date and UTC-time of the
  //     declaration, the local time of the intended das of the flight,
  //     the task ID, the number of points and a text string which can
  //     be used to describe the task
  //
  // L : Logbook entry (multiple lines, optional, multiple currencies,
  //     may only appear after the H, I and J records, but before the G-record)
  //
  // D : Differential GPS (single line, optional, placed after all H, I, J
  //     records but before the first B-record)
  //
  // F : Initial Satellite Constellation (single line, optional)
  //
  // <---------------------------- End of Header ---------------------------->
  //
  // B : Logged Fix (multiple lines)
  //
  // K : Extension data as defined in the J Record (single line,
  //     multiple currencies)
  //
  // F : Constellation change (single line, multiple currencies)
  //
  // E : Pilot-Event [PEV] (single lines, multiple currencies)
  //
  // <----------------------------- End of Body ----------------------------->
  //
  // G : Digital signature of the file
  //

  int lastProgress = 0;
  int readChar     = 0;

  while (!stream.atEnd())
    {
      QCoreApplication::processEvents();

      if( importProgress.wasCanceled() )
        {
          importProgress.close();
          igcFile.close();
          return false;
        }

      lineCount++;

      QString s = stream.readLine();

      readChar += s.length();

      if( s.trimmed().isEmpty() )
        {
          continue;
        }

      int progress = readChar * 100 / fileLength;

      if( lastProgress != progress  )
        {
          lastProgress = progress;
          importProgress.setValue( progress > 100 ? 100 : progress );
        }

      // First character of the read line is the key.
      QChar key = s.at(0);

      if( key == 'B' )
        {
          //
          // We have a point.
          // But we must proof the line syntax first.
          //

          if(bRecord.indexIn(s) == -1)
            {
              // IO-Error !!!
              QString lineNr = QString::number(lineCount);

              QMessageBox::warning(_mainWindow, QObject::tr("Syntax-error in IGC-file"),
                  "<html>" + QObject::tr("Syntax-error while loading igc-file"
                  "<BR><B>%1</B><BR>Aborting!").arg(igcFile.fileName()) + "</html>",
                  QMessageBox::Ok);

              qWarning( "KFLog: Error in reading line %d in igc-file %s",
                        lineCount, igcFile.fileName().toLatin1().data() );
              return false;
            }

          QChar valid = s.at(24);

          if( valid == 'V') // void, not valid;
            {
              continue;
            }
          else if( valid != 'A') //isValid = true;
            {
              qWarning("KFLog: Wrong value found in igc-line!");
              continue;
            }

          sscanf(s.mid(1,23).toLatin1().data(), "%2d%2d%2d%2d%5d%1c%3d%5d%1c",
              &hh, &mm, &ss, &lat, &latmin, &latChar, &lon, &lonmin, &lonChar);
          latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;

          sscanf(s.mid(25,10).toLatin1().data(),"%5d%5d", &baroAltTemp, &gpsAltTemp);

          // Scan the optional parts of the B record
          newPoint.engineNoise = -1;
#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp strnicmp
#endif

          for ( int i = 0; i < options.size(); i++ )
            {
              // Parse only known options
              if( strncasecmp((options.at(i)).mnemonic, "ENL", 3) == 0 )

              {
                sscanf( s.mid( (options.at(i).begin), (options.at(i).length)).toLatin1().data(),
                        "%d", &newPoint.engineNoise );
              }
           }

          // Ignoring a wrong point ...
          if( latTemp == 0 && lonTemp == 0 )
            {
              continue;
            }

          curTime = timeOfFlightDay + 3600 * hh + 60 * mm + ss;

          newPoint.time = curTime;
          newPoint.origP = WGSPoint(latTemp, lonTemp);
          newPoint.projP = _globalMapMatrix->wgsToMap(newPoint.origP);
          newPoint.surfaceHeight = ef->elevation(newPoint.origP, newPoint.projP);
          newPoint.height = baroAltTemp;
          newPoint.gpsHeight = gpsAltTemp;

          if( curTime < preTime )
            {
              // The new fix as a smaller time stamp. Therefore we assume, that
              // we have an overnight-flight. So we must add one day (e.g. 86400 sec.)
              timeOfFlightDay += 86400;
              curTime += 86400;
              newPoint.time = curTime;
            }

          preTime = curTime;

          FlightPoint* point = new FlightPoint;
          *point = newPoint;
          flightRoute.append( point );
        }
      else if( key == 'A' )
        {
          // We have an manufacturer identifier
          QString manufactureCode = s.mid(1,3).toUpper();

          if( s.at(1) == QChar('X') )
            {
              manufactureCode = "XXX";
            }

          if( m_manufactures.contains( manufactureCode ) )
            {
              fsd.frManufacturer = m_manufactures.value( manufactureCode );
            }
          else
            {
              fsd.frManufacturer = QObject::tr("unknown manufacturer");
            }

          fsd.frManufacturer += " (" + manufactureCode + ")";

          QString id = s.mid(4,3).toUpper();

          if( id.isEmpty() == false )
            {
              fsd.frManufacturer += ", " + id;
            }
        }
      else if( key == 'H' )
        {
          // We have a headline
          // The general format of the H-Record is: H, data source (S), subtype (CCC), subtype long name,
          // colon, text string. The long name and text string are intended as an aid for people reading the file.

          QString htype = s.mid(1, 4).toUpper();

          if( htype == "FPLT" ) // pilot in charge
              fsd.pilot = s.mid(s.indexOf(':') + 1);
          else if( htype == "PCM2" ) // copilot
              fsd.copilot = s.mid(s.indexOf(':') + 1);
          else if( htype== "FGTY" ) // glider type
              fsd.gliderType = s.mid(s.indexOf(':')+1);
          else if( htype == "FGID" ) // gilder Id
              fsd.gliderRegistration = s.mid(s.indexOf(':')+1);
          else if( htype == "FRFW" ) // firmeware version
              fsd.firmewareVersion = s.mid(s.indexOf(':')+1);
          else if( htype == "FRHW" ) // hardware version
              fsd.hardwareVersion = s.mid(s.indexOf(':')+1);
          else if( htype == "FFTY" ) // flight recorder type
              fsd.frType = s.mid(s.indexOf(':')+1);
          else if( htype == "FGPS" ) // GPS manufacture
              fsd.gpsManufacturer = s.mid(5);
          else if( htype == "FDTM" ) // GPS datum
              fsd.gpsDatum = s.mid(s.indexOf(':')+1);
          else if( htype == "FPRS" ) // pressure sensor
               fsd.altitudePressureSensor = s.mid(s.indexOf(':')+1);
          else if( htype == "FDTE") // date of flight
            {
              bool ok = false;
              int year = s.mid(9, 2).toInt(&ok);
              int mon  = s.mid(7, 2).toInt(&ok);
              int day  = s.mid(5, 2).toInt(&ok);

              QString century;

              if( ok && year > 80 )
                {
                  // seems to be an old flight
                  century = "19";
                  year += 1900;
                }
              else
        	{
        	  century = "20";
        	  year += 2000;
        	}

              // Begin time of flight
              fsd.date = century + s.mid(9, 2) + "-" + s.mid(7, 2) + "-" + s.mid(5, 2);

              // Set current flight date time. This will consider flights over
              // midnight.
              QDate date( year, mon, day );

              timeOfFlightDay = timeToDay(date.year(), date.month(), date.day());
            }
          else if( htype == "FCID" )
            {
              fsd.competitionId = s.mid(s.indexOf(':') +1);
            }
          else if( htype == "FCCL" )
            {
              fsd.competitionClass = s.mid(s.indexOf(':') +1);
            }
        }
      else if ( key == 'I' )
        {
          // This record defines the extension of the mandatory fix B Record. Only one I record is allowed in each file.
          // This record has to be located before the first B Record, immediately after the H record.
          // Format of I Record:
          //    I N N S S F F M M M S S F F M M M CR LF
          // Description             Size          Element   Remarks
          //   # of  extensions        2 bytes       NN        Valid characters 0-9
          //   Start byte number       2 bytes       SS        Valid characters 0-9
          //   Finish byte number      2 bytes       FF        Valid characters 0-9
          //   Mnemonic                3 bytes       MMM       Valid characters alphanumeric
          // The byte count starts from the beginning of the B Record starting at 1.

          int nrOfOpts = s.mid(1, 2).toInt();
          bOption opt;

          if ( nrOfOpts < 1 || nrOfOpts > 10 )
          {
            // Must be wrong
            qWarning("KFLog: Too much options in line %d of igc-file %s",
                     lineCount, igcFile.fileName().toLatin1().data() );
          }

          // Select the options announced in this igc file
          options.clear();

          for (int i = 0; i < nrOfOpts; i++ )
          {
            sscanf(s.mid(3+i*7, 7).toLatin1().data(), "%2d%2d%3s", &opt.begin, &opt.length, opt.mnemonic);
            opt.begin -= 1; // B record starts with 1!
            opt.length = opt.length - opt.begin;
            options.append(opt);
          }
        }
      else if( key == 'C' && isHeader)
        {
          if( ( ( ( s.mid( 8,1) == "N" ) || ( s.mid( 8,1) == "S" ) ) ||
                ( ( s.mid(17,1) == "W" ) || ( s.mid(17,1) == "E" ) ) ))
            {
              // We have a waypoint
              sscanf(s.mid(1,17).toLatin1().data(), "%2d%5d%1c%3d%5d%1c",
                  &lat, &latmin, &latChar, &lon, &lonmin, &lonChar);

              latTemp = lat * 600000 + latmin * 10;
              lonTemp = lon * 600000 + lonmin * 10;

              if(latTemp != 0 && lonTemp != 0)
                {
                  if(latChar == 'S') latTemp = -latTemp;
                  if(lonChar == 'W') lonTemp = -lonTemp;

                  newWP = new Waypoint;
                  newWP->name = s.mid(18,20);
                  newWP->origP = WGSPoint(latTemp, lonTemp);
                  newWP->projP = _globalMapMatrix->wgsToMap(newWP->origP);
                  newWP->type = Flight::NotSet;
                  if(isFirstWP || NULL == preWP)
                      newWP->distance = 0;
                  else
                      newWP->distance = dist(newWP, preWP);

                  fsd.waypoints.append(newWP);
                  isFirstWP = false;
                  preWP = newWP;
                }
              else
                {
                  // Sinnvoller wäre es aus der IGC Datei auszulesen wieviele
                  // WendePunkte es gibt. <- Ist IGC Datei immer korrekt??
                  if(wp_count != 0 && last0 != (int)(wp_count - 1))
                    {
                      newWP = new Waypoint;
                      newWP->name =  preWP->name;
                      newWP->origP = preWP->origP;
                      newWP->projP = preWP->projP;

                      fsd.waypoints.append(newWP);
                    }
                  last0 = wp_count;
                }
              wp_count++;
            }
        }

#if 0
      else if(s.at(0) == 'L')
        {
          // comment lines in IGC file
        }
#endif

    }

  igcFile.close();

  if( flightRoute.count() == 0 )
    {
      QMessageBox::warning( _mainWindow,
                            QObject::tr("File contains no flight"),
                            "<html>" +
                            QObject::tr("The selected file<BR><B>%1</B><BR>contains no flight!").arg(igcFile.fileName()) +
                            "</html>",
                            QMessageBox::Ok,
                            0);
      return false;
    }

  importProgress.setLabelText(
        "<html>" + QObject::tr("Please wait while checking airspaces") + "</html>");

  importProgress.repaint();
  QCoreApplication::processEvents();

  Flight* newFlight = new Flight( igcFile.fileName(),
                                  flightRoute,
                                  fsd );

  _globalMapContents->appendFlight( newFlight) ;
  return true;
}

void FlightLoader::slot_CancelLoad()
{
  // This slot is needed to get set the cancel flag of the progress dialog.
}

/** Parses a file downloaded with Gardown in DOS or a Garmin *.trk file */
bool FlightLoader::openGardownFile(QFile& gardownFile, QFileInfo& fInfo)
{
  QProgressDialog importProgress( _mainWindow );
  importProgress.setWindowModality(Qt::WindowModal);
  importProgress.setWindowTitle(QObject::tr("Loading flight..."));
  importProgress.setLabelText(
      "<html>" + QObject::tr("Please wait while loading file<BR><B>%1</B>").arg(gardownFile.fileName()) + "</html>");
  importProgress.setMinimumWidth(importProgress.sizeHint().width() + 45);
  importProgress.setRange(0, 200);
  importProgress.setVisible(true);
  importProgress.setMinimumDuration(0);
  importProgress.setValue(0);

  unsigned int fileLength = fInfo.size();
  unsigned int filePos = 0;
  QString s;
  QTextStream stream(&gardownFile);

  QString pilotName, gliderType, gliderID, recorderID;
  QDate date;
  char latChar, lonChar;
  int lat, latmin, latTemp, lon, lonmin, lonTemp;
  int hh = 0, mm = 0, ss = 0, height;
  time_t curTime = 0, timeOfFlightDay = 0;

  QList<FlightPoint*> flightRoute;
  QList<Waypoint*> wpList;

  //
  // This regexp is used to check the syntax of the position-lines in
  // the file.
  //
  //  Format spec:
  //
  QRegExp bRecord("^[$]GPRMC,[0-9][0-9][0-9][0-9][0-9][0-9],[AV],[0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[NS],[0-9][0-9][0-9][0-9][0-9]\\.[0-9][0-9][0-9],[EW],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9]\\.[0-9],[0-9][0-9][0-9][0-9][0-9][0-9][0-9],[0-9][0-9][0-9]\\.[0-9],[EW],[*][0-9][0-9]$");

  extern MapMatrix *_globalMapMatrix;
  extern MapContents *_globalMapContents;
  ElevationFinder * ef=ElevationFinder::instance();

  int lineCount = 0;

  float fLat, fLon;
  int day, month, year;

  while (!stream.atEnd())
    {
      if(importProgress.wasCanceled())
        {
          return false;
        }

      lineCount++;

      s = stream.readLine();
      filePos += s.length();
      importProgress.setValue(( filePos * 200 ) / fileLength);

      if(s.mid(0,2) == "T ")
        {
          //
          // We have a point.
          // But we must proof the line syntax first.
          //
/*          if(bRecord.match(s) == -1)
            {
              // IO-Error !!!
              QString lineNr;
              lineNr = lineNr.asprintf("%d", lineCount);
              KMessageBox::error(0,
//                  QObject::tr("Syntax-error while loading FlightGear-file"
//                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
//                  QObject::tr("Error in FlightGear-file"));
//              warning("KFLog: Error in reading line %d in FlightGear-file %s",
                QObject::tr("Sorry, but this function is not yet available"
                      "<BR><B>%1</B><BR>Aborting!").arg(flightgearFile.name()),
                QObject::tr("Sorry..."));
                warning("KFLog: reading line %d in unspported FlightGear-file %s",
                lineCount, (const char*)flightgearFile.name());

              return false;
            }
*/
          // Example of my garmin 90:
          //H  LATITUDE    LONGITUDE    DATE      TIME     ALT    ;track
          //T  N4815.60836 E01229.15449 30-JUL-96 12:59:01 -9999

          fLat = 0;
          fLon = 0;

          // file is ok, now read data from line
          sscanf(s.mid(3,11).toLatin1().data(),  "%1c%2d %f", &latChar, &lat, &fLat);
          sscanf(s.mid(15,12).toLatin1().data(), "%1c%3d %f", &lonChar, &lon, &fLon);
          sscanf(s.mid(28,9).toLatin1().data(), "%2d-%*3s-%2d", &day, &year);

          if ( year > 70 )
            year += 1900;
          else
            year += 2000;

          month = 0;

          timeOfFlightDay = timeToDay(year, month, day, s.mid(31, 3).toLatin1().data());
          sscanf(s.mid(38, 8).toLatin1().data(), "%2d:%2d:%2d", &hh, &mm, &ss);
          curTime = timeOfFlightDay + 3600 * hh + 60 * mm + ss;
          sscanf(s.mid(47, 5).toLatin1().data(), "%d", &height);
          if ( height < 0 )
            height = 0;


          // skip if lat & lon = 000.0N
          if ((lat == 0.0) && (lon == 0.0))
              continue;

          latmin = (int) fLat * 1000;
          lonmin = (int) fLon * 1000;

          // convert to internal KFLog format
          latTemp = lat * 600000 + latmin * 10;
          lonTemp = lon * 600000 + lonmin * 10;

          if(latChar == 'S') latTemp = -latTemp;
          if(lonChar == 'W') lonTemp = -lonTemp;

          FlightPoint* newPoint = new FlightPoint;

          newPoint->time = curTime;
          newPoint->origP = WGSPoint(latTemp, lonTemp);
          newPoint->projP = _globalMapMatrix->wgsToMap(newPoint->origP);
          newPoint->surfaceHeight = ef->elevation(newPoint->origP, newPoint->projP);
          newPoint->height = height;
          newPoint->gpsHeight = height;

          flightRoute.append(newPoint);
        }
      else
        {
          // ignore other lines in file for now...
          continue;
        }
    }

  // close the import dialog, clean up and add the FlightRoute we just created
  importProgress.close();

  if(!flightRoute.count())
    {
      QMessageBox::warning( _mainWindow,
                            QObject::tr("File contains no flight"),
                            "<html>" +
                            QObject::tr("The selected file<BR><B>%1</B><BR>exists but contains no QNH value").arg(gardownFile.fileName()) +
                            "</html>",
                            QMessageBox::Ok,
                            0 );
      return false;
    }

  recorderID = "gardown";
  pilotName  = "gardown";
  gliderType = "gardown";
  gliderID   = "gardown";

  Flight::FlightStaticData fsd;

  fsd.frRecorderId       = "gardown";
  fsd.pilot              = "gardown";
  fsd.gliderType         = "gardown";
  fsd.gliderRegistration = "gardown";

  _globalMapContents->appendFlight( new Flight(gardownFile.fileName(),
                                               flightRoute,
                                               fsd) );
  return true;
}
