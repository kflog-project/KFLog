//
// C++ Implementation: elevationfinder
//
// Description:
//
//
// Author: André Somers <andre@kflog.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "elevationfinder.h"
#include <qapplication.h>
#include <q3textstream.h>
#include <qdir.h>
#include <qrect.h>
#include <mapcontents.h>
#include <mapmatrix.h>

ElevationFinder * ElevationFinder::_instance = 0;

ElevationFinder::ElevationFinder(QObject *parent, const char *name)
 : QObject(parent, name)
{
  if ( tryOpenGLIGCexplorer() )
    useOGIE = true;

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));

  demFile = 0;
  demStream = 0;
}


ElevationFinder::~ElevationFinder()
{
  if (timer)
    delete timer;

  timeout(); //takes care of deleting demStream and demFile
}

ElevationFinder * ElevationFinder::instance()
{
  if (! _instance )
    _instance = new ElevationFinder(qApp, "elevationFinder");
  return _instance;
}

int ElevationFinder::elevationWgs(const QPoint& coordinates)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  if (useOGIE) //use openGLIGCexplorer's DEM file
    return findDEMelevation(coordinates);

  //use the 'old' method
  return _globalMapContents.getElevation(_globalMapMatrix.wgsToMap(coordinates));
}

int ElevationFinder::elevationMap(const QPoint& coordinates)
{
  extern MapContents _globalMapContents;
  extern MapMatrix _globalMapMatrix;

  if (useOGIE) //use openGLIGCexplorer's DEM file
    return findDEMelevation(_globalMapMatrix.mapToWgs(coordinates));

  //use the 'old' method
  return _globalMapContents.getElevation(coordinates);
}

int ElevationFinder::elevation(const QPoint& WgsCoordinates, const QPoint& MapCoordinates)
{
  extern MapContents _globalMapContents;

  if (useOGIE) //use openGLIGCexplorer's DEM file
    return findDEMelevation(WgsCoordinates);

  //use the 'old' method
  return _globalMapContents.getElevation(MapCoordinates);
}

int ElevationFinder::findDEMelevation(const QPoint& coordinates)
{
  timer->stop();

  if (!demFile) {
    demFile = new QFile(demFileName);
    demFile->open(QIODevice::ReadOnly);
    demStream = new QDataStream(demFile);
  }

  QRect r(QPoint(demBR.x(), demTL.y()), QPoint(demTL.x(), demBR.y()));
  if ( !r.contains(coordinates.x(), coordinates.y()) ) {
    //qDebug("Requested point outside DEM coverage.");
    return -1;
  }
  int row = (demTL.x() - coordinates.x()) / demGridLat;
  //qDebug("row: %d", row);
  int col = -1 * (demTL.y() - coordinates.y()) / demGridLon;
  //qDebug("col: %d, demCols: %d", col, demCols);
  Q_INT64 cell = 2 * (row * demCols + col);
  //qDebug("cell: %d", cell);
  Q_INT16 h;

  demFile->at(cell);
  *demStream>>h;
  if (h == -9999)
    h=0;

  return h;

  timer->start(10000, true);
}

void ElevationFinder::timeout()
{
  if (demStream)
    delete demStream;

  if (demFile) {
    demFile->close();
    delete demFile;
  }

  demStream = 0;
  demFile = 0;
}

bool ElevationFinder::tryOpenGLIGCexplorer()
{
  QString line;
  int flags = 0;
  int dem_name = 1;
  int dem_top = 2;
  int dem_bottom = 4;
  int dem_left = 8;
  int dem_right = 16;
  int dem_rows = 32;
  int dem_cols = 64;
  int dem_grid_lat = 128;
  int dem_grid_lon = 256;


  //can we find openGLIGCexplorer's configuration file?
  QFile f(QDir::home().path() + "/.openGLIGCexplorerrc");
  if ( !f.exists() )
    return false;

  qDebug("config file found.");

  if (! f.open(QIODevice::ReadOnly)) {
    qDebug("error opening file.");
    return false;
  }

  Q3TextStream s(&f);
  //let's try to find the DEM file in use from the configuration file
  while (true) {
    line = s.readLine();
    if ( line.isNull() )
      break;
    if ( line.isEmpty() )
      continue;
    //qDebug("line: %s",line.latin1());

    if ( line.startsWith("DEM_FILE") ) {
      flags |= dem_name;
      demFileName = line.mid(9).stripWhiteSpace();
    }
    if ( line.startsWith("DEM_ROWS") ) {
      flags |= dem_rows;
      demRows = line.mid(9).stripWhiteSpace().toInt();
    }
    if ( line.startsWith("DEM_COLUMNS") ) {
      flags |= dem_cols;
      demCols = line.mid(12).stripWhiteSpace().toInt();
    }
    if ( line.startsWith("DEM_LAT_MIN") ) {
      flags |= dem_top;
      demBR.setX( (int)line.mid(12).stripWhiteSpace().toDouble() * 600000);
    }
    if ( line.startsWith("DEM_LAT_MAX") ) {
      flags |= dem_bottom;
      demTL.setX( (int)line.mid(12).stripWhiteSpace().toDouble() * 600000);
    }
    if ( line.startsWith("DEM_LON_MIN") ) {
      flags |= dem_left;
      demTL.setY( (int)line.mid(12).stripWhiteSpace().toDouble() * 600000 );
    }
    if ( line.startsWith("DEM_LON_MAX") ) {
      flags |= dem_right;
      demBR.setY( (int)line.mid(12).stripWhiteSpace().toDouble() * 600000 );
    }
    if ( line.startsWith("DEM_GRID_LAT") ) {
      flags |= dem_grid_lat;
      demGridLat = int(line.mid(13).stripWhiteSpace().toDouble() * 600000.0);
      qDebug("%s -> %d", line.mid(13).stripWhiteSpace().latin1(), demGridLat);
    }
    if ( line.startsWith("DEM_GRID_LON") ) {
      flags |= dem_grid_lon;
      demGridLon = int(line.mid(13).stripWhiteSpace().toDouble() * 600000.0);
      qDebug("%s -> %d", line.mid(13).stripWhiteSpace().latin1(), demGridLat);
    }

    if (flags == 511) //all needed fields are found
      break;
  }
  //qDebug("flags: %d", flags);

  if (flags != 511) //we did not find all the fields we need
    return false;
  qDebug("demTL.x(): %d, demTL.y() %d", demTL.x(), demTL.y());
  qDebug("demBR.x(): %d, demBR.y() %d", demBR.x(), demBR.y());

  qDebug("good: found DEM file using openGLIGCexplorer's configuration file!");
  return true;
}

//#include "elevationfinder.moc"
