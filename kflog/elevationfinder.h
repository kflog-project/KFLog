//
// C++ Interface: elevationfinder
//
// Description: 
//
//
// Author: André Somers <andre@kflog.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ELEVATIONFINDER_H
#define ELEVATIONFINDER_H

#include <qobject.h>
#include <qpoint.h>
#include <qtimer.h>
#include <qfile.h>
#include <qdatastream.h>

/**
 * @short Finds the ground elevation for a given coordinate. Singleton.
 * This singleton class can use multiple (well, currently two) methods
 * to find the elevation for a given coordinate. 
 * 1. Using a DEM file configured for openGLIGCexplorer
 * 2. Using the internal isohypses
 * The preferred method is number 1, as it is much faster and more detailed
 * in comparison to the latter. 
 * @author André Somers
 */
class ElevationFinder : public QObject
{
Q_OBJECT
public:
    ElevationFinder(QObject *parent = 0, const char *name = 0);

    ~ElevationFinder();
    /**
     * Find an elevation.
     * @returns Elevation in meters or -1 if no valid result was found.
     * @args coordinates Coordinate of the point in wgs format, that is, unprojected.
     */
    int elevationWgs(const QPoint& coordinates);
    /**
     * Find an elevation.
     * @returns Elevation in meters or -1 if no valid result was found.
     * @args coordinates Coordinate of the point in map projection format.
     */
    int elevationMap(const QPoint& coordinates);
    /**
     * Find an elevation.
     * @returns Elevation in meters or -1 if no valid result was found.
     * @args WgsCoordinates Coordinate of the point in wgs format, that is, unprojected.
     * @args MapCoordinates Coordinate of the point in map projection format.
     * Both arguments should represent the same point. The format that results in the 
     * fastest lookup for the method in use will be used. This is the preferred
     * method of getting the elevation.
     */
    int elevation(const QPoint& WgsCoordinates, const QPoint& MapCoordinates);
    /**
     * @returns A pointer to the instance of the object to use. Use only this static
     * method to get an ElevationFinder object!
     */
    static ElevationFinder * instance();
    
private slots:
    void timeout();

private:
    int findDEMelevation(const QPoint&);
    bool tryOpenGLIGCexplorer();
    
    static ElevationFinder * _instance;
    bool useOGIE;
    
    //name of DEM (Digital Elevation Model) file
    QString demFileName;
    //top left coordinate of Dem file 
    QPoint demTL;
    //bottom right coordinate of Dem file
    QPoint demBR;
    int demRows;
    int demCols;
    int demGridLat;
    int demGridLon;
    QTimer * timer;
    QFile * demFile;
    QDataStream * demStream;
};

#endif
